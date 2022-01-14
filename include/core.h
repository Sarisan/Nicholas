#include <json-c/json_object.h>
#include <stddef.h>

#define BOT_VERSION "3.2.3"

struct bot_curl_string {
    char *string;
    size_t length;
};

extern char bot_username[64];

struct bot_update {
    json_object *update;
    int update_id;
    const char *inline_query;
    const char *callback_data;
    const char *message_text;
};

size_t bot_curl_writefunction(void *data, size_t size, size_t nmemb, struct bot_curl_string *string);

json_object *bot_get(const char *method, json_object *json);

int bot_post(const char *method, json_object *json);

int bot_get_username();

json_object *bot_get_update(int offset);

int bot_command_parse(const char *input, const char *command_text);

int bot_command_getarg(const char *input, size_t max_args, size_t max_length, char array[max_args][max_length]);
