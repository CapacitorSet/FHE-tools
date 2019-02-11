const assert = require("assert");
const fs = require("fs");
const parser = require("./parser.js");
const toposort = require("toposort");

if (process.argv.length != 3) {
	console.error(`Syntax: ${process.argv[0]} ${process.argv[1]} file.v`);
	process.exit(1);
}

const target = process.argv[2];
const source = fs.readFileSync(target, "utf8");
// If we stripped comments ourselves, the line numbering in parser errors would be off.
if (/\(\*[^*]+\*\)\n/gm.test(source)) {
	console.error("The file contains comments, please strip them:\n\n    grep -Fv '(*' original.txt > stripped.txt\n");
	process.exit(2);
}

let ast;
try {
	ast = parser.parse(source);
} catch (e) {
	console.error(`[${target}:${e.location.start.line}:${e.location.start.column}] ${e.message}`);
	process.exit(3);
}
// Only one module at a time is supported
assert(ast.length == 1);
const _ = ast[0];

let buf = "/* Generated automatically by CapacitorSet/FHE-tools */\n\n";

const outputs = _.decls
	.filter(it => it.type === "output")
	.map(it => (it.size == 1 ? "bit_t" : "bitspan_t") + " " + it.name);
const inputs = _.decls
	.filter(it => it.type === "input")
	.map(it => "const " + (it.size == 1 ? "bit_t" : "bitspan_t") + " " + it.name);
let prototype = `void ${_.name}(${outputs.concat(inputs).join(", ")})`;

buf += `${prototype};

${prototype} {
`;

// Assignments shouldn't appear often in optimized code, and at this time they're not implemented
// correctly (we need to keep track of the width of variables).
assert(_.assignments.length == 0);
/*

// Assignments like "wire <- (input/constant)" are to be put first.
function is_initial_assignment(it) {
	const srcType = it.src.type == "constant" ? "constant" : vars.get(it.src.value);
	const dstType = vars.get(it.dst.value);
	return (srcType == "input" || srcType == "constant") && (dstType == "wire");
};
// Assignments like "output <- (wire/input/constant)" are to be put last.
function is_final_assignment(it) {
	const srcType = it.src.type == "constant" ? "constant" : vars.get(it.src.value);
	const dstType = vars.get(it.dst.value);
	return (srcType == "input" || srcType == "wire" || srcType == "constant") && (dstType == "output");
};
const unexpected_assignments = _.assignments
	.filter(it => !is_initial_assignment(it))
	.filter(it => !is_final_assignment(it));
if (unexpected_assignments.length != 0) {
	console.error("Some assignments are neither initial nor final:");
	console.error(JSON.stringify(unexpected_assignments, null, 2));
	process.exit(4);
}

function transpile_assignment(it) {
	const {src, dst} = it;
	assert(dst.type == "identifier");
	if (!dst.index || !dst.index.end) {
		const cpp_dst = dst.index ? `${dst.value}[${dst.index.begin}]` : dst.value;
		switch (src.type) {
		case "constant":
			assert(!dst.index);
			assert(dst.type == "identifier");
			return `  _constant(${cpp_dst}, 0x${src.hex});\n`;
		case "identifier":
			if (src.index) {
				assert(!src.index.end);
				return `  _copy(${cpp_dst}, ${src.value}[${src.index.begin}]);\n`;
			} else {
				return `  _copy(${cpp_dst}, ${src.value});\n`;
			}
		default:
			assert(false);
		}
	} else {
		assert(false);
	}
}
const initial_assignments = _.assignments
	.filter(is_initial_assignment)
	.map(transpile_assignment)
	.join("");
buf += initial_assignments;
*/

// Add indices where necessary
function transpile_identifier(it) {
	assert(it.type == "identifier");
	if (!it.index)
		return it.value;
	assert(!it.index.end);
	return it.value + "[" + it.index.begin + "]";
}

let metadata = {};
for (const decl of _.decls) {
	metadata[decl.name] = {
		type: decl.type,
		size: decl.size
	};
}

// Maps identifiers to the ID of the gate whose output they are
let outputMappings = {};

for (let i = 0; i < _.gates.length; i++)
	outputMappings[transpile_identifier(_.gates[i].z)] = i;

function addFirstDeclaration(it, i) {
	if (!("firstDeclared" in metadata[it]))
		metadata[it].firstDeclared = i;
}
function addLastDeclaration(it, i) {
	if (!("lastDeclared" in metadata[it]))
		metadata[it].lastDeclared = i;
}

/* The circuit must be topologically sorted, i.e. the inputs of each gate must
 * be calculated before the gate itself.
 */
const edges = [];
for (const gate of _.gates) {
	switch (gate.gate) {
	case "mux":
		edges.push([transpile_identifier(gate.sel), transpile_identifier(gate.z)]);
		edges.push([transpile_identifier(gate.t), transpile_identifier(gate.z)]);
		edges.push([transpile_identifier(gate.f), transpile_identifier(gate.z)]);
		break;
	case "not":
		edges.push([transpile_identifier(gate.a), transpile_identifier(gate.z)]);
		break;
	default:
		edges.push([transpile_identifier(gate.a), transpile_identifier(gate.z)]);
		edges.push([transpile_identifier(gate.b), transpile_identifier(gate.z)]);
		break;
	}
}

// The list of wires/outputs in topological order
const path = toposort(edges);
// The list of corresponding gates in topological order
const gateList = path
	// Exclude everything which isn't output by a gate (i.e. inputs)
	.filter(it => it in outputMappings)
	.map(it => {
		const gateID = outputMappings[it];
		return _.gates[gateID];
	});


/* Track the first and the last time each wire was used. This information can
 * be used to decide when to allocate and deallocate the corresponding bits.
 */
for (let i = 0; i < gateList.length; i++) {
	const gate = gateList[i];
	switch (gate.gate) {
	case "mux":
		addFirstDeclaration(gate.z.value, i);
		addFirstDeclaration(gate.sel.value, i);
		addFirstDeclaration(gate.t.value, i);
		addFirstDeclaration(gate.f.value, i);
		break;
	case "not":
		addFirstDeclaration(gate.z.value, i);
		addFirstDeclaration(gate.a.value, i);
		break;
	default:
		addFirstDeclaration(gate.z.value, i);
		addFirstDeclaration(gate.a.value, i);
		addFirstDeclaration(gate.b.value, i);
		break;
	}
}

for (let i = gateList.length; i --> 0;) {
	const gate = gateList[i];
	switch (gate.gate) {
	case "mux":
		addLastDeclaration(gate.z.value, i);
		addLastDeclaration(gate.sel.value, i);
		addLastDeclaration(gate.t.value, i);
		addLastDeclaration(gate.f.value, i);
		break;
	case "not":
		addLastDeclaration(gate.z.value, i);
		addLastDeclaration(gate.a.value, i);
		break;
	default:
		addLastDeclaration(gate.z.value, i);
		addLastDeclaration(gate.a.value, i);
		addLastDeclaration(gate.b.value, i);
		break;
	}
}

// Allocate memory, if we marked this place as the first place the bit is used.
function declare_if_needed(it, i) {
	assert(it.type == "identifier");
	const {type, size, firstDeclared} = metadata[it.value];
	if (type != "wire")
		return "";
	if (firstDeclared != i)
		return "";

	if (size === 1)
		return `  bit_t ${it.value} = make_bit();\n`;
	else
		return `  bitspan_t ${it.value} = make_bitspan(${size});\n`;
}

// Free memory, if we marked this place as the last place the bit is used.
function free_if_needed(it, i) {
	assert(it.type == "identifier");
	const {type, size, lastDeclared} = metadata[it.value];
	if (type != "wire")
		return "";
	if (lastDeclared != i)
		return "";

	return `  free_bitspan(${it.value});\n`;
}

const gates = gateList
	.map((it, i) => {
		switch (it.gate) {
		case "mux": {
			let ret = "";
			ret += declare_if_needed(it.z, i);
			ret += declare_if_needed(it.sel, i);
			ret += declare_if_needed(it.t, i);
			ret += declare_if_needed(it.f, i);
			ret += `  _mux(${transpile_identifier(it.z)}, ${transpile_identifier(it.sel)}, ${transpile_identifier(it.t)}, ${transpile_identifier(it.f)});\n`;
			ret += free_if_needed(it.z, i);
			ret += free_if_needed(it.sel, i);
			ret += free_if_needed(it.t, i);
			ret += free_if_needed(it.f, i);
			return ret;
		}
		case "not": {
			let ret = "";
			ret += declare_if_needed(it.z, i);
			ret += declare_if_needed(it.a, i);
			ret += `  _not(${transpile_identifier(it.z)}, ${transpile_identifier(it.a)});\n`;
			ret += free_if_needed(it.z, i);
			ret += free_if_needed(it.a, i);
			return ret;
		}
		default: {
			let ret = "";
			ret += declare_if_needed(it.z, i);
			ret += declare_if_needed(it.a, i);
			ret += declare_if_needed(it.b, i);
			ret += `  _${it.gate}(${transpile_identifier(it.z)}, ${transpile_identifier(it.a)}, ${transpile_identifier(it.b)});\n`;
			ret += free_if_needed(it.z, i);
			ret += free_if_needed(it.a, i);
			ret += free_if_needed(it.b, i);
			return ret;
		}
		}
	})
	.join("");
buf += gates;

/*
const final_assignments = _.assignments
	.filter(is_final_assignment)
	.map(transpile_assignment)
	.join("");
buf += final_assignments;
*/

buf += "}\n"

console.log(buf);