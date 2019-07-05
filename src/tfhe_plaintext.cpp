#include <tfhe.h>

#warning You are compiling a plaintext binary.

ClientKey read_client_key(char const *) {
	return reinterpret_cast<ClientKey>(1);
}
ClientKey read_client_key(FILE *) { return reinterpret_cast<ClientKey>(1); }
void free_client_key(ClientKey) {}

ServerKey read_server_key(char const *) {
	return reinterpret_cast<ServerKey>(1);
}
ServerKey read_server_key(FILE *) { return reinterpret_cast<ServerKey>(1); }
void free_server_key(ServerKey) {}

ClientParams::ClientParams(ClientKey){};

ServerParams::ServerParams(ServerKey) {}

ServerParams::ServerParams(const ClientParams &) {}

int decrypt(bit_t dst, ClientParams) { return *dst.data(); }

bit_t make_bit(WeakParams) {
	auto ptr = gsl::shared_ptr_unsynchronized<bool>(
	    reinterpret_cast<bool *>(malloc(1)));
	return gsl::span<1>(ptr, 1);
}

bitspan_t make_bitspan(int N, WeakParams) {
	auto ptr = gsl::shared_ptr_unsynchronized<bool>(
	    reinterpret_cast<bool *>(malloc(N)));
	return gsl::span<>(ptr, N);
}

void encrypt(bit_t dst, bool src, ClientParams) { *dst.data() = src; }

void _unsafe_constant(bit_t dst, bool src, WeakParams) { *dst.data() = src; };

void _not(bit_t dst, bit_t src, WeakParams) { *dst.data() = !*src.data(); }

#define BINARY_OPERATOR(LibName, CppExpr)                                      \
	void _##LibName(bit_t dst, const bit_t a, const bit_t b, WeakParams) {     \
		auto A = *a.data();                                                    \
		auto B = *b.data();                                                    \
		*dst.data() = (CppExpr);                                               \
	}

BINARY_OPERATOR(and, A &&B)
BINARY_OPERATOR(andyn, A && !B)
BINARY_OPERATOR(andny, !A && B)
BINARY_OPERATOR(nand, !(A && B))
BINARY_OPERATOR(or, (A || B))
BINARY_OPERATOR(oryn, A || !B)
BINARY_OPERATOR(orny, !A || B)
BINARY_OPERATOR(nor, !(A || B))
BINARY_OPERATOR(xor, A ^ B)
BINARY_OPERATOR(xnor, !(A ^ B))

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b, WeakParams) {
	*dst.data() = *cond.data() ? *a.data() : *b.data();
}

void _copy(bit_t dst, bit_t src, WeakParams) { *dst.data() = *src.data(); }
