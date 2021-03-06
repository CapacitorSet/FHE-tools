#ifndef GLOVEBOX_STRUCTHELPER_H
#define GLOVEBOX_STRUCTHELPER_H

class StructHelper {
  public:
	StructHelper(uint16_t length) {
		this->length = length;
		data = ::make_bitvec(length);
	}

	bitvec_t finalize() {
		if (offset != length) {
			fprintf(stderr,
			        "StructHelper: called finalize() with %d bits out of %d. "
			        "Exiting.\n",
			        offset, length);
			abort();
		}
		assert(offset == length);
		return data;
	}

	bit_t make_bit() {
		assert_fits(1);
		return data[offset++];
	}

	bitvec_t make_bitvec(uint8_t length) {
		assert_fits(length);
		auto ret = data.subspan(offset, length);
		offset += length;
		return ret;
	}

	template <uint8_t size> gb::bitvec<size> make_bitvec() {
		assert_fits(size);
		auto ret = data.subspan<size>(offset);
		offset += size;
		return gb::bitvec<size>(ret);
	}

  private:
	uint16_t offset = 0;
	uint16_t length;
	// This is private; users must access it via finalize() to enforce size
	// checks
	bitvec_t data;

	void assert_fits(uint8_t size) {
		if ((length - offset) < size) {
			fprintf(stderr,
			        "StructHelper: tried to allocate %d bits, only %d left. "
			        "Exiting.\n",
			        size, (length - offset));
			abort();
		}
	}
};
#endif