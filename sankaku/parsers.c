

#include <json/json.h>
#include <sankaku/api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

#define RATING_SAFE "Safe"
#define RATING_QUESTIONABLE "Questionable"
#define RATING_EXPLICIT "Explicit"
#define RATING_UNKNOWN "Unknown"
#define SIZE_LENGTH 10
#define SIZE_GIB 1073741824
#define SIZE_MIB 1048576
#define SIZE_KIB 1024
#define DATE_LENGTH 16
#define DATE_TZ (-10)
#define POST_TEMPLATE "<a href=\"%s\">&#8203;</a>" \
    "<b>ID:</b> <code>%d</code>\n<b>Rating:</b> %s\n<b>Status:</b> %s\n" \
    "<b>Author:</b> %s\n<b>Sample resolution:</b> %sx%s\n" \
    "<b>Resolution:</b> %sx%s\n<b>Size:</b> <code>%.0f</code> bytes %s\n" \
    "<b>Type:</b> %s\n<b>Date:</b> <code>%s</code>\n<b>Has children:</b> %s\n" \
    "<b>Parent ID:</b> %s\n<b>MD5:</b> <code>%s</code>\n<b>Fav count:</b> %d\n" \
    "<b>Vote count:</b> %.0f\n<b>Vote average:</b> %.2f\n<b>Source:</b> %s"

static char *sankaku_rating(const char *rating)
{
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
            return RATING_UNKNOWN;
    }
}

static char *sankaku_size(float size)
{
    char *size_string = malloc(SIZE_LENGTH + 1);

    if (!size_string)
        return 0;

    if(size > SIZE_GIB)
        snprintf(size_string, SIZE_LENGTH, "%.2f GiB", size / SIZE_GIB);
    else if(size > SIZE_MIB)
        snprintf(size_string, SIZE_LENGTH, "%.2f MiB", size / SIZE_MIB);
    else if(size > SIZE_KIB)
        snprintf(size_string, SIZE_LENGTH, "%.2f KiB", size / SIZE_KIB);
    else
        size_string[0] = 0;

    return size_string;
}

char *sankaku_date(time_t rawtime)
{
    char *date_string = malloc(DATE_LENGTH + 1);
    struct tm *date = 0;

    if (!date_string)
        return 0;

    rawtime += 3600 * DATE_TZ;
    date = gmtime(&rawtime);

    strftime(date_string, DATE_LENGTH + 1, "%Y-%m-%d %R", date);

    return date_string;
}

/*char *sankaku_post(json_object *data)
{
    const char *rating = json_string(data, "rating");
    char *author = string_escape(json_string(data, "author.name"), 256);
    const char *sample_url = json_string(data, "sample_url");
    const char *preview_url = json_string(data, "preview_url");
    const char *file_url = json_string(data, "file_url");
    float size = json_int(data, "file_size");
    const char *filetype = json_string(data, "file_type");
    time_t rawtime = json_int(data, "created_at.s");
    const char *parent_id = json_string(data, "parent_id");
    float vote_count = json_int(data, "vote_count");
    float vote_average = json_int(data, "total_score") / vote_count;
    const char *source = json_string(data, "source");
    size_t length = strlen(POST_TEMPLATE);
} */
