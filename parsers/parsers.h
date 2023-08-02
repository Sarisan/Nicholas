#include <json-c/json.h>
#include <stdbool.h>

typedef struct command_args command_args;

const char *callback_data(json_object *update);
const char *command_message(json_object *update);
int command_compare(json_object *config,
    const char *input, const char *command_text);
command_args *command_args_parse(const char *input, size_t n, bool ignore_arg);
size_t command_args_count(command_args *args);
const char *command_args_get(command_args *args, size_t n);
void command_args_free(command_args *args);
const char *inline_query(json_object *update);
int inline_compare(const char *input, const char *command);
