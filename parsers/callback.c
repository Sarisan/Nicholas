#include <json/json.h>
#include <parsers/parsers.h>

const char *callback_data(json_object *update)
{
    return json_string(update, "callback_query.data");
}
