#include <cassert>
#include <cstdio>
#include <cstring>
#include <glovebox.h>
#include <rpc/server.h>

TFHEServerParams_t default_server_params;
weak_params_t default_weak_params;

int main() {
	puts("Initializing TFHE...");
	FILE *cloud_key = fopen("cloud.key", "rb");
	if (cloud_key == nullptr) {
		puts("cloud.key not found: run ./keygen first.");
		return 1;
	}
	default_weak_params = default_server_params =
	    makeTFHEServerParams(cloud_key);
	fclose(cloud_key);

	rpc::server srv(8000);

	srv.bind("fibonacci", [](int times, std::string a, std::string b) {
		puts("Received request.");
		// Note the elegance in automatic deserialization.
		Int8 first = a;
		Int8 second = b;
		Int8 ret(0, default_server_params);
		for (int i = 0; i < times; i++) {
			printf("Iteration %d\n", i);
			ret.add(first, second);
			// todo: implement copy/move ctor for Int so that the following
			// works
			/*
			first = second;
			second = x;
			*/
			first.copy(second);
			second.copy(ret);
		}
		return ret.serialize();
	});

	srv.run();

	return 0;
}
