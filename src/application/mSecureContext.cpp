#include <Looper.h>
#include <algorithm>
#include <random>
#include "mConstant.h"
#include "mLogger.h"
#include "mSecureContext.h"
#include "../common/mSysLogin.h"

static const char hexChars[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};
const UUID invalidUuid = {
    '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0',
    '0', '0', '0', '0',
    '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'
};

bool UUIDtoString(const UUID& id, char* str, size_t strLength)
{
    if(strLength < id.size() + 1)
        return false;

    for(size_t i = 0; i < id.max_size(); i++)
        str[i] = id[i];
    str[id.max_size()] = '\0';

    return true;
}

UUID BytesToUUID(const void* bytes)
{
    std::array<char, 32> uuid;
    std::copy_n((const unsigned char*)bytes, 32, uuid.begin());
    return uuid;
}

SecureContext::SecureContext(const LWSettings* settings)
: BLooper("Secure context looper", B_URGENT_PRIORITY,
    B_LOOPER_PORT_DEFAULT_CAPACITY),
  fSettings(settings),
  fCurrentAuthMethod(settings->AuthenticationMethod()),
  fApplicationAccount(settings->DefaultUser())
{
}

void SecureContext::MessageReceived(BMessage* message)
{
    switch(message->what)
    {
        case B_COUNT_PROPERTIES:
        case B_CREATE_PROPERTY:
        case B_DELETE_PROPERTY:
        case B_EXECUTE_PROPERTY:
        case B_GET_PROPERTY:
        case B_SET_PROPERTY:
        {
            // Reject foreign scripting messages
            if(message->IsSourceRemote() || message->WasDropped()) {
                PostEventToLog(EVT_CRITICAL,
                    "External attempt to use scripting "
                    "(this could be used to spy on the password field).");
                break;
            }

            return BLooper::MessageReceived(message);
        }
        case M_SECURECTX_CLEANUP:
            fRequestsArchive.clear();
            break;
        default:
            return BLooper::MessageReceived(message);
    }
}

UUID SecureContext::MakeAuthenticationRequest(const void* identifier, const void* key)
{
    if(!identifier)
        return invalidUuid;

    auto requestId = GenerateUUID();
    status_t result = Login(fCurrentAuthMethod, identifier, key);
    struct RequestData data {
        .result = result,
        .expired = false
    };
    if(requestId != invalidUuid)
        fRequestsArchive.emplace(requestId, std::move(data));

    return requestId;
}

status_t SecureContext::GetAuthenticationResult(UUID requestId)
{
    if(requestId == invalidUuid) // Invalid request
        return B_BAD_VALUE;

    status_t result = B_ERROR;
    if(auto search = fRequestsArchive.find(requestId); search != fRequestsArchive.end()) {
        if(search->second.expired) // Used requests cannot be reused to fake further success
            return B_TIMED_OUT;

        result = search->second.result;
        search->second.expired = true;
    }
    else // Request not found
        result = B_NAME_NOT_FOUND;

    return result;
}

UUID SecureContext::GenerateUUID()
{
    std::random_device rd("default");
    std::mt19937 mte(rd());
    std::uniform_int_distribution<> distribution(0, B_COUNT_OF(hexChars) - 1);

    UUID data;
    for(size_t i = 0; i < data.max_size(); i++)
        data[i] = hexChars[distribution(mte)];
    return data;
}

status_t SecureContext::Login(AuthMethod method, const void* user, const void* pass)
{
    char* username = (char*)user;
    char* password = (char*)pass;

    status_t status = B_ERROR;
    switch(method)
    {
        case AUTH_SYSTEM_ACCOUNT:
            status = try_login(username, password);
            break;
        case AUTH_APP_ACCOUNT:
        default:
            status = try_app_login(fSettings, username, password);
            break;
    }

    return status;
}
