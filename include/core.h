#include <json-c/json_object.h>
#include <stddef.h>

#define BOT_VERSION "4.0-rc1"

#define bot_quiet (*__bot_quiet())
#define bot_api (*__bot_api())

extern char bot_username[64];

int *__bot_quiet();
char **__bot_api();

void bot_log(int error, const char *format, ...);
json_object *bot_get(const char *method, json_object *json);
int bot_post(const char *method, json_object *json);
int bot_get_username();
json_object *bot_get_update(int offset);
int bot_command_parse(const char *input, const char *command_text);
int bot_command_inline_parse(const char *input, const char *command_text);
int bot_command_getarg(const char *input, size_t max_args, size_t max_length, char array[max_args][max_length]);
char *bot_strenc(const char *input_string, size_t max_length);
void bot_free(size_t number, ...);
