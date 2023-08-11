#include <config/config.h>
#include <debug/debug.h>
#include <json/json.h>

static json_object *config = 0;

int config_init(void)
{
    config = json_new();

    if (!config) {
        debug_log(EMEM, "config_init: %s", debug_message(EMEM));

        return EMEM;
    }

    return 0;
}

int config_set_int64(const char *name, int64_t value)
{
    int ret = 0;
    json_object *object = json_get(config, name);

    if (object) {
        if (json_set_int64(object, value) == 1)
            ret = 0;
        else
            ret = 1;
    } else {
        ret = json_add_int64(config, name, value);
    }

    if (ret)
        debug_log(EINV, "config_set_int64: Failed to set %s", name);

    return ret;
}

int64_t config_get_int64(const char *name)
{
    return json_int64(config, name);
}

int config_set_string(const char *name, const char *string)
{
    int ret = 0;
    json_object *object = json_get(config, name);

    if (object) {
        if (json_set_string(object, string) == 1)
            ret = 0;
        else
            ret = 1;
    } else {
        ret = json_add_string(config, name, string);
    }

    if (ret)
        debug_log(EINV, "config_set_string: Failed to set %s", name);

    return ret;
}

const char *config_get_string(const char *name)
{
    return json_string(config, name);
}

int config_destroy(void)
{
    return json_put(config);
}
