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

thread_local int debug_error_code = 0;

__attribute__ ((format (printf, 2, 3)))
void debug_log(int error, const char *format, ...)
{
    time_t current_time = time(0);
    struct tm *date = localtime(&current_time);
    va_list args;
    char log_string[MAX_LENGTH];
    size_t log_size = 0;

    strftime(log_string, MAX_LENGTH, "[%Y-%m-%d %T] ", date);
    va_start(args, format);

    log_size = strlen(log_string);

    if (error) {
        log_size += snprintf(&log_string[log_size],
            MAX_LENGTH - log_size, "[%d] ", error);

        vsnprintf(&log_string[log_size], MAX_LENGTH - log_size, format, args);
        fprintf(stderr, "%s\n", log_string);
    } else {
        vsnprintf(&log_string[log_size], MAX_LENGTH - log_size, format, args);
        printf("%s\n", log_string);
    }

    debug_error_code = error;

    va_end(args);
}

const char *debug_message(int error)
{
    for (size_t err = 0; error_table[err].error; err++)
        if (error_table[err].error == error)
            return error_table[err].message;

    return 0;
}
