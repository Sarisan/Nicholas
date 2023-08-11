#include <init/help.h>
#include <stdio.h>
#include <version.h>

void init_help(const char *command)
{
    printf("Nicholas Bot %u.%u\n", VERSION_MAJOR, VERSION_MINOR);
    printf("Usage: %s [options]\n", command);
    printf("  -h, --help\t\tShow this message\n");
    printf("  -s, --server=<arg>\tTelegram Bot "
            "API server, default: 0.0.0.0:8081\n");
    printf("  -t, --token=<arg>\tTelegram Bot API token\n");
    printf("  -o, --offset=<arg>\tRestore bot from last offset\n");
    printf("  -m, --message=<arg>\tSet custom head of help message\n");
    printf("  -l, --login=<arg>\tSankaku Channel login\n");
    printf("  -p, --password=<arg>\tSankaku Channel password\n");
}
