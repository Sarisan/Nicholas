#include <json-c/json_object.h>
#include <time.h>

#define CSC_API_URL "https://capi-v2.sankakucomplex.com"
#define CSC_API_AUTH "auth/token"
#define CSC_POST_URL "https://beta.sankakucomplex.com/post/show"
#define CSC_POOL_URL "https://beta.sankakucomplex.com/books"
#define CSC_DPREVIEW_URL "https://s.sankakucomplex.com/download-preview.png"
#define CSC_TZ (-10)

#define csc_login (*__csc_login())
#define csc_password (*__csc_password())

char **__csc_login();
char **__csc_password();

int csc_auth();
void csc_check(time_t *start);
json_object *csc_request(long timeout, const char *api_data, ...);
void csc_getdate(char *csc_date, size_t size, time_t rawtime);
void bot_csc_post(char *csc_info, size_t length, json_object *csc_data, int csc_id);
void bot_csc_pool(char *csc_info, size_t length, json_object *csc_data, int csc_id);
void bot_csc_tag(char *csc_tag, size_t length, json_object *csc_data, int csc_id);
