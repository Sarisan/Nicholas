#include <stddef.h>

char *bot_strenc(const char *input_string, size_t max_length);

void bot_free(size_t number, ...);

char *strntcpy(char *dest, const char *src, size_t n);

char *strntcat(char *dest, const char *src, size_t n);
