/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <pwd.h>
#include <shadow.h>
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
