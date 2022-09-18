
#include "auth.h"
#include <stdio.h>
#include <SDL.h>
#include <curl/curl.h>

const char* GET_AUTHENTICATED = "https://users.roblox.com/v1/users/authenticated";
const char* GET_AVATAR = "https://avatar.roblox.com/v1/avatar";
const char* GET_USER_AVATAR = "https://avatar.roblox.com/v1/users/%ld/avatar";
const char* CRSF_HEADER = "X-CSRF-TOKEN: %s";
const char* CRSF_NODE = "data-token=\"";

typedef struct auth_rjwc {
    json_tokener* tok;
    json_object* curr;
} auth_rjwc_t;

typedef struct auth_rcrsft {
    char token[13];
} auth_rcrsft_t;

static size_t auth_requestJsonWriteCallback (char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realsize = size * nmemb;
    auth_rjwc_t *tmp = (auth_rjwc_t *) userdata;
    json_tokener *tok = tmp->tok;
    tmp->curr = json_tokener_parse_ex(tok, ptr, (int) realsize);

    if (json_tokener_get_error(tok) != json_tokener_continue) {
        return 0;
    }

    return realsize;
}

static size_t auth_requestCrsfTokenWriteCallback (char *orig, size_t size, size_t nmemb, void *userdata) {
    size_t realsize = size * nmemb;
    auth_rcrsft_t *data = (auth_rcrsft_t*) userdata;

    if (realsize <= 12) {
        return realsize;
    }

    char *ptr;
    orig[realsize] = 0;

    for (int i = 0; i < realsize - 12; ++i) {
        ptr = strstr(orig, CRSF_NODE);

        if (ptr != NULL) { // yuck
            memcpy(data->token, &ptr[12], 12);
            data->token[12] = 0;
            return 0;
        }
    }

    return realsize;
}

static char* auth_requestCrsfToken (user_t *user) {
    CURL *curl = user->curl;
    auth_rcrsft_t data;

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, auth_requestCrsfTokenWriteCallback);

    struct curl_slist *list = NULL;
    list = curl_slist_append(list, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9");

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_URL, "https://roblox.com/home");

    char cookie[1040];
    sprintf(cookie, ".ROBLOSECURITY=%s;", user->cookie);

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);

    curl_easy_perform(curl);

    curl_slist_free_all(list);

    char* token = malloc(sizeof(data.token));
    if (token == NULL) {
        printf("Out of memory! Can't allocate for CRSF token.\n");
        return NULL;
    }
    memcpy(token, &data.token, sizeof(data.token));
    return token;
}

json_object* auth_requestJson (user_t *user, const char *url, const char* postData) {
    CURL *curl = user->curl;
    auth_rjwc_t tmp;
    json_tokener *tok = json_tokener_new();
    tmp.tok = tok;

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tmp);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, auth_requestJsonWriteCallback);

    struct curl_slist *list = NULL;

    list = curl_slist_append(list, "Content-Type: application/json;charset=utf-8");
    list = curl_slist_append(list, "Accept: application/json");

    if (*user->crsf != 0) {
        char crsfToken[30];
        sprintf(crsfToken, CRSF_HEADER, user->crsf);
        list = curl_slist_append(list, crsfToken);
    }

    list = curl_slist_append(list, "User-Agent: Outfittable/1.0");

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    if (postData != NULL) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
    }

    char cookie[1040];
    sprintf(cookie, ".ROBLOSECURITY=%s;", user->cookie);

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);

    curl_easy_perform(curl);
    curl_slist_free_all(list);

    if (json_tokener_get_error(tok) != json_tokener_success) {
        json_tokener_free(tok);
        return NULL;
    }

    json_tokener_free(tok);
    return tmp.curr;
}

static int auth_getUserId (user_t *user) {
    json_object* obj = auth_requestJson(user, GET_AUTHENTICATED, NULL);
    if (obj == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Userid error", "Failed to authenticate user.", NULL);
        return 1;
    }

    json_object* t;
    if (json_object_object_get_ex(obj, "id", &t)) {
        const char *ptr = json_object_get_string(t);
        strncpy_s(user->userId, 16, ptr, _TRUNCATE);
        json_object_put(obj);
    } else {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Userid error", "Failed to authenticate user, no id provided.", NULL);
        json_object_put(obj);
        return 1;
    }
    return 0;
}

user_t* auth_fromCookie () {
    user_t* user = malloc(sizeof(user_t));

    if (user == NULL) {
        return NULL;
    }

    *user->crsf = 0;
    user->curl = curl_easy_init();

    if (user->curl == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "cURL Failure", "Failed cURL init.", NULL);
        free(user);
        return NULL;
    }

    {
        FILE* f;
        fopen_s(&f, "cookie.txt", "r");

        if (f == NULL) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "No cookie!", "No cookie.txt! Create a file (same directory, cwd) with your cookie after in it (plain-text).", NULL);
            free(user);
            return NULL;
        }

        fseek(f, 0, SEEK_END);

        long size = ftell(f);
        printf("Cookie size: %ld bytes\n", size);

        if (size >= 1024) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "No cookie!", "Cookie too large.", NULL);
            fclose(f);
            free(user);
            return NULL;
        }

        fseek(f, 0, SEEK_SET);

        char* ptr = user->cookie;

        int c;
        while (!feof(f)) {
            c = fgetc(f);
            *ptr = (char) c;
            ++ptr;
        }
        --ptr; // adds an extra space
        *ptr = '\0';

        fclose(f);
    }

    if (auth_getUserId(user)) {
        free(user);
        return NULL;
    }

    char* crsf = auth_requestCrsfToken(user);

    if (crsf == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Token validation failure", "Failed to validate token.", NULL);
        free(user);
        return NULL;
    }
    memcpy(user->crsf, crsf, sizeof(user->crsf));
    free(crsf);

    return user;
}

const char* auth_getWearing (user_t* user) {
    json_object* obj = auth_requestJson(user, GET_AVATAR, NULL);
    if (obj == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Avatar error", "Failed to get avatar.", NULL);
        return NULL;
    }
    if (json_object_object_get(obj, "emotes") != NULL)
        json_object_object_del(obj, "emotes");
    if (json_object_object_get(obj, "defaultShirtApplied") != NULL)
        json_object_object_del(obj, "defaultShirtApplied");
    if (json_object_object_get(obj, "defaultPantsApplied") != NULL)
        json_object_object_del(obj, "defaultPantsApplied");
    const char* str = _strdup(json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY));
    json_object_put(obj);
    return str;
}

const char* auth_getUserWearing (user_t* user, uintmax_t id) {
    char url[256];
    sprintf_s(url, 256, GET_USER_AVATAR, id);
    json_object* obj = auth_requestJson(user, url, NULL);
    if (obj == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Avatar error", "Failed to get avatar.", NULL);
        return NULL;
    }
    if (json_object_object_get(obj, "emotes") != NULL)
        json_object_object_del(obj, "emotes");
    if (json_object_object_get(obj, "defaultShirtApplied") != NULL)
        json_object_object_del(obj, "defaultShirtApplied");
    if (json_object_object_get(obj, "defaultPantsApplied") != NULL)
        json_object_object_del(obj, "defaultPantsApplied");
    const char* str = _strdup(json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY));
    json_object_put(obj);
    return str;
}
