#include <curl/curl.h>
#include <errno.h>
#include <json-c/json_tokener.h>
#include <stdlib.h>
#include <string.h>
#include <string_ext.h>

struct bot_curl_string {
    char *string;
    size_t length;
};

int bot_quiet = 0;
char *bot_api = 0;
char bot_username[64];

int *__bot_quiet() {
    return &bot_quiet;
}

char **__bot_api() {
    return &bot_api;
}

void bot_log(int error, const char *format, ...) {
    if(!bot_quiet) {
        time_t current_time = time(0);
        struct tm *date = localtime(&current_time);

        char date_string[32];
        strftime(date_string, sizeof(date_string), "%Y-%m-%d %T", date);

        va_list args;
        va_start(args, format);

        if(error) {
            fprintf(stderr, "[%s] [%d] ", date_string, error);
            vfprintf(stderr, format, args);
        } else {
            printf("[%s] ", date_string);
            vprintf(format, args);
        }

        va_end(args);
    }
}

size_t bot_curl_writefunction(void *data, size_t size, size_t nmemb, struct bot_curl_string *string) {
    size_t realsize = size * nmemb;

    string->string = realloc(string->string, string->length + realsize + 1);

    if(!string->string)
        return 0;

    memcpy(&string->string[string->length], data, realsize);
    string->length += realsize;
    string->string[string->length] = 0;

    return realsize;
}

json_object *bot_get(const char *method, json_object *json) {
    size_t length = strlen(bot_api) + strlen(method) + 2;
    char *method_url = malloc(sizeof(char) * length);

    if(!method_url)
        return 0;

    snprintf(method_url, length, "%s/%s", bot_api, method);

    CURL *send_request = curl_easy_init();
    struct curl_slist *slist_json = curl_slist_append(0, "Content-Type: application/json");
    struct bot_curl_string string = {0};

    curl_easy_setopt(send_request, CURLOPT_URL, method_url);
    if(json) {
        curl_easy_setopt(send_request, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
        curl_easy_setopt(send_request, CURLOPT_HTTPHEADER, slist_json);
        curl_easy_setopt(send_request, CURLOPT_CUSTOMREQUEST, "POST");
    } else {
        curl_easy_setopt(send_request, CURLOPT_CUSTOMREQUEST, "GET");
    }
    curl_easy_setopt(send_request, CURLOPT_WRITEFUNCTION, bot_curl_writefunction);
    curl_easy_setopt(send_request, CURLOPT_WRITEDATA, &string);
    curl_easy_perform(send_request);

    curl_slist_free_all(slist_json);
    curl_easy_cleanup(send_request);
    free(method_url);

    if(!string.string) {
        bot_log(EINVAL, "bot_get: no answer from server\n");
        return 0;
    }

    json_object *data = json_tokener_parse(string.string);
    free(string.string);

    return data;
}

int bot_post(const char *method, json_object *json) {
    size_t length = strlen(bot_api) + strlen(method) + 2;
    char *method_url = malloc(sizeof(char) * length);

    if(!method_url)
        return -ENOMEM;

    snprintf(method_url, length, "%s/%s", bot_api, method);

    CURL *send_request = curl_easy_init();
    struct curl_slist *slist_json = curl_slist_append(0, "Content-Type: application/json");
    struct bot_curl_string string = {0};

    curl_easy_setopt(send_request, CURLOPT_URL, method_url);
    curl_easy_setopt(send_request, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
    curl_easy_setopt(send_request, CURLOPT_HTTPHEADER, slist_json);
    curl_easy_setopt(send_request, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(send_request, CURLOPT_WRITEFUNCTION, bot_curl_writefunction);
    curl_easy_setopt(send_request, CURLOPT_WRITEDATA, &string);
    curl_easy_perform(send_request);

    curl_slist_free_all(slist_json);
    curl_easy_cleanup(send_request);
    free(method_url);

    if(!string.string) {
        bot_log(EINVAL, "bot_post: no answer from server\n");
        return -EINVAL;
    }

    json_object *data = json_tokener_parse(string.string);
    free(string.string);

    if(data) {
        int error_code = json_object_get_int(json_object_object_get(data, "error_code"));
        const char *description = json_object_get_string(json_object_object_get(data, "description"));

        if(error_code && description) {
            bot_log(error_code, "bot_post: %s\n", description);
            json_object_put(data);
            return error_code;
        }
    } else {
        bot_log(EINVAL, "bot_post: unknown error\n");
        json_object_put(data);
        return -EINVAL;
    }

    json_object_put(data);
    return 0;
}

int bot_get_username() {
    json_object *bot_json = bot_get("getMe", 0);

    if(bot_json) {
        if(!json_object_get_boolean(json_object_object_get(bot_json, "ok"))) {
            const char *error_description = json_object_get_string(json_object_object_get(bot_json, "description"));

            if(error_description)
                bot_log(EINVAL, "bot_get_username: %s\n", error_description);
            else
                bot_log(EINVAL, "bot_get_username: unknown error\n");

            json_object_put(bot_json);
            return -EINVAL;
        }

        strntcpy(bot_username, json_object_get_string(json_object_object_get(json_object_object_get(bot_json, "result"), "username")), sizeof(bot_username));
        bot_log(0, "bot_get_username: @%s\n", bot_username);
    } else {
        bot_log(EINVAL, "bot_get_username: unable to get username\n");
        json_object_put(bot_json);
        return -EINVAL;
    }

    json_object_put(bot_json);
    return 0;
}

json_object *bot_get_update(int offset) {
    json_object *get_updates = json_object_new_object();
    json_object_object_add(get_updates, "offset", json_object_new_int(offset));

    json_object *update_json = bot_get("getUpdates", get_updates);
    json_object_put(get_updates);

    if(json_object_get_type(json_object_object_get(update_json, "result")) != json_type_array) {
        bot_log(0, "bot_get_update: unable to get updates\n");
        json_object_put(update_json);
        return 0;
    }

    json_object *update_idx = json_object_array_get_idx(json_object_object_get(update_json, "result"), 0);
    json_object *update = 0;

    if(update_idx)
        json_object_deep_copy(update_idx, &update, 0);

    json_object_put(update_json);
    return update;
}

int bot_command_parse(const char *input, const char *command_text) {
    char command_from_input[128];

    if(sscanf(input, "/%97s", command_from_input)) {
        if(strcmp(command_from_input, command_text)) {
            char command[128];
            snprintf(command, sizeof(command), "%s@%s", command_text, bot_username);
            if(strcmp(command_from_input, command))
                return -EINVAL;
        }
    } else {
        return -EBADR;
    }

    return 0;
}

int bot_command_inline_parse(const char *input, const char *command_text) {
    char command_from_input[1024];

    if(sscanf(input, "%1023s", command_from_input)) {
        if(strcmp(command_from_input, command_text))
            return -EINVAL;
    } else {
        return -EBADR;
    }

    return 0;
}

int bot_command_getarg(const char *input, size_t max_args, size_t max_length, char array[max_args][max_length]) {
    char arguments[20480];
    strntcpy(arguments, input, sizeof(arguments));

    for(short s = 0; arguments[s]; s++) {
        if(arguments[s] == ' ' && arguments[s + 1] == ' ') {
            strntcpy(&arguments[s], &arguments[s + 1], sizeof(arguments) - s);
            s--;
        }
    }

    char command[20480], first_argument[20480];
    int init = sscanf(input, "%20479s %20479s", command, first_argument);

    if(init < 1) {
        bot_log(EBADR, "bot_command_getarg: not a command\n");
        return -EBADR;
    }

    int count = 0;

    if(init == 2) {
        strntcpy(arguments, &arguments[strlen(command) + 1], sizeof(arguments));

        while(count < max_args) {
            char argument[20480];
            int args = sscanf(arguments, "%20479s %20479s", argument, first_argument);

            if(args >= 1) {
                strntcpy(array[count], argument, max_length);
                count++;
            }

            if(args == 1)
                break;

            if(args == 2)
                strntcpy(arguments, &arguments[strlen(argument) + 1], sizeof(arguments));
        }
    }

    return count;
}

char *bot_strenc(const char *input_string, size_t max_length) {
    size_t length = max_length ? max_length : strlen(input_string);
    char *output_string = malloc((sizeof(char) * length ? length * 5 : 0) + 1);
    if(!output_string)
        return 0;

    size_t string_size = 0;

    for(size_t c = 0; input_string[c] && c < length; c++) {
        if(input_string[c] == '<') {
            memcpy(&output_string[string_size], "&#60;", 5);
            string_size += 5;
        } else if(input_string[c] == '>') {
            memcpy(&output_string[string_size], "&#62;", 5);
            string_size += 5;
        } else {
            memcpy(&output_string[string_size], &input_string[c], 1);
            string_size += 1;
        }
    }

    output_string[string_size] = 0;

    return output_string;
}

void bot_free(size_t number, ...) {
    va_list args;
    va_start(args, number);

    for(size_t n = 0; n < number; n++)
        free(va_arg(args, void *));

    va_end(args);
}
