#include <stdarg.h>
#include <stdlib.h>

void *bot_memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;

    for(size_t length = 0; length < n; length++)
        d[length] = s[length];

    return dest;
}

size_t bot_strlen(const char *string) {
    size_t length = 0;

    while(string[length])
        length++;

    return length;
}

char *bot_strncpy(char *dest, const char *src, size_t n) {
    size_t length = bot_strlen(src);
    bot_memcpy(dest, src, n - 1);

    if(n > length)
        dest[length] = 0;
    else
        dest[n - 1] = 0;

    return dest;
}

char *bot_strncat(char *dest, const char *src, size_t n) {
    size_t length1 = bot_strlen(dest);
    size_t length2 = bot_strlen(src);
    bot_memcpy(&dest[length1], src, n - 1);

    if(n > length2)
        dest[length1 + length2] = 0;
    else
        dest[length1 + n - 1] = 0;

    return dest;
}

int bot_strcmp(const char *string1, const char *string2) {
    char char1 = 0;
    char char2 = 0;

    for(size_t c = 0; char1 == char2; c++) {
        char1 = string1[c];
        char2 = string2[c];

        if(!char1)
            return char1 - char2;
    }

    return char1 - char2;
}

char *bot_strenc(const char *input_string, size_t max_length) {
    size_t length = max_length ? max_length : bot_strlen(input_string);
    char *output_string = malloc((sizeof(char) * length ? length * 5 : 0) + 1);
    if(!output_string)
        return 0;

    size_t string_size = 0;

    for(size_t c = 0; input_string[c] && c < length; c++) {
        if(input_string[c] == '<') {
            bot_memcpy(&output_string[string_size], "&#60;", 5);
            string_size += 5;
        } else if(input_string[c] == '>') {
            bot_memcpy(&output_string[string_size], "&#62;", 5);
            string_size += 5;
        } else {
            bot_memcpy(&output_string[string_size], &input_string[c], 1);
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
