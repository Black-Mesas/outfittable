
#include <SDL.h>
#include <stdio.h>
#include "auth.h"
#include "fit.h"
#include <inttypes.h>

int main(int argc, char** argv) {
    user_t* user = auth_fromCookie();

    if (user == NULL) {
        return 1;
    }

    printf("Successfully loaded from cookie. uid: %s\n", user->userId);
    printf("CRSF token for session: %s\n", user->crsf);

    if (argc == 1) {
        FILE *fp;
        if (fopen_s(&fp, "curr.fit", "w")) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to write to curr.fit", NULL);
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
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Success!", "Wrote current avatar to curr.fit", NULL);
        return 0;
    }

    if (argc > 2) {
        for (int i = 1; i < argc; ++i) {
            const char* arg = argv[i];
            if (!strcmp(arg, "-u")) {
                const char* target = argv[++i];
                errno = 0;
                uintmax_t id = strtoumax(target, NULL, 0);

                if (errno) {
                    fprintf(stderr, "%s\n", strerror(errno));
                    return 1;
                }
                char filename[256];
                sprintf_s(filename, 256, "%lld.fit", id);

                FILE *fp;
                if (fopen_s(&fp, filename, "w")) {
                    char buff[1024];
                    sprintf_s(buff, 1024,"Failed to write to %s", filename);
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buff, NULL);
                    return 1;
                }

                const char* output = auth_getUserWearing(user, id);

                if (output == NULL) {
                    return 1;
                }

                fprintf(fp, "%s\n", output);

                char buff[1024];
                sprintf_s(buff, 1024, "Wrote avatar for uid %lld to %s", id, filename);

                curl_easy_cleanup(user->curl);
                free(user);
                free((char*) output);
                fflush(fp);
                fclose(fp);
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Success!", buff, NULL);

                return 0;
            } else if (!strcmp(arg, "--")) {
                break;
            }
        }
        return 0;
    }

    const char* target = argv[1];

    printf("Setting avatar to %s\n", target);

    if (fit_setWearing(user, target)) {
        return 1;
    }
    char buff[1024];
    sprintf(buff, "Successfully changed outfit to %s", target);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Success!", buff, NULL);
    curl_easy_cleanup(user->curl);
    free(user);
    return 0;
}
