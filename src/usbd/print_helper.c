#include "printf.h"
#include <stdarg.h>
int my_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int cnt = vprintf(format, args);
    va_end(args);
    return cnt;
}