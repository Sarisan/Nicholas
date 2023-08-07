#include <debug/debug.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_LENGTH 1024

struct error_info
{
    const int error;
    const char *message;
};

static const struct error_info error_table[] =
{
    {EARG, "Invalid argument"},
    {EDAT, "No data received"},
    {EINV, "Unknown error"},
    {EMEM, "Out of memory"}
};

thread_local int debug_error = 0;

__attribute__ ((format (printf, 2, 3)))
void debug_log(int error, const char *format, ...)
{
    time_t current_time = time(0);
    struct tm *date = localtime(&current_time);
    va_list args = {0};
    char string[MAX_LENGTH] = {0};
    size_t size = 0;

    strftime(string, MAX_LENGTH, "[%Y-%m-%d %T] ", date);
    va_start(args, format);

    size = strlen(string);

    vsnprintf(&string[size], MAX_LENGTH - size, format, args);

    if (error)
        fprintf(stderr, "%s\n", string);
    else
        printf("%s\n", string);

    debug_error = error;

    va_end(args);
}

const char *debug_message(int error)
{
    for (size_t err = 0; error_table[err].error; err++)
        if (error_table[err].error == error)
            return error_table[err].message;

    return 0;
}
