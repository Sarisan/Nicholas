#include <stddef.h>

struct string_curl
{
    char *string;
    size_t length;
};

size_t string_curl(void *data, size_t size,
        size_t nmemb, struct string_curl *string);
char *string_escape(const char *input_string, size_t max_length);
void string_free(size_t number, ...);
char *string_copy(char *dst, const char *src, size_t max_length);
char *string_cat(char *dst, const char *src, size_t max_length);
char *string_duplicate(const char *input_string);
