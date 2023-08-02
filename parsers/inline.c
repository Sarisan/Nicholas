#include <debug/debug.h>
#include <json/json.h>
#include <parsers/parsers.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

const char *inline_query(json_object *update)
{
    return json_string(update, "inline_query.query");
}

int inline_compare(const char *input, const char *command_message)
{
    char *ainput = string_duplicate(input);
    char *command = ainput;
    char *space = 0;

    if (!command)
        return EMEM;

    space = strchr(command, ' ');

    if (space)
        space[0] = 0;

    if (strcmp(command, command_message))
        goto err;

    free(ainput);

    return 0;

err:
    free(ainput);

    return EARG;
}
