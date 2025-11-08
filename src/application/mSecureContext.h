/*
 * Copyright 2025, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mSecureContext_h_
#define _mSecureContext_h_

#include <Looper.h>
#include <String.h>
#include <array>
#include <map>
#include "../common/LockWorkstationConfig.h"

using UUID = std::array<char, 32>;
bool UUIDtoString(const UUID& id, char* str, size_t strLength);
UUID BytesToUUID(const void* bytes);

struct RequestData {
    status_t result = B_NOT_INITIALIZED;
    bool expired = false;
};

class SecureContext : public BLooper
{
public:
    SecureContext(const LWSettings* settings);

    void MessageReceived(BMessage* message) override;

    UUID MakeAuthenticationRequest(const void* identifier, const void* key);
    status_t GetAuthenticationResult(UUID requestId);
private:
    UUID GenerateUUID();
    status_t Login(AuthMethod method, const void* user, const void* pass);

    SecureContext(BMessage* data) = delete;
private:
    const LWSettings* fSettings;
    const AuthMethod fCurrentAuthMethod;
    const BString fApplicationAccount;
    std::map<UUID, RequestData> fRequestsArchive;
};

#endif /* _mSecureContext_h_ */
