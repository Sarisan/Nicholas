#include <api/api.h>
#include <config/config.h>
#include <sankaku/api.h>
#include <parsers/parsers.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

void bot_legacy_callback(const char *callback_data, json_object *update) {
    short action = 0;
    int id = 0, offset = 0, data = 0, data1 = 0, csc_id = 0;

    sscanf(callback_data, "%hd_%d_%d_%d_%d", &action, &id, &offset, &data, &data1);

    json_object *csc_answer = json_object_new_object();
    const char *callback_query_id = json_object_get_string(json_object_object_get(json_object_object_get(update, "callback_query"), "id"));

    json_object_object_add(csc_answer, "callback_query_id", json_object_new_string(callback_query_id));

    if(action) {
        json_object *csc_data = 0;

        if(action == 1 || action == 3)
            csc_data = sankaku_request("posts/%d/", id);
        else if(action == 2 || action == 4)
            csc_data = sankaku_request("pools/%d/", id);
        else if(action == 5)
            csc_data = sankaku_request("posts?page=%d&tags=pool:%d", data, id);

        if(action != 5) {
            csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));
        } else {
            if(json_object_get_type(csc_data) == json_type_array)
                csc_id = json_object_get_int(json_object_object_get(json_object_array_get_idx(csc_data, data > 1 ? offset - 20 * (data - 1) : offset), "id"));
        }

        if(csc_id) {
            if(action == 1 || action == 2) {
                json_object *csc_tags_array = json_object_object_get(csc_data, "tags");
                short tags_array = offset;
                char csc_copyright[20480]= {0}, csc_studio[20480]= {0}, csc_character[20480]= {0}, csc_artist[20480]= {0}, csc_medium[20480]= {0}, csc_general[20480]= {0}, csc_meta[20480]= {0}, csc_genre[20480]= {0}, csc_unknown[20480]= {0};

                for(short tags_count = 0; json_object_array_get_idx(csc_tags_array, tags_array) && tags_count < 60; tags_count++, tags_array++) {
                    char *tag_encoded = string_escape(json_object_get_string(json_object_object_get(json_object_array_get_idx(csc_tags_array, tags_array), "name")), 64);

                    char tag[512];
                    snprintf(tag, sizeof(tag), "<code>%s</code> ", tag_encoded);
                    string_free(1, tag_encoded);

                    switch(json_object_get_int(json_object_object_get(json_object_array_get_idx(csc_tags_array, tags_array), "type"))) {
                        case 3:
                            string_cat(csc_copyright, tag, sizeof(csc_copyright) - strlen(csc_copyright));
                            break;
                        case 2:
                            string_cat(csc_studio, tag, sizeof(csc_studio) - strlen(csc_studio));
                            break;
                        case 4:
                            string_cat(csc_character, tag, sizeof(csc_character) - strlen(csc_character));
                            break;
                        case 1:
                            string_cat(csc_artist, tag, sizeof(csc_artist) - strlen(csc_artist));
                            break;
                        case 8:
                            string_cat(csc_medium, tag, sizeof(csc_medium) - strlen(csc_medium));
                            break;
                        case 0:
                            string_cat(csc_general, tag, sizeof(csc_general) - strlen(csc_general));
                            break;
                        case 9:
                            string_cat(csc_meta, tag, sizeof(csc_meta) - strlen(csc_meta));
                            break;
                        case 5:
                            string_cat(csc_genre, tag, sizeof(csc_genre) - strlen(csc_genre));
                            break;
                        default:
                            string_cat(csc_unknown, tag, sizeof(csc_unknown) - strlen(csc_unknown));
                            break;
                    }
                }

                char csc_copyright_s[20480]= {0};
                if(csc_copyright[0])
                    snprintf(csc_copyright_s, sizeof(csc_copyright_s), "<b>Copyright:</b> %s\n", csc_copyright);

                char csc_studio_s[20480]= {0};
                if(csc_studio[0])
                    snprintf(csc_studio_s, sizeof(csc_studio_s), "<b>Studio:</b> %s\n", csc_studio);

                char csc_character_s[20480]= {0};
                if(csc_character[0])
                    snprintf(csc_character_s, sizeof(csc_character_s), "<b>Character:</b> %s\n", csc_character);

                char csc_artist_s[20480]= {0};
                if(csc_artist[0])
                    snprintf(csc_artist_s, sizeof(csc_artist_s), "<b>Artist:</b> %s\n", csc_artist);

                char csc_medium_s[20480]= {0};
                if(csc_medium[0])
                    snprintf(csc_medium_s, sizeof(csc_medium_s), "<b>Medium:</b> %s\n", csc_medium);

                char csc_general_s[20480]= {0};
                if(csc_general[0])
                    snprintf(csc_general_s, sizeof(csc_general_s), "<b>General:</b> %s\n", csc_general);

                char csc_meta_s[20480]= {0};
                if(csc_meta[0])
                    snprintf(csc_meta_s, sizeof(csc_meta_s), "<b>Meta:</b> %s\n", csc_meta);

                char csc_genre_s[20480]= {0};
                if(csc_genre[0])
                    snprintf(csc_genre_s, sizeof(csc_genre_s), "<b>Genre:</b> %s\n", csc_genre);

                char csc_unknown_s[20480]= {0};
                if(csc_unknown[0])
                    snprintf(csc_unknown_s, sizeof(csc_unknown_s), "<b>Unknown:</b> %s", csc_unknown);

                char csc_info[20480];
                snprintf(csc_info, sizeof(csc_info), "%s%s%s%s%s%s%s%s%s", csc_copyright_s, csc_studio_s, csc_character_s, csc_artist_s, csc_medium_s, csc_general_s, csc_meta_s, csc_genre_s, csc_unknown_s);

                if(csc_info[0]) {
                    char callback_data[32];
                    if(action == 1)
                        snprintf(callback_data, sizeof(callback_data), "3_%d", id);
                    else if(action == 2)
                        snprintf(callback_data, sizeof(callback_data), "4_%d", id);

                    char callback_data1[64];
                    snprintf(callback_data1, sizeof(callback_data1), "%d_%d_%d", action, id, offset - 60);

                    char callback_data2[64];
                    snprintf(callback_data2, sizeof(callback_data2), "%d_%d_%d", action, id, tags_array);

                    json_object *info = json_object_new_object();
                    json_object *button = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(json_object_object_get(update, "callback_query"), "message"), "chat"), "id"));
                    int message_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(update, "callback_query"), "message"), "message_id"));
                    const char *inline_message_id = json_object_get_string(json_object_object_get(json_object_object_get(update, "callback_query"), "inline_message_id"));

                    if(chat_id && message_id) {
                        json_object_object_add(info, "chat_id", json_object_new_string(chat_id));
                        json_object_object_add(info, "message_id", json_object_new_int(message_id));
                    } else if(inline_message_id) {
                        json_object_object_add(info, "inline_message_id", json_object_new_string(inline_message_id));
                    }
                    json_object_object_add(info, "text", json_object_new_string(csc_info));
                    json_object_object_add(info, "parse_mode", json_object_new_string("HTML"));
                    json_object_object_add(button, "text", json_object_new_string("Back"));
                    json_object_object_add(button, "callback_data", json_object_new_string(callback_data));
                    json_object_array_add(inline_keyboard2, button);
                    if(offset >= 60) {
                        json_object *button1 = json_object_new_object();

                        json_object_object_add(button1, "text", json_object_new_string("Previous"));
                        json_object_object_add(button1, "callback_data", json_object_new_string(callback_data1));
                        json_object_array_add(inline_keyboard2, button1);
                    }
                    if(json_object_array_length(csc_tags_array) - tags_array) {
                        json_object *button2 = json_object_new_object();

                        json_object_object_add(button2, "text", json_object_new_string("Next"));
                        json_object_object_add(button2, "callback_data", json_object_new_string(callback_data2));
                        json_object_array_add(inline_keyboard2, button2);
                    }
                    json_object_array_add(inline_keyboard1, inline_keyboard2);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(info, "reply_markup", inline_keyboard);

                    api_post("editMessageText", info);
                    json_object_put(info);
                } else {
                    json_object_object_add(csc_answer, "text", json_object_new_string("No tags found"));
                }
            } else if(action == 3) {
                char csc_info[4096];
                sankaku_get_post(csc_info, sizeof(csc_info), csc_data, csc_id);

                char csc_button[128];
                snprintf(csc_button, sizeof(csc_button), "%s/%d", SANKAKU_POST_URL, csc_id);

                char callback_data[32];
                snprintf(callback_data, sizeof(callback_data), "1_%d", id);

                char csc_button_text[16];
                snprintf(csc_button_text, sizeof(csc_button_text), "Tags (%zu)", json_object_array_length(json_object_object_get(csc_data, "tags")));

                json_object *info = json_object_new_object();
                json_object *button = json_object_new_object();
                json_object *button1 = json_object_new_object();
                json_object *inline_keyboard = json_object_new_object();
                json_object *inline_keyboard1 = json_object_new_array();
                json_object *inline_keyboard2 = json_object_new_array();

                const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(json_object_object_get(update, "callback_query"), "message"), "chat"), "id"));
                int message_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(update, "callback_query"), "message"), "message_id"));
                const char *inline_message_id = json_object_get_string(json_object_object_get(json_object_object_get(update, "callback_query"), "inline_message_id"));

                if(chat_id && message_id) {
                    json_object_object_add(info, "chat_id", json_object_new_string(chat_id));
                    json_object_object_add(info, "message_id", json_object_new_int(message_id));
                } else if(inline_message_id) {
                    json_object_object_add(info, "inline_message_id", json_object_new_string(inline_message_id));
                }
                json_object_object_add(info, "text", json_object_new_string(csc_info));
                json_object_object_add(info, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(button, "text", json_object_new_string("Post link"));
                json_object_object_add(button, "url", json_object_new_string(csc_button));
                json_object_array_add(inline_keyboard2, button);
                json_object_object_add(button1, "text", json_object_new_string(csc_button_text));
                json_object_object_add(button1, "callback_data", json_object_new_string(callback_data));
                json_object_array_add(inline_keyboard2, button1);
                json_object_array_add(inline_keyboard1, inline_keyboard2);
                json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                json_object_object_add(info, "reply_markup", inline_keyboard);

                api_post("editMessageText", info);
                json_object_put(info);
            } else if(action == 4) {
                char csc_info[20480];
                sankaku_get_pool(csc_info, sizeof(csc_info), csc_data, csc_id);

                char csc_button[128];
                snprintf(csc_button, sizeof(csc_button), "%s/%d", SANKAKU_POOL_URL, csc_id);

                char callback_data[32];
                snprintf(callback_data, sizeof(callback_data), "5_%d_0_1_%d", id, json_object_get_int(json_object_object_get(csc_data, "visible_post_count")) - 1);

                char callback_data1[32];
                snprintf(callback_data1, sizeof(callback_data1), "2_%d", id);

                char csc_button_text[16];
                snprintf(csc_button_text, sizeof(csc_button_text), "Tags (%zu)", json_object_array_length(json_object_object_get(csc_data, "tags")));

                json_object *info = json_object_new_object();
                json_object *button = json_object_new_object();
                json_object *button1 = json_object_new_object();
                json_object *button2 = json_object_new_object();
                json_object *inline_keyboard = json_object_new_object();
                json_object *inline_keyboard1 = json_object_new_array();
                json_object *inline_keyboard2 = json_object_new_array();

                const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(json_object_object_get(update, "callback_query"), "message"), "chat"), "id"));
                int message_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(update, "callback_query"), "message"), "message_id"));
                const char *inline_message_id = json_object_get_string(json_object_object_get(json_object_object_get(update, "callback_query"), "inline_message_id"));

                if(chat_id && message_id) {
                    json_object_object_add(info, "chat_id", json_object_new_string(chat_id));
                    json_object_object_add(info, "message_id", json_object_new_int(message_id));
                } else if(inline_message_id) {
                    json_object_object_add(info, "inline_message_id", json_object_new_string(inline_message_id));
                }
                json_object_object_add(info, "text", json_object_new_string(csc_info));
                json_object_object_add(info, "parse_mode", json_object_new_string("HTML"));
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

                api_post("editMessageText", info);
                json_object_put(info);
            } else if(action == 5) {
                short pages_array = data > 1 ? offset - 20 * (data - 1) : offset;
                json_object *csc_page = json_object_array_get_idx(csc_data, pages_array);

                const char *csc_sample_url = json_object_get_string(json_object_object_get(csc_page, "sample_url"));
                const char *csc_preview_url = json_object_get_string(json_object_object_get(csc_page, "preview_url"));
                const char *csc_file_url = json_object_get_string(json_object_object_get(csc_page, "file_url"));
                float csc_size = json_object_get_int(json_object_object_get(csc_page, "file_size"));
                const char *csc_filetype = json_object_get_string(json_object_object_get(csc_page, "file_type"));

                char csc_image_url[512];

                if(csc_filetype) {
                    if(!strcmp(csc_filetype, "image/jpeg")) {
                        string_copy(csc_image_url, csc_sample_url ? csc_sample_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
                    } else if(!strcmp(csc_filetype, "image/png")) {
                        string_copy(csc_image_url, csc_sample_url ? csc_sample_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
                    } else if(!strcmp(csc_filetype, "image/gif")) {
                        if(csc_size <= 20971520)
                            string_copy(csc_image_url, csc_file_url ? csc_file_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
                        else
                            string_copy(csc_image_url, csc_preview_url ? csc_preview_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
                    } else if(!strcmp(csc_filetype, "video/mp4")) {
                        if(csc_size <= 20971520)
                            string_copy(csc_image_url, csc_file_url ? csc_file_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
                        else
                            string_copy(csc_image_url, csc_preview_url ? csc_preview_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
                    } else if(!strcmp(csc_filetype, "video/webm")) {
                        string_copy(csc_image_url, csc_preview_url ? csc_preview_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
                    } else {
                        string_copy(csc_image_url, SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
                    }
                } else {
                    string_copy(csc_image_url, SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
                }

                char csc_book[1024];
                snprintf(csc_book, sizeof(csc_book), "<a href=\"%s\">&#8203;</a><b>Page:</b> %d / %d\n<b>ID:</b> <code>%d</code>", csc_image_url, offset + 1, data1 + 1, csc_id);

                char callback_data[32];
                snprintf(callback_data, sizeof(callback_data), "4_%d", id);

                char callback_data1[32];
                snprintf(callback_data1, sizeof(callback_data1), "5_%d_%d_%d_%d", id, offset - 1, pages_array == 0 ? data - 1 : data, data1);

                char callback_data2[32];
                snprintf(callback_data2, sizeof(callback_data2), "5_%d_%d_%d_%d", id, offset + 1, pages_array == 19 ? data + 1 : data, data1);

                json_object *book_page = json_object_new_object();
                json_object *button = json_object_new_object();
                json_object *inline_keyboard = json_object_new_object();
                json_object *inline_keyboard1 = json_object_new_array();
                json_object *inline_keyboard2 = json_object_new_array();

                const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(json_object_object_get(update, "callback_query"), "message"), "chat"), "id"));
                int message_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(update, "callback_query"), "message"), "message_id"));
                const char *inline_message_id = json_object_get_string(json_object_object_get(json_object_object_get(update, "callback_query"), "inline_message_id"));

                if(chat_id && message_id) {
                    json_object_object_add(book_page, "chat_id", json_object_new_string(chat_id));
                    json_object_object_add(book_page, "message_id", json_object_new_int(message_id));
                } else if(inline_message_id) {
                    json_object_object_add(book_page, "inline_message_id", json_object_new_string(inline_message_id));
                }
                json_object_object_add(book_page, "text", json_object_new_string(csc_book));
                json_object_object_add(book_page, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(button, "text", json_object_new_string("Back"));
                json_object_object_add(button, "callback_data", json_object_new_string(callback_data));
                json_object_array_add(inline_keyboard2, button);
                if(offset) {
                    json_object *button1 = json_object_new_object();

                    json_object_object_add(button1, "text", json_object_new_string("Previous"));
                    json_object_object_add(button1, "callback_data", json_object_new_string(callback_data1));
                    json_object_array_add(inline_keyboard2, button1);
                }
                if(offset < data1) {
                    json_object *button2 = json_object_new_object();

                    json_object_object_add(button2, "text", json_object_new_string("Next"));
                    json_object_object_add(button2, "callback_data", json_object_new_string(callback_data2));
                    json_object_array_add(inline_keyboard2, button2);
                }
                json_object_array_add(inline_keyboard1, inline_keyboard2);
                json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                json_object_object_add(book_page, "reply_markup", inline_keyboard);

                api_post("editMessageText", book_page);
                json_object_put(book_page);
            } else {
                json_object_object_add(csc_answer, "text", json_object_new_string("Invalid action"));
            }
        } else {
            const char *error_code = json_object_get_string(json_object_object_get(csc_data, "code"));

            char error_description[256];

            if(error_code)
                snprintf(error_description, sizeof(error_description), "Error: %s", error_code);
            else
                snprintf(error_description, sizeof(error_description), "Unknown error");

            json_object_object_add(csc_answer, "text", json_object_new_string(error_description));
        }
        json_object_put(csc_data);
    } else {
        json_object_object_add(csc_answer, "text", json_object_new_string("Invalid callback data"));
    }

    api_post("answerCallbackQuery", csc_answer);
    json_object_put(csc_answer);
}
