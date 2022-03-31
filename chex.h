#ifndef STEDO_CHEX_H
#define STEDO_CHEX_H
/*
MIT License

Copyright (c) 2022 Tero 'stedo' Liukko

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**
 * @brief check whether given ASCII character is a hexadecimal digit
 * @param[in] h ASCII character to decode
 * @return TRUE if h is hexadecimal, FALSE otherwise
**/
static unsigned chex_isxdigit(unsigned h){
  unsigned char n09 = h - '0';
  unsigned char nAF = (h | 0x20) - 'a';
  return (n09 <= (9 - 0)) || (nAF <= (0xf - 0xa));
}


/**
 * @brief encode nibble to hexadecimal ASCII character
 * @param[in] nibble value to encode
 * @return hexadecimal ASCII value
 * @note the four most significant bits are ignored
**/
static char chex_toxdigit(unsigned nibble){
  const char* lut = "0123456789abcdef";
  return lut[nibble & 0xf];
}


/**
 * @brief decode a single case-insensitive hexadecimal ASCII character to its decimal value
 * @param[in] h ASCII character to decode
 * @return decimal value of the input
 * @note the input is assumed to be a valid
**/
static unsigned char chex_fromxdigit(unsigned h){
  return ((h & 0xf) + (h >> 6) * 9);
}


/**
 * @brief encode an array of bytes to an array of hexadecimal ASCII characters
 * @param[out]  hex   buffer to write to
 * @param[in]   hlen  maximum number of characters to write
 * @param[in]   bin   array of bytes to encode
 * @param[in]   blen  number of bytes to encode
 * @return number of characters written
 * @note null terminator is not included in the output
**/
static unsigned chex_encode(char* hex, unsigned hlen, const void* bin, unsigned blen){
  const unsigned char* b = (const unsigned char*)bin;
  unsigned i, j;
  for(i = 0, j = 0; (i < blen) && (j+1 < hlen); ++i, j+=2){
    hex[j+0] = chex_toxdigit(b[i]>>4);
    hex[j+1] = chex_toxdigit(b[i]>>0);
  }
  return j;
}


/**
 * @brief decode an array of hexadecimal ASCII characters to an array of bytes
 * @param[out]  bin   buffer to write to
 * @param[in]   blen  maximum number of bytes to write
 * @param[in]   hex   array of hex characters to decode
 * @param[in]   hlen  number of hex characters to decode
 * @return number of bytes written
 * @note hex input is assumed valid and its length a multiple of two
**/
static unsigned chex_decode(void* bin, unsigned blen, const char* hex, unsigned hlen){
  unsigned i, j;
  for(i = 0, j = 0; (i < blen) && (j+1 < hlen); ++i, j+=2){
    unsigned char hi = chex_fromxdigit(hex[j+0]);
    unsigned char lo = chex_fromxdigit(hex[j+1]);
    ((unsigned char*)bin)[i] = (hi << 4) | lo;
  }
  return i;
}


#endif /* STEDO_CHEX_H */

