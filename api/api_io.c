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

static json_object *api_io(json_object *config,
    const char *method, json_object *json)
{
    const char *bot_token = config_get_string(config, BOT_TOKEN);
    const char *bot_server = config_get_string(config, BOT_SERVER);
    size_t length = 0;
    char *api_url = 0;
    CURL *send_request = 0;
    struct curl_slist *slist_json = 0;
    struct string_curl string = {0};
    CURLcode error = 0;
    json_object *data = 0;

    if (!bot_token) {
        debug_log(EDAT, "api_io: No access token specified");

        return 0;
    }

    if (!bot_server)
        bot_server = ADDRESS;

    length = strlen(bot_server)
            + strlen(SUB_ADDRESS)
            + strlen(bot_token)
            + strlen(method)
            + 2;

    api_url = malloc(length);

    if (!api_url) {
        debug_log(EMEM, "api_io: %s", debug_message(EMEM));

        goto err;
    }

    sprintf(api_url, "%s%s%s/%s", bot_server, SUB_ADDRESS, bot_token, method);

    send_request = curl_easy_init();

    if (!send_request) {
        debug_log(EMEM, "api_io: %s", debug_message(EMEM));

        goto err;
    }

    slist_json = curl_slist_append(0, HTTP_JSON);

    if (!slist_json) {
        debug_log(EMEM, "api_io: %s", debug_message(EMEM));

        goto err;
    }

    if (json) {
        curl_easy_setopt(send_request,
                    CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(send_request,
                    CURLOPT_HTTPHEADER, slist_json);
        curl_easy_setopt(send_request,
                    CURLOPT_POSTFIELDS, json_to_string(json));
    } else {
        curl_easy_setopt(send_request,
                CURLOPT_CUSTOMREQUEST, "GET");
    }
    curl_easy_setopt(send_request,
            CURLOPT_TIMEOUT, TIMEOUT);
    curl_easy_setopt(send_request,
                CURLOPT_URL, api_url);
    curl_easy_setopt(send_request,
            CURLOPT_WRITEDATA, &string);
    curl_easy_setopt(send_request,
            CURLOPT_WRITEFUNCTION, string_curl_writefunction);

    error = curl_easy_perform(send_request);

    curl_slist_free_all(slist_json);
    curl_easy_cleanup(send_request);
    free(api_url);

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

        if (error_code && description) {
            debug_log(error_code, "api_io: %s", description);
        }
    } else {
        debug_log(EDAT, "api_io: %s", debug_message(EDAT));
    }

    return data;

err:
    free(api_url);
    curl_easy_cleanup(send_request);
    curl_slist_free_all(slist_json);

    return 0;
}

json_object *api_get(json_object *config,
    const char *method, json_object *json)
{
    return api_io(config, method, json);
}

int api_post(json_object *config, const char *method, json_object *json)
{
    json_object *data = api_io(config, method, json);
    int ret = 0;

    if (!data)
        ret = debug_error_code;

    json_put(data);

    return ret;
}
