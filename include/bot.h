#include <core.h>

#define custom_acquisition (*__custom_acquisition())

char **__custom_acquisition();
void bot_commands(struct bot_update *result);
void bot_inline(struct bot_update *result);
void bot_callback(struct bot_update *result);
void bot_commands_private(struct bot_update *result);
