
#include <SDL.h>
#include <stdio.h>
#include <inttypes.h>
#include <cargs.h>
#include "auth.h"
#include "fit.h"

static struct cag_option options[] = {
    {
        .identifier = 'u',
        .access_letters = "u",
        .access_name = "user",
        .value_name = "USERID",
        .description = "User id of avatar to download",
    },

    {
        .identifier = 'c',
        .access_letters = "c",
        .access_name = "cookie",
        .value_name = "FILE",
        .description = "Cookie file location",
    },

    {
        .identifier = 'o',
        .access_letters = "o",
        .access_name = "output",
        .value_name = "FILE",
        .description = "Output file location"
    },

    {
        .identifier = 'h',
        .access_letters = "h",
        .access_name = "help",
        .description = "Shows command help",
    }
};

typedef struct fit_config {
    const char* cookie_path;
    const char* output_path;
    const char* user_id;
} fit_config_t;

int main(int argc, char** argv) {
    char id;

    fit_config_t config = { .cookie_path = NULL, .output_path = NULL, .user_id = NULL };
    cag_option_context context;
    cag_option_prepare(&context, options, CAG_ARRAY_SIZE(options), argc, argv);

    while (cag_option_fetch(&context)) {
        id = cag_option_get(&context);
        switch (id) {
        case 'o':
            config.output_path = cag_option_get_value(&context);
            break;
        case 'c':
            config.cookie_path = cag_option_get_value(&context);
            break;
        case 'u':
            config.user_id = cag_option_get_value(&context);
            break;
        case 'h':
            printf("Usage: outfittable [OPTION] FILE\n");
            printf("Downloads Roblox avatars to file.\n");
            cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
            return 0;
        default:
            break;
        }
    }

    if (config.user_id != NULL) {
        const char* target = config.user_id;
        errno = 0;
        uintmax_t uid = strtoumax(target, NULL, 0);

        if (errno) {
            fprintf(stderr, "%s\n", strerror(errno));
            return 1;
        }
        if (config.output_path == NULL || strlen(config.output_path) > 256) { // prevent buffer overflow
            char filename[256];
            sprintf_s(filename, 256, "%lld.fit", uid);
            config.output_path = filename;
        }

        FILE *fp;
        if (fopen_s(&fp, config.output_path, "w")) {
            char buff[280];
            sprintf_s(buff, 280,"Failed to write to %s", config.output_path);
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buff, NULL);
            return 1;
        }

        const char* output = auth_getUserWearing(uid);

        if (output == NULL) {
            return 1;
        }

        fprintf(fp, "%s\n", output);

        char buff[1024];
        sprintf_s(buff, 1024, "Wrote avatar for uid %lld to %s", uid, config.output_path);

        free((char*) output);
        fflush(fp);
        fclose(fp);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Success!", buff, NULL);

        return 0;
    }

    if (config.cookie_path == NULL || strlen(config.cookie_path) > 256) {
        config.cookie_path = "cookie.txt";
    }
    if (config.output_path == NULL || strlen(config.output_path) > 256) {
        config.output_path = "curr.fit";
    }

    user_t* user = auth_fromCookie(config.cookie_path);

    if (user == NULL) {
        return 1;
    }

    int index = cag_option_get_index(&context);

    printf("Successfully loaded from cookie. uid: %s\n", user->userId);
    printf("CRSF token for session: ***\n");

    for (int i = index; i < argc; i++) {
        const char* file = argv[i];

        if (strlen(file) > 256) {
            return 1;
        }

        printf("Setting avatar to %s\n", file);

        if (fit_setWearing(user, file)) {
            return 1;
        }
        char buff[1024];
        sprintf(buff, "Successfully changed outfit to %s", file);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Success!", buff, NULL);
        curl_easy_cleanup(user->curl);
        free(user);
        return 0;
    }

    FILE *fp;
    if (fopen_s(&fp, config.output_path, "w")) {
        char buff[1024];
        sprintf(buff, "Failed to write to %s", config.output_path);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buff, NULL);
        return 1;
    }

    const char* output = auth_getWearing(user);

    if (output == NULL) {
        return 1;
    }

    fprintf(fp, "%s\n", output);

    curl_easy_cleanup(user->curl);
    free(user);
    free((char*) output);
    fflush(fp);
    fclose(fp);

    char buff[1024];
    sprintf(buff, "Wrote current avatar to %s", config.output_path);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Success!", buff, NULL);
    return 0;
}
