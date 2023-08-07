#include <api/api.h>
#include <config/config.h>
#include <debug/debug.h>

int api_username(void)
{
    json_object *bot = 0;
    const char *username = 0;

    debug_log(0, "api_username: Receiving bot username...");

    bot = api_get("getMe", 0);

    if (!bot) {
        debug_log(EINV, "api_username: Failed");

        return EINV;
    }

    username = json_string(bot, "result.username");

    if (!username) {
        debug_log(EINV, "api_username: %s", debug_message(EINV));
        json_put(bot);
    }

    config_set_string(BOT_USERNAME, username);
    debug_log(0, "api_username: Username: %s", username);
    debug_log(0, "api_username: Successful");
    json_put(bot);

    return 0;
}
