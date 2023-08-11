#include <config/config.h>
#include <debug/debug.h>
#include <json/json.h>
#include <parsers/parsers.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

typedef struct command_args
{
    size_t count;
    char **args;
} command_args;

const char *command_message(json_object *update)
{
    const char *message = json_string(update, "message.text");

    if(!message)
        message = json_string(update, "message.caption");

    return message;
}


int command_compare(const char *input, const char *command_message)
{
    char *ainput = string_duplicate(input);
    char *command = ainput;
    char *space = 0;
    char *at = 0;
    const char *username = config_get_string(BOT_USERNAME);

    if (!command)
        return EMEM;

    if (command[0] != '/')
        goto err;

    memmove(&command[0], &command[1], strlen(command));

    space = strchr(command, ' ');

    if (space)
        space[0] = 0;

    at = strchr(command, '@');

    if (at)
        at[0] = 0;

    if (strcmp(command, command_message))
        goto err;

    if (at) {
        command = &at[1];

        if (strcmp(command, username))
            goto err;
    }

    free(ainput);

    return 0;

err:
    free(ainput);

    return EARG;
}

command_args *command_args_parse(const char *input, size_t n, bool ignore_arg)
{
    char *ainput = string_duplicate(input);
    char *arguments = ainput;
    size_t args_l = 0;
    command_args *args = 0;
    bool first_arg = ignore_arg;

    if (!arguments)
        return 0;

    args_l = strlen(arguments);

    for(size_t c = 0; arguments[c]; c++) {
        if(arguments[c] == ' ' && arguments[c + 1] == ' ') {
            memmove(&arguments[c], &arguments[c + 1], args_l - c);

            args_l--;
            c--;
        }
    }

    arguments[args_l] = 0;

    args = malloc(sizeof(command_args));

    if (!args)
        goto out;

    args->count = 0;
    args->args = 0;

    while (args->count < n) {
        char *separator = 0;
        size_t length = 0;
        char **args_tmp = 0;

        separator = strchr(arguments, ' ');

        if (separator)
            separator[0] = 0;

        if (!first_arg) {
            length = strlen(arguments);
            args_tmp = realloc(args->args, sizeof(char *) * (args->count + 1));

            if (!args_tmp)
                break;

            args->args = args_tmp;
            args->args[args->count] = malloc(length + 1);

            if (!args->args[args->count])
                break;

            memmove(args->args[args->count], arguments, length);

            args->args[args->count][length] = 0;
            args->count++;
        }

        if (!separator)
            break;

        first_arg = false;
        arguments = &separator[1];
    }

out:
    free(ainput);

    return args;
}

size_t command_args_count(command_args *args)
{
    if (args)
        return args->count;

    return 0;
}

const char *command_args_get(command_args *args, size_t n)
{
    if (args)
        if (n < args->count)
            return args->args[n];

    return 0;
}

void command_args_free(command_args *args)
{
    for (size_t count = 0; count < args->count; count++)
        free(args->args[count]);

    free(args->args);
    free(args);
}
