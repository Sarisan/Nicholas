#include <api/api.h>
#include <config/config.h>
#include <curl/curl.h>
#include <sankaku/sankaku.h>
#include <parsers/parsers.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

#define HELP_WELCOME "I'm Nicholas, the first Fletcher-class ship to be " \
    "launched... I'd rather stay at the base than head out to battle... " \
    "You don't mind, right?"

#define HELP_MESSAGE "%s\n\n" \
    "<b>Search arguments</b>\n" \
    "The first argument is the page number, can be from 1 to 100.\n" \
    "<code>b</code> - Switches search to book search\n" \
    "<code>n</code> - Sub-argument of 'b', switches search " \
    "by tags to search by name\n" \
    "<code>t</code> - Switches search to tag search\n" \
    "<code>p</code> - Replaces all animated content with its preview\n" \
    "<code>a</code> - Enables auto-paging mode\n" \
    "<code>q</code> - Adds quick access buttons: " \
    "original, preview, information\n" \
    "Example: <code>1bnpaq</code>\n\n" \
    "<b>Inline mode commands</b>\n" \
    "<code>original</code> - Get original file of post by id\n" \
    "<code>post</code> - Get information about post by id\n" \
    "<code>book</code> - Get information about book by id\n" \
    "<code>short</code> - Create inline mode shortcut\n\n" \
    "<b>Commands</b>\n" \
    "/help - Helpful information about me\n" \
    "/original - Get original file of post by id\n" \
    "/post - Get information about post by id\n" \
    "/book - Get information about book by id\n" \
    "/tag - Get information about tag by tag id or name\n" \
    "/short - Create inline mode shortcut"

void bot_legacy_command(const char *message_text, json_object *update) {
    const char *username = config_get_string(BOT_USERNAME);

    if (!command_compare(message_text,
        "start") || !command_compare(message_text, "help"))
    {
        const char *custom_message = config_get_string(BOT_MESSAGE);
        char *msg_text = malloc(strlen(custom_message ? custom_message :
                                HELP_WELCOME) + strlen(HELP_MESSAGE) + 1);

        if(msg_text) {
            const char *chat_id = json_object_get_string(
                json_object_object_get(json_object_object_get(
                    json_object_object_get(update, "message"), "chat"), "id"));
            int64_t message_id = json_object_get_int64(
                json_object_object_get(json_object_object_get(
                                update, "message"), "message_id"));
            int64_t reply_id = json_object_get_int64(
                json_object_object_get(json_object_object_get(
                        json_object_object_get(update, "message"),
                                "reply_to_message"), "message_id"));

            json_object *info = json_object_new_object();
            json_object *button = json_object_new_object();
            json_object *button1 = json_object_new_object();
            json_object *button2 = json_object_new_object();
            json_object *inline_keyboard = json_object_new_object();
            json_object *inline_keyboard1 = json_object_new_array();
            json_object *inline_keyboard2 = json_object_new_array();

            sprintf(msg_text, HELP_MESSAGE, custom_message ?
                                custom_message : HELP_WELCOME);

            json_object_object_add(info, "chat_id",
                    json_object_new_string(chat_id));
            json_object_object_add(info, "text",
                json_object_new_string(msg_text));
            json_object_object_add(info, "parse_mode",
                        json_object_new_string("HTML"));
            json_object_object_add(info, "reply_to_message_id",
                json_object_new_int64(reply_id ? reply_id : message_id));
            json_object_object_add(button, "text",
                json_object_new_string("Search posts"));
            json_object_object_add(button,
                "switch_inline_query_current_chat", json_object_new_string(
                                                            "1a rating:safe"));
            json_object_array_add(inline_keyboard2, button);
            json_object_object_add(button1, "text",
                json_object_new_string("Search books"));
            json_object_object_add(button1,
                "switch_inline_query_current_chat", json_object_new_string(
                                                        "1ba rating:safe"));
            json_object_array_add(inline_keyboard2, button1);
            json_object_object_add(button2, "text",
                json_object_new_string("Search tags"));
            json_object_object_add(button2,
                "switch_inline_query_current_chat", json_object_new_string(
                                                                    "1ta"));
            json_object_array_add(inline_keyboard2, button2);
            json_object_array_add(inline_keyboard1, inline_keyboard2);
            json_object_object_add(inline_keyboard,
                "inline_keyboard", inline_keyboard1);
            json_object_object_add(info, "reply_markup", inline_keyboard);

            api_post("sendMessage", info);
            json_object_put(info);
            free(msg_text);
        }
    } else if(!command_compare(message_text, "original") || !command_compare(message_text, "post") || !command_compare(message_text, "book")) {
        const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(update, "message"), "chat"), "id"));
        int message_id = json_object_get_int(json_object_object_get(json_object_object_get(update, "message"), "message_id"));

        command_args *arguments = command_args_parse(message_text, 20, true);
        size_t args = command_args_count(arguments);

        if(!args) {
            char argument[16];

            json_object *reply_to_message = json_object_object_get(json_object_object_get(update, "message"), "reply_to_message");
            if(json_object_get_string(json_object_object_get(reply_to_message, "caption"))
              && json_object_get_boolean(json_object_object_get(json_object_object_get(reply_to_message, "via_bot"), "is_bot"))
              && !strcmp(json_object_get_string(json_object_object_get(json_object_object_get(reply_to_message, "via_bot"), "username")), username)
              && sscanf(json_object_get_string(json_object_object_get(reply_to_message, "caption")), "ID: %15s", argument) == 1) {
                command_args_free(arguments);

                arguments = command_args_parse(argument, 1, false);
                args = command_args_count(arguments);
            } else {
                json_object *error = json_object_new_object();

                json_object_object_add(error, "chat_id", json_object_new_string(chat_id));
                json_object_object_add(error, "text", json_object_new_string("<b>You must specify an ID or reply to the message with inline mode result</b>"));
                json_object_object_add(error, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(error, "reply_to_message_id", json_object_new_int(message_id));

                api_post("sendMessage", error);
                json_object_put(error);
            }
        }

        for(size_t arg = 0; arg < args; arg++) {
            json_object *info = json_object_new_object();
            const char *argument = command_args_get(arguments, arg);

            json_object_object_add(info, "chat_id", json_object_new_string(chat_id));

            json_object *csc_data;

            if(command_compare(message_text, "book"))
                csc_data = sankaku_request("posts/%s/", argument);
            else
                csc_data = sankaku_request("pools/%s/", argument);

            int csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));

            if(csc_id) {
                int reply_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(update, "message"), "reply_to_message"), "message_id"));

                if(!command_compare(message_text, "original")) {
                    json_object *button = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    const char *document = json_object_get_string(json_object_object_get(csc_data, "file_url"));
                    int csc_size = json_object_get_int(json_object_object_get(csc_data, "file_size"));
                    const char *csc_filetype = json_object_get_string(json_object_object_get(csc_data, "file_type"));

                    if(csc_filetype && csc_size <= 20971520) {
                        json_object_object_add(info, "document", json_object_new_string(document));
                        json_object_object_add(info, "reply_to_message_id", json_object_new_int(reply_id ? reply_id : message_id));
                    } else {
                        if(csc_size > 20971520)
                            json_object_object_add(info, "text", json_object_new_string("<b>File size must not exceed 20 MiB</b>"));
                        else
                            json_object_object_add(info, "text", json_object_new_string("<b>Unsupported media type</b>"));
                        json_object_object_add(info, "parse_mode", json_object_new_string("HTML"));
                        json_object_object_add(info, "reply_to_message_id", json_object_new_int(message_id));
                    }

                    json_object_object_add(button, "text", json_object_new_string("Download manually, link expires in 3 hours"));
                    json_object_object_add(button, "url", json_object_new_string(document));
                    json_object_array_add(inline_keyboard2, button);
                    json_object_array_add(inline_keyboard1, inline_keyboard2);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(info, "reply_markup", inline_keyboard);
                } else if(!command_compare(message_text, "post")) {
                    char *csc_info = sankaku_post(csc_data);

                    char csc_button[128];
                    snprintf(csc_button, sizeof(csc_button), "%s/%d", SANKAKU_POST_URL, csc_id);

                    char callback_data[32];
                    snprintf(callback_data, sizeof(callback_data), "1_%d", csc_id);

                    char csc_button_text[16];
                    snprintf(csc_button_text, sizeof(csc_button_text), "Tags (%zu)", json_object_array_length(json_object_object_get(csc_data, "tags")));

                    json_object *button = json_object_new_object();
                    json_object *button1 = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    json_object_object_add(info, "text", json_object_new_string(csc_info));
                    json_object_object_add(info, "parse_mode", json_object_new_string("HTML"));
                    json_object_object_add(info, "reply_to_message_id", json_object_new_int(reply_id ? reply_id : message_id));
                    json_object_object_add(button, "text", json_object_new_string("Post link"));
                    json_object_object_add(button, "url", json_object_new_string(csc_button));
                    json_object_array_add(inline_keyboard2, button);
                    json_object_object_add(button1, "text", json_object_new_string(csc_button_text));
                    json_object_object_add(button1, "callback_data", json_object_new_string(callback_data));
                    json_object_array_add(inline_keyboard2, button1);
                    json_object_array_add(inline_keyboard1, inline_keyboard2);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(info, "reply_markup", inline_keyboard);

                    free(csc_info);
                } else if(!command_compare(message_text, "book")) {
                    char *csc_info = sankaku_book(csc_data);

                    char csc_button[128];
                    snprintf(csc_button, sizeof(csc_button), "%s/%d", SANKAKU_POOL_URL, csc_id);

                    char callback_data[32];
                    snprintf(callback_data, sizeof(callback_data), "5_%d_0_1_%d", csc_id, json_object_get_int(json_object_object_get(csc_data, "visible_post_count")) - 1);

                    char callback_data1[32];
                    snprintf(callback_data1, sizeof(callback_data1), "2_%d", csc_id);

                    char csc_button_text[16];
                    snprintf(csc_button_text, sizeof(csc_button_text), "Tags (%zu)", json_object_array_length(json_object_object_get(csc_data, "tags")));

                    json_object *button = json_object_new_object();
                    json_object *button1 = json_object_new_object();
                    json_object *button2 = json_object_new_object(); 
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    json_object_object_add(info, "text", json_object_new_string(csc_info));
                    json_object_object_add(info, "parse_mode", json_object_new_string("HTML"));
                    json_object_object_add(info, "reply_to_message_id", json_object_new_int(reply_id ? reply_id : message_id));
                    json_object_object_add(button, "text", json_object_new_string("Book link"));
                    json_object_object_add(button, "url", json_object_new_string(csc_button));
                    json_object_array_add(inline_keyboard2, button);
                    json_object_object_add(button1, "text", json_object_new_string("Read"));
                    json_object_object_add(button1, "callback_data", json_object_new_string(callback_data));
                    json_object_array_add(inline_keyboard2, button1);
                    json_object_object_add(button2, "text", json_object_new_string(csc_button_text));
                    json_object_object_add(button2, "callback_data", json_object_new_string(callback_data1));
                    json_object_array_add(inline_keyboard2, button2);
                    json_object_array_add(inline_keyboard1, inline_keyboard2);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(info, "reply_markup", inline_keyboard);

                    free(csc_info);
                }
            } else {
                const char *error_code = json_object_get_string(json_object_object_get(csc_data, "code"));

                char error_description[256];

                if(error_code && !strcmp(error_code, "snackbar__server-error_not-found")) {
                    if(command_compare(message_text, "book"))
                        snprintf(error_description, sizeof(error_description), "<b>Wrong post ID:</b> <code>%s</code>", argument);
                    else
                        snprintf(error_description, sizeof(error_description), "<b>Wrong book ID:</b> <code>%s</code>", argument);
                } else if(error_code) {
                    snprintf(error_description, sizeof(error_description), "<b>Error:</b> %s", error_code);
                } else {
                    snprintf(error_description, sizeof(error_description), "<b>Unknown error</b>");
                }

                json_object_object_add(info, "text", json_object_new_string(error_description));
                json_object_object_add(info, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(info, "reply_to_message_id", json_object_new_int(message_id));
            }
            json_object_put(csc_data);

            const char *document = json_object_get_string(json_object_object_get(info, "document"));

            if(document) {
                if(api_post("sendDocument", info)) {
                    json_object *error = json_object_new_object();
                    json_object *button = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    json_object_object_add(error, "chat_id", json_object_new_string(chat_id));
                    json_object_object_add(error, "text", json_object_new_string("<b>Failed to access file</b>"));
                    json_object_object_add(error, "parse_mode", json_object_new_string("HTML"));
                    json_object_object_add(error, "reply_to_message_id", json_object_new_int(message_id));
                    json_object_object_add(button, "text", json_object_new_string("Download manually, link expires in 3 hours"));
                    json_object_object_add(button, "url", json_object_new_string(document));
                    json_object_array_add(inline_keyboard2, button);
                    json_object_array_add(inline_keyboard1, inline_keyboard2);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(error, "reply_markup", inline_keyboard);

                    api_post("sendMessage", error);
                    json_object_put(error);
                }
            } else {
                api_post("sendMessage", info);
            }

            json_object_put(info);
        }

        command_args_free(arguments);
    } else if(!command_compare(message_text, "tag")) {
        const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(update, "message"), "chat"), "id"));
        int message_id = json_object_get_int(json_object_object_get(json_object_object_get(update, "message"), "message_id"));

        command_args *arguments = command_args_parse(message_text, 20, true);
        size_t args = command_args_count(arguments);

        if(!args) {
            json_object *error = json_object_new_object();

            json_object_object_add(error, "chat_id", json_object_new_string(chat_id));
            json_object_object_add(error, "text", json_object_new_string("<b>You must specify a tag ID or name</b>"));
            json_object_object_add(error, "parse_mode", json_object_new_string("HTML"));
            json_object_object_add(error, "reply_to_message_id", json_object_new_int(message_id));

            api_post("sendMessage", error);
            json_object_put(error);
        }

        for(size_t arg = 0; arg < args; arg++) {
            json_object *tag = json_object_new_object();
            const char *argument = command_args_get(arguments, arg);

            json_object_object_add(tag, "chat_id", json_object_new_string(chat_id));

            CURL *encode_argument = curl_easy_init();
            char *encoded_argument = curl_easy_escape(encode_argument, argument, 0);

            json_object *csc_data = sankaku_request("tags/%s/", encoded_argument);

            curl_free(encoded_argument);
            curl_easy_cleanup(encode_argument);

            int csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));

            if(csc_id) {
                const char *csc_name = json_object_get_string(json_object_object_get(csc_data, "name"));

                char *csc_tag = sankaku_tag(csc_data);

                char csc_button[1024];
                snprintf(csc_button, sizeof(csc_button), "1a %s", csc_name);

                char csc_button1[1024];
                snprintf(csc_button1, sizeof(csc_button1), "1ba %s", csc_name);

                json_object *button = json_object_new_object();
                json_object *button1 = json_object_new_object();
                json_object *inline_keyboard = json_object_new_object();
                json_object *inline_keyboard1 = json_object_new_array();
                json_object *inline_keyboard2 = json_object_new_array();

                int reply_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(update, "message"), "reply_to_message"), "message_id"));

                json_object_object_add(tag, "text", json_object_new_string(csc_tag));
                json_object_object_add(tag, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(tag, "reply_to_message_id", json_object_new_int(reply_id ? reply_id : message_id));
                if(json_object_get_int(json_object_object_get(csc_data, "post_count"))) {
                    json_object_object_add(button, "text", json_object_new_string("Search posts"));
                    json_object_object_add(button, "switch_inline_query_current_chat", json_object_new_string(csc_button));
                    json_object_array_add(inline_keyboard2, button);
                }
                if(json_object_get_int(json_object_object_get(csc_data, "pool_count"))) {
                    json_object_object_add(button1, "text", json_object_new_string("Search books"));
                    json_object_object_add(button1, "switch_inline_query_current_chat", json_object_new_string(csc_button1));
                    json_object_array_add(inline_keyboard2, button1);
                }
                json_object_array_add(inline_keyboard1, inline_keyboard2);
                json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                json_object_object_add(tag, "reply_markup", inline_keyboard);

                free(csc_tag);
            } else {
                const char *error_code = json_object_get_string(json_object_object_get(csc_data, "code"));

                char error_description[1024];

                if(error_code && !strcmp(error_code, "snackbar__server-error_not-found"))
                    snprintf(error_description, sizeof(error_description), "<b>Wrong tag ID or name:</b> <code>%s</code>", argument);
                else if(error_code)
                    snprintf(error_description, sizeof(error_description), "<b>Error:</b> %s", error_code);
                else
                    snprintf(error_description, sizeof(error_description), "<b>Unknown error</b>");

                json_object_object_add(tag, "text", json_object_new_string(error_description));
                json_object_object_add(tag, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(tag, "reply_to_message_id", json_object_new_int(message_id));
            }
            json_object_put(csc_data);

            api_post("sendMessage", tag);
            json_object_put(tag);
        }

        command_args_free(arguments);
    } else if(!command_compare(message_text, "short")) {
        const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(update, "message"), "chat"), "id"));
        int message_id = json_object_get_int(json_object_object_get(json_object_object_get(update, "message"), "message_id"));

        command_args *arguments = command_args_parse(message_text, 20, true);
        size_t args = command_args_count(arguments);

        json_object *shortcut = json_object_new_object();

        if(args) {
            char short_query[1024] = {0};

            for(size_t arg = 0; arg < args; arg++) {
                const char *argument = command_args_get(arguments, arg);

                string_cat(short_query, argument, sizeof(short_query) - strlen(short_query));
                if(arg < args - 1)
                    string_cat(short_query, " ", sizeof(short_query) - strlen(short_query));
            }

            char message[2048];
            snprintf(message, sizeof(message), "<b>Shortcut:</b> <code>%s</code>", short_query);

            json_object *button = json_object_new_object();
            json_object *inline_keyboard = json_object_new_object();
            json_object *inline_keyboard1 = json_object_new_array();
            json_object *inline_keyboard2 = json_object_new_array();

            int reply_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(update, "message"), "reply_to_message"), "message_id"));

            json_object_object_add(shortcut, "chat_id", json_object_new_string(chat_id));
            json_object_object_add(shortcut, "text", json_object_new_string(message));
            json_object_object_add(shortcut, "parse_mode", json_object_new_string("HTML"));
            json_object_object_add(shortcut, "reply_to_message_id", json_object_new_int(reply_id ? reply_id : message_id));
            json_object_object_add(button, "text", json_object_new_string("Open inline mode"));
            json_object_object_add(button, "switch_inline_query_current_chat", json_object_new_string(short_query));
            json_object_array_add(inline_keyboard2, button);
            json_object_array_add(inline_keyboard1, inline_keyboard2);
            json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
            json_object_object_add(shortcut, "reply_markup", inline_keyboard);
        } else {
            json_object_object_add(shortcut, "chat_id", json_object_new_string(chat_id));
            json_object_object_add(shortcut, "text", json_object_new_string("<b>You must specify a query</b>"));
            json_object_object_add(shortcut, "parse_mode", json_object_new_string("HTML"));
            json_object_object_add(shortcut, "reply_to_message_id", json_object_new_int(message_id));
        }

        api_post("sendMessage", shortcut);
        json_object_put(shortcut);
        command_args_free(arguments);
    }
}
