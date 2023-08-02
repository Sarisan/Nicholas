#include <json/json.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

json_object *json_get(json_object *object, const char *path)
{
    json_object *ret_object = object;
    char *apath = string_duplicate(path);
    char *point = 0;
    char *keyword = apath;

    if (!keyword)
        return 0;

    while (true) {
        point = strchr(keyword, '.');

        if (point)
            point[0] = 0;

        ret_object = json_object_object_get(ret_object, keyword);

        if (!point || !ret_object)
            break;

        keyword = &point[1];
    }

    free(apath);

    return ret_object;
}
