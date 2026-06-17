#pragma once

#include <assert.h>
#include <stdexcept>
#include <string>

#include <valijson/exceptions.hpp>

/*
  Basic UTF-8 manipulation routines, adapted from code that was released into
  the public domain by Jeff Bezanson.
*/

namespace valijson {
namespace utils {

/* is c the start of a utf8 sequence? */
inline bool isutf(char c)
{
    return ((c & 0xC0) != 0x80);
}

/* number of characters */
inline uint64_t u8_strlen(const char *s)
{
    uint64_t count = 0;

    while (*s) {
        unsigned char p = static_cast<unsigned char>(*s);

        size_t seqLen = p < 0x80   ? 1  // 0xxxxxxx: 1-byte (ASCII)
                        : p < 0xE0 ? 2  // 110xxxxx: 2-byte sequence
                        : p < 0xF0 ? 3  // 1110xxxx: 3-byte sequence
                        : p < 0xF8 ? 4  // 11110xxx: 4-byte sequence
                                   : 1; // treat as a single character

        for (size_t i = 1; i < seqLen; ++i) {
            if (s[i] == 0 || isutf(s[i])) {
                seqLen = i;
                break;
            }
        }

        s += seqLen;
        count++;
    }

    return count;
}

} // namespace utils
} // namespace valijson
