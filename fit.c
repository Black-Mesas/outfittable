
#include <stdio.h>
#include <SDL.h>
#include "fit.h"

const char* SET_WEARING_ASSETS = "https://avatar.roblox.com/v2/avatar/set-wearing-assets";
const char* SET_SCALES = "https://avatar.roblox.com/v1/avatar/set-scales";
const char* SET_PLAYER_AVATAR_TYPE = "https://avatar.roblox.com/v1/avatar/set-player-avatar-type";
const char* SET_BODY_COLORS = "https://avatar.roblox.com/v1/avatar/set-body-colors";
const char* REDRAW_THUMBNAIL = "https://avatar.roblox.com/v1/avatar/redraw-thumbnail";

int fit_setWearing (user_t *user, const char *file) {
    FILE *fp;
    if (fopen_s(&fp, file, "rb")) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Avatar error", "Outfit file does not exist", NULL);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);

    printf("Outfit size: %ld bytes\n", len);
    rewind(fp);

    if (len >= ~0u) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Avatar error", "Outfit file too large! (>2147483647 bytes)", NULL);
        return 1;
    }

    char* buff = malloc(len + 1);

    if (buff == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Avatar error", "Out of memory!", NULL);
        return 1;
    }

    fread(buff, len, 1, fp);

    if (ferror(fp)) {
        printf("Encountered error while reading file.\n");
        perror("File error: ");
        fclose(fp);
        free(buff);
        return 1;
    }

    fclose(fp);

    buff[len] = 0;

    json_tokener* tok = json_tokener_new();
    json_object* obj = json_tokener_parse_ex(tok, buff, (int) len);

    if (json_tokener_get_error(tok) != json_tokener_success) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Avatar error", json_tokener_error_desc(json_tokener_get_error(tok)), NULL);
        json_tokener_free(tok);
        free(buff);
        return 1;
    }

    json_tokener_free(tok);
    free(buff);

    json_object* tmp;
    if (json_object_object_get_ex(obj, "assets", &tmp)) {
        json_object *assets = json_object_new_object();
        json_object_object_add(assets, "assets", tmp);
        tmp = auth_requestJson(user, SET_WEARING_ASSETS, json_object_to_json_string(assets));
        if (json_object_is_type(tmp, json_type_object) && json_object_object_get_ex(tmp, "errors", &tmp)) {
            printf("Failed to set currently worn assets.\n");
        }
        json_object_put(assets);
    }
    if (json_object_object_get_ex(obj, "bodyColors", &tmp)) {
        tmp = auth_requestJson(user, SET_BODY_COLORS, json_object_to_json_string(tmp));
        if (json_object_is_type(tmp, json_type_object) && json_object_object_get_ex(tmp, "errors", &tmp)) {
            printf("Failed to set body colors.\n");
        }
    }
    if (json_object_object_get_ex(obj, "scales", &tmp)) {
        tmp = auth_requestJson(user, SET_SCALES, json_object_to_json_string(tmp));
        if (json_object_is_type(tmp, json_type_object) && json_object_object_get_ex(tmp, "errors", &tmp)) {
            printf("Failed to set body scale.\n");
        }
    }
    if (json_object_object_get_ex(obj, "playerAvatarType", &tmp)) {
        json_object *avatarType = json_object_new_object();
        json_object_object_add(avatarType, "playerAvatarType", tmp);

        tmp = auth_requestJson(user, SET_PLAYER_AVATAR_TYPE, json_object_to_json_string(avatarType));
        if (json_object_is_type(tmp, json_type_object) && json_object_object_get_ex(tmp, "errors", &tmp)) {
            printf("Failed to set playerAvatarType.\n");
        }
        json_object_put(avatarType);
    }
    tmp = auth_requestJson(user, REDRAW_THUMBNAIL, "");
    if (json_object_is_type(tmp, json_type_object) && json_object_object_get_ex(tmp, "errors", &tmp)) {
        printf("Failed to redraw thumbnail.\n");
    }

    json_object_put(obj);
    return 0;
}