#include <json/json.h>

#define SANKAKU_POST_URL "https://sankaku.app/post/show"
#define SANKAKU_POOL_URL "https://sankaku.app/books"
#define SANKAKU_DPREVIEW_URL "https://s.sankakucomplex.com/download-preview.png"

int sankaku_authorization(void);
json_object *sankaku_request(const char *api_data, ...);
const char *sankaku_string(const char *string);
char *sankaku_escape(json_object *data, const char *key, size_t max_length);
char *sankaku_code(json_object *data, const char *key, size_t max_length);
const char *sankaku_image(json_object *data);
const char *sankaku_rating(json_object *data);
char *sankaku_size(json_object *data);
const char *sankaku_file_type(json_object *data);
char *sankaku_date(json_object *data);
const char *sankaku_children(json_object *data);
char *sankaku_vote(json_object *data);
const char *sankaku_source(json_object *data);
const char *sankaku_tag_type(json_object *data);
char *sankaku_date(json_object *data);
char *sankaku_post(json_object *data);
char *sankaku_book(json_object *data);
char *sankaku_tag(json_object *data);
