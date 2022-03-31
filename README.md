# see, hex?

## tl;dr
```C
/* Copyright (c) 2022 Tero 'stedo' Liukko, MIT License */
static unsigned char chex_fromxdigit(unsigned h){
  return ((h & 0xf) + (h >> 6) * 9);
}

static unsigned chex_decode(void* bin, unsigned blen, const char* hex, unsigned hlen){
  unsigned i, j;
  for(i = 0, j = 0; (i < blen) && (j+1 < hlen); ++i, j+=2){
    unsigned char hi = chex_fromxdigit(hex[j+0]);
    unsigned char lo = chex_fromxdigit(hex[j+1]);
    ((unsigned char*)bin)[i] = (hi << 4) | lo;
  }
  return i;
}

static unsigned chex_isxdigit(unsigned h){
  unsigned char n09 = h - '0';
  unsigned char nAF = (h | 0x20) - 'a';
  return (n09 <= (9 - 0)) || (nAF <= (0xf - 0xa));
}
```

## Features
- does exactly what it says on the tin
- single header file only
- no dependencies, not even the standard library
- written in ANSI C, can be used in C++
- cross-platform / platform-agnostic
- branchless check whether input is valid hex digit, without look-up tables
- branchless decode from hex digit to decimal integer, without look-up tables
- branchless encode from decimal integer to hex digit, with a 16-byte LUT
- functions to encode and decode buffer in a single pass
- *no input validation whatsoever*
- suitable for embedded development and microcontrollers

## Background
The Web is full of examples (in various languages) on how to decode a hex
string to an array of bytes, but none has been as elegant as one would hope;
they either
- use libraries (like `sscanf` from `cstdio`)
- use look-up tables that take space
- use multiple branches to evaluate whether input is in the hexadecimal
  character set and whether it's in uppercase or lowercase

A more elegant elegant solution exists to map the input space to output space;
consider the ASCII character table and the characters we are interested in
| ASCII | hex | bin       | ASCII | hex | bin       | ASCII | hex | bin
| ----- | --- | --------- | ----- | --- | --------- | ----- | --- | ---------
| 0     | 30  | 0011 0000 |       |     |           |       |     |
| 1     | 31  | 0011 0001 | A     | 41  | 0100 0001 | a     | 61  | 0110 0001
| 2     | 32  | 0011 0010 | B     | 42  | 0100 0010 | b     | 62  | 0110 0010
| 3     | 33  | 0011 0011 | C     | 43  | 0100 0011 | c     | 63  | 0110 0011
| 4     | 34  | 0011 0100 | D     | 44  | 0100 0100 | d     | 64  | 0110 0100
| 5     | 35  | 0011 0101 | E     | 45  | 0100 0101 | e     | 65  | 0110 0101
| 6     | 36  | 0011 0110 | F     | 46  | 0100 0110 | f     | 66  | 0110 0110
| 7     | 37  | 0011 0111 |       |     |           |       |     |
| 8     | 38  | 0011 1000 |       |     |           |       |     |
| 9     | 39  | 0011 1001 |       |     |           |       |     |

From the above one can observe that the second most significant bit can be used
to determine whether the input is 0xA (10 in decimal) or above, and that the
lower nibble can be used (almost) as-is in either case, and by _almost_ I mean
that if the input is indeed 10 or above, we can always add 9 to the lower
nibble to get the actual value.

In (pseudo-)code this means
```C
// char hex = ...
uint8_t dec = (hex & 0x0f);   // always use the lowest 4 bits
bool add_more = (hex & 0x40); // ??
if(add_more) dec += 9;        // profit
```
which we can improve to make it a neat (or ugly, YMMV) one-liner by replacing
the `if` branch with some basic arithmetic, like
```C
(hex & 0x0f) + (!!(hex & 0x40)) * 9
```
or, even better like
```C
(hex & 0x0f) + (hex >> 6) * 9
```
which can be trivially used in a loop to decode a buffer of hex characters. The
latter variant also seems to compile without resulting in actual branching
instructions. The most significant bit is assumed to be always 0. See the
implementation in `chex.h`.

It should be noted that the single mask bit (`0x40`) works for both lowercase
as well as uppercase hexadecimal characters, i.e. valid inputs are the
characters `0123456789ABCDEFabcdef`.

A fast, trivial implementation to encode a nibble to a hex character exists by
indexing a 16-character look-up table; the implementation is included in the
accompanying header file and is not elaborated here further.

An alternative implementation to `isxdigit()` of the standard library is also
included in the accompanying header file; the gist here is to avoid comparing
the ASCII character ranges from both ends by mapping the character set to start
at 0 and only comparing the upper limit of the range while values below the
range are mapped to even bigger values due to unsigned underflow.

`chex_isxdigit` compiles without resulting in branch instructions, and it also
does not need the 384-byte look-up table used by the common implementations of
the standard library functions found in the `ctype.h` header.

## Test
Run the snippet below for a simple test
```C
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "chex.h"


int test_chex_isxdigit(void){
  /* check all possible 8-bit values */
  unsigned i;
  for(i = 0; i <= 0xff; ++i){
    assert(!!isxdigit(i) == !!chex_isxdigit(i));
  }
  assert(i == 0x100);
  return 0;
}


int test_chex_encode(void){
  /* check all possible 16-bit values in network byte order */
  unsigned i;
  for(i = 0; i <= 0xffff; ++i){
    /* encode to hex string using our implementation */
    const unsigned char org[2] = {i>>8, i};
    char hex[5];
    chex_encode(hex, sizeof(hex), org, sizeof(org));

    /* decode from hex string using a function from the standard library */
    unsigned char bin[2];
    int n = sscanf(hex, "%02hhx%02hhx", bin, bin+1);
    assert(n == sizeof(bin));

    /* decoded matches the original */
    assert(!memcmp(org, bin, sizeof(bin)));
  }
  assert(i == 0x10000);
  return 0;
}

int test_chex_decode(void){
  /* check all possible 16-bit values in network byte order */
  unsigned i;
  for(i = 0; i <= 0xffff; ++i){
    /* encode to hex string using a function from the standard library */
    char hex[5];
    snprintf(hex, sizeof(hex), "%02hhX%02hhx", i>>8, i);

    /* decode from hex string using our implementation */
    unsigned char bin[2];
    unsigned n = chex_decode(bin, sizeof(bin), hex, sizeof(hex));
    assert(n == sizeof(bin));

    /* decoded matches the original */
    const unsigned char org[2] = {i>>8, i};
    assert(!memcmp(org, bin, sizeof(bin)));
  }
  assert(i = 0x10000);
  return 0;
}

int main(void){
  test_chex_isxdigit();
  test_chex_encode();
  test_chex_decode();
  return 0;
}
```

## Disclaimer
The encoding and decoding algorithms assume they are always given valid inputs.

## Author
Tero 'stedo' Liukko

## License
MIT
