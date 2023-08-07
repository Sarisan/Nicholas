#include <config/config.h>
#include <curl/curl.h>
#include <debug/debug.h>
#include <json/json.h>
#include <sankaku/api.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

#define TIMEOUT 5L
#define ADDRESS "https://capi-v2.sankakucomplex.com/"
#define AUTH "auth/token"
#define HTTP_JSON "Content-Type: application/json"
#define HTTP_HEADER "Authorization: Bearer %s"
#define REQUEST_LENGHT 1024

#undef curl_easy_setopt

static json_object *sankaku_io(const char *url,
        const char *post_fields, const char *header)
{
    CURL *sankaku = 0;
    struct curl_slist *slist = 0;
    struct string_curl string = {0};
    CURLcode error = 0;
    json_object *data = 0;

    sankaku = curl_easy_init();

    if (!sankaku) {
        debug_log(EMEM, "sankaku_io: %s", debug_message(EMEM));

        goto err;
    }

    slist = curl_slist_append(0, header);

    if (!slist) {
        debug_log(EMEM, "sankaku_io: %s", debug_message(EMEM));

        goto err;
    }

    if (post_fields)
        curl_easy_setopt(sankaku,
                CURLOPT_CUSTOMREQUEST, "POST");
    else
        curl_easy_setopt(sankaku,
                CURLOPT_CUSTOMREQUEST, "GET");
    if (slist)
        curl_easy_setopt(sankaku,
                CURLOPT_HTTPHEADER, slist);
    if (post_fields)
        curl_easy_setopt(sankaku,
                CURLOPT_POSTFIELDS, post_fields);
    curl_easy_setopt(sankaku,
            CURLOPT_TIMEOUT, TIMEOUT);
    curl_easy_setopt(sankaku,
            CURLOPT_URL, url);
    curl_easy_setopt(sankaku,
            CURLOPT_USERAGENT, "Nicholas");
    curl_easy_setopt(sankaku,
            CURLOPT_WRITEDATA, &string);
    curl_easy_setopt(sankaku,
            CURLOPT_WRITEFUNCTION, string_curl_writefunction);

    error = curl_easy_perform(sankaku);

    curl_slist_free_all(slist);
    curl_easy_cleanup(sankaku);

    if (error) {
        debug_log(error, "sankaku_io: %s", curl_easy_strerror(error));
        free(string.string);

        return 0;
    }

    data = json_from_string(string.string);

    free(string.string);

    if (data) {
        const char *error = json_string(data, "error");

        if(error)
            debug_log(EINV, "sankaku_io: %s", error);
    } else {
        debug_log(EDAT, "sankaku_io: %s", debug_message(EDAT));
    }

    return data;

err:
    curl_slist_free_all(slist);
    curl_easy_cleanup(sankaku);

    return 0;
}

int sankaku_authorization(void)
{
    const char *login = config_get_string(SANKAKU_LOGIN);
    const char *password = config_get_string(SANKAKU_PASSWORD);
    json_object *login_field = 0;
    size_t length = 0;
    const char *field = 0;
    char *url = 0;
    json_object *login_object = 0;
    const char *token = 0;
    char *header = 0;

    debug_log(0, "sankaku_authorization: Refreshing access token...");

    if (!login) {
        debug_log(EDAT, "sankaku_authorization: No login specified");

        return EDAT;
    }

    if (!password) {
        debug_log(EDAT, "sankaku_authorization: No password specified");

        return EDAT;
    }

    length = strlen(ADDRESS) + strlen(AUTH) + 1;
    login_field = json_new();

    if (!login_field) {
        debug_log(EMEM, "sankaku_authorization: %s", debug_message(EMEM));

        return EMEM;
    }

    json_add_string(login_field, "login", login);
    json_add_string(login_field, "password", password);

    field = json_to_string(login_field);
    url = malloc(length);

    if(!url) {
        debug_log(EMEM, "sankaku_authorization: %s", debug_message(EMEM));
        json_put(login_field);

        return EMEM;
    }

    snprintf(url, length, "%s%s", ADDRESS, AUTH);

    login_object = sankaku_io(url, field, HTTP_JSON);

    json_put(login_field);
    free(url);

    if (!login_object)
        return EINV;

    token = json_string(login_object, "access_token");

    if (!token) {
        debug_log(EINV, "sankaku_authorization: %s", debug_message(EINV));
        json_put(login_object);

        return EINV;
    }

    header = malloc(strlen(HTTP_HEADER) + strlen(token));

    if (!header) {
        debug_log(EMEM, "sankaku_authorization: %s", debug_message(EMEM));
        json_put(login_object);

        return EMEM;
    }

    sprintf(header, HTTP_HEADER, token);
    config_set_string(SANKAKU_TOKEN, header);
    debug_log(0, "sankaku_authorization: Successful");

    free(header);
    json_put(login_object);

    return 0;
}

__attribute__ ((format (printf, 1, 2)))
json_object *sankaku_request(const char *api_data, ...)
{
    const char *token = config_get_string(SANKAKU_TOKEN);
    va_list args = {0};
    char args_string[REQUEST_LENGHT] = {0};
    size_t length = 0;
    char *url = 0;
    json_object *data = 0;

    if (!token)
        debug_log(EDAT, "sankaku_request: No authorization token found");

    va_start(args, api_data);
    vsnprintf(args_string, REQUEST_LENGHT, api_data, args);
    va_end(args);

    length = strlen(ADDRESS) + strlen(args_string) + 1;
    url = malloc(length);

    if (!url) {
        debug_log(EMEM, "sankaku_request: %s", debug_message(EMEM));

        return 0;
    }

    snprintf(url, length, "%s%s", ADDRESS, args_string);

    data = sankaku_io(url, 0, token);

    return data;
}
