#ifndef XV6_STRING_H
#define XV6_STRING_H

#include "kernel/types.h"

int strcmp(const char*, const char*);
int strncmp(const char*, const char*, unsigned int);
void* memset(void*, int, unsigned int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* memmove(void*, const void*, int);
int memcmp(const void*, const void*, unsigned int);
void* memcpy(void*, const void*, unsigned int);

#endif

