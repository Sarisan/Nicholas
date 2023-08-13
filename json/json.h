#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#define json_put(arg) json_object_put(arg)
#define json_type(arg) json_object_get_type(arg)
#define json_to_string(arg) json_object_to_json_string(arg)
#define json_new() json_object_new_object()
#define json_add(arg1, arg2, arg3) json_object_object_add(arg1, arg2, arg3)
#define json_new_array() json_object_new_array()
#define json_array_length(arg) json_object_array_length(arg)
#define json_add_array(arg1, arg2) json_object_array_add(arg1, arg2)
#define json_array_idx(arg1, arg2) json_object_array_get_idx(arg1, arg2)
#define json_deep_copy(arg1, arg2, arg3) json_object_deep_copy(arg1, arg2, arg3)
#define json_new_boolean(arg) json_object_new_boolean(arg)
#define json_boolean(arg1, arg2) json_object_get_boolean(json_get(arg1, arg2))
#define json_set_boolean(arg1, arg2) json_object_set_boolean(arg1, arg2)
#define json_new_int(arg) json_object_new_int(arg)
#define json_int(arg1, arg2) json_object_get_int(json_get(arg1, arg2))
#define json_set_int(arg1, arg2) json_object_set_int(arg1, arg2)
#define json_new_int64(arg) json_object_new_int64(arg)
#define json_int64(arg1, arg2) json_object_get_int64(json_get(arg1, arg2))
#define json_set_int64(arg1, arg2) json_object_set_int64(arg1, arg2)
#define json_new_uint64(arg) json_object_new_int64(arg)
#define json_uint64(arg1, arg2) json_object_get_int64(json_get(arg1, arg2))
#define json_set_uint64(arg1, arg2) json_object_set_int64(arg1, arg2)
#define json_new_double(arg) json_object_new_double(arg)
#define json_double(arg1, arg2) json_object_get_double(json_get(arg1, arg2))
#define json_set_double(arg1, arg2) json_object_set_double(arg1, arg2)
#define json_new_string(arg) json_object_new_string(arg)
#define json_string(arg1, arg2) json_object_get_string(json_get(arg1, arg2))
#define json_set_string(arg1, arg2) json_object_set_string(arg1, arg2)

#define json_add_boolean(arg1, arg2, arg3) json_add(arg1, arg2, json_new_boolean(arg3))
#define json_add_int(arg1, arg2, arg3) json_add(arg1, arg2, json_new_int(arg3))
#define json_add_int64(arg1, arg2, arg3) json_add(arg1, arg2, json_new_int64(arg3))
#define json_add_uint64(arg1, arg2, arg3) json_add(arg1, arg2, json_new_uint64(arg3))
#define json_add_double(arg1, arg2, arg3) json_add(arg1, arg2, json_new_double(arg3))
#define json_add_string(arg1, arg2, arg3) json_add(arg1, arg2, json_new_string(arg3))

#define json_from_string(arg) json_tokener_parse(arg)

json_object *json_get(json_object *object, const char *path);