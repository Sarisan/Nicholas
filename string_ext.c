#include <string.h>

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
