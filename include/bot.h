#include <json-c/json_object.h>

#define BOT_VERSION "4.0-rc2"

struct bot_update {
    json_object *update;
    int update_id;
    const char *inline_query;
    const char *callback_data;
    const char *message_text;
};

#define custom_acquisition (*__custom_acquisition())

char **__custom_acquisition();

void bot_commands(struct bot_update *result);
void bot_inline(struct bot_update *result);
void bot_callback(struct bot_update *result);
void bot_commands_private(struct bot_update *result);
