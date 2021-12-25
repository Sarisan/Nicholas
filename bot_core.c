#include <curl/curl.h>
#include <json-c/json_tokener.h>
#include <stdlib.h>
#include <string.h>

struct bot_curl_string {
    char *string;
    size_t length;
};

extern char *api;
char bot_username[32];

size_t bot_curl_writefunction(void *data, size_t size, size_t nmemb, struct bot_curl_string *string) {
    size_t realsize = size * nmemb;

    string->string = realloc(string->string, string->length + realsize + 1);
    if(string->string == 0)
        return 0;

    memcpy(string->string + string->length, data, realsize);
    string->length += realsize;
    string->string[string->length] = 0;
    
    return realsize;
}

json_object *bot_get(const char *method, json_object **json) {
    char method_url[strlen(api) + strlen(method) + 2];
    struct bot_curl_string string = {0};

    snprintf(method_url, sizeof(method_url), "%s/%s", api, method);

    struct curl_slist *slist_json = curl_slist_append(0, "Content-Type: application/json");
    CURL *send_request = curl_easy_init();

    curl_easy_setopt(send_request, CURLOPT_URL, method_url);
    if(json) {
        curl_easy_setopt(send_request, CURLOPT_POSTFIELDS, json_object_to_json_string(*json));
        curl_easy_setopt(send_request, CURLOPT_HTTPHEADER, slist_json);
        curl_easy_setopt(send_request, CURLOPT_CUSTOMREQUEST, "POST");
    } else {
        curl_easy_setopt(send_request, CURLOPT_CUSTOMREQUEST, "GET");
    }
    curl_easy_setopt(send_request, CURLOPT_WRITEFUNCTION, bot_curl_writefunction);
    curl_easy_setopt(send_request, CURLOPT_WRITEDATA, &string);
    curl_easy_perform(send_request);

    curl_easy_cleanup(send_request);
    curl_slist_free_all(slist_json);

    if(!string.string)
        return 0;

    json_object *data = json_tokener_parse(string.string);
    free(string.string);

    return data;
}

int bot_post(const char *method, json_object **json) {
    char method_url[strlen(api) + strlen(method) + 2];
    struct bot_curl_string string = {0};

    snprintf(method_url, sizeof(method_url), "%s/%s", api, method);

    struct curl_slist *slist_json = curl_slist_append(0, "Content-Type: application/json");
    CURL *send_request = curl_easy_init();

    curl_easy_setopt(send_request, CURLOPT_URL, method_url);
    curl_easy_setopt(send_request, CURLOPT_POSTFIELDS, json_object_to_json_string(*json));
    curl_easy_setopt(send_request, CURLOPT_HTTPHEADER, slist_json);
    curl_easy_setopt(send_request, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(send_request, CURLOPT_WRITEFUNCTION, bot_curl_writefunction);
    curl_easy_setopt(send_request, CURLOPT_WRITEDATA, &string);
    curl_easy_perform(send_request);

    curl_easy_cleanup(send_request);
    curl_slist_free_all(slist_json);

    json_object *data = json_tokener_parse(string.string);
    free(string.string);

    if(data) {
        int error_code = json_object_get_int(json_object_object_get(data, "error_code"));
        const char *description = json_object_get_string(json_object_object_get(data, "description"));

        if(error_code && description) {
            fprintf(stderr, "Post: %d, %s\n", error_code, description);
            json_object_put(data);
            return error_code;
        }
    } else {
        fprintf(stderr, "Post: unknown error\n");
        json_object_put(data);
        return -1;
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
                fprintf(stderr, "Bot: %s\n", error_description);
            else
                fprintf(stderr, "Bot: unknown error\n");

            json_object_put(bot_json);
            return 1;
        }

        strcpy(bot_username, json_object_get_string(json_object_object_get(json_object_object_get(bot_json, "result"), "username")));
        printf("Bot: @%s\n", bot_username);
    } else {
        fprintf(stderr, "Bot: unable to get username\n");
        json_object_put(bot_json);
        return 1;
    }

    json_object_put(bot_json);
    return 0;
}

json_object *bot_get_update(int offset) {
    json_object *get_updates = json_object_new_object();

    json_object_object_add(get_updates, "offset", json_object_new_int(offset));

    json_object *update_json = bot_get("getUpdates", &get_updates);
    json_object_put(get_updates);

    if(json_object_get_type(json_object_object_get(update_json, "result")) != json_type_array) {
        fprintf(stderr, "Update: unable to get updates\n");
        json_object_put(update_json);
        return 0;
    }

    json_object *update_idx = json_object_array_get_idx(json_object_object_get(update_json, "result"), 0);

    if(!update_idx) {
        json_object_put(update_json);
        return 0;
    }

    json_object *update = 0;
    json_object_deep_copy(update_idx, &update, 0);

    json_object_put(update_json);
    return update;
}

int bot_command_parse(const char *input, const char *command_text) {
    char command_from_input[97], command[97];

    if(sscanf(input, "/%97s", command_from_input)) {
        if(strcmp(command_from_input, command_text)) {
            snprintf(command, sizeof(command), "%s@%s", command_text, bot_username);
            if(strcmp(command_from_input, command))
                return 1;
        }
    } else {
        return 2;
    }

    return 0;
}

char *bot_strenc(const char *input_string, size_t max_length) {
    size_t length = max_length ? max_length : strlen(input_string);
    char *output_string = malloc((sizeof(char) * length ? length * 5 : 0) + 1);
    if(output_string == 0)
        return 0;

    size_t string_size = 0;

    for(short c = 0; input_string[c] && c < length; c++) {
        if(input_string[c] == '<') {
            memcpy(output_string + string_size, "&#60;", 5);
            string_size += 5;
        } else if(input_string[c] == '>') {
            memcpy(output_string + string_size, "&#62;", 5);
            string_size += 5;
        } else {
            memcpy(output_string + string_size, &input_string[c], 1);
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
