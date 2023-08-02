#include <parsers/parsers.h>
#include <json/json.h>

const char *callback_data(json_object *update)
{
    return json_string(update, "callback_query.data");
}
