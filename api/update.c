#include <api/api.h>
#include <config/config.h>
#include <debug/debug.h>

json_object *api_update(json_object *config, int64_t offset)
{
    json_object *get_updates = json_new();
    json_object *update_json = 0;
    json_object *update_idx = 0;
    json_object *update = 0;

    if (!get_updates)
        return 0;

    json_add_int64(get_updates, "offset", offset);

    update_json = api_get(config, "getUpdates", get_updates);

    if (json_type(json_get(update_json, "result")) != json_type_array) {
        debug_log(0, "api_update: Failed to receive update %ld", offset);
        json_put(update_json);

        return get_updates;
    }

    json_put(get_updates);

    update_idx = json_array_idx(json_get(update_json, "result"), 0);

    if (update_idx)
        json_deep_copy(update_idx, &update, 0);

    json_put(update_json);
    return update;
}
