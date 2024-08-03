/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <private/app/RegistrarDefs.h>
#include <private/kernel/util/KMessage.h>
#include <private/libroot/user_group.h>
#include <private/shared/AutoLocker.h>
#include <ctime>
#include <string>
#include <pwd.h>
#include <shadow.h>
#include <unistd.h>
#include "AuthenticationUtils.h"

BStringList get_system_users(bool excludeSvcAccounts)
{
	BStringList userlist;
	struct passwd* pwd;

    auto is_noshell = [=](const char* shell) {
        return strcmp(shell, "/bin/true") == 0 ||
               strcmp(shell, "/bin/false") == 0;
    };

	setpwent();
	while((pwd = getpwent()) != NULL) {
        if(excludeSvcAccounts) {
            struct spwd* sp = getspnam(pwd->pw_name);
            if(is_noshell(pwd->pw_shell) ||
            (sp != NULL && strcmp(sp->sp_pwdp, "!") == 0))
                continue;
        }
		userlist.Add(BString(pwd->pw_name));
	}

	endpwent();
	return userlist;
}

status_t try_change_pwd(const char* name, const char* oldPassword, const char* newPassword)
{
    if(name == NULL)
        return B_BAD_VALUE;

    struct passwd* fUserEntry = getpwnam(name);
    if(fUserEntry == NULL)
        return B_ENTRY_NOT_FOUND;
    struct spwd* fUserShadowEntry = getspnam(name);

    if(try_login(name, oldPassword) != B_OK)
        return B_PERMISSION_DENIED;

    KMessage kmsg(BPrivate::B_REG_UPDATE_USER);
    kmsg.AddInt32("uid", fUserEntry->pw_uid);
    kmsg.AddInt32("gid", fUserEntry->pw_gid);
    kmsg.AddString("name", fUserEntry->pw_name);
    kmsg.AddString("password", "x");
    kmsg.AddString("home", fUserEntry->pw_dir);
    kmsg.AddString("shell", fUserEntry->pw_shell);
    kmsg.AddString("real name", fUserEntry->pw_gecos);
    if(fUserShadowEntry != NULL) {
        kmsg.AddInt32("min", fUserShadowEntry->sp_min);
        kmsg.AddInt32("max", fUserShadowEntry->sp_max);
        kmsg.AddInt32("warn", fUserShadowEntry->sp_warn);
        kmsg.AddInt32("inactive", fUserShadowEntry->sp_inact);
        kmsg.AddInt32("expiration", fUserShadowEntry->sp_expire);
        kmsg.AddInt32("flags", fUserShadowEntry->sp_flag);
    }
    else {
        kmsg.AddInt32("min", 0); // instant access to password change
        kmsg.AddInt32("max", 0);
        kmsg.AddInt32("warn", 0);
        kmsg.AddInt32("inactive", 0);
        kmsg.AddInt32("expiration", 99999); // In almost 274 years
        kmsg.AddInt32("flags", 0);
    }
    const char* encrypted;
    if(strcmp(newPassword, "") == 0)
        encrypted = "";
    else
        encrypted = crypt(newPassword, std::to_string(std::rand()).c_str());
    kmsg.AddString("shadow password", encrypted);
    long today = static_cast<long>(std::time(nullptr));
    kmsg.AddInt32("last changed", today);

    KMessage kreply;

    BPrivate::user_group_lock();
    BPrivate::send_authentication_request_to_registrar(kmsg, kreply);
    BPrivate::user_group_unlock();

    return kreply.What();
}

bool system_has_user(const char* name)
{
    if(name == NULL)
        return false;

    return getpwnam(name) != NULL;
}
