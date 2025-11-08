// mApp.cpp
#include <cstdio>
#include <ctime>
#include <Application.h>
#include <private/app/RosterPrivate.h>
#include "mApp.h"

int main(int argc, char** argv)
{
    mApp *app = new mApp();
    app->Run();
    delete app;
    return 0;
}

mApp::mApp()
: BApplication(ApplicationSignature)
{
    BPath settingsPath;
    find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
    settingsPath.Append(mPathToConfigFile);
    fSettings = new LWSettings(settingsPath.Path());

    fSecureCtx = new SecureContext(fSettings);
    if(!fSecureCtx || fSecureCtx->Run() < 0)
        throw std::runtime_error("Failure to initialize secure context.");

    BPath logPath;
    find_directory(B_SYSTEM_LOG_DIRECTORY, &logPath);
    logPath.Append("LockWorkstation.log");
    fEventLogCtx = new mLogger(fSettings, logPath.Path());
    if(!fEventLogCtx || fEventLogCtx->Run() < 0)
        throw std::runtime_error("Failure to initialize event log context.");

    MainWindow = new mWindow("Main Window", fSettings);
}

mApp::~mApp()
{
    if(fEventLogCtx && fEventLogCtx->Lock())
        fEventLogCtx->Quit();

    if(fSecureCtx && fSecureCtx->Lock())
        fSecureCtx->Quit();
}

void mApp::ReadyToRun()
{
    if (MainWindow != NULL)
        MainWindow->Show();
    else {
        fprintf(stderr, "Something went wrong, closing app...\n");
		be_app->PostMessage(B_QUIT_REQUESTED);
    }
}

void mApp::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case B_COUNT_PROPERTIES:
        case B_CREATE_PROPERTY:
        case B_DELETE_PROPERTY:
        case B_EXECUTE_PROPERTY:
        case B_GET_PROPERTY:
        case B_SET_PROPERTY:
        {
            // Reject foreign scripting messages: could be used to sniff
            //  into the password control of login box.
            if(msg->IsSourceRemote() || msg->WasDropped()) {
                Logger()->AddEvent(EVT_CRITICAL,
                    "External attempt to use scripting "
                    "(this could be used to spy on the password field).");
                break;
            }

            return BApplication::MessageReceived(msg);
        }
        case BUTTON_LOGIN:
        {
            BString username;
            const void* ptr = NULL;
            ssize_t length = 0;
            if(msg->FindString("username", &username) == B_OK &&
            msg->FindData("request_id", B_RAW_TYPE, &ptr, &length) == B_OK) {
                auto requestId = BytesToUUID(ptr);
                auto result = SecretsLooper()->GetAuthenticationResult(requestId);

                EventLevel resultLevel;
                BString resultDescription;
                if(result == B_OK) {
                    resultLevel = EVT_INFO;
                    resultDescription.SetToFormat("Login successful for username: %s",
                        username.String());
                }
                else {
                    resultLevel = EVT_ERROR;
                    resultDescription.SetToFormat("Login failed for username: %s",
                        username.String());
                }
                Logger()->AddEvent(resultLevel, resultDescription);

                if(result == B_OK) {
                    QuitRequested();
                }
                else {
                    BMessenger messenger;
                    if(msg->FindMessenger("messenger", &messenger) == B_OK) {
                        BMessage reply(M_LOGIN_FAILED);
                        reply.AddInt32("errorCode", result);
                        reply.AddString("username", username);
                        messenger.SendMessage(&reply);
                    }
                }
            }
            break;
        }
        case M_LOGGING_REQUESTED:
        {
            const void* timeptr = NULL;
            ssize_t timesize = 0;
            EventLevel level;
            BString description;
            if(msg->FindUInt32("log_level", (uint32*)&level) == B_OK &&
            msg->FindString("log_description", &description) == B_OK) {
                BDateTime datetime;
                if(msg->FindData("log_timestamp", B_TIME_TYPE, &timeptr, &timesize) == B_OK) {
                    uint8* buffer = new uint8[timesize];
                    memcpy(buffer, timeptr, timesize);
                    time_t time = *(reinterpret_cast<time_t*>(buffer));
                    datetime.SetTime_t(time);
                    delete[] buffer;
                }
                else
                    datetime.SetTime_t(std::time(NULL));

                Logger()->AddEvent(datetime, level, description.String());
            }
            break;
        }
        case M_SHUTDOWN_REQUESTED:
        case M_RESTART_REQUESTED:
            SystemShutdown(msg->what == M_RESTART_REQUESTED, false, false);
            break;
        default:
            BApplication::MessageReceived(msg);
            break;
    }
}

void mApp::SystemShutdown(bool restart, bool confirm, bool sync)
{
    BRoster roster;
    BRoster::Private privateRoster(roster);
    privateRoster.ShutDown(restart, confirm, sync);
}

SecureContext* mApp::SecretsLooper() const
{
    return fSecureCtx;
}

mLogger* mApp::Logger() const
{
    return fEventLogCtx;
}
