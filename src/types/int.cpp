#include <cassert>
#include <cstring>
#include <unistd.h>
#include <sstream>

#include <types/int.h>

void Int8::add(bit_t overflow, Int8 a, Int8 b) {
	::add8(data, overflow, a.data, b.data, p);
}

void Int8::add(Int8 a, Int8 b) {
	bit_t overflow = make_bit(p);
	this->add(overflow, a, b);
}

void Int8::increment_if(bit_t cond) {
	auto tmp = make_bitspan<8>(p);
	incr_if(tmp, cond, data, p);
	_copy<8>(data, tmp, p);
}

void Int8::mul(bit_t overflow, Int8 a, Int8 b, uint8_t truncate_from) {
	auto tmp = make_bitspan<16>(p);
	::mul8(tmp, a.data, b.data, p);
	this->round(overflow, tmp, truncate_from);
}

void Int8::mul(Int8 a, Int8 b, uint8_t truncate_from) {
	bit_t over_or_underflow = make_bit(p);
	this->mul(over_or_underflow, a, b, truncate_from);
}

void Int8::div(Int8 a, Int8 b) {
	::div8(data, a.data, b.data, p);
}

void Int8::copy(Int8 src) {
	for (int i = 0; i < 8; i++)
		_copy(data[i], src.data[i], p);
}

void Int8::round(bit_t overflow, const bitspan_t &src, uint8_t truncate_from) {
	bit_t should_increment = make_bit(p);
	bool incr = round_helper(should_increment, overflow, src, truncate_from);
	if (incr)
		this->increment_if(should_increment);
}

void Int16::add(bit_t overflow, Int16 a, Int16 b) {
	::add16(data, overflow, a.data, b.data, p);
}

void Int16::add(Int16 a, Int16 b) {
	bit_t overflow = make_bit(p);
	this->add(overflow, a, b);
}

void Int16::increment_if(bit_t cond) {
	auto tmp = make_bitspan<16>(p);
	incr_if(tmp, cond, data, p);
	_copy<16>(data, tmp, p);
}

void Int16::mul(bit_t overflow, Int16 a, Int16 b, uint8_t truncate_from) {
	auto tmp = make_bitspan<32>(p);
	::mul16(tmp, a.data, b.data, p);
	this->round(overflow, tmp, truncate_from);
}

void Int16::mul(Int16 a, Int16 b, uint8_t truncate_from) {
	bit_t overflow = make_bit(p);
	this->mul(overflow, a, b, truncate_from);
}

void Int16::div(Int16 a, Int16 b) {
	::div16(data, a.data, b.data, p);
}

void Int16::copy(Int16 src) {
	for (int i = 0; i < 16; i++)
		_copy(data[i], src.data[i], p);
}

void Int16::round(bit_t overflow, const bitspan_t &src, uint8_t truncate_from) {
	bit_t should_increment = make_bit(p);
	bool incr = round_helper(should_increment, overflow, src, truncate_from);
	if (incr)
		this->increment_if(should_increment);
}