#include <cstdio>
#include <cstring>
#include <pwd.h>
#include <shadow.h>
#include <unistd.h>
#include "mSysLogin.h"

#if defined(DEBUG) || defined(_DEBUG)
#define trace(x, ...) fprintf(stderr, "%s " x, __func__, ##__VA_ARGS__)
#else
#define trace(x, ...)
#endif

status_t try_login(const char* in_username, const char* in_password)
{
    trace("start\n");

    if(in_username == nullptr || strcmp(in_username, "") == 0) {
        trace("error: user name must not be empty.\n");
        return B_BAD_VALUE;
    }

    struct passwd* pwd = getpwnam(in_username);
    if(pwd == nullptr) {
        trace("error: user not found.\n");
        return B_ENTRY_NOT_FOUND;
    }

    if(strcmp(pwd->pw_passwd, "") == 0 && strcmp(in_password, "") == 0) {
        // case of empty password
        trace("info: login successful.\n");
        return B_OK;
    }
    else if(strcmp(pwd->pw_passwd, "*NP*") == 0) {
        trace("error: NIS+ network passwords are not supported.\n");
        return B_NOT_SUPPORTED;
    }
    else if(strcmp(pwd->pw_passwd, "x") == 0) {
        // password in 'shadow'
        struct spwd* sp = getspnam(in_username);
        if(sp == NULL) {
            trace("error: user not found in 'shadow', despite being in 'passwd'.\n");
            return B_ENTRY_NOT_FOUND;
        }

        if(strcmp(crypt(in_password, sp->sp_pwdp), sp->sp_pwdp) == 0) {
            trace("info: login successful.\n");
            return B_OK;
        }
        else {
            trace("error: login failed.\n");
            return B_ERROR;
        }
    }
    else {
        // password in 'passwd', as the "good old days"
        if(strcmp(crypt(in_password, pwd->pw_passwd), pwd->pw_passwd) == 0) {
            trace("info: login successful.\n");
            return B_OK;
        }
        else {
            // here we could check if for some reason the password in passwd
            //   is unencrypted, but who does that nowadays?
            trace("error: login failed.\n");
            return B_ERROR;
        }
    }

    return B_DONT_DO_THAT;
}
