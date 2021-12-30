#include <stddef.h>

void *bot_memcpy(void *dest, const void *src, size_t n);

size_t bot_strlen(const char *string);

char *bot_strncpy(char *dest, const char *src, size_t n);

char *bot_strncat(char *dest, const char *src, size_t n);

int bot_strcmp(const char *string1, const char *string2);

int bot_strncmp(const char *string1, const char *string2, size_t n);

char *bot_strstr(const char *string1, const char *string2);

char *bot_strenc(const char *input_string, size_t max_length);

void bot_free(size_t number, ...);
