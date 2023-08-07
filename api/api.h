#include <json/json.h>
#include <stdint.h>

json_object *api_get(const char *method, json_object *json);
int api_post(const char *method, json_object *json);
json_object *api_update(int64_t offset);
int api_username(void);
