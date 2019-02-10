#include <cassert>
#include <cstring>
#include <istream>
#include <types.h>
#include <unistd.h>

void ClientInt::write(int64_t val) {
	assert(size() == 8);
	for (int i = 0; i < 8; i++) {
		encrypt(data[i], (val >> i) & 1, p);
	}
}