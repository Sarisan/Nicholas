#include <json-c/json_object.h>
#include <time.h>

#define CSC_POST_URL "https://beta.sankakucomplex.com/post/show"
#define CSC_POOL_URL "https://beta.sankakucomplex.com/books"

int csc_auth();

void csc_check(time_t *start);

json_object *csc_request(long timeout, const char *api_data, ...);

void bot_csc_post(char *csc_info, size_t length, json_object *csc_data, int csc_id);

void bot_csc_pool(char *csc_info, size_t length, json_object *csc_data, int csc_id);

void bot_csc_tag(char *csc_tag, size_t length, json_object *csc_data, int csc_id);
