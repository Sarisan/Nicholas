#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

char *bot_strenc(const char *input_string, size_t max_length) {
    size_t length = max_length ? max_length : strlen(input_string);
    char *output_string = malloc((sizeof(char) * length ? length * 5 : 0) + 1);
    if(!output_string)
        return 0;

    size_t string_size = 0;

    for(size_t c = 0; input_string[c] && c < length; c++) {
        if(input_string[c] == '<') {
            memcpy(&output_string[string_size], "&#60;", 5);
            string_size += 5;
        } else if(input_string[c] == '>') {
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

void bot_free(size_t number, ...) {
    va_list args;
    va_start(args, number);

    for(size_t n = 0; n < number; n++)
        free(va_arg(args, void *));

    va_end(args);
}

char *strntcpy(char *dest, const char *src, size_t n) {
    size_t length = strlen(src);

    if(n > length) {
        memmove(dest, src, length);
        dest[length] = 0;
    } else {
        memmove(dest, src, n - 1);
        dest[n - 1] = 0;
    }

    return dest;
}

char *strntcat(char *dest, const char *src, size_t n) {
    size_t length1 = strlen(dest);
    size_t length2 = strlen(src);

    if(n > length2) {
        memmove(&dest[length1], src, length2);
        dest[length1 + length2] = 0;
    } else {
        memmove(&dest[length1], src, n - 1);
        dest[length1 + n - 1] = 0;
    }

    return dest;
}
