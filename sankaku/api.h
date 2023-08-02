#include <json-c/json_object.h>
#include <time.h>

#define SANKAKU_POST_URL "https://sankaku.app/post/show"
#define SANKAKU_POOL_URL "https://sankaku.app/books"
#define SANKAKU_DPREVIEW_URL "https://s.sankakucomplex.com/download-preview.png"

int sankaku_authorization(json_object *config);
json_object *sankaku_request(json_object *config, const char *api_data, ...);
void sankaku_get_date(char *csc_date, size_t size, time_t rawtime);
void sankaku_get_post(char *csc_info, size_t length, json_object *csc_data, int csc_id);
void sankaku_get_pool(char *csc_info, size_t length, json_object *csc_data, int csc_id);
void sankaku_get_tag(char *csc_tag, size_t length, json_object *csc_data, int csc_id);
