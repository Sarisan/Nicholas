#include "core.h"
#include "csc_core.h"
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include "string.h"

char *api, *admin, *login, *password;
int global_signal;

void bot_commands(struct bot_update *result);
void bot_inline(struct bot_update *result);
void bot_callback(struct bot_update *result);
void bot_commands_private(struct bot_update *result);

void signal_handler(int signal_int) {
    global_signal = signal_int;
}

void *bot_parse(void *data) {
    struct bot_update result = *(struct bot_update *)data;

    result.message_text = json_object_get_string(json_object_object_get(json_object_object_get(result.update, "message"), "text"));
    if(!result.message_text)
        result.message_text = json_object_get_string(json_object_object_get(json_object_object_get(result.update, "message"), "caption"));
    result.inline_query = json_object_get_string(json_object_object_get(json_object_object_get(result.update, "inline_query"), "query"));
    result.callback_data = json_object_get_string(json_object_object_get(json_object_object_get(result.update, "callback_query"), "data"));

    if(result.message_text)
        bot_commands(&result);

    if(result.inline_query)
        bot_inline(&result);

    if(result.callback_data)
        bot_callback(&result);

    if(result.message_text && admin && !bot_strcmp(json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(result.update, "message"), "from"), "id")), admin))
        bot_commands_private(&result);

    json_object_put(result.update);

    return 0;
}

int main(int argc, char **argv) {
    signal(SIGINT, signal_handler);

    int print_help = 0;

    while(1) {
        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"api", required_argument, 0, 0},
            {"admin", required_argument, 0, 1},
            {"login", required_argument, 0, 2},
            {"password", required_argument, 0, 3},
            {0, 0, 0, 0}
        };

        int option = getopt_long(argc, argv, "h", long_options, 0);
        if(option == -1)
            break;

        switch(option) {
            case 'h':
                print_help = 1;
                break;
            case 0:
                api = optarg;
                break;
            case 1:
                admin = optarg;
                break;
            case 2:
                login = optarg;
                break;
            case 3:
                password = optarg;
                break;
            default:
                return 1;
        }
    }

    if(print_help) {
        printf("Nicholas Bot %s\n", BOT_VERSION);
        printf("Usage: %s [options]\n", argv[0]);
        printf("  -h, --help\t\tShow help information\n");
        printf("      --api=<arg>\tTelegram Bot API server URL with token\n");
        printf("      --admin=<arg>\tYour Telegram user ID\n");
        printf("      --login=<arg>\tYour Sankaku Channel login\n");
        printf("      --password=<arg>\tYour Sankaku Channel password\n");
        return 0;
    } if(!api || (api && !api[0])) {
        fprintf(stderr, "%s: Telegram Bot API server URL is not set\n", argv[0]);
        return 1;
    } if(!login || (login && !api[0])) {
        fprintf(stderr, "%s: Sankaku Channel login is not set\n", argv[0]);
        return 1;
    } if(!password || (password && !password[0])) {
        fprintf(stderr, "%s: Sankaku Channel password is not set\n", argv[0]);
        return 1;
    }

    int offset = -1;
    time_t csc_auth_time = time(0);

    if(bot_get_username())
        return 1;
    if(csc_auth())
        return 1;

    while(offset) {
        struct bot_update result;
        result.update = bot_get_update(offset);

        if(result.update) {
            result.update_id = json_object_get_int(json_object_object_get(result.update, "update_id"));
            pthread_t thread = result.update_id;
            offset = result.update_id + 1;

            pthread_create(&thread, 0, bot_parse, &result);
            pthread_detach(thread);
        }

        if(global_signal == 2)
            break;

        csc_check(&csc_auth_time);
    }
    return 0;
}
