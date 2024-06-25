/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mSysLogin_h_
#define _mSysLogin_h_

#include <SupportDefs.h>

status_t try_login(const char* in_username, const char* in_password);
status_t is_password_not_expired(const char* in_username);


#endif /* _mSysLogin_h_ */
