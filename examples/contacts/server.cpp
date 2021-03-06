#include <cassert>
#include <glovebox.h>
#include <rpc/server.h>

#include "contact.h"

ServerParams server_params;

Contact contacts[] = {
    {"", 504'305'6784}, {"", 208'348'4604}, {"", 713'729'7840}, {"", 714'543'1510},
    {"", 907'553'8994}, {"", 704'551'2763}, {"", 172'265'8920}, {"", 583'642'3948},
    {"", 754'768'2428}, {"", 921'915'3647}, {"", 790'793'8139}, {"", 936'327'0855},
};

int main(int argc, char **argv) {
	assert(argc <= 2);
	uint16_t port = (argc == 2) ? atoi(argv[1]) : 8000;
	ClientKey key = read_client_key("cloud.key");
	if (key == nullptr) {
		puts("cloud.key not found: run ./keygen first.");
		return 1;
	}
	server_params = ServerParams(key);

	rpc::server srv(port);
	printf("Listening on port %d.\n", port);
	srv.bind("isKnownContact", [](std::string _userNumber) {
		puts("Received request.");
		PhoneNumber userNumber(_userNumber);
		bit_t isKnown = make_bit();
		constant(isKnown, false);
		for (const auto &contact : contacts) {
			PhoneNumber contactNo(contact.phoneNumber);
			bit_t matches = equals(contactNo.data, userNumber.data);
			isKnown |= matches;
		}
		return serialize(isKnown);
	});

	srv.run();
	return 0;
}
