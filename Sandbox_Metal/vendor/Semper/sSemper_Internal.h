#pragma once

#include "sSemper.h"

namespace Semper
{
    void      update_input_events(bool trickle_fast_inputs);
    void      update_keyboard_inputs();
    void      update_mouse_inputs();
    void      update_mouse_wheel();
    sKeyData* get_key_data(sKey key);
    int       calc_typematic_repeat_amount(float t0, float t1, float repeat_delay, float repeat_rate);
    bool      is_mouse_drag_past_threshold(sMouseButton button, float lock_threshold);

    // Helper: sBitArray
    inline bool sBitArrayTestBit(const u32* arr, int n) { u32 mask = (u32)1 << (n & 31); return (arr[n >> 5] & mask) != 0; }
    inline void sBitArrayClearBit(u32* arr, int n) { u32 mask = (u32)1 << (n & 31); arr[n >> 5] &= ~mask; }
    inline void sBitArraySetBit(u32* arr, int n) { u32 mask = (u32)1 << (n & 31); arr[n >> 5] |= mask; }
    inline void sBitArraySetBitRange(u32* arr, int n, int n2) // Works on range [n..n2)
    {
        n2--;
        while (n <= n2)
        {
            int a_mod = (n & 31);
            int b_mod = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
            u32 mask = (u32)(((u64)1 << b_mod) - 1) & ~(u32)(((u64)1 << a_mod) - 1);
            arr[n >> 5] |= mask;
            n = (n + 32) & ~31;
        }
    }

    // Helper: ImBitArray class (wrapper over ImBitArray functions)
    // Store 1-bit per value.
    template<int BITCOUNT>
    struct sBitArray
    {
        static const int size = BITCOUNT;
        u32             storage[(BITCOUNT + 31) >> 5];
        sBitArray()     { ClearAllBits(); }
        void            ClearAllBits() { memset(storage, 0, sizeof(storage)); }
        void            SetAllBits() { memset(storage, 255, sizeof(storage)); }
        bool            TestBit(int n) const { S_ASSERT(n < BITCOUNT); return sBitArrayTestBit(storage, n); }
        void            SetBit(int n) { S_ASSERT(n < BITCOUNT); sBitArraySetBit(storage, n); }
        void            ClearBit(int n) { S_ASSERT(n < BITCOUNT); sBitArrayClearBit(storage, n); }
        void            SetBitRange(int n, int n2) { sBitArraySetBitRange(storage, n, n2); } // Works on range [n..n2)
    };
    template<int BITCOUNT>
    const int sBitArray<BITCOUNT>::size;

	// Convert UTF-8 to 32-bit character, process single character input.
	// A nearly-branchless UTF-8 decoder, based on work of Christopher Wellons (https://github.com/skeeto/branchless-utf8).
	// We handle UTF-8 decoding error by skipping forward.
	static int
	sTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end)
	{
		static const char lengths[32] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };
		static const int masks[] = { 0x00, 0x7f, 0x1f, 0x0f, 0x07 };
		static const uint32_t mins[] = { 0x400000, 0, 0x80, 0x800, 0x10000 };
		static const int shiftc[] = { 0, 18, 12, 6, 0 };
		static const int shifte[] = { 0, 6, 4, 2, 0 };
		int len = lengths[*(const unsigned char*)in_text >> 3];
		int wanted = len + !len;

		if (in_text_end == NULL)
			in_text_end = in_text + wanted; // Max length, nulls will be taken into account.

		// Copy at most 'len' bytes, stop copying at 0 or past in_text_end. Branch predictor does a good job here,
		// so it is fast even with excessive branching.
		unsigned char s[4];
		s[0] = in_text + 0 < in_text_end ? in_text[0] : 0;
		s[1] = in_text + 1 < in_text_end ? in_text[1] : 0;
		s[2] = in_text + 2 < in_text_end ? in_text[2] : 0;
		s[3] = in_text + 3 < in_text_end ? in_text[3] : 0;

		// Assume a four-byte character and load four bytes. Unused bits are shifted out.
		*out_char = (uint32_t)(s[0] & masks[len]) << 18;
		*out_char |= (uint32_t)(s[1] & 0x3f) << 12;
		*out_char |= (uint32_t)(s[2] & 0x3f) << 6;
		*out_char |= (uint32_t)(s[3] & 0x3f) << 0;
		*out_char >>= shiftc[len];

		// Accumulate the various error conditions.
		int e = 0;
		e = (*out_char < mins[len]) << 6; // non-canonical encoding
		e |= ((*out_char >> 11) == 0x1b) << 7;  // surrogate half?
		e |= (*out_char > S_UNICODE_CODEPOINT_MAX) << 8;  // out of range?
		e |= (s[1] & 0xc0) >> 2;
		e |= (s[2] & 0xc0) >> 4;
		e |= (s[3]) >> 6;
		e ^= 0x2a; // top two bits of each tail byte correct?
		e >>= shifte[len];

		if (e)
		{
			// No bytes are consumed when *in_text == 0 || in_text == in_text_end.
			// One byte is consumed in case of invalid first byte of in_text.
			// All available bytes (at most `len` bytes) are consumed on incomplete/invalid second to last bytes.
			// Invalid or incomplete input may consume less bytes than wanted, therefore every byte has to be inspected in s.
			wanted = Semper::get_min(wanted, !!s[0] + !!s[1] + !!s[2] + !!s[3]);
			*out_char = S_UNICODE_CODEPOINT_INVALID;
		}

		return wanted;
	}

}