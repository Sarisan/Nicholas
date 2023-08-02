#include <json/json.h>
#include <stdint.h>

#define BOT_SERVER "bot_server"
#define BOT_TOKEN "bot_token"
#define BOT_OFFSET "bot_offset"
#define BOT_MESSAGE "bot_message"
#define SANKAKU_LOGIN "sankaku_login"
#define SANKAKU_PASSWORD "sankaku_password"
#define SANKAKU_TOKEN "sankaku_token"
#define BOT_USERNAME "bot_username"

json_object *config_init(void);
int config_set_int64(json_object *config, const char *name, int64_t value);
int64_t config_get_int64(json_object *config, const char *name);
int config_set_string(json_object *config,
        const char *name, const char *string);
const char *config_get_string(json_object *config, const char *name);
int config_free(json_object *config);
