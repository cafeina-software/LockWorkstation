/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <cstdio>
#include <cstring>
#include <ctime>
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
    else if(strcmp(pwd->pw_passwd, "!") == 0 || strcmp(pwd->pw_passwd, "*") == 0) {
        // password authentication not allowed, other methods still possible
        trace("error: this account cannot login via password authentication.\n");
        return B_PERMISSION_DENIED;
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
            return B_MISMATCHED_VALUES;
        }

        if(strcmp(sp->sp_pwdp, "!") == 0 || strcmp(sp->sp_pwdp, "*") == 0) {
            trace("error: this account cannot login via password authentication.\n");
            return B_PERMISSION_DENIED;
        }
        else {
            if(strcmp(crypt(in_password, sp->sp_pwdp), sp->sp_pwdp) == 0) {
                if(is_password_not_expired(in_username) == B_NOT_ALLOWED) {
                    trace("error: login failed because the account expired.\n");
                    return B_NOT_ALLOWED;
                }

                trace("info: login successful.\n");
                return B_OK;
            }
            else if(strcmp(in_password, "") == 0 && strcmp(sp->sp_pwdp, "") == 0) {
                // case of empty password in shadow
                if(is_password_not_expired(in_username) == B_NOT_ALLOWED) {
                    trace("error: login failed because the account expired.\n");
                    return B_ERROR;
                }

                trace("info: login successful (void).\n");
                return B_OK;
            }
            else {
                trace("error: login failed.\n");
                return B_ERROR;
            }
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

status_t try_app_login(const LWSettings* in_cfg, const char* in_username, const char* in_password)
{
    if(in_username == nullptr || strcmp(in_username, "") == 0) {
        trace("error: user name must not be empty.\n");
        return B_BAD_VALUE;
    }

    if(strcmp(in_username, in_cfg->DefaultUser()) != 0)
        return B_NAME_NOT_FOUND;

    if(strcmp(in_password, in_cfg->DefaultUserPassword()) == 0)
        return B_OK;

    return B_ERROR;
}

status_t is_password_not_expired(const char* in_username)
/* Precondition: user exists */
{
    std::time_t now = std::time(nullptr);

    struct spwd* sp = getspnam(in_username);
    if(sp == NULL) // user is not 'shadow-ed', so it is not subject to expiration
        return B_OK;

    if(sp->sp_expire > 0 && sp->sp_expire <= (now / (60*60*24))) // account expired
        return B_NOT_ALLOWED;

    long int lastch = sp->sp_lstchg;
    if(lastch < 0 || sp->sp_max < 0) // pw expiration disabled
        return B_OK;
    else if(lastch == 0) // pw expired and must be changed immediately
        return B_TIMED_OUT;
    else {
        if(lastch + sp->sp_max < now) // pw expired
            return B_TIMED_OUT;
        else
            return B_OK;
    }
}
