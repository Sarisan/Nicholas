#include <core.h>
#include <csc_core.h>
#include <curl/curl.h>
#include <string.h>
#include <string_ext.h>

void bot_commands(struct bot_update *result) {
    if(!bot_command_parse(result->message_text, "start") || !bot_command_parse(result->message_text, "help")) {
        json_object *info = json_object_new_object();
        json_object *button = json_object_new_object();
        json_object *button1 = json_object_new_object();
        json_object *button2 = json_object_new_object();
        json_object *inline_keyboard = json_object_new_object();
        json_object *inline_keyboard1 = json_object_new_array();
        json_object *inline_keyboard2 = json_object_new_array();

        const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "chat"), "id"));
        int reply_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "reply_to_message"), "message_id"));
        if(!reply_id)
            reply_id = json_object_get_int(json_object_object_get(json_object_object_get(result->update, "message"), "message_id"));

        json_object_object_add(info, "chat_id", json_object_new_string(chat_id));
        json_object_object_add(info, "text", json_object_new_string("I'm Nicholas, the first Fletcher-class ship to be launched... Here I can help you to search on Sankaku Channel via inline mode.\n\n<b>Search arguments</b>\nThe first argument is the page number, can be from 1 to 100.\n<code>b</code> - Switches search to book search\n<code>n</code> - Sub-argument of 'b', switches search by tags to search by name\n<code>t</code> - Switches search to tag search\n<code>p</code> - Replaces all animated content with its preview\n<code>a</code> - Enables auto-paging mode\n<code>q</code> - Adds quick access buttons: original, preview, information\nExample: <code>1bnpaq</code>\n\n<b>Commands</b>\n/help - Helpful information about me\n/original - Get original file of post by id\n/post - Get information about post by id\n/book - Get information about book by id\n/tag - Get information about tag by tag id or name\n/short - Create inline mode shortcut"));
        json_object_object_add(info, "parse_mode", json_object_new_string("HTML"));
        json_object_object_add(info, "reply_to_message_id", json_object_new_int(reply_id));
        json_object_object_add(button, "text", json_object_new_string("Search posts"));
        json_object_object_add(button, "switch_inline_query_current_chat", json_object_new_string("1a rating:safe"));
        json_object_array_add(inline_keyboard2, button);
        json_object_object_add(button1, "text", json_object_new_string("Search books"));
        json_object_object_add(button1, "switch_inline_query_current_chat", json_object_new_string("1ba rating:safe"));
        json_object_array_add(inline_keyboard2, button1);
        json_object_object_add(button2, "text", json_object_new_string("Search tags"));
        json_object_object_add(button2, "switch_inline_query_current_chat", json_object_new_string("1ta"));
        json_object_array_add(inline_keyboard2, button2);
        json_object_array_add(inline_keyboard1, inline_keyboard2);
        json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
        json_object_object_add(info, "reply_markup", inline_keyboard);

        bot_post("sendMessage", info);
        json_object_put(info);
    } else if(!bot_command_parse(result->message_text, "original") || !bot_command_parse(result->message_text, "post") || !bot_command_parse(result->message_text, "book")) {
        const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "chat"), "id"));
        int message_id = json_object_get_int(json_object_object_get(json_object_object_get(result->update, "message"), "message_id"));

        char arguments[20][16];
        int args = bot_command_getarg(result->message_text, 20, 16, arguments);

        if(!args) {
            json_object *reply_to_message = json_object_object_get(json_object_object_get(result->update, "message"), "reply_to_message");
            if(json_object_get_string(json_object_object_get(reply_to_message, "caption"))
              && json_object_get_boolean(json_object_object_get(json_object_object_get(reply_to_message, "via_bot"), "is_bot"))
              && !strcmp(json_object_get_string(json_object_object_get(json_object_object_get(reply_to_message, "via_bot"), "username")), bot_username)
              && sscanf(json_object_get_string(json_object_object_get(reply_to_message, "caption")), "ID: %15s", arguments[0]) == 1) {
                args = 1;
            } else {
                json_object *error = json_object_new_object();

                json_object_object_add(error, "chat_id", json_object_new_string(chat_id));
                json_object_object_add(error, "text", json_object_new_string("<b>You must specify an ID or reply to the message with inline mode result</b>"));
                json_object_object_add(error, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(error, "reply_to_message_id", json_object_new_int(message_id));

                bot_post("sendMessage", error);
                json_object_put(error);
            }
        }

        for(short arg = 0; arg < args; arg++) {
            json_object *info = json_object_new_object();

            json_object_object_add(info, "chat_id", json_object_new_string(chat_id));

            json_object *csc_data;

            if(bot_command_parse(result->message_text, "book"))
                csc_data = csc_request(0, "posts/%s/", arguments[arg]);
            else
                csc_data = csc_request(0, "pools/%s/", arguments[arg]);

            int csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));

            if(csc_id) {
                int reply_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "reply_to_message"), "message_id"));
                if(!reply_id)
                    reply_id = message_id;

                if(!bot_command_parse(result->message_text, "original")) {
                    json_object *button = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    const char *document = json_object_get_string(json_object_object_get(csc_data, "file_url"));
                    int csc_size = json_object_get_int(json_object_object_get(csc_data, "file_size"));
                    const char *csc_filetype = json_object_get_string(json_object_object_get(csc_data, "file_type"));

                    if(csc_filetype && strcmp(csc_filetype, "video/webm") && csc_size <= 20971520) {
                        json_object_object_add(info, "document", json_object_new_string(document));
                        json_object_object_add(info, "reply_to_message_id", json_object_new_int(reply_id));
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
                } else if(!bot_command_parse(result->message_text, "post")) {
                    char csc_info[4096];
                    bot_csc_post(csc_info, sizeof(csc_info), csc_data, csc_id);

                    char csc_button[128];
                    snprintf(csc_button, sizeof(csc_button), "%s/%d", CSC_POST_URL, csc_id);

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
                    json_object_object_add(info, "reply_to_message_id", json_object_new_int(reply_id));
                    json_object_object_add(button, "text", json_object_new_string("Post link"));
                    json_object_object_add(button, "url", json_object_new_string(csc_button));
                    json_object_array_add(inline_keyboard2, button);
                    json_object_object_add(button1, "text", json_object_new_string(csc_button_text));
                    json_object_object_add(button1, "callback_data", json_object_new_string(callback_data));
                    json_object_array_add(inline_keyboard2, button1);
                    json_object_array_add(inline_keyboard1, inline_keyboard2);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(info, "reply_markup", inline_keyboard);
                } else if(!bot_command_parse(result->message_text, "book")) {
                    char csc_info[20480];
                    bot_csc_pool(csc_info, sizeof(csc_info), csc_data, csc_id);

                    char csc_button[128];
                    snprintf(csc_button, sizeof(csc_button), "%s/%d", CSC_POOL_URL, csc_id);

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
                    json_object_object_add(info, "reply_to_message_id", json_object_new_int(reply_id));
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
                }
            } else {
                const char *error_code = json_object_get_string(json_object_object_get(csc_data, "code"));

                char error_description[256];

                if(error_code && !strcmp(error_code, "snackbar__server-error_not-found")) {
                    if(bot_command_parse(result->message_text, "book"))
                        snprintf(error_description, sizeof(error_description), "<b>Wrong post ID:</b> <code>%s</code>", arguments[arg]);
                    else
                        snprintf(error_description, sizeof(error_description), "<b>Wrong book ID:</b> <code>%s</code>", arguments[arg]);
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
                if(bot_post("sendDocument", info)) {
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

                    bot_post("sendMessage", error);
                    json_object_put(error);
                }
            } else {
                bot_post("sendMessage", info);
            }

            json_object_put(info);
        }
    } else if(!bot_command_parse(result->message_text, "tag")) {
        const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "chat"), "id"));
        int message_id = json_object_get_int(json_object_object_get(json_object_object_get(result->update, "message"), "message_id"));

        char arguments[20][1024];
        int args = bot_command_getarg(result->message_text, 20, 1024, arguments);

        if(!args) {
            json_object *error = json_object_new_object();

            json_object_object_add(error, "chat_id", json_object_new_string(chat_id));
            json_object_object_add(error, "text", json_object_new_string("<b>You must specify a tag ID or name</b>"));
            json_object_object_add(error, "parse_mode", json_object_new_string("HTML"));
            json_object_object_add(error, "reply_to_message_id", json_object_new_int(message_id));

            bot_post("sendMessage", error);
            json_object_put(error);
        }

        for(short arg = 0; arg < args; arg++) {
            json_object *tag = json_object_new_object();

            json_object_object_add(tag, "chat_id", json_object_new_string(chat_id));

            CURL *encode_argument = curl_easy_init();
            char *encoded_argument = curl_easy_escape(encode_argument, arguments[arg], 0);

            json_object *csc_data = csc_request(0, "tags/%s/", encoded_argument);

            curl_free(encoded_argument);
            curl_easy_cleanup(encode_argument);

            int csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));

            if(csc_id) {
                const char *csc_name = json_object_get_string(json_object_object_get(csc_data, "name"));

                char csc_tag[20480];
                bot_csc_tag(csc_tag, sizeof(csc_tag), csc_data, csc_id);

                char csc_button[1024];
                snprintf(csc_button, sizeof(csc_button), "1a %s", csc_name);

                char csc_button1[1024];
                snprintf(csc_button1, sizeof(csc_button1), "1ba %s", csc_name);

                json_object *button = json_object_new_object();
                json_object *button1 = json_object_new_object();
                json_object *inline_keyboard = json_object_new_object();
                json_object *inline_keyboard1 = json_object_new_array();
                json_object *inline_keyboard2 = json_object_new_array();

                int reply_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "reply_to_message"), "message_id"));
                if(!reply_id)
                    reply_id = message_id;

                json_object_object_add(tag, "text", json_object_new_string(csc_tag));
                json_object_object_add(tag, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(tag, "reply_to_message_id", json_object_new_int(reply_id));
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
            } else {
                const char *error_code = json_object_get_string(json_object_object_get(csc_data, "code"));

                char error_description[1024];

                if(error_code && !strcmp(error_code, "snackbar__server-error_not-found"))
                    snprintf(error_description, sizeof(error_description), "<b>Wrong tag ID or name:</b> <code>%s</code>", arguments[arg]);
                else if(error_code)
                    snprintf(error_description, sizeof(error_description), "<b>Error:</b> %s", error_code);
                else
                    snprintf(error_description, sizeof(error_description), "<b>Unknown error</b>");

                json_object_object_add(tag, "text", json_object_new_string(error_description));
                json_object_object_add(tag, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(tag, "reply_to_message_id", json_object_new_int(message_id));
            }
            json_object_put(csc_data);

            bot_post("sendMessage", tag);
            json_object_put(tag);
        }
    } else if(!bot_command_parse(result->message_text, "short")) {
        const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "chat"), "id"));
        int message_id = json_object_get_int(json_object_object_get(json_object_object_get(result->update, "message"), "message_id"));

        char arguments[20][1024];
        int args = bot_command_getarg(result->message_text, 20, 1024, arguments);

        json_object *shortcut = json_object_new_object();

        if(args) {
            char short_query[1024] = "";

            for(int arg = 0; arg < args; arg++) {
                strntcat(short_query, arguments[arg], sizeof(short_query) - strlen(short_query));
                if(arg < args - 1)
                    strntcat(short_query, " ", sizeof(short_query) - strlen(short_query));
            }

            char message[2048];
            snprintf(message, sizeof(message), "<b>Shortcut:</b> <code>%s</code>", short_query);

            json_object *button = json_object_new_object();
            json_object *inline_keyboard = json_object_new_object();
            json_object *inline_keyboard1 = json_object_new_array();
            json_object *inline_keyboard2 = json_object_new_array();

            int reply_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "reply_to_message"), "message_id"));
            if(!reply_id)
                reply_id = message_id;

            json_object_object_add(shortcut, "chat_id", json_object_new_string(chat_id));
            json_object_object_add(shortcut, "text", json_object_new_string(message));
            json_object_object_add(shortcut, "parse_mode", json_object_new_string("HTML"));
            json_object_object_add(shortcut, "reply_to_message_id", json_object_new_int(reply_id));
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

        bot_post("sendMessage", shortcut);
        json_object_put(shortcut);
    }
}
