#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <setjmp.h>

#define EXCEPTION_CANT_OPEN_DISK 1

void exception_throwException(int code);
void exception_printErrorMessage(int code);
#endif