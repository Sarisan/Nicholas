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

static json_object *sankaku_io(const char *sankaku_url, const char *post_fields,
                        const char *http_header)
{
    CURL *sankaku = 0;
    struct curl_slist *slist = 0;
    struct string_curl string = {0};
    CURLcode error = 0;
    json_object *sankaku_json = 0;

    sankaku = curl_easy_init();

    if (!sankaku) {
        debug_log(EMEM, "sankaku_io: %s", debug_message(EMEM));

        goto err;
    }

    slist = curl_slist_append(0, http_header);

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
            CURLOPT_URL, sankaku_url);
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

    sankaku_json = json_from_string(string.string);

    free(string.string);

    if (sankaku_json) {
        const char *error = json_string(sankaku_json, "error");

        if(error)
            debug_log(EINV, "sankaku_io: %s", error);
    } else {
        debug_log(EDAT, "sankaku_io: %s", debug_message(EDAT));
    }

    return sankaku_json;

err:
    curl_slist_free_all(slist);
    curl_easy_cleanup(sankaku);

    return 0;
}

int sankaku_authorization(json_object *config)
{
    const char *login = config_get_string(config, SANKAKU_LOGIN);
    const char *password = config_get_string(config, SANKAKU_PASSWORD);
    json_object *login_object = 0;
    size_t length = 0;
    const char *login_field = 0;
    char *sankaku_url = 0;
    json_object *login_json = 0;
    const char *access_token = 0;
    char *http_header = 0;

    debug_log(0, "sankaku_authorization: Refreshing access token...");

    if (!login) {
        debug_log(EDAT, "sankaku_authorization: No login specified");

        return EDAT;
    }

    if (!password) {
        debug_log(EDAT, "sankaku_authorization: No password specified");

        return EDAT;
    }

    length = strlen(ADDRESS)
            + strlen(AUTH)
            + 1;
    login_object = json_new();

    if (!login_object) {
        debug_log(EMEM, "sankaku_authorization: %s", debug_message(EMEM));

        return EMEM;
    }

    json_add_string(login_object, "login", login);
    json_add_string(login_object, "password", password);

    login_field = json_to_string(login_object);
    sankaku_url = malloc(length);

    if(!sankaku_url) {
        debug_log(EMEM, "sankaku_authorization: %s", debug_message(EMEM));
        json_put(login_object);

        return EMEM;
    }

    snprintf(sankaku_url, length, "%s%s", ADDRESS, AUTH);

    login_json = sankaku_io(sankaku_url, login_field, HTTP_JSON);

    json_put(login_object);
    free(sankaku_url);

    access_token = json_string(login_json, "access_token");

    if (!access_token) {
        debug_log(EINV, "sankaku_authorization: %s", debug_message(EINV));
        json_put(login_json);

        return EINV;
    }

    http_header = malloc(strlen(HTTP_HEADER) + strlen(access_token));

    if (!http_header) {
        debug_log(EMEM, "sankaku_authorization: %s", debug_message(EMEM));
        json_put(login_json);

        return EMEM;
    }

    sprintf(http_header, HTTP_HEADER, access_token);
    config_set_string(config, SANKAKU_TOKEN, http_header);
    debug_log(0, "sankaku_authorization: Successful");

    free(http_header);
    json_put(login_json);

    return 0;
}

__attribute__ ((format (printf, 2, 3)))
json_object *sankaku_request(json_object *config, const char *api_data, ...)
{
    const char *token = config_get_string(config, SANKAKU_TOKEN);
    va_list args = {0};
    char api_data_args[REQUEST_LENGHT] = {0};
    size_t length = 0;
    char *sankaku_url = 0;
    json_object *data = 0;

    if (!token)
        debug_log(EDAT, "sankaku_request: No authorization token found");

    va_start(args, api_data);
    vsnprintf(api_data_args, REQUEST_LENGHT, api_data, args);
    va_end(args);

    length = strlen(ADDRESS) + strlen(api_data_args) + 1;
    sankaku_url = malloc(length);

    if (!sankaku_url) {
        debug_log(EMEM, "sankaku_request: %s", debug_message(EMEM));

        return 0;
    }

    snprintf(sankaku_url, length, "%s%s", ADDRESS, api_data_args);

    data = sankaku_io(sankaku_url, 0, token);

    return data;
}
