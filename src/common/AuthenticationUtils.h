/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _AuthenticationUtils_h_
#define _AuthenticationUtils_h_

#include <StringList.h>
#include "mSysLogin.h"

BStringList get_system_users(bool excludeSvcAccounts = true);
bool system_has_user(const char* name = NULL);
status_t try_change_pwd(const char* name = NULL, const char* oldPassword = "", const char* newPassword = "");

#endif /* _AuthenticationUtils_h_ */
