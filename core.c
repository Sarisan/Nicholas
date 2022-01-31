#include <core.h>
#include <csc_core.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *api = 0, *admin = 0, *login = 0, *password = 0;
int global_signal = 0, quiet = 0;

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

    if(result.message_text) {
        bot_log(0, "bot_parse: %d: message_text: %s\n", result.update_id, result.message_text[0] ? result.message_text : "empty");
        bot_commands(&result);

        if(admin && !strcmp(json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(result.update, "message"), "from"), "id")), admin))
            bot_commands_private(&result);
    }

    if(result.inline_query) {
        bot_log(0, "bot_parse: %d: inline_query: %s\n", result.update_id, result.inline_query[0] ? result.inline_query : "empty");
        bot_inline(&result);
    }

    if(result.callback_data) {
        bot_log(0, "bot_parse: %d: callback_data: %s\n", result.update_id, result.callback_data[0] ? result.callback_data : "empty");
        bot_callback(&result);
    }

    json_object_put(result.update);

    return 0;
}

int main(int argc, char **argv) {
    signal(SIGINT, signal_handler);

    int print_help = 0, offset = -1;;

    while(1) {
        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"api", required_argument, 0, 'a'},
            {"admin", required_argument, 0, 'd'},
            {"login", required_argument, 0, 'l'},
            {"password", required_argument, 0, 'p'},
            {"offset", required_argument, 0, 'o'},
            {"quiet", no_argument, 0, 'q'},
            {0, 0, 0, 0}
        };

        int option = getopt_long(argc, argv, "ha:d:l:p:o:q", long_options, 0);
        if(option == -1)
            break;

        switch(option) {
            case 'h':
                print_help = 1;
                break;
            case 'a':
                api = optarg;
                break;
            case 'd':
                admin = optarg;
                break;
            case 'l':
                login = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            case 'o':
                offset = atoi(optarg);
                break;
            case 'q':
                quiet = 1;
                break;
            default:
                return EINVAL;
        }
    }

    if(print_help) {
        printf("Nicholas Bot %s\n", BOT_VERSION);
        printf("Usage: %s [options]\n", argv[0]);
        printf("  -h, --help\t\tShow help information\n");
        printf("  -a, --api=<arg>\tTelegram Bot API server URL with token\n");
        printf("  -d, --admin=<arg>\tYour Telegram user ID\n");
        printf("  -l, --login=<arg>\tYour Sankaku Channel login\n");
        printf("  -p, --password=<arg>\tYour Sankaku Channel password\n");
        printf("  -o, --offset=<arg>\tPrevious offset to continue the bot process\n");
        printf("  -q, --quiet\t\tDisable logs\n");
        return 0;
    } if(!api || (api && !api[0])) {
        fprintf(stderr, "%s: Telegram Bot API server URL is not set\n", argv[0]);
        return ENODATA;
    } if(!login || (login && !api[0])) {
        fprintf(stderr, "%s: Sankaku Channel login is not set\n", argv[0]);
        return ENODATA;
    } if(!password || (password && !password[0])) {
        fprintf(stderr, "%s: Sankaku Channel password is not set\n", argv[0]);
        return ENODATA;
    }

    if(bot_get_username())
        return EINVAL;
    if(csc_auth())
        return EINVAL;

    time_t csc_auth_time = time(0);

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
