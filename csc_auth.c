#include <core.h>
#include <curl/curl.h>
#include <errno.h>
#include <json-c/json_tokener.h>
#include <pthread.h>
#include <string.h>

char csc_authorization_header[512];

void *csc_authorization() { 
    json_object *login_object = json_object_new_object();

    extern char *login, *password;

    json_object_object_add(login_object, "login", json_object_new_string(login));
    json_object_object_add(login_object, "password", json_object_new_string(password));

    CURL *authorize = curl_easy_init();
    struct curl_slist *slist_json = curl_slist_append(0, "Content-Type: application/json");
    struct bot_curl_string string = {0};

    curl_easy_setopt(authorize, CURLOPT_URL, "https://capi-v2.sankakucomplex.com/auth/token");
    curl_easy_setopt(authorize, CURLOPT_POSTFIELDS, json_object_to_json_string(login_object));
    curl_easy_setopt(authorize, CURLOPT_HTTPHEADER, slist_json);
    curl_easy_setopt(authorize, CURLOPT_USERAGENT, "Nicholas");
    curl_easy_setopt(authorize, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(authorize, CURLOPT_WRITEFUNCTION, bot_curl_writefunction);
    curl_easy_setopt(authorize, CURLOPT_WRITEDATA, &string);
    curl_easy_perform(authorize);

    curl_slist_free_all(slist_json);
    curl_easy_cleanup(authorize);
    json_object_put(login_object);

    if(!string.string) {
        bot_log(EINVAL, "csc_authorization: no answer from API\n");
        return (void *)1;
    }

    json_object *login_json = json_tokener_parse(string.string);
    bot_free(1, string.string);

    if(!json_object_get_boolean(json_object_object_get(login_json, "success"))) {
        const char *error = json_object_get_string(json_object_object_get(login_json, "error"));

        if(error)
            bot_log(EINVAL, "csc_authorization: %s\n", error);
        else
            bot_log(EINVAL, "csc_authorization: unknown error\n");

        json_object_put(login_json);
        return (void *)1;
    }

    snprintf(csc_authorization_header, sizeof(csc_authorization_header), "Authorization: Bearer %s", json_object_get_string(json_object_object_get(login_json, "access_token")));
    bot_log(0, "csc_authorization: authorized successfully\n");

    json_object_put(login_json);
    return 0;
}

int csc_auth() {
    pthread_t auth_thread;
    void *auth;

    pthread_create(&auth_thread, 0, csc_authorization, 0);
    pthread_join(auth_thread, &auth);

    if(auth)
        return -EINVAL;

    return 0;
}

void csc_check(time_t *start) {
    time_t check = time(0);

    if(difftime(check, *start) >= 72000) {
        pthread_t auth_thread;

        pthread_create(&auth_thread, 0, csc_authorization, 0);
        pthread_detach(auth_thread);
        
        *start = check;
    }
}
