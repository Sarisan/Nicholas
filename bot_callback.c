#include "core.h"
#include "csc_core.h"
#include <stdio.h>
#include <string.h>

void bot_callback(struct bot_update *result) {
    short action;
    int id, offset, data, data1, csc_id = 0;

    const char *callback_query_id = json_object_get_string(json_object_object_get(json_object_object_get(result->update, "callback_query"), "id"));

    if(sscanf(result->callback_data, "%hd_%d_%d_%d_%d", &action, &id, &offset, &data, &data1) != 5) {
        action = 0;
        id = 0;
        offset = 0;
        data = 0;
        data1 = 0;
    }

    json_object *csc_answer = json_object_new_object();

    json_object_object_add(csc_answer, "callback_query_id", json_object_new_string(callback_query_id));

    if(action) {
        json_object *csc_data = 0;

        if(action == 1 || action == 3)
            csc_data = csc_request(5L, "posts/%d/", id);
        else if(action == 2 || action == 4)
            csc_data = csc_request(5L, "pools/%d/", id);
        else if(action == 5)
            csc_data = csc_request(5L, "posts?page=%d&tags=pool:%d", data, id);

        if(action != 5) {
            csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));
        } else {
            if(json_object_get_type(csc_data) == json_type_array)
                csc_id = json_object_get_int(json_object_object_get(json_object_array_get_idx(csc_data, data > 1 ? offset - 20 * (data - 1) : offset), "id"));
        }

        if(csc_id) {
            if(action == 1 || action == 2) {
                char csc_copyright[20480], csc_copyright_s[20480], csc_studio[20480], csc_studio_s[20480], csc_character[20480], csc_character_s[20480], csc_artist[20480], csc_artist_s[20480], csc_medium[20480], csc_medium_s[20480], csc_general[20480], csc_general_s[20480], csc_meta[20480], csc_meta_s[20480], csc_genre[20480], csc_genre_s[20480], csc_info[20480];
                short tags_array = offset;

                json_object *csc_tags_array = json_object_object_get(csc_data, "tags");

                strcpy(csc_copyright, "");
                strcpy(csc_copyright_s, "");
                strcpy(csc_studio, "");
                strcpy(csc_studio_s, "");
                strcpy(csc_character, "");
                strcpy(csc_character_s, "");
                strcpy(csc_artist, "");
                strcpy(csc_artist_s, "");
                strcpy(csc_medium, "");
                strcpy(csc_medium_s, "");
                strcpy(csc_general, "");
                strcpy(csc_general_s, "");
                strcpy(csc_meta, "");
                strcpy(csc_meta_s, "");
                strcpy(csc_genre, "");
                strcpy(csc_genre_s, "");

                for(short tags_count = 0; json_object_array_get_idx(csc_tags_array, tags_array) && tags_count < 60; tags_count++, tags_array++) {
                    char tag[512];

                    char *tag_encoded = bot_strenc(json_object_get_string(json_object_object_get(json_object_array_get_idx(csc_tags_array, tags_array), "name")), 64);
                    sprintf(tag, "<code>%s</code> ", tag_encoded);
                    bot_free(tag_encoded);

                    switch(json_object_get_int(json_object_object_get(json_object_array_get_idx(csc_tags_array, tags_array), "type"))) {
                        case 3:
                            strcat(csc_copyright, tag);
                            break;
                        case 2:
                            strcat(csc_studio, tag);
                            break;
                        case 4:
                            strcat(csc_character, tag);
                            break;
                        case 1:
                            strcat(csc_artist, tag);
                            break;
                        case 8:
                            strcat(csc_medium, tag);
                            break;
                        case 0:
                            strcat(csc_general, tag);
                            break;
                        case 9:
                            strcat(csc_meta, tag);
                            break;
                        case 5:
                            strcat(csc_genre, tag);
                            break;
                    }
                }

                if(strcmp(csc_copyright, ""))
                    sprintf(csc_copyright_s, "<b>Copyright:</b> %s\n", csc_copyright);
                if(strcmp(csc_studio, ""))
                    sprintf(csc_studio_s, "<b>Studio:</b> %s\n", csc_studio);
                if(strcmp(csc_character, ""))
                    sprintf(csc_character_s, "<b>Character:</b> %s\n", csc_character);
                if(strcmp(csc_artist, ""))
                    sprintf(csc_artist_s, "<b>Artist:</b> %s\n", csc_artist);
                if(strcmp(csc_medium, ""))
                    sprintf(csc_medium_s, "<b>Medium:</b> %s\n", csc_medium);
                if(strcmp(csc_general, ""))
                    sprintf(csc_general_s, "<b>General:</b> %s\n", csc_general);
                if(strcmp(csc_meta, ""))
                    sprintf(csc_meta_s, "<b>Meta:</b> %s\n", csc_meta);
                if(strcmp(csc_genre, ""))
                    sprintf(csc_genre_s, "<b>Genre:</b> %s", csc_genre);

                sprintf(csc_info, "%s%s%s%s%s%s%s%s", csc_copyright_s, csc_studio_s, csc_character_s, csc_artist_s, csc_medium_s, csc_general_s, csc_meta_s, csc_genre_s);

                if(strcmp(csc_info, "")) {
                    char callback_data[32], callback_data1[64], callback_data2[64];

                    const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "callback_query"), "message"), "chat"), "id"));
                    int message_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "callback_query"), "message"), "message_id"));
                    const char *inline_message_id = json_object_get_string(json_object_object_get(json_object_object_get(result->update, "callback_query"), "inline_message_id"));

                    if(action == 1)
                        sprintf(callback_data, "3_%d_0_0_0", id);
                    else if(action == 2)
                        sprintf(callback_data, "4_%d_0_0_0", id);
                    sprintf(callback_data1, "%d_%d_%d_0_0", action, id, offset - 60);
                    sprintf(callback_data2, "%d_%d_%d_0_0", action, id, tags_array);

                    json_object *info = json_object_new_object();
                    json_object *button = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

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

                    bot_post("editMessageText", &info);
                    json_object_put(info);
                } else {
                    json_object_object_add(csc_answer, "text", json_object_new_string("No tags found"));
                }
            } else if(action == 3) {
                char csc_info[4096], csc_button[128], callback_data[32], csc_button_text[16];

                const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "callback_query"), "message"), "chat"), "id"));
                int message_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "callback_query"), "message"), "message_id"));
                const char *inline_message_id = json_object_get_string(json_object_object_get(json_object_object_get(result->update, "callback_query"), "inline_message_id"));

                bot_csc_post(csc_info, &csc_data, csc_id);
                sprintf(csc_button, "%s%d", CSC_POST_URL, csc_id);
                sprintf(callback_data, "1_%d_0_0_0", id);
                sprintf(csc_button_text, "Tags (%zu)", json_object_array_length(json_object_object_get(csc_data, "tags")));

                json_object *info = json_object_new_object();
                json_object *button = json_object_new_object();
                json_object *button1 = json_object_new_object();
                json_object *inline_keyboard = json_object_new_object();
                json_object *inline_keyboard1 = json_object_new_array();
                json_object *inline_keyboard2 = json_object_new_array();

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

                bot_post("editMessageText", &info);
                json_object_put(info);
            } else if(action == 4) {
                char csc_info[20480], csc_button[128], callback_data[32], callback_data1[32], csc_button_text[16];

                const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "callback_query"), "message"), "chat"), "id"));
                int message_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "callback_query"), "message"), "message_id"));
                const char *inline_message_id = json_object_get_string(json_object_object_get(json_object_object_get(result->update, "callback_query"), "inline_message_id"));

                bot_csc_pool(csc_info, &csc_data, csc_id);
                sprintf(csc_button, "%s%d", CSC_POOL_URL, csc_id);
                sprintf(callback_data, "5_%d_0_1_%d", id, json_object_get_int(json_object_object_get(csc_data, "visible_post_count")) - 1);
                sprintf(callback_data1, "2_%d_0_0_0", id);
                sprintf(csc_button_text, "Tags (%zu)", json_object_array_length(json_object_object_get(csc_data, "tags")));

                json_object *info = json_object_new_object();
                json_object *button = json_object_new_object();
                json_object *button1 = json_object_new_object();
                json_object *button2 = json_object_new_object();
                json_object *inline_keyboard = json_object_new_object();
                json_object *inline_keyboard1 = json_object_new_array();
                json_object *inline_keyboard2 = json_object_new_array();

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

                bot_post("editMessageText", &info);
                json_object_put(info);
            } else if(action == 5) {
                char csc_book[1024], csc_sample_url[512], callback_data[32], callback_data1[32], callback_data2[32];

                const char *chat_id = json_object_get_string(json_object_object_get(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "callback_query"), "message"), "chat"), "id"));
                int message_id = json_object_get_int(json_object_object_get(json_object_object_get(json_object_object_get(result->update, "callback_query"), "message"), "message_id"));
                const char *inline_message_id = json_object_get_string(json_object_object_get(json_object_object_get(result->update, "callback_query"), "inline_message_id"));

                short pages_array = data > 1 ? offset - 20 * (data - 1) : offset;
                json_object *csc_page = json_object_array_get_idx(csc_data, pages_array);

                float csc_size = json_object_get_int(json_object_object_get(csc_page, "file_size"));
                const char *csc_filetype = json_object_get_string(json_object_object_get(csc_page, "file_type"));

                if(csc_filetype) {
                    if(!strcmp(csc_filetype, "image/jpeg")) {
                        strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_page, "sample_url")));
                    } else if(!strcmp(csc_filetype, "image/png")) {
                        strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_page, "sample_url")));
                    } else if(!strcmp(csc_filetype, "image/gif")) {
                        if(csc_size <= 20971520)
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_page, "file_url")));
                        else
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_page, "preview_url")));
                    } else if(!strcmp(csc_filetype, "video/mp4")) {
                        if(csc_size <= 20971520)
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_page, "file_url")));
                        else
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_page, "preview_url")));
                    } else if(!strcmp(csc_filetype, "video/webm")) {
                        strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_page, "preview_url")));
                    } else {
                        strcpy(csc_sample_url, "https://s.sankakucomplex.com/download-preview.png");
                    }
                } else {
                    strcpy(csc_sample_url, "https://s.sankakucomplex.com/download-preview.png");
                }

                sprintf(csc_book, "<a href=\"%s\">&#8203;</a><b>Page:</b> %d / %d\n<b>ID:</b> <code>%d</code>", csc_sample_url, offset + 1, data1 + 1, csc_id);
                sprintf(callback_data, "4_%d_0_0_0", id);
                sprintf(callback_data1, "5_%d_%d_%d_%d", id, offset - 1, pages_array == 0 ? data - 1 : data, data1);
                sprintf(callback_data2, "5_%d_%d_%d_%d", id, offset + 1, pages_array == 19 ? data + 1 : data, data1);

                json_object *book_page = json_object_new_object();
                json_object *button = json_object_new_object();
                json_object *inline_keyboard = json_object_new_object();
                json_object *inline_keyboard1 = json_object_new_array();
                json_object *inline_keyboard2 = json_object_new_array();

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

                bot_post("editMessageText", &book_page);
                json_object_put(book_page);
            } else {
                json_object_object_add(csc_answer, "text", json_object_new_string("Invalid action"));
            }
        } else {
            char error_description[256];

            const char *error_code = json_object_get_string(json_object_object_get(csc_data, "code"));

            if(error_code)
                sprintf(error_description, "Error: %.128s", error_code);
            else
                sprintf(error_description, "Unknown error");

            json_object_object_add(csc_answer, "text", json_object_new_string(error_description));
        }
        json_object_put(csc_data);
    } else {
        json_object_object_add(csc_answer, "text", json_object_new_string("Invalid callback data"));
    }

    bot_post("answerCallbackQuery", &csc_answer);
    json_object_put(csc_answer);
}
