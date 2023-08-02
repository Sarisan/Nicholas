#include <config/config.h>
#include <curl/curl.h>
#include <sankaku/api.h>
#include <json-c/json_tokener.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

#define SANKAKU_TZ (-10)

void sankaku_get_date(char *csc_date, size_t size, time_t rawtime) {
    rawtime += 3600 * SANKAKU_TZ;
    struct tm *date = gmtime(&rawtime);
    strftime(csc_date, size, "%Y-%m-%d %R", date);
}

void sankaku_get_post(char *csc_info, size_t length, json_object *csc_data, int csc_id) {
    const char *csc_rating = json_object_get_string(json_object_object_get(csc_data, "rating"));
    char *csc_author = string_escape(json_object_get_string(json_object_object_get(json_object_object_get(csc_data, "author"), "name")), 256);
    const char *csc_sample_url = json_object_get_string(json_object_object_get(csc_data, "sample_url"));
    const char *csc_preview_url = json_object_get_string(json_object_object_get(csc_data, "preview_url"));
    const char *csc_file_url = json_object_get_string(json_object_object_get(csc_data, "file_url"));
    float csc_size = json_object_get_int(json_object_object_get(csc_data, "file_size"));
    const char *csc_filetype = json_object_get_string(json_object_object_get(csc_data, "file_type"));
    time_t rawtime = json_object_get_int(json_object_object_get(json_object_object_get(csc_data, "created_at"), "s"));
    const char *csc_parent_id = json_object_get_string(json_object_object_get(csc_data, "parent_id"));
    float csc_vote_count = json_object_get_int(json_object_object_get(csc_data, "vote_count"));
    float csc_vote_average = json_object_get_int(json_object_object_get(csc_data, "total_score")) / csc_vote_count;
    const char *csc_source = json_object_get_string(json_object_object_get(csc_data, "source"));

    char *csc_rating_s;

    if(!strcmp(csc_rating, "s"))
        csc_rating_s = "safe";
    else if(!strcmp(csc_rating, "q"))
        csc_rating_s = "questionable";
    else if(!strcmp(csc_rating, "e"))
        csc_rating_s = "explicit";
    else
        csc_rating_s = "unknown";

    char csc_size_s[16] = "";

    if(csc_size > 1073741824)
        snprintf(csc_size_s, sizeof(csc_size_s), "(%.2f GiB)", csc_size / 1073741824);
    else if(csc_size > 1048576)
        snprintf(csc_size_s, sizeof(csc_size_s), "(%.2f MiB)", csc_size / 1048576);
    else if(csc_size > 1024)
        snprintf(csc_size_s, sizeof(csc_size_s), "(%.2f KiB)", csc_size / 1024);

    char *csc_format, csc_image_url[512];

    if(csc_filetype) {
        if(!strcmp(csc_filetype, "image/jpeg")) {
            csc_format = "JPEG";
            string_copy(csc_image_url, csc_sample_url ? csc_sample_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
        } else if(!strcmp(csc_filetype, "image/png")) {
            csc_format = "PNG";
            string_copy(csc_image_url, csc_sample_url ? csc_sample_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
        } else if(!strcmp(csc_filetype, "image/gif")) {
            csc_format = "GIF";
            if(csc_size <= 20971520)
                string_copy(csc_image_url, csc_file_url ? csc_file_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
            else
                string_copy(csc_image_url, csc_preview_url ? csc_preview_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
        } else if(!strcmp(csc_filetype, "video/mp4")) {
            csc_format = "MP4";
            if(csc_size <= 20971520)
                string_copy(csc_image_url, csc_file_url ? csc_file_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
            else
                string_copy(csc_image_url, csc_preview_url ? csc_preview_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
        } else if(!strcmp(csc_filetype, "video/webm")) {
            csc_format = "WEBM";
            string_copy(csc_image_url, csc_preview_url ? csc_preview_url : SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
        } else {
            csc_format = "unknown";
            string_copy(csc_image_url, SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
        }
    } else {
        csc_format = "SWF";
        string_copy(csc_image_url, SANKAKU_DPREVIEW_URL, sizeof(csc_image_url));
    }

    char *csc_has_children_s;

    if(json_object_get_boolean(json_object_object_get(csc_data, "has_children")))
        csc_has_children_s = "yes";
    else
        csc_has_children_s = "no";

    char csc_parent_id_s[32];

    if(csc_parent_id)
        snprintf(csc_parent_id_s, sizeof(csc_parent_id_s), "<code>%s</code>", csc_parent_id);
    else
        string_copy(csc_parent_id_s, "none", sizeof(csc_parent_id_s));

    if(csc_vote_average != (float)csc_vote_average)
        csc_vote_average = 0;

    char csc_source_s[1024];

    if(csc_source) {
        if(csc_source[0])
            snprintf(csc_source_s, sizeof(csc_source_s), "%s", csc_source);
        else
            string_copy(csc_source_s, "none", sizeof(csc_source_s));
    } else {
        string_copy(csc_source_s, "none", sizeof(csc_source_s));
    }

    char csc_date[16 + 1];
    sankaku_get_date(csc_date, sizeof(csc_date), rawtime);

    snprintf(csc_info, length, "<a href=\"%s\">&#8203;</a><b>ID:</b> <code>%d</code>\n<b>Rating:</b> %s\n<b>Status:</b> %s\n<b>Author:</b> %s\n<b>Sample resolution:</b> %sx%s\n<b>Resolution:</b> %sx%s\n<b>Size:</b> <code>%.0f</code> bytes %s\n<b>Type:</b> %s\n<b>Date:</b> <code>%s</code>\n<b>Has children:</b> %s\n<b>Parent ID:</b> %s\n<b>MD5:</b> <code>%s</code>\n<b>Fav count:</b> %d\n<b>Vote count:</b> %.0f\n<b>Vote average:</b> %.2f\n<b>Source:</b> %s", csc_image_url, csc_id, csc_rating_s, json_object_get_string(json_object_object_get(csc_data, "status")), csc_author, json_object_get_string(json_object_object_get(csc_data, "sample_width")), json_object_get_string(json_object_object_get(csc_data, "sample_height")), json_object_get_string(json_object_object_get(csc_data, "width")), json_object_get_string(json_object_object_get(csc_data, "height")), csc_size, csc_size_s, csc_format, csc_date, csc_has_children_s, csc_parent_id_s, json_object_get_string(json_object_object_get(csc_data, "md5")), json_object_get_int(json_object_object_get(csc_data, "fav_count")), csc_vote_count, csc_vote_average, csc_source_s);
    string_free(1, csc_author);
}

void sankaku_get_pool(char *csc_info, size_t length, json_object *csc_data, int csc_id) {
    const char *csc_description = json_object_get_string(json_object_object_get(csc_data, "description"));
    char *csc_author = string_escape(json_object_get_string(json_object_object_get(json_object_object_get(csc_data, "author"), "name")), 256);
    const char *csc_rating = json_object_get_string(json_object_object_get(csc_data, "rating"));
    const char *csc_parent_id = json_object_get_string(json_object_object_get(csc_data, "parent_id"));
    float csc_vote_count = json_object_get_int(json_object_object_get(csc_data, "vote_count"));
    float csc_vote_average = json_object_get_int(json_object_object_get(csc_data, "total_score")) / csc_vote_count;
    json_object *cover_post = json_object_object_get(csc_data, "cover_post");
    const char *csc_sample_url = json_object_get_string(json_object_object_get(cover_post, "sample_url"));
    const char *csc_preview_url = json_object_get_string(json_object_object_get(cover_post, "preview_url"));
    const char *csc_file_url = json_object_get_string(json_object_object_get(cover_post, "file_url"));
    float csc_size = json_object_get_int(json_object_object_get(cover_post, "file_size"));
    const char *csc_filetype = json_object_get_string(json_object_object_get(cover_post, "file_type"));
    char *csc_name = string_escape(json_object_get_string(json_object_object_get(csc_data, "name")), 1024);

    char csc_description_s[11264];

    if(csc_description[0]) {
        char *csc_description_encoded = string_escape(csc_description, 2048);
        snprintf(csc_description_s, sizeof(csc_description_s), "<code>%s</code>", csc_description_encoded);
        string_free(1, csc_description_encoded);
    } else {
        string_copy(csc_description_s, "none", sizeof(csc_description));
    }

    char *csc_is_active_s;

    if(json_object_get_boolean(json_object_object_get(csc_data, "is_active")))
        csc_is_active_s = "yes";
    else
        csc_is_active_s = "no";

    char *csc_rating_s;

    if(!strcmp(csc_rating, "s"))
        csc_rating_s = "safe";
    else if(!strcmp(csc_rating, "q"))
        csc_rating_s = "questionable";
    else if(!strcmp(csc_rating, "e"))
        csc_rating_s = "explicit";
    else
        csc_rating_s = "unknown";

    char csc_parent_id_s[32];

    if(csc_parent_id)
        snprintf(csc_parent_id_s, sizeof(csc_parent_id_s), "<code>%s</code>", csc_parent_id);
    else
        string_copy(csc_parent_id_s, "none", sizeof(csc_parent_id_s));

    char *csc_has_children_s;

    if(json_object_get_boolean(json_object_object_get(csc_data, "has_children")))
        csc_has_children_s = "yes";
    else
        csc_has_children_s = "no";

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

    if(csc_vote_average != (float)csc_vote_average)
        csc_vote_average = 0;

    snprintf(csc_info, length, "<a href=\"%s\">&#8203;</a><b>ID:</b> <code>%d</code>\n<b>Description:</b> %s\n<b>Date:</b> <code>%s</code>\n<b>Author:</b> %s\n<b>Active</b>: %s\n<b>Pages:</b> %d\n<b>Rating:</b> %s\n<b>Parent ID:</b> %s\n<b>Has children:</b> %s\n<b>Vote count:</b> %.0f\n<b>Vote average:</b> %.2f\n<b>Cover post ID:</b> <code>%d</code>\n<b>Name:</b> <code>%s</code>", csc_image_url, csc_id, csc_description_s, json_object_get_string(json_object_object_get(csc_data, "created_at")), csc_author, csc_is_active_s, json_object_get_int(json_object_object_get(csc_data, "visible_post_count")), csc_rating_s, csc_parent_id_s, csc_has_children_s, csc_vote_count, csc_vote_average, json_object_get_int(json_object_object_get(cover_post, "id")), csc_name);
    string_free(2, csc_author, csc_name);
}

void sankaku_get_tag(char *csc_tag, size_t length, json_object *csc_data, int csc_id) {
    const char *csc_name_en = json_object_get_string(json_object_object_get(csc_data, "name_en"));
    const char *csc_name_ja = json_object_get_string(json_object_object_get(csc_data, "name_ja"));
    char *csc_name = string_escape(json_object_get_string(json_object_object_get(csc_data, "name")), 1024);
    const char *csc_rating = json_object_get_string(json_object_object_get(csc_data, "rating"));

    char csc_name_en_s[6144];

    if(csc_name_en) {
        char *csc_name_en_encoded = string_escape(csc_name_en, 1024);
        snprintf(csc_name_en_s, sizeof(csc_name_en_s), "<code>%s</code>", csc_name_en_encoded);
        string_free(1, csc_name_en_encoded);
    } else {
        string_copy(csc_name_en_s, "none", sizeof(csc_name_en_s));
    }

    char csc_name_ja_s[6144];

    if(csc_name_ja) {
        char *csc_name_ja_encoded = string_escape(csc_name_ja, 1024);
        snprintf(csc_name_ja_s, sizeof(csc_name_ja_s), "<code>%s</code>", csc_name_ja_encoded);
        string_free(1, csc_name_ja_encoded);
    } else {
        string_copy(csc_name_ja_s, "none", sizeof(csc_name_ja_s));
    }

    char *csc_rating_s;

    if(csc_rating) {
        if(!strcmp(csc_rating, "s"))
            csc_rating_s = "safe";
        else if(!strcmp(csc_rating, "q"))
            csc_rating_s = "questionable";
        else if(!strcmp(csc_rating, "e"))
            csc_rating_s = "explicit";
        else
            csc_rating_s = "unknown";
    } else {
        csc_rating_s = "none";
    }

    char *csc_type_s;

    switch(json_object_get_int(json_object_object_get(csc_data, "type"))) {
        case 0:
            csc_type_s = "general";
            break;
        case 1:
            csc_type_s = "artist";
            break;
        case 2:
            csc_type_s = "studio";
            break;
        case 3:
            csc_type_s = "copyright";
            break;
        case 4:
            csc_type_s = "character";
            break;
        case 5:
            csc_type_s = "genre";
            break;
        case 8:
            csc_type_s = "medium";
            break;
        case 9:
            csc_type_s = "meta";
            break;
        default:
            csc_type_s = "unknown";
            break;
    }

    snprintf(csc_tag, length, "<b>ID:</b> <code>%d</code>\n<b>Eng name:</b> %s\n<b>Jap name:</b> %s\n<b>Type:</b> %s\n<b>Post count:</b> %d\n<b>Book count:</b> %d\n<b>Rating:</b> %s\n<b>Name:</b> <code>%s</code>", csc_id, csc_name_en_s, csc_name_ja_s, csc_type_s, json_object_get_int(json_object_object_get(csc_data, "post_count")), json_object_get_int(json_object_object_get(csc_data, "pool_count")), csc_rating_s, csc_name);
    string_free(1, csc_name);
}
