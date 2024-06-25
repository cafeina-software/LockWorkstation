/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _AuthenticationUtils_h_
#define _AuthenticationUtils_h_

#include <StringList.h>

BStringList get_system_users(bool excludeSvcAccounts = true);

#endif /* _AuthenticationUtils_h_ */
