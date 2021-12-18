#include "core.h"
#include "csc_core.h"

void bot_commands_private(struct bot_update *result) {
    if(!bot_command_parse(result->message_text, "cscauth")) {
        const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "chat"), "id"));
        int message_id = json_object_get_int(json_object_object_get(json_object_object_get(result->update, "message"), "message_id"));
        int reply_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "message"), "reply_to_message"), "message_id"));
        if(!reply_id)
            reply_id = message_id;

        json_object *cscauth = json_object_new_object();

        json_object_object_add(cscauth, "chat_id", json_object_new_string(chat_id));
        if(!csc_auth())
            json_object_object_add(cscauth, "text", json_object_new_string("<b>Authorized successfully</b>"));
        else
            json_object_object_add(cscauth, "text", json_object_new_string("<b>Error occurred</b>"));
        json_object_object_add(cscauth, "parse_mode", json_object_new_string("HTML"));
        json_object_object_add(cscauth, "reply_to_message_id", json_object_new_int(reply_id));

        bot_post("sendMessage", &cscauth);
        json_object_put(cscauth);
    }
}
