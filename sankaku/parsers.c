

#include <json/json.h>
#include <sankaku/sankaku.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>
#include <time.h>

#define MEDIA_SIZE 20971520
#define INTEGER_LENGTH 20
#define UPLOADER_LENGTH 256
#define DESCRIPTION_LENGTH 2048
#define NAME_LENGTH 1024

#define TYPE_GIF 0
#define TYPE_JPEG 1
#define TYPE_MP4 2
#define TYPE_PNG 3
#define TYPE_WEBM 4
#define TYPE_SWF 5

#define SANKAKU_NONE "None"
#define SANKAKU_UNKNOWN "Unknown"
#define SANKAKU_TRUE "Yes"
#define SANKAKU_FALSE "No"

#define CODE_FORMAT "<code>%s</code>"

#define RATING_SAFE "Safe"
#define RATING_QUESTIONABLE "Questionable"
#define RATING_EXPLICIT "Explicit"

#define SIZE_LENGTH 10
#define SIZE_GIB 1073741824
#define SIZE_MIB 1048576
#define SIZE_KIB 1024

#define DATE_LENGTH 16
#define DATE_FORMAT "<code>%Y-%m-%d %R</code>"
#define DATE_TZ (-10)

#define VOTE_FORMAT "%.2f (%.0f)"

#define POST_TEMPLATE "<a href=\"%s\">&#8203;</a>" \
    "<b>ID:</b> <code>%lu</code>\n<b>Rating:</b> %s\n<b>Uploader:</b> %s\n" \
    "<b>Resolution:</b> %lux%lu\n<b>Size:</b> %s\n<b>Type:</b> %s\n" \
    "<b>Date:</b> %s\n<b>Has Children:</b> %s\n<b>MD5:</b> %s\n" \
    "<b>Parent ID:</b> %s\n<b>Favorites:</b> %lu\n" \
    "<b>Vote Average:</b> %s\n<b>Source:</b> %s"

#define BOOK_TEMPLATE "<a href=\"%s\">&#8203;</a>" \
    "<b>ID:</b> <code>%lu</code>\n<b>Description:</b> %s\n<b>Date:</b> %s\n" \
    "<b>Uploader:</b> %s\n<b>Pages:</b> %lu\n<b>Rating:</b> %s\n" \
    "<b>Parent ID:</b> %s\n<b>Has Children:</b> %s\n<b>Favorites:</b> %lu\n" \
    "<b>Vote Average:</b> %s\n<b>Cover Post ID:</b> %s\n<b>Name:</b> %s"

#define TAG_TEMPLATE "<b>ID:</b> <code>%lu</code>\n<b>Name EN:</b> %s\n" \
    "<b>Name JA:</b> %s\n<b>Type:</b> %s\n<b>Posts:</b> %lu\n" \
    "<b>Books:</b> %lu\n<b>Rating:</b> %s\n<b>Name:</b> %s"

struct data_type
{
    const size_t id;
    const char *type;
    const char *name;
};

static const struct data_type file_types[] =
{
    {TYPE_GIF, "image/gif", "GIF"},
    {TYPE_JPEG, "image/jpeg", "JPEG"},
    {TYPE_MP4, "video/mp4", "MP4"},
    {TYPE_PNG, "image/png", "PNG"},
    {TYPE_WEBM, "video/webm", "WEBM"},
    {TYPE_SWF, SANKAKU_NONE, "SWF"}
};

static const struct data_type tag_types[] =
{
    {0, 0, "General"},
    {1, 0, "Artist"},
    {2, 0, "Studio"},
    {3, 0, "Copyright"},
    {4, 0, "Character"},
    {5, 0, "Genre"},
    {8, 0, "Medium"},
    {9, 0, "Meta"}
};

const char *sankaku_string(const char *string)
{
    if (string) {
        if (string[0])
            return string;
    }

    return SANKAKU_NONE;
}

char *sankaku_escape(json_object *data, const char *key, size_t max_length)
{
    const char *string = json_string(data, key);

    if (string) {
        if (string[0])
            return string_escape(string, max_length);
    }

    return 0;
}

char *sankaku_code(json_object *data, const char *key, size_t max_length)
{
    char *string = sankaku_escape(data, key, max_length);
    size_t length = 0;
    char *code = 0;

    if (string)
        length = strlen(CODE_FORMAT) + strlen(string);
    else
        length = strlen(SANKAKU_NONE);

    code = malloc(length + 1);

    if (!code)
        goto out;

    if (string)
        sprintf(code, CODE_FORMAT, string);
    else
        sprintf(code, SANKAKU_NONE);

out:
    free(string);

    return code;
}

const char *sankaku_image(json_object *data)
{
    const char *sample_url = json_string(data, "sample_url");
    const char *preview_url = json_string(data, "preview_url");
    const char *file_url = json_string(data, "file_url");
    size_t size = json_uint64(data, "file_size");
    const char *type = sankaku_string(json_string(data, "file_type"));

    for (size_t arr = 0; file_types[arr].name; arr++) {
        if (!strcmp(file_types[arr].type, type)) {
            switch (file_types[arr].id) {
                case TYPE_GIF:
                case TYPE_MP4:
                    if (size <= MEDIA_SIZE) {
                        if (file_url)
                            return file_url;
                        else
                            return SANKAKU_DPREVIEW_URL;
                    } else {
                        if (preview_url)
                            return preview_url;
                        else
                            return SANKAKU_DPREVIEW_URL;
                    }
                case TYPE_JPEG:
                case TYPE_PNG:
                    if (sample_url)
                        return sample_url;
                    else
                        return SANKAKU_DPREVIEW_URL;
                case TYPE_WEBM:
                    if (preview_url)
                        return preview_url;
                    else
                        return SANKAKU_DPREVIEW_URL;
                case TYPE_SWF:
                    return SANKAKU_DPREVIEW_URL;
                default:
                    return SANKAKU_DPREVIEW_URL;
            }
        }
    }

    return SANKAKU_DPREVIEW_URL;
}

const char *sankaku_rating(json_object *data)
{
    const char *rating = json_string(data, "rating");

    if (!rating)
        return SANKAKU_NONE;

    switch (rating[0]) {
        case 's':
            return RATING_SAFE;
            break;
        case 'q':
            return RATING_QUESTIONABLE;
            break;
        case 'e':
            return RATING_EXPLICIT;
            break;
        default:
            return SANKAKU_UNKNOWN;
    }
}

char *sankaku_size(json_object *data)
{
    double size = json_double(data, "file_size");
    char *string = malloc(INTEGER_LENGTH + SIZE_LENGTH + 1);

    if (!string)
        return 0;

    if(size > SIZE_GIB)
        sprintf(string, "%.2f GiB", size / SIZE_GIB);
    else if(size > SIZE_MIB)
        sprintf(string, "%.2f MiB", size / SIZE_MIB);
    else if(size > SIZE_KIB)
        sprintf(string, "%.2f KiB", size / SIZE_KIB);
    else
        sprintf(string, "%.0f B", size);

    return string;
}

const char *sankaku_file_type(json_object *data)
{
    const char *type = sankaku_string(json_string(data, "file_type"));

    for (size_t arr = 0; file_types[arr].name; arr++)
        if (!strcmp(file_types[arr].type, type))
            return file_types[arr].name;

    return SANKAKU_UNKNOWN;
}

char *sankaku_date(json_object *data)
{
    time_t rawtime = json_int(data, "created_at.s") + 3600 * DATE_TZ;
    struct tm *date = gmtime(&rawtime);
    size_t length = strlen(DATE_FORMAT) + DATE_LENGTH + 1;
    char *string = 0;

    string = malloc(length);

    if (!string)
        return 0;

    strftime(string, length, DATE_FORMAT, date);

    return string;
}

const char *sankaku_children(json_object *data)
{
    bool children = json_boolean(data, "has_children");

    if (children)
        return SANKAKU_TRUE;
    else
        return SANKAKU_FALSE;
}

char *sankaku_vote(json_object *data)
{
    double count = json_double(data, "vote_count");
    double score = json_double(data, "total_score");
    size_t length = 0;
    char *string = 0;

    if (count)
        length = INTEGER_LENGTH * 2 + strlen(VOTE_FORMAT);
    else
        length = strlen(SANKAKU_NONE);

    string = malloc(length + 1);

    if (!string)
        return 0;

    if (count)
        sprintf(string, VOTE_FORMAT, score / count, count);
    else
        sprintf(string, SANKAKU_NONE);

    return string;
}

const char *sankaku_source(json_object *data)
{
    const char *source = json_string(data, "source");

    if (source)
        if (source[0])
            return source;
        else
            return SANKAKU_NONE;
    else
        return SANKAKU_NONE;
}

const char *sankaku_tag_type(json_object *data)
{
    size_t id = json_uint64(data, "type");

    for (size_t arr = 0; tag_types[arr].name; arr++)
        if (tag_types[arr].id == id)
            return tag_types[arr].name;

    return SANKAKU_UNKNOWN;
}

char *sankaku_post(json_object *data)
{
    const char *image = sankaku_image(data);
    size_t id = json_uint64(data, "id");
    const char *rating = sankaku_rating(data);
    char *raw_author = sankaku_escape(data, "author.name", UPLOADER_LENGTH);
    const char *author = sankaku_string(raw_author);
    size_t width = json_uint64(data, "width");
    size_t height = json_uint64(data, "height");
    char *raw_size = sankaku_size(data);
    const char *size = sankaku_string(raw_size);
    const char *type = sankaku_file_type(data);
    char *raw_date = sankaku_date(data);
    const char *date = sankaku_string(raw_date);
    const char *children = sankaku_children(data);
    char *raw_md5 = sankaku_code(data, "md5", 0);
    const char *md5 = sankaku_string(raw_md5);
    char *raw_parent = sankaku_code(data, "parent_id", 0);
    const char *parent = sankaku_string(raw_parent);
    size_t fav_count = json_uint64(data, "fav_count");
    char *raw_vote = sankaku_vote(data);
    const char *vote = sankaku_string(raw_vote);
    const char *source = sankaku_source(data);
    size_t length = strlen(POST_TEMPLATE);
    char *post = 0;

    length += strlen(image)
            + INTEGER_LENGTH
            + strlen(rating)
            + strlen(author)
            + INTEGER_LENGTH
            + INTEGER_LENGTH
            + strlen(size)
            + strlen(type)
            + strlen(date)
            + strlen(children)
            + strlen(md5)
            + strlen(parent)
            + INTEGER_LENGTH
            + strlen(vote)
            + strlen(source);

    post = malloc(length + 1);

    if (!post)
        goto out;

    sprintf(post, POST_TEMPLATE, image, id, rating, author, width, height,
            size, type, date, children, md5, parent, fav_count, vote, source);

out:
    string_free(5, raw_author, raw_size, raw_date, raw_md5, raw_parent);

    return post;
}

char *sankaku_book(json_object *data)
{
    json_object *cover_post = json_get(data, "cover_post");
    const char *image = sankaku_image(cover_post);
    size_t id = json_uint64(data, "id");
    char *raw_desc = sankaku_escape(data, "description", DESCRIPTION_LENGTH);
    const char *description = sankaku_string(raw_desc);
    char *raw_date = sankaku_code(data, "created_at", 0);
    const char *date = sankaku_string(raw_date);
    char *raw_author = sankaku_escape(data, "author.name", UPLOADER_LENGTH);
    const char *author = sankaku_string(raw_author);
    size_t page_count = json_uint64(data, "visible_post_count");
    const char *rating = sankaku_rating(data);
    char *raw_parent = sankaku_code(data, "parent_id", 0);
    const char *parent = sankaku_string(raw_parent);
    const char *children = sankaku_children(data);
    size_t fav_count = json_uint64(data, "fav_count");
    char *raw_vote = sankaku_vote(data);
    const char *vote = sankaku_string(raw_vote);
    char *raw_cover_id = sankaku_code(cover_post, "id", 0);
    const char *cover_id = sankaku_string(raw_cover_id);
    char *raw_name = sankaku_code(data, "name", NAME_LENGTH);
    const char *name = sankaku_string(raw_name);
    size_t length = strlen(BOOK_TEMPLATE);
    char *book = 0;

    length += strlen(image)
            + INTEGER_LENGTH
            + strlen(description)
            + strlen(date)
            + strlen(author)
            + INTEGER_LENGTH
            + strlen(rating)
            + strlen(parent)
            + strlen(children)
            + INTEGER_LENGTH
            + strlen(vote)
            + strlen(cover_id)
            + strlen(name);

    book = malloc(length + 1);

    if (!book)
        goto out;

    sprintf(book, BOOK_TEMPLATE, image, id, description, date, author,
            page_count, rating, parent, children,fav_count, vote, cover_id,
            name);

out:
    string_free(7, raw_desc, raw_date, raw_author,
            raw_parent, raw_vote, raw_cover_id, raw_name);

    return book;
}

char *sankaku_tag(json_object *data)
{
    size_t id = json_uint64(data, "id");
    char *raw_name_en = sankaku_code(data, "name_en", NAME_LENGTH);
    const char *name_en = sankaku_string(raw_name_en);
    char *raw_name_ja = sankaku_code(data, "name_ja", NAME_LENGTH);
    const char *name_ja = sankaku_string(raw_name_ja);
    const char *type = sankaku_tag_type(data);
    size_t post_count = json_uint64(data, "post_count");
    size_t pool_count = json_uint64(data, "pool_count");
    const char *rating = sankaku_rating(data);
    char *raw_name = sankaku_code(data, "name", NAME_LENGTH);
    const char *name = sankaku_string(raw_name);
    size_t length = strlen(TAG_TEMPLATE);
    char *tag = 0;

    length += INTEGER_LENGTH
            + strlen(name_en)
            + strlen(name_ja)
            + strlen(type)
            + INTEGER_LENGTH
            + INTEGER_LENGTH
            + strlen(rating)
            + strlen(name);

    tag = malloc(length + 1);

    if (!tag)
        goto out;

    sprintf(tag, TAG_TEMPLATE, id, name_en, name_ja, type, post_count,
            pool_count, rating, name);

out:
    string_free(3, raw_name_en, raw_name_ja, raw_name);

    return tag;
}
