#ifndef FHETOOLS_FIXED32_H
#define FHETOOLS_FIXED32_H

#include <types/int.h>

// Are you seeing the error "Base specifier must name a class" at this line?
// Then the size of your fixed is too large.
#define BASE_INT smallest_Int<INT_SIZE + FRAC_SIZE>
template <uint8_t INT_SIZE, uint8_t FRAC_SIZE>
class Fixed : BASE_INT {
	static const int typeID = FIXED_TYPE_ID;
	static const int SIZE = INT_SIZE + FRAC_SIZE;
	static_assert(SIZE <= 8, "Size not supported");
	using native_type_t = smallest_int_t<SIZE>;

	// Scale the number and return it as an integer
	static native_type_t scale(double src) {
		double scaled = src * (1 << FRAC_SIZE);
		// Assert that the scaled number fits
		assert(log2(fabs(scaled)) <= SIZE);
		return native_type_t(scaled);
	}

public:
	explicit Fixed(TFHEServerParams_t _p = default_server_params)
		: BASE_INT(_p) {};
	explicit Fixed(double src, TFHEServerParams_t _p = default_server_params)
		: BASE_INT(scale(src), _p) {};

	void add(bit_t overflow, Fixed<INT_SIZE, FRAC_SIZE> a, Fixed<INT_SIZE, FRAC_SIZE> b) {
		BASE_INT::add(overflow, a, b);
	}
	void mul(bit_t overflow, Fixed<INT_SIZE, FRAC_SIZE> a, Fixed<INT_SIZE, FRAC_SIZE> b) {
		BASE_INT::mul(overflow, a, b, FRAC_SIZE);
	}

	double toDouble(TFHEClientParams_t p = default_client_params) {
		native_type_t tmp = this->toInt(p);
		return double(tmp) / (1 << FRAC_SIZE);
	};

	void copy(Fixed<INT_SIZE, FRAC_SIZE> src) {
		for (int i = 0; i < this->data.size(); i++)
			_copy(this->data[i], src.data[i], this->p);
	}
};

#endif //FHETOOLS_FIXED32_H
