#include <config/config.h>

json_object *config_init()
{
    return json_new();
}

int config_set_int64(json_object *config, const char *name, int64_t value)
{
    int ret;
    json_object *object = json_get(config, name);

    if (object)
        ret = json_set_int64(object, value);
    else
        ret = json_add_int64(config, name, value);

    return ret;
}

int64_t config_get_int64(json_object *config, const char *name)
{
    return json_int64(config, name);
}

int config_set_string(json_object *config,
        const char *name, const char *string)
{
    int ret;
    json_object *object = json_get(config, name);

    if (object)
        ret = json_set_string(object, string);
    else
        ret = json_add_string(config, name, string);

    return ret;
}

const char *config_get_string(json_object *config, const char *name)
{
    return json_string(config, name);
}

int config_free(json_object *config)
{
    return json_put(config);
}
