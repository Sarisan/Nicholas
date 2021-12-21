#include "core.h"
#include "csc_core.h"
#include <curl/curl.h>
#include <string.h>

void bot_inline(struct bot_update *result) {
    char inline_query[1024], arguments[8], *pre_query, query[4096];
    short page, action = 0, preview = 0, autopaging = 0;

    const char *inline_query_id = json_object_get_string(json_object_object_get(json_object_object_get(result->update, "inline_query"), "id"));

    sprintf(inline_query, "%.1024s", result->inline_query);
    strcpy(arguments, "");
    pre_query = "";
    strcpy(query, "");

    if(sscanf(result->inline_query, "%hd", &page) == 1) {
        char page_buffer[8], first_word[4];

        sprintf(page_buffer, "%d", page);

        memmove(&inline_query[0], &inline_query[strlen(page_buffer)], strlen(inline_query));

        for(short s = 0; inline_query[s]; s++) {
            if(inline_query[s] == ' ' && inline_query[s + 1] == ' ') {
                memmove(&inline_query[s], &inline_query[s + 1], strlen(inline_query));
                s--;
            }
        }

        for(short args = 0; args < 5; args++) {
            if(inline_query[0] == 'b' && !action) {
                action = 1;
                memmove(&inline_query[0], &inline_query[1], strlen(inline_query));
                strcat(arguments, "b");
            } else if(inline_query[0] == 'n' && action == 1) {
                action = 2;
                memmove(&inline_query[0], &inline_query[1], strlen(inline_query));
                strcat(arguments, "n");
            } else if(inline_query[0] == 't' && !action) {
                action = 3;
                memmove(&inline_query[0], &inline_query[1], strlen(inline_query));
                strcat(arguments, "t");
            } else if(inline_query[0] == 'p' && !preview) {
                preview = 1;
                memmove(&inline_query[0], &inline_query[1], strlen(inline_query));
                strcat(arguments, "p");
            } else if(inline_query[0] == 'a' && !autopaging) {
                autopaging = 1;
                memmove(&inline_query[0], &inline_query[1], strlen(inline_query));
                strcat(arguments, "a");
            }
        }

        if(inline_query[0]) {
            if(inline_query[0] == ' ') {
                sscanf(inline_query, "%4s", first_word);
                pre_query = strstr(inline_query, first_word);
                CURL *encode_query = curl_easy_init();
                char *encoded_query = curl_easy_escape(encode_query, pre_query, 0);
                strcpy(query, encoded_query);
                curl_free(encoded_query);
                curl_easy_cleanup(encode_query);
            } else {
                page = 0;
            }
        }
    } else {
        page = 0;
    }

    json_object *csc_answer = json_object_new_object();
    json_object *csc_results = json_object_new_array();

    json_object_object_add(csc_answer, "inline_query_id", json_object_new_string(inline_query_id));
    json_object_object_add(csc_answer, "results", csc_results);

    if(page >= 1 && page <= 100) {
        json_object *data = 0;
        short array = 0;

        int offset = json_object_get_int(json_object_object_get(json_object_object_get(result->update, "inline_query"), "offset"));

        if(offset)
            page = offset + 1;

        if(!action)
            data = csc_request(9L, "posts?page=%d&tags=%s", page, query);
        else if(action == 1)
            data = csc_request(9L, "pools?page=%d&tags=%s", page, query);
        else if(action == 2)
            data = csc_request(9L, "pools?page=%d&name=%s", page, query);
        else if(action == 3)
            data = csc_request(9L, "tags?page=%d&name=%s", page, query);

        if(json_object_get_type(data) == json_type_array) {
            while(json_object_array_get_idx(data, array)) {
                json_object *csc_data = json_object_array_get_idx(data, array);
                json_object *csc_result = json_object_new_object();
                if(!action) {
                    char *csc_rating_s = "", csc_size_s[16], *csc_method, *csc_key, *csc_width, *csc_height, *csc_mime, *csc_format, csc_sample_url[512], csc_preview_url[512], csc_date[16], csc_caption[128], csc_button[128], csc_button_text[32], csc_button_text1[32], csc_button1[2048], csc_button2[32], csc_button3[32];

                    int csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));
                    const char *csc_rating = json_object_get_string(json_object_object_get(csc_data, "rating"));
                    int csc_swidth = json_object_get_int(json_object_object_get(csc_data, "sample_width"));
                    int csc_sheight = json_object_get_int(json_object_object_get(csc_data, "sample_height"));
                    float csc_size = json_object_get_int(json_object_object_get(csc_data, "file_size"));
                    const char *csc_filetype = json_object_get_string(json_object_object_get(csc_data, "file_type"));
                    time_t rawtime = json_object_get_int(json_object_object_get(json_object_object_get(csc_data, "created_at"), "s"));
                    struct tm *date = localtime(&rawtime);

                    if(!strcmp(csc_rating, "s"))
                        csc_rating_s = "Safe";
                    else if(!strcmp(csc_rating, "q"))
                        csc_rating_s = "Questionable";
                    else if(!strcmp(csc_rating, "e"))
                        csc_rating_s = "Explicit";

                    if(csc_size > 1073741824)
                        sprintf(csc_size_s, "%.2f GiB", csc_size / 1073741824);
                    else if(csc_size > 1048576)
                        sprintf(csc_size_s, "%.2f MiB", csc_size / 1048576);
                    else if(csc_size > 1024)
                        sprintf(csc_size_s, "%.2f KiB", csc_size / 1024);
                    else
                        sprintf(csc_size_s, "%f B", csc_size);

                    if(csc_filetype) {
                        if(!strcmp(csc_filetype, "image/jpeg")) {
                            csc_method = "photo";
                            csc_key = "photo_url";
                            csc_width = "photo_width";
                            csc_height = "photo_height";
                            csc_mime = "null";
                            csc_format = "JPEG";
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "sample_url")));
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                        } else if(!strcmp(csc_filetype, "image/png")) {
                            csc_method = "photo";
                            csc_key = "photo_url";
                            csc_width = "photo_width";
                            csc_height = "photo_height";
                            csc_mime = "null";
                            csc_format = "PNG";
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "sample_url")));
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                        } else if(!strcmp(csc_filetype, "image/gif")) {
                            csc_format = "GIF";
                            if(csc_size <= 20971520 && !preview) {
                                csc_method = "gif";
                                csc_key = "gif_url";
                                csc_width = "gif_width";
                                csc_height = "gif_height";
                                csc_mime = "image/jpeg";
                                strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "file_url")));
                            } else {
                                csc_method = "photo";
                                csc_key = "photo_url";
                                csc_width = "photo_width";
                                csc_height = "photo_height";
                                csc_mime = "null";
                                strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                            }
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                        } else if(!strcmp(csc_filetype, "video/mp4")) {
                            csc_format = "MP4";
                            if(csc_size <= 20971520 && !preview) {
                                csc_method = "mpeg4_gif";
                                csc_key = "mpeg4_url";
                                csc_width = "mpeg4_width";
                                csc_height = "mpeg4_height";
                                csc_mime = "image/jpeg";
                                strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "file_url")));
                            } else {
                                csc_method = "photo";
                                csc_key = "photo_url";
                                csc_width = "photo_width";
                                csc_height = "photo_height";
                                csc_mime = "null";
                                strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                            }
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                        } else if(!strcmp(csc_filetype, "video/webm")) {
                            csc_method = "photo";
                            csc_key = "photo_url";
                            csc_width = "photo_width";
                            csc_height = "photo_height";
                            csc_mime = "null";
                            csc_format = "WEBM";
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                            csc_swidth = json_object_get_int(json_object_object_get(csc_data, "preview_width"));
                            csc_sheight = json_object_get_int(json_object_object_get(csc_data, "preview_height"));
                        } else {
                            csc_method = "photo";
                            csc_key = "photo_url";
                            csc_width = "photo_width";
                            csc_height = "photo_height";
                            csc_mime = "null";
                            csc_format = "";
                            strcpy(csc_sample_url, "https://s.sankakucomplex.com/download-preview.png");
                            strcpy(csc_preview_url, "https://s.sankakucomplex.com/download-preview.png");
                            csc_swidth = 150;
                            csc_sheight = 150;
                        }
                    } else {
                        csc_method = "photo";
                        csc_key = "photo_url";
                        csc_width = "photo_width";
                        csc_height = "photo_height";
                        csc_mime = "null";
                        csc_format = "SWF";
                        strcpy(csc_sample_url, "https://s.sankakucomplex.com/download-preview.png");
                        strcpy(csc_preview_url, "https://s.sankakucomplex.com/download-preview.png");
                        csc_swidth = 150;
                        csc_sheight = 150;
                    }

                    strftime(csc_date, sizeof(csc_date), "%d.%m.%Y", date);
                    sprintf(csc_caption, "<b>ID:</b> <code>%d</code>\n<b>Date:</b> <code>%s</code>", csc_id, csc_date);
                    sprintf(csc_button, "%s%d", CSC_POST_URL, csc_id);
                    sprintf(csc_button_text, "%s %s", csc_rating_s, csc_format);
                    sprintf(csc_button1, "%d", page);
                    sprintf(csc_button_text1, "%sx%s %s", json_object_get_string(json_object_object_get(csc_data, "width")), json_object_get_string(json_object_object_get(csc_data, "height")), csc_size_s);
                    sprintf(csc_button2, "original %d", csc_id);
                    sprintf(csc_button3, "post %d", csc_id);

                    if(strcmp(arguments, ""))
                        strcat(csc_button1, arguments);
                    if(strcmp(pre_query, ""))
                        sprintf(csc_button1 + strlen(csc_button1), " %.1024s", pre_query);

                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();
                    json_object *inline_keyboard3 = json_object_new_array();
                    json_object *inline_keyboard4 = json_object_new_array();
                    json_object *button = json_object_new_object();
                    json_object *button1 = json_object_new_object();
                    json_object *button2 = json_object_new_object();
                    json_object *button3 = json_object_new_object();

                    json_object_object_add(csc_result, "type", json_object_new_string(csc_method));
                    json_object_object_add(csc_result, "id", json_object_new_int64(csc_id * 1000 + page));
                    json_object_object_add(csc_result, csc_key, json_object_new_string(csc_sample_url));
                    json_object_object_add(csc_result, csc_width, json_object_new_int(csc_swidth));
                    json_object_object_add(csc_result, csc_height, json_object_new_int(csc_sheight));
                    json_object_object_add(csc_result, "thumb_url", json_object_new_string(csc_preview_url));
                    json_object_object_add(csc_result, "thumb_mime_type", json_object_new_string(csc_mime));
                    json_object_object_add(csc_result, "caption", json_object_new_string(csc_caption));
                    json_object_object_add(csc_result, "parse_mode", json_object_new_string("HTML"));
                    json_object_object_add(button, "text", json_object_new_string(csc_button_text));
                    json_object_object_add(button, "url", json_object_new_string(csc_button));
                    json_object_array_add(inline_keyboard2, button);
                    json_object_object_add(button1, "text", json_object_new_string(csc_button_text1));
                    json_object_object_add(button1, "switch_inline_query_current_chat", json_object_new_string(csc_button1));
                    if(csc_swidth < csc_sheight || csc_swidth < 500) {
                        json_object_array_add(inline_keyboard3, button1);
                        json_object_array_add(inline_keyboard1, inline_keyboard2);
                        json_object_array_add(inline_keyboard1, inline_keyboard3);
                    } else {
                        json_object_array_add(inline_keyboard2, button1);
                        json_object_array_add(inline_keyboard1, inline_keyboard2);
                        json_object_put(inline_keyboard3);
                    }
                    json_object_object_add(button2, "text", json_object_new_string("Original file"));
                    json_object_object_add(button2, "switch_inline_query_current_chat", json_object_new_string(csc_button2));
                    json_object_array_add(inline_keyboard4, button2);
                    json_object_object_add(button3, "text", json_object_new_string("Information"));
                    json_object_object_add(button3, "switch_inline_query_current_chat", json_object_new_string(csc_button3));
                    json_object_array_add(inline_keyboard4, button3);
                    json_object_array_add(inline_keyboard1, inline_keyboard4);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(csc_result, "reply_markup", inline_keyboard);
                } if(action == 1 || action == 2) {
                    char *csc_rating_s = "", *csc_method, *csc_key, *csc_width, *csc_height, *csc_mime, csc_sample_url[512], csc_preview_url[512], csc_caption[6144], csc_button[128], csc_button_text[16], csc_button_text1[32], csc_button1[2048], csc_button2[32], csc_button3[32];

                    int csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));
                    const char *csc_date = json_object_get_string(json_object_object_get(csc_data, "created_at"));
                    short csc_pages = json_object_get_int(json_object_object_get(csc_data, "visible_post_count"));
                    const char *csc_rating = json_object_get_string(json_object_object_get(csc_data, "rating"));
                    json_object *cover_post = json_object_object_get(csc_data, "cover_post");
                    int csc_swidth = json_object_get_int(json_object_object_get(cover_post, "sample_width"));
                    int csc_sheight = json_object_get_int(json_object_object_get(cover_post, "sample_height"));
                    int csc_size = json_object_get_int(json_object_object_get(cover_post, "file_size"));
                    const char *csc_filetype = json_object_get_string(json_object_object_get(cover_post, "file_type"));
                    char *csc_name = bot_strenc(json_object_get_string(json_object_object_get(csc_data, "name")), 1024);

                    if(!strcmp(csc_rating, "s"))
                        csc_rating_s = "Safe";
                    else if(!strcmp(csc_rating, "q"))
                        csc_rating_s = "Questionable";
                    else if(!strcmp(csc_rating, "e"))
                        csc_rating_s = "Explicit";

                    if(csc_filetype) {
                        if(!strcmp(csc_filetype, "image/jpeg")) {
                            csc_method = "photo";
                            csc_key = "photo_url";
                            csc_width = "photo_width";
                            csc_height = "photo_height";
                            csc_mime = "null";
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "sample_url")));
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                        } else if(!strcmp(csc_filetype, "image/png")) {
                            csc_method = "photo";
                            csc_key = "photo_url";
                            csc_width = "photo_width";
                            csc_height = "photo_height";
                            csc_mime = "null";
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "sample_url")));
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                        } else if(!strcmp(csc_filetype, "image/gif")) {
                            if(csc_size <= 20971520 && !preview) {
                                csc_method = "gif";
                                csc_key = "gif_url";
                                csc_width = "gif_width";
                                csc_height = "gif_height";
                                csc_mime = "image/jpeg";
                                strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "file_url")));
                            } else {
                                csc_method = "photo";
                                csc_key = "photo_url";
                                csc_width = "photo_width";
                                csc_height = "photo_height";
                                csc_mime = "null";
                                strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                            }
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                        } else if(!strcmp(csc_filetype, "video/mp4")) {
                            if(csc_size <= 20971520 && !preview) {
                                csc_method = "mpeg4_gif";
                                csc_key = "mpeg4_url";
                                csc_width = "mpeg4_width";
                                csc_height = "mpeg4_height";
                                csc_mime = "image/jpeg";
                                strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "file_url")));
                            } else {
                                csc_method = "photo";
                                csc_key = "photo_url";
                                csc_width = "photo_width";
                                csc_height = "photo_height";
                                csc_mime = "null";
                                strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                            }
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                        } else if(!strcmp(csc_filetype, "video/webm")) {
                            csc_method = "photo";
                            csc_key = "photo_url";
                            csc_width = "photo_width";
                            csc_height = "photo_height";
                            csc_mime = "null";
                            strcpy(csc_sample_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                            strcpy(csc_preview_url, json_object_get_string(json_object_object_get(csc_data, "preview_url")));
                            csc_swidth = json_object_get_int(json_object_object_get(csc_data, "preview_width"));
                            csc_sheight = json_object_get_int(json_object_object_get(csc_data, "preview_height"));
                        } else {
                            csc_method = "photo";
                            csc_key = "photo_url";
                            csc_width = "photo_width";
                            csc_height = "photo_height";
                            csc_mime = "null";
                            strcpy(csc_sample_url, "https://s.sankakucomplex.com/download-preview.png");
                            strcpy(csc_preview_url, "https://s.sankakucomplex.com/download-preview.png");
                            csc_swidth = 150;
                            csc_sheight = 150;
                        }
                    } else {
                        csc_method = "photo";
                        csc_key = "photo_url";
                        csc_width = "photo_width";
                        csc_height = "photo_height";
                        csc_mime = "null";
                        strcpy(csc_sample_url, "https://s.sankakucomplex.com/download-preview.png");
                        strcpy(csc_preview_url, "https://s.sankakucomplex.com/download-preview.png");
                        csc_swidth = 150;
                        csc_sheight = 150;
                    }

                    sprintf(csc_caption, "<b>ID:</b> <code>%d</code>\n<b>Date:</b> %s\n\n<b>%s</b>", csc_id, csc_date, csc_name);
                    bot_free(csc_name);
                    sprintf(csc_button, "%s%d", CSC_POOL_URL, csc_id);
                    sprintf(csc_button_text, "%s", csc_rating_s);
                    sprintf(csc_button1, "%d", page);
                    sprintf(csc_button_text1, "%d pages", csc_pages);
                    sprintf(csc_button2, "1a pool:%d", csc_id);
                    sprintf(csc_button3, "book %d", csc_id);

                    if(strcmp(arguments, ""))
                        strcat(csc_button1, arguments);
                    if(strcmp(pre_query, ""))
                        sprintf(csc_button1 + strlen(csc_button1), " %.1024s", pre_query);
                    if(csc_pages == 1)
                        csc_button_text1[6] = 0;

                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();
                    json_object *inline_keyboard3 = json_object_new_array();
                    json_object *button = json_object_new_object();
                    json_object *button1 = json_object_new_object();
                    json_object *button2 = json_object_new_object();
                    json_object *button3 = json_object_new_object();

                    json_object_object_add(csc_result, "type", json_object_new_string(csc_method));
                    json_object_object_add(csc_result, "id", json_object_new_int64(csc_id * 1000 + page));
                    json_object_object_add(csc_result, csc_key, json_object_new_string(csc_sample_url));
                    json_object_object_add(csc_result, csc_width, json_object_new_int(csc_swidth));
                    json_object_object_add(csc_result, csc_height, json_object_new_int(csc_sheight));
                    json_object_object_add(csc_result, "thumb_url", json_object_new_string(csc_preview_url));
                    json_object_object_add(csc_result, "thumb_mime_type", json_object_new_string(csc_mime));
                    json_object_object_add(csc_result, "caption", json_object_new_string(csc_caption));
                    json_object_object_add(csc_result, "parse_mode", json_object_new_string("HTML"));
                    json_object_object_add(button, "text", json_object_new_string(csc_button_text));
                    json_object_object_add(button, "url", json_object_new_string(csc_button));
                    json_object_array_add(inline_keyboard2, button);
                    json_object_object_add(button1, "text", json_object_new_string(csc_button_text1));
                    json_object_object_add(button1, "switch_inline_query_current_chat", json_object_new_string(csc_button1));
                    json_object_array_add(inline_keyboard2, button1);
                    json_object_array_add(inline_keyboard1, inline_keyboard2);
                    json_object_object_add(button2, "text", json_object_new_string("Preview"));
                    json_object_object_add(button2, "switch_inline_query_current_chat", json_object_new_string(csc_button2));
                    json_object_array_add(inline_keyboard3, button2);
                    json_object_object_add(button3, "text", json_object_new_string("Information"));
                    json_object_object_add(button3, "switch_inline_query_current_chat", json_object_new_string(csc_button3));
                    json_object_array_add(inline_keyboard3, button3);
                    json_object_array_add(inline_keyboard1, inline_keyboard3);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(csc_result, "reply_markup", inline_keyboard);
                } else if(action == 3) {
                    char csc_tag[20480], csc_button[2048], csc_button1[2048];

                    int csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));
                    const char *csc_name = json_object_get_string(json_object_object_get(csc_data, "name"));

                    bot_csc_tag(csc_tag, &csc_data, csc_id);
                    sprintf(csc_button, "1a %.1024s", csc_name);
                    sprintf(csc_button1, "1ba %.1024s", csc_name);

                    json_object *input_message_content = json_object_new_object();
                    json_object *button = json_object_new_object();
                    json_object *button1 = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    json_object_object_add(csc_result, "type", json_object_new_string("article"));
                    json_object_object_add(csc_result, "id", json_object_new_int64(csc_id * 1000 + page));
                    json_object_object_add(csc_result, "title", json_object_new_string(csc_name));
                    json_object_object_add(input_message_content, "message_text", json_object_new_string(csc_tag));
                    json_object_object_add(input_message_content, "parse_mode", json_object_new_string("HTML"));
                    json_object_object_add(csc_result, "input_message_content", input_message_content);
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
                    json_object_object_add(csc_result, "reply_markup", inline_keyboard);
                    json_object_object_add(csc_result, "description", json_object_new_string("Click to send tag information"));
                }
                json_object_array_add(csc_results, csc_result);
                array++;
            }

            if(!offset && !array) {
                json_object *csc_result = json_object_new_object();
                json_object *input_message_content = json_object_new_object();

                json_object_object_add(csc_result, "type", json_object_new_string("article"));
                json_object_object_add(csc_result, "id", json_object_new_int(1));
                json_object_object_add(csc_result, "title", json_object_new_string("No results found"));
                json_object_object_add(input_message_content, "message_text", json_object_new_string("<b>No results found with this query</b>"));
                json_object_object_add(input_message_content, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(csc_result, "input_message_content", input_message_content);
                if(!action || action == 1 )
                    json_object_object_add(csc_result, "description", json_object_new_string("Try different tags or page"));
                else if(action == 2 || action == 3)
                    json_object_object_add(csc_result, "description", json_object_new_string("Try different name or page"));

                json_object_array_add(csc_results, csc_result);
            }
        } else if(!offset) {
            char error_description[256];

            const char *error_code = json_object_get_string(json_object_object_get(data, "code"));

            if(error_code)
                sprintf(error_description, "Error: %.128s", error_code);
            else
                sprintf(error_description, "Unknown error");

            json_object *csc_result = json_object_new_object();
            json_object *input_message_content = json_object_new_object();

            json_object_object_add(csc_result, "type", json_object_new_string("article"));
            json_object_object_add(csc_result, "id", json_object_new_int(1));
            json_object_object_add(csc_result, "title", json_object_new_string("Error occurred"));
            json_object_object_add(input_message_content, "message_text", json_object_new_string("<b>No results found, error occurred</b>"));
            json_object_object_add(input_message_content, "parse_mode", json_object_new_string("HTML"));
            json_object_object_add(csc_result, "input_message_content", input_message_content);
            json_object_object_add(csc_result, "description", json_object_new_string(error_description));

            json_object_array_add(csc_results, csc_result);
        }

        if(autopaging && array)
            json_object_object_add(csc_answer, "next_offset", json_object_new_int(page));

        json_object_put(data);
    } else if(sscanf(result->inline_query, "%8s", query) == 1 && (!strcmp(query, "original") || !strcmp(query, "post") || !strcmp(query, "book"))) {
        char argument[16];

        if(sscanf(result->inline_query, "%8s %16s", query, argument) != 2)
            strcpy(argument, "");

        json_object *csc_result = json_object_new_object();

        if(strcmp(argument, "")) {
            json_object *csc_data;

            if(strcmp(query, "book"))
                csc_data = csc_request(9L, "posts/%s/", argument);
            else
                csc_data = csc_request(9L, "pools/%s/", argument);

            int csc_id = json_object_get_int(json_object_object_get(csc_data, "id"));

            if(csc_id) {
                if(!strcmp(query, "original")) {
                    char csc_title[64];

                    const char *csc_preview_url = json_object_get_string(json_object_object_get(csc_data, "preview_url"));
                    const char *document = json_object_get_string(json_object_object_get(csc_data, "file_url"));
                    int csc_size = json_object_get_int(json_object_object_get(csc_data, "file_size"));
                    const char *csc_filetype = json_object_get_string(json_object_object_get(csc_data, "file_type"));

                    json_object *button = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    if(csc_filetype && strcmp(csc_filetype, "video/webm") && csc_size <= 20971520) {
                        sprintf(csc_title, "Original file of post %d", csc_id);

                        json_object_object_add(csc_result, "type", json_object_new_string("document"));
                        json_object_object_add(csc_result, "id", json_object_new_int(1));
                        json_object_object_add(csc_result, "title", json_object_new_string(csc_title));
                        json_object_object_add(csc_result, "document_url", json_object_new_string(document));
                        json_object_object_add(csc_result, "mime_type", json_object_new_string("application/zip"));
                        json_object_object_add(csc_result, "description", json_object_new_string("Click to send file"));
                        json_object_object_add(csc_result, "thumb_url", json_object_new_string(csc_preview_url));
                    } else {
                        sprintf(csc_title, "Link to original file of post %d", csc_id);

                        json_object *input_message_content = json_object_new_object();

                        json_object_object_add(csc_result, "type", json_object_new_string("article"));
                        json_object_object_add(csc_result, "id", json_object_new_int(1));
                        json_object_object_add(csc_result, "title", json_object_new_string(csc_title));
                        if(csc_size > 20971520)
                            json_object_object_add(input_message_content, "message_text", json_object_new_string("<b>File size must not exceed 20 MiB</b>"));
                        else
                            json_object_object_add(input_message_content, "message_text", json_object_new_string("<b>Unsupported media type</b>"));
                        json_object_object_add(input_message_content, "parse_mode", json_object_new_string("HTML"));
                        json_object_object_add(csc_result, "input_message_content", input_message_content);
                        json_object_object_add(csc_result, "description", json_object_new_string("Click to send link"));
                        json_object_object_add(csc_result, "thumb_url", json_object_new_string(csc_preview_url));
                    }

                    json_object_object_add(button, "text", json_object_new_string("Download manually, link expires in 3 hours"));
                    json_object_object_add(button, "url", json_object_new_string(document));
                    json_object_array_add(inline_keyboard2, button);
                    json_object_array_add(inline_keyboard1, inline_keyboard2);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(csc_result, "reply_markup", inline_keyboard);
                } else if(!strcmp(query, "post")) {
                    char csc_info[4096], csc_title[32], csc_button[128], callback_data[32], csc_button_text[16];

                    const char *csc_preview_url = json_object_get_string(json_object_object_get(csc_data, "preview_url"));

                    bot_csc_post(csc_info, &csc_data, csc_id);
                    sprintf(csc_title, "Post %d", csc_id);
                    sprintf(csc_button, "%s%d", CSC_POST_URL, csc_id);
                    sprintf(callback_data, "1_%d_0_0_0", csc_id);
                    sprintf(csc_button_text, "Tags (%zu)", json_object_array_length(json_object_object_get(csc_data, "tags")));

                    json_object *input_message_content = json_object_new_object();
                    json_object *button = json_object_new_object();
                    json_object *button1 = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    json_object_object_add(csc_result, "type", json_object_new_string("article"));
                    json_object_object_add(csc_result, "id", json_object_new_int(1));
                    json_object_object_add(csc_result, "title", json_object_new_string(csc_title));
                    json_object_object_add(input_message_content, "message_text", json_object_new_string(csc_info));
                    json_object_object_add(input_message_content, "parse_mode", json_object_new_string("HTML"));
                    json_object_object_add(csc_result, "input_message_content", input_message_content);
                    json_object_object_add(csc_result, "description", json_object_new_string("Click to send post information"));
                    json_object_object_add(csc_result, "thumb_url", json_object_new_string(csc_preview_url));
                    json_object_object_add(button, "text", json_object_new_string("Post link"));
                    json_object_object_add(button, "url", json_object_new_string(csc_button));
                    json_object_array_add(inline_keyboard2, button);
                    json_object_object_add(button1, "text", json_object_new_string(csc_button_text));
                    json_object_object_add(button1, "callback_data", json_object_new_string(callback_data));
                    json_object_array_add(inline_keyboard2, button1);
                    json_object_array_add(inline_keyboard1, inline_keyboard2);
                    json_object_object_add(inline_keyboard, "inline_keyboard", inline_keyboard1);
                    json_object_object_add(csc_result, "reply_markup", inline_keyboard);
                } else if(!strcmp(query, "book")) {
                    char csc_info[20480], csc_title[32], csc_button[128], callback_data[32], callback_data1[32], csc_button_text[16];

                    const char *csc_preview_url = json_object_get_string(json_object_object_get(csc_data, "preview_url"));

                    bot_csc_pool(csc_info, &csc_data, csc_id);
                    sprintf(csc_title, "Book %d", csc_id);
                    sprintf(csc_button, "%s%d", CSC_POOL_URL, csc_id);
                    sprintf(callback_data, "5_%d_0_1_%d", csc_id, json_object_get_int(json_object_object_get(csc_data, "visible_post_count")) - 1);
                    sprintf(callback_data1, "2_%d_0_0_0", csc_id);
                    sprintf(csc_button_text, "Tags (%zu)", json_object_array_length(json_object_object_get(csc_data, "tags")));

                    json_object *input_message_content = json_object_new_object();
                    json_object *button = json_object_new_object();
                    json_object *button1 = json_object_new_object();
                    json_object *button2 = json_object_new_object();
                    json_object *inline_keyboard = json_object_new_object();
                    json_object *inline_keyboard1 = json_object_new_array();
                    json_object *inline_keyboard2 = json_object_new_array();

                    json_object_object_add(csc_result, "type", json_object_new_string("article"));
                    json_object_object_add(csc_result, "id", json_object_new_int(1));
                    json_object_object_add(csc_result, "title", json_object_new_string(csc_title));
                    json_object_object_add(input_message_content, "message_text", json_object_new_string(csc_info));
                    json_object_object_add(input_message_content, "parse_mode", json_object_new_string("HTML"));
                    json_object_object_add(csc_result, "input_message_content", input_message_content);
                    json_object_object_add(csc_result, "description", json_object_new_string("Click to send book information"));
                    json_object_object_add(csc_result, "thumb_url", json_object_new_string(csc_preview_url));
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
                    json_object_object_add(csc_result, "reply_markup", inline_keyboard);
                }
            } else {
                char error_description[256];

                const char *error_code = json_object_get_string(json_object_object_get(csc_data, "code"));

                if(error_code && !strcmp(error_code, "snackbar__server-error_not-found")) {
                    if(strcmp(query, "book"))
                        sprintf(error_description, "Wrong post ID");
                    else
                        sprintf(error_description, "Wrong book ID");
                } else if(error_code) {
                    sprintf(error_description, "Error: %.128s", error_code);
                } else {
                    sprintf(error_description, "Unknown error");
                }

                json_object *input_message_content = json_object_new_object();

                json_object_object_add(csc_result, "type", json_object_new_string("article"));
                json_object_object_add(csc_result, "id", json_object_new_int(1));
                json_object_object_add(csc_result, "title", json_object_new_string("Error occurred"));
                json_object_object_add(input_message_content, "message_text", json_object_new_string("<b>No results found, error occurred</b>"));
                json_object_object_add(input_message_content, "parse_mode", json_object_new_string("HTML"));
                json_object_object_add(csc_result, "input_message_content", input_message_content);
                json_object_object_add(csc_result, "description", json_object_new_string(error_description));
            }
            json_object_put(csc_data);
        } else {
            json_object *input_message_content = json_object_new_object();

            json_object_object_add(csc_result, "type", json_object_new_string("article"));
            json_object_object_add(csc_result, "id", json_object_new_int(1));
            json_object_object_add(csc_result, "title", json_object_new_string("No ID specified"));
            json_object_object_add(input_message_content, "message_text", json_object_new_string("<b>No results found, no ID specified</b>"));
            json_object_object_add(input_message_content, "parse_mode", json_object_new_string("HTML"));
            json_object_object_add(csc_result, "input_message_content", input_message_content);
            json_object_object_add(csc_result, "description", json_object_new_string("You must specify an ID"));
        }
        json_object_array_add(csc_results, csc_result);
    } else {
        json_object *csc_result = json_object_new_object();
        json_object *input_message_content = json_object_new_object();

        json_object_object_add(csc_result, "type", json_object_new_string("article"));
        json_object_object_add(csc_result, "id", json_object_new_int(1));
        json_object_object_add(csc_result, "title", json_object_new_string("No argument specified"));
        json_object_object_add(input_message_content, "message_text", json_object_new_string("<b>No results found, no argument specified</b>"));
        json_object_object_add(input_message_content, "parse_mode", json_object_new_string("HTML"));
        json_object_object_add(csc_result, "input_message_content", input_message_content);
        if(!strcmp(result->inline_query, ""))
            json_object_object_add(csc_result, "description", json_object_new_string("You must specify an argument"));
        else
            json_object_object_add(csc_result, "description", json_object_new_string("Invalid argument"));

        json_object_array_add(csc_results, csc_result);
    }

    json_object_object_add(csc_answer, "cache_time", json_object_new_int(0));

    bot_post("answerInlineQuery", &csc_answer);
    json_object_put(csc_answer);
}
