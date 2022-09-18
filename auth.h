
#ifndef OUTFITTABLE_AUTH_H
#define OUTFITTABLE_AUTH_H

#include <json-c/json.h>
#include <curl/curl.h>

typedef struct user {
    char cookie[1024];
    char userId[16];
    char crsf[13];
    CURL *curl;
} user_t;

json_object* auth_requestJson (user_t *user, const char *url, const char* postData);

user_t* auth_fromCookie ();
const char* auth_getWearing (user_t* user);
const char* auth_getUserWearing (user_t* user, uintmax_t id);

#endif //OUTFITTABLE_AUTH_H
