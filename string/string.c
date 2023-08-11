#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

size_t string_curl(void *data, size_t size,
        size_t nmemb, struct string_curl *string)
{
    size_t realsize = size * nmemb;
    string->string = realloc(string->string, string->length + realsize + 1);

    if (!string->string)
        return 0;

    memcpy(&string->string[string->length], data, realsize);

    string->length += realsize;
    string->string[string->length] = 0;

    return realsize;
}

char *string_escape(const char *input_string, size_t max_length)
{
    size_t length = max_length;
    char *output_string = 0;
    size_t string_size = 0;

    if (!input_string)
        return 0;

    if (!length)
        length = strlen(input_string);

    output_string = malloc(length * 5 + 1);

    if (!output_string)
        return 0;

    for (size_t c = 0; input_string[c] && c < length; c++) {
        if (input_string[c] == '<') {
            memcpy(&output_string[string_size], "&#60;", 5);

            string_size += 5;
        } else if (input_string[c] == '>') {
            memcpy(&output_string[string_size], "&#62;", 5);

            string_size += 5;
        } else {
            memcpy(&output_string[string_size], &input_string[c], 1);

            string_size += 1;
        }
    }

    output_string[string_size] = 0;

    return output_string;
}

void string_free(size_t number, ...)
{
    va_list args;

    va_start(args, number);

    for (size_t n = 0; n < number; n++)
        free(va_arg(args, void *));

    va_end(args);
}

char *string_copy(char *dst, const char *src, size_t max_length)
{
    size_t length = strlen(src);

    if (length > max_length)
        length = max_length;

    memmove(dst, src, length);

    dst[length] = 0;

    return dst;
}

char *string_cat(char *dst, const char *src, size_t max_length)
{
    size_t length = strlen(dst);

    return string_copy(&dst[length], src, max_length);
}

char *string_duplicate(const char *input_string)
{
    size_t length = strlen(input_string);
    char *output_string = malloc(length + 1);

    if (!output_string)
        return 0;

    string_copy(output_string, input_string, length);

    return output_string;
}
