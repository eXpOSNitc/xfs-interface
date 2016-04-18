#include "exception.h"
#include <stdio.h>

extern jmp_buf exp_point;

void exception_throwException(int code)
{
	longjmp(exp_point,code);
}

void exception_printErrorMessage(int code)
{
	switch(code)
	{
		case EXCEPTION_CANT_OPEN_DISK:
			printf("Unable to open disk file\n");
			break;

	}
}