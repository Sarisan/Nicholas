#include <api/api.h>
#include <config/config.h>
#include <debug/debug.h>

int api_username(json_object *config)
{
    json_object *bot_json = 0;
    const char *username = 0;

    debug_log(0, "api_username: Receiving bot username...");

    bot_json = api_get(config, "getMe", 0);

    if (!bot_json) {
        debug_log(EINV, "api_username: Failed");

        return EINV;
    }

    username = json_string(bot_json, "result.username");

    if (!username) {
        debug_log(EINV, "api_username: %s", debug_message(EINV));
        json_put(bot_json);
    }

    config_set_string(config, BOT_USERNAME, username);
    debug_log(0, "api_username: Username: %s", username);
    debug_log(0, "api_username: Successful");
    json_put(bot_json);

    return 0;
}
