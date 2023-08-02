#include <init/help.h>
#include <stdio.h>
#include <version.h>

void init_help(const char *arg)
{
    printf("Nicholas Bot %d.%d\n",
            Nicholas_VERSION_MAJOR,
            Nicholas_VERSION_MINOR);
    printf("Usage: %s [options]\n", arg);
    printf("  -h, --help\t\tShow help information\n");
    printf("  -s, --server=<arg>\tTelegram Bot API server, " \
                                    "default: 0.0.0.0:8081\n");
    printf("  -t, --token=<arg>\tTelegram Bot API token\n");
    printf("  -o, --offset=<arg>\tLast offset to continue the bot\n");
    printf("  -m, --message=<arg>\tSet custom help message start\n");
    printf("  -l, --login=<arg>\tSankaku Channel login\n");
    printf("  -p, --password=<arg>\tSankaku Channel password\n");
}
