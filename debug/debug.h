#include <threads.h>

#define EARG 1000
#define EDAT 1001
#define EINV 1002
#define EMEM 1003

extern thread_local int debug_error;

void debug_log(int error, const char *format, ...);
const char *debug_message(int error);
