#ifndef FHETOOLS_PRIMITIVES_H
#define FHETOOLS_PRIMITIVES_H

#include <tfhe.h>

void zero(bitspan_t src, TFHEClientParams_t);
void zero(bitspan_t src, TFHEServerParams_t = default_server_params);

bit_t is_zero(bitspan_t src, TFHEServerParams_t = default_server_params);
bit_t is_nonzero(bitspan_t src, TFHEServerParams_t = default_server_params);

// No bounds checking is done!
void _memcpy(bitspan_t dst, bitspan_t src, size_t size,
             TFHEServerParams_t = default_server_params);
void _copy(bitspan_t dst, bitspan_t src,
           TFHEServerParams_t = default_server_params);
template <uint16_t N>
void _copy(fixed_bitspan_t<N> dst, fixed_bitspan_t<N> src,
           TFHEServerParams_t p = default_server_params) {
	for (uint16_t i = 0; i < N; i++)
		_copy(dst[i], src[i], p);
}

void incr_if(bitspan_t out, bit_t cond, bitspan_t src, TFHEServerParams_t);

void add8(fixed_bitspan_t<8> out, bit_t overflow, fixed_bitspan_t<8> a,
          fixed_bitspan_t<8> b, TFHEServerParams_t);
void add16(fixed_bitspan_t<16> out, bit_t overflow, fixed_bitspan_t<16> a,
           fixed_bitspan_t<16> b, TFHEServerParams_t);
void mul8(fixed_bitspan_t<16> out, fixed_bitspan_t<8> a, fixed_bitspan_t<8> b,
          TFHEServerParams_t);
void mul16(fixed_bitspan_t<32> out, fixed_bitspan_t<16> a,
           fixed_bitspan_t<16> b, TFHEServerParams_t);
void div8(fixed_bitspan_t<8> out, fixed_bitspan_t<8> a, fixed_bitspan_t<8> b,
          TFHEServerParams_t);
void div16(fixed_bitspan_t<16> out, fixed_bitspan_t<16> a,
           fixed_bitspan_t<16> b, TFHEServerParams_t);

#endif // FHETOOLS_PRIMITIVES_H