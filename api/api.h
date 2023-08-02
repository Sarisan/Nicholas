#include <json/json.h>
#include <stdint.h>

int api_post(json_object *config, const char *method, json_object *json);
json_object *api_get(json_object *config,
    const char *method, json_object *json);
json_object *api_update(json_object *config, int64_t offset);
int api_username(json_object *config);
