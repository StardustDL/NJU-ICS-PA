#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *sc;
  for(sc = s; *sc != '\0'; ++sc);
  return (sc-s);
}

char *strcpy(char* dst,const char* src) {
  const char* su;
  char *sc;
  for(sc = dst, su = src; (*sc++ = *su++) != '\0';);
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  const char* su;
  char *sc;
  size_t i = 0;
  for(sc = dst, su = src; i<n && (*sc++ = *su++) != '\0';i++);
  for(; i<n ;i++) *sc++ = '\0';
  return dst;
}

char* strcat(char* dst, const char* src) {
  char *s;
  for(s=dst; *s != '\0'; s++);
  for(; (*s=*src) != '\0'; s++,src++);
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  while(*s1 && *s2){
    if(*s1 != *s2)
      return *s1 - *s2;
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  size_t i = 0;
  while(*s1 && *s2 && i < n){
    if(*s1 != *s2)
      return *s1 - *s2;
    s1++;
    s2++;
    i++;
  }
  return i==n ? 0 : (*s1 - *s2);
}

void* memset(void* v,int c,size_t n) {
  const unsigned char _c = c;
  unsigned char* t = v;
  size_t i=0;
  while(i<n){
    *t++ = _c;
    i++;
  }
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  size_t i = 0;
  unsigned char *t = out;
  const unsigned char *_in = in;
  while(i < n){
    *t = *_in;
    t++;
    _in++;
    i++;
  }
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
  size_t i = 0;
  const unsigned char *_s1 = s1;
  const unsigned char *_s2 = s2;
  while(i < n){
    unsigned char _1 = *_s1;
    unsigned char _2 = *_s2;
    if(_1<_2)
      return -1;
    else if(_1>_2)
      return 1;
    s1++;
    s2++;
    i++;
  }
  return 0;
}

#endif
