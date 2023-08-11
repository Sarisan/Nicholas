#include <api/api.h>
#include <config/config.h>
#include <curl/curl.h>
#include <debug/debug.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <string/string.h>

#define ADDRESS "0.0.0.0:8081"
#define SUB_ADDRESS "/bot"
#define HTTP_JSON "Content-Type: application/json"
#define TIMEOUT 5L

#undef curl_easy_setopt

static json_object *api_io(const char *method, json_object *json)
{
    const char *token = config_get_string(BOT_TOKEN);
    const char *server = config_get_string(BOT_SERVER);
    size_t length = 0;
    char *url = 0;
    CURL *request = 0;
    struct curl_slist *slist = 0;
    struct string_curl string = {0};
    CURLcode error = 0;
    json_object *data = 0;

    if (!token) {
        debug_log(EDAT, "api_io: No access token specified");

        return 0;
    }

    if (!server)
        server = ADDRESS;

    length = strlen(server)
            + strlen(SUB_ADDRESS)
            + strlen(token)
            + strlen(method)
            + 1;

    url = malloc(length + 1);

    if (!url) {
        debug_log(EMEM, "api_io: %s", debug_message(EMEM));

        goto err;
    }

    sprintf(url, "%s%s%s/%s", server, SUB_ADDRESS, token, method);

    request = curl_easy_init();

    if (!request) {
        debug_log(EMEM, "api_io: %s", debug_message(EMEM));

        goto err;
    }

    slist = curl_slist_append(0, HTTP_JSON);

    if (!slist) {
        debug_log(EMEM, "api_io: %s", debug_message(EMEM));

        goto err;
    }

    if (json) {
        curl_easy_setopt(request,
                    CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(request,
                    CURLOPT_HTTPHEADER, slist);
        curl_easy_setopt(request,
                    CURLOPT_POSTFIELDS, json_to_string(json));
    } else {
        curl_easy_setopt(request,
                CURLOPT_CUSTOMREQUEST, "GET");
    }
    curl_easy_setopt(request,
            CURLOPT_TIMEOUT, TIMEOUT);
    curl_easy_setopt(request,
                CURLOPT_URL, url);
    curl_easy_setopt(request,
            CURLOPT_WRITEDATA, &string);
    curl_easy_setopt(request,
            CURLOPT_WRITEFUNCTION, string_curl);

    error = curl_easy_perform(request);

    curl_slist_free_all(slist);
    curl_easy_cleanup(request);
    free(url);

    if (error) {
        debug_log(error, "api_io: %s", curl_easy_strerror(error));
        free(string.string);

        return 0;
    }

    data = json_from_string(string.string);

    free(string.string);

    if (data) {
        int error_code = json_int(data, "error_code");
        const char *description = json_string(data, "description");

        if (error_code && description)
            debug_log(error_code, "api_io: %s", description);
    } else {
        debug_log(EDAT, "api_io: %s", debug_message(EDAT));
    }

    return data;

err:
    free(url);
    curl_easy_cleanup(request);
    curl_slist_free_all(slist);

    return 0;
}

json_object *api_get(const char *method, json_object *json)
{
    return api_io(method, json);
}

int api_post(const char *method, json_object *json)
{
    json_object *data = api_io(method, json);
    int ret = 0;

    if (!data)
        ret = debug_error;

    json_put(data);

    return ret;
}
