#include <json-c/json.h>

void bot_legacy_callback(const char *callback_data, json_object *config, json_object *update);
void bot_legacy_command(const char *message_text, json_object *config, json_object *update);
void bot_legacy_inline(const char *inline_query_data, json_object *config, json_object *update);
