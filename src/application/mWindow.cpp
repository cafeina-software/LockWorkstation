#include <Catalog.h>
#include <LayoutBuilder.h>
#include <private/app/RosterPrivate.h>
#include "mWindow.h"
#include "mBackgroundView.h"
#include "mSessionBar.h"
#include "../common/mSysLogin.h"
#include "../common/LockWorkstationConfig.h"

const char* mDefaultPathToSelBG =			"/login_gfx";
const char* mDefaultPathToSelUI =			"/UserImage";
const char* mDefaultPathToSelNUI =			"/NoUserImage";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Main window"

/**********************************************************/
mWindow::mWindow(const char* mWindowTitle)
: BWindow(BRect(200, 200, 2000, 2000), mWindowTitle, B_NO_BORDER_WINDOW_LOOK,
    LW_WSCREEN_WINDOW_FEEL, B_WILL_ACCEPT_FIRST_CLICK |
    B_FLOATING_SUBSET_WINDOW_FEEL | B_NOT_CLOSABLE | B_NOT_ZOOMABLE |
    B_NOT_RESIZABLE, B_ALL_WORKSPACES)
{
    // Initialize app data from settings file
    InitUIData();

    BPath path;
    find_directory(B_SYSTEM_LOG_DIRECTORY, &path);
    path.Append("LockWorkstation.log");
    logger = new mLogger(settings, path.Path());

    SetPulseRate(1000000);

    // const rgb_color mWhite = {255, 255, 255};
    // const rgb_color mBlack = {0, 0, 0};

    // Child boxes
    loginbox = new mLoginBox(BRect(0, 0, 0, 0), settings);
    infoview = new mSystemInfo(BRect(0, 0, 0, 0));
    if(!settings->SystemInfoPanelIsEnabled())
        infoview->Hide();
    sessionbar = new mSessionBar(B_HORIZONTAL, this);
    if(!settings->SessionBarIsEnabled())
        sessionbar->Hide();

    mClock = new mClockView(BRect(0, 0, 100, 100), settings);
    if(!settings->ClockIsEnabled())
        mClock->Hide();

    // Background view
    mView = new mBackgroundView(BRect(0, 0, 2000, 2000), NULL, B_FOLLOW_ALL,
        B_WILL_DRAW | B_FRAME_EVENTS, settings);

    // Layout kit
    BLayoutBuilder::Group<>(mView, B_VERTICAL, 0)
        .SetInsets(B_USE_WINDOW_INSETS)
        .AddGlue(0.1f)
        .AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING, 0.1f)
            .AddGlue()
            .Add(mClock)
            .AddGlue()
        .End()
        .AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING, 0.7f)
            .AddGlue()
            .AddGroup(B_VERTICAL)
                .AddGlue()
                .Add(loginbox)
                .AddStrut(12.0f)
                .Add(sessionbar)
                .AddGlue()
            .End()
            .AddGlue()
            .AddGroup(B_VERTICAL)
                .AddGlue()
                .Add(infoview)
                .AddGlue()
            .End()
            .AddGlue()
        .End()
        .AddGlue(0.1f)
    .End();

    AddChild(mView);

    // Quick key combinations
    if(settings->KillerShortcutIsEnabled())
        AddShortcut(B_SPACE, B_COMMAND_KEY | B_CONTROL_KEY,
            new BMessage(M_BYPASS_REQUESTED));
    AddShortcut('Q', B_COMMAND_KEY, new BMessage(LOGIN_CHANGED));

    // Fit to screen, otherwise parts of the running environment will be shown
    //   if using high definitions resolutions
    MoveTo(0, 0);
    ResizeToScreen();
}

/**********************************************************/
mWindow::~mWindow()
{
    delete logger;
    delete settings;
}

/**********************************************************/
/* da controllare */
void mWindow::MessageReceived(BMessage* message)
{
    switch(message->what)
    {
        case LOGIN_CHANGED:
            logger->AddEvent(EVT_WARNING, "User tried to exit the locker application (Cmd+Q).");
            break;
        case BUTTON_LOGIN:
        {
            const char* user, *pass;
            if(message->FindString("username", &user) == B_OK &&
            message->FindString("password", &pass) == B_OK) {
                status_t status = B_OK;
                if((status = Login(settings->AuthenticationMethod(), user, pass)) == B_OK) {
                    BString desc;
                    desc.SetToFormat("Login successful for username: %s", user);
                    logger->AddEvent(desc.String());
                    be_app->PostMessage(B_QUIT_REQUESTED);
                }
                else {
                    BString desc;
                    desc.SetToFormat("Login failed for username: %s", user);
                    logger->AddEvent(EVT_ERROR, desc.String());

                    BMessage reply(M_LOGIN_FAILED);
                    reply.AddInt32("errorCode", status);
                    reply.AddString("username", user);
                    message->SendReply(&reply);
                }
            }
            else {
                logger->AddEvent(EVT_ERROR, "Login failed: missing user or password.");
                BMessage reply(M_LOGIN_FAILED);
                reply.AddBool("errorCode", B_BAD_DATA);
                message->SendReply(&reply);
            }
            break;
        }
        case M_RESTART_REQUESTED:
        {
            logger->AddEvent("Restart requested.");
            BMessage request(LBM_NOTIFY_SESSION_EVENT);
            request.AddUInt32("what", message->what);
            PostMessage(&request, loginbox);
            SystemShutdown(true, false, false);
            break;
        }
        case M_SHUTDOWN_REQUESTED:
        {
            logger->AddEvent("Shut down requested.");
            BMessage request(LBM_NOTIFY_SESSION_EVENT);
            request.AddUInt32("what", message->what);
            PostMessage(&request, loginbox);
            SystemShutdown(false, false, false);
            break;
        }
        case M_BYPASS_REQUESTED:
            logger->AddEvent(EVT_WARNING, "Shutdown using security-bypass requested.");
            QuitRequested();
            break;
        case B_QUIT_REQUESTED:
            QuitRequested();
            break;
        default:
            BWindow::MessageReceived(message);
            break;
    }
}

/**********************************************************/
bool mWindow::QuitRequested()
{
    be_app->PostMessage(B_QUIT_REQUESTED);
    return BWindow::QuitRequested();
}

void mWindow::ScreenChanged(BRect frame, color_space mode)
{
    // Upon monitor resolution change, trigger window resize
    ResizeToScreen();
}

void mWindow::ResizeToScreen()
{
    BScreen screen(B_MAIN_SCREEN_ID);
    display_mode dmode;
    screen.GetMode(&dmode);
    ResizeTo(dmode.virtual_width, dmode.virtual_height);

    mView->MoveTo(0, 0);
    mView->ResizeTo(Frame().Width(), Frame().Height());
}

status_t mWindow::Login(AuthMethod mthd, const char* usr, const char* pwd)
{
    status_t status = B_ERROR;

    switch(mthd)
    {
        case AUTH_SYSTEM_ACCOUNT:
        {
            status = try_login(usr, pwd);
            break;
        }
        case AUTH_APP_ACCOUNT:
        default:
        {
            bool userMatch = strcmp(usr, settings->DefaultUser()) == 0;
            bool passMatch = strcmp(pwd, settings->DefaultUserPassword()) == 0;
            status = userMatch && passMatch ? B_OK : B_ERROR;
            break;
        }
    }

    return status;
}

void mWindow::InitUIData()
{
    BPath path;
    find_directory(B_USER_SETTINGS_DIRECTORY, &path);
    path.Append(mPathToConfigFile);

    settings = new LWSettings(path.Path());
}

void mWindow::SystemShutdown(bool restart, bool confirm, bool sync)
{
    BRoster roster;
    BRoster::Private privroster(roster);
    privroster.ShutDown(restart, confirm, sync);
}
