#include <config/config.h>
#include <curl/curl.h>
#include <getopt.h>
#include <init/init.h>
#include <init/help.h>
#include <debug/debug.h>
#include <signal.h>
#include <stdbool.h>

static struct option long_options[] =
{
    {"help", no_argument, 0, 'h'},
    {"server", required_argument, 0, 's'},
    {"token", required_argument, 0, 't'},
    {"offset", required_argument, 0, 'o'},
    {"message", required_argument, 0, 'm'},
    {"login", required_argument, 0, 'l'},
    {"password", required_argument, 0, 'p'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv)
{
    int ret = 0;
    json_object *config = 0;

    signal(SIGINT, init_signal);
    curl_global_init(CURL_GLOBAL_ALL);

    config = config_init();

    while (true) {
        int option = getopt_long(argc, argv, "hs:t:qm:o:p:", long_options, 0);
        if (option == -1)
            break;

        switch (option) {
            case 'h':
                init_help(argv[0]);
                goto out;
            case 's':
                ret = config_set_string(config, BOT_SERVER, optarg);
                if (ret)
                    goto out;

                break;
            case 't':
                ret = config_set_string(config, BOT_TOKEN, optarg);
                if (ret)
                    goto out;

                break;
            case 'o':
                ret = config_set_string(config, BOT_OFFSET, optarg);
                if (ret)
                    goto out;

                break;
            case 'm':
                ret = config_set_string(config, BOT_MESSAGE, optarg);
                if (ret)
                    goto out;

                break;
            case 'l':
                ret = config_set_string(config, SANKAKU_LOGIN, optarg);
                if (ret)
                    goto out;

                break;
            case 'p':
                ret = config_set_string(config, SANKAKU_PASSWORD, optarg);
                if (ret)
                    goto out;

                break;
            default:
                return EINV;
        }
    }

    init_bot(config);

out:
    curl_global_cleanup();
    config_free(config);
    return ret;
}
