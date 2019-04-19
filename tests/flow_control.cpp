#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using FlowControlTest = FHEContext;

TEST_F(FlowControlTest, If) {
	::rc::detail::checkGTest([=](bool plaintext_cond) {
		bit_t should_match_cond = make_bit(params);
		encrypt(should_match_cond, false, params);
		bit_t cond = make_bit(params);
		encrypt(cond, plaintext_cond, params);
		_if(cond, [&](bit_t mask) { _copy(should_match_cond, mask, params); });
		RC_ASSERT(decrypt(should_match_cond, params) == plaintext_cond);
	});
}

TEST_F(FlowControlTest, IfElse) {
	::rc::detail::checkGTest([=](bool plaintext_cond) {
		bit_t touched_if_true = make_bit(params);
		bit_t touched_if_false = make_bit(params);
		encrypt(touched_if_true, false, params);
		encrypt(touched_if_false, false, params);
		bit_t cond = make_bit(params);
		encrypt(cond, plaintext_cond, params);
		_if_else(
		    cond, [&](bit_t mask) { _copy(touched_if_true, mask, params); },
		    [&](bit_t mask) { _copy(touched_if_false, mask, params); }, params);
		if (plaintext_cond) {
			RC_ASSERT(decrypt(touched_if_true, params) == true);
			RC_ASSERT(decrypt(touched_if_false, params) == false);
		} else {
			RC_ASSERT(decrypt(touched_if_true, params) == false);
			RC_ASSERT(decrypt(touched_if_false, params) == true);
		}
	});
}

TEST_F(FlowControlTest, DISABLED_While) {}

TEST_F(FlowControlTest, Times) {
	::rc::detail::checkGTest([=](int8_t plaintext_n, uint8_t max) {
		// Increment `tmp` for `n` times.
		// If n <= max then we expect tmp = n.
		// Otherwise we expect tmp = max, overflow == false.
		Int8 n(plaintext_n, params);
		Int8 tmp(0, params);
		bit_t overflow = times(
		    n, max, [&](bit_t mask) { tmp.increment_if(mask); }, params);
		if (plaintext_n < 0) {
			// Expect that nothing was done.
			RC_ASSERT(tmp.toInt(params) == 0);
			return;
		}
		if (plaintext_n <= max) {
			RC_ASSERT(decrypt(overflow, params) == false);
			RC_ASSERT(tmp.toInt(params) == plaintext_n);
		} else {
			RC_ASSERT(decrypt(overflow, params) == true);
			RC_ASSERT(tmp.toInt(params) == max);
		}
	});
}