#include <Catalog.h>
#include <LayoutBuilder.h>
#include <private/app/RosterPrivate.h>
#include "mWindow.h"
#include "mBackgroundView.h"
#include "mSessionBar.h"
#include "../common/LockWorkstationConfig.h"
#include "mSysLogin.h"

const char* mDefaultPathToSelBG =			"/login_gfx";
const char* mDefaultPathToSelUI =			"/UserImage";
const char* mDefaultPathToSelNUI =			"/NoUserImage";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Main window"

/**********************************************************/
mWindow::mWindow(const char* mWindowTitle)
: BWindow(BRect(200, 200, 2000, 2000), mWindowTitle, B_NO_BORDER_WINDOW_LOOK,
    B_MODAL_ALL_WINDOW_FEEL, B_WILL_ACCEPT_FIRST_CLICK |
    B_FLOATING_SUBSET_WINDOW_FEEL | B_NOT_CLOSABLE | B_NOT_ZOOMABLE |
    B_NOT_RESIZABLE, B_ALL_WORKSPACES)
{
    BPath path;
    find_directory(B_SYSTEM_LOG_DIRECTORY, &path);
    path.Append("LockWorkstation.log");
    logger = new mLogger(mNameConfigEvtLoggingOn, path.Path());

    SetPulseRate(1000000);

    // Initialize app data from settings file
    InitUIData();
    const rgb_color mWhite = {255, 255, 255};
    const rgb_color mBlack = {0, 0, 0};

    // Child boxes
    loginbox = new mLoginBox(BRect(0, 0, 0, 0));
    infoview = new mSystemInfo(BRect(0, 0, 0, 0));
    if(!settings->SystemInfoPanelIsEnabled())
        infoview->Hide();
    sessionbar = new mSessionBar(B_HORIZONTAL, this);
    if(!settings->SessionBarIsEnabled())
        sessionbar->Hide();

    // Background view
    mView = new mBackgroundView(BRect(0, 0, 2000, 2000), NULL, B_FOLLOW_NONE,
        B_WILL_DRAW, settings->BackgroundColor(), settings->BackgroundMode(),
        imgpath, settings->BackgroundImageSnooze());

    BLayoutBuilder::Group<>(mView, B_HORIZONTAL, 0)
        .SetInsets(B_USE_WINDOW_INSETS)
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
    .End();

    AddChild(mView);

    // Quick key combinations
    if(settings->KillerShortcutIsEnabled())
        AddShortcut(B_SPACE, B_COMMAND_KEY | B_CONTROL_KEY,
            new BMessage(B_QUIT_REQUESTED));

    // Fit to screen, otherwise parts of the running environment will be shown
    //   if using high definitions resolutions
    MoveTo(0, 0);
    ResizeToScreen();
    mView->ResizeTo(Frame().Width(), Frame().Height());
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
            break;
        case BUTTON_LOGIN:
        {
            const char* user, *pass;
            if(message->FindString("username", &user) == B_OK &&
            message->FindString("password", &pass) == B_OK) {
                if(Login(settings->AuthenticationMethod(), user, pass) == B_OK) {
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
                    reply.AddBool("emptyFields", false);
                    // reply.AddBool("wrongUsername", !userMatch);
                    // reply.AddBool("wrongPassword", !passMatch);
                    message->SendReply(&reply);
                }
            }
            else {
                logger->AddEvent(EVT_ERROR, "Login failed: missing user or password.");
                BMessage reply(M_LOGIN_FAILED);
                reply.AddBool("emptyFields", true);
                message->SendReply(&reply);
            }
            break;
        }
        case M_RESTART_REQUESTED:
            logger->AddEvent("Restart requested.");
            SystemShutdown(true, false, false);
            break;
        case M_SHUTDOWN_REQUESTED:
            logger->AddEvent("Shut down requested.");
            SystemShutdown(false, false, false);
            break;
        case B_QUIT_REQUESTED:
            QuitRequested();
            break;
        //Kill the Team manager
        case B_SOME_APP_ACTIVATED:
        {
            /* THIS PIECE OF CODE HAS BEEN "BORROWED" FROM BeLogin, I HAVE NOT
            TALKED WITH THEM, BUT I JUST WANT TO MAKE SURE THAT THEY GET SOME
            OF THE CREDIT
            And my program doesnt use it at all BTW :-) */
            BString strSignature;
            message->FindString("be:signature", &strSignature);
            if (strSignature.ICompare ("application/x-vnd.Be-input_server") == 0)
            {
                app_info info;
                be_roster->GetAppInfo (strSignature.String(), &info);

                BMessage msgGetProperty, msgSetProperty, msgReply;
                status_t result;
                msgGetProperty.what = B_GET_PROPERTY;
                msgGetProperty.AddSpecifier("Messenger");
                msgGetProperty.AddSpecifier("Window", "Team monitor");
                result = BMessenger(info.signature, info.team).SendMessage(&msgGetProperty, &msgReply);

                if (result == B_OK)
                {
                    BMessenger msng;
                    if (msgReply.FindMessenger("result", &msng) == B_OK)
                    {
                        // msng.SendMessage(QUIT_TEAM_MONITOR);
                        new BMessage(B_QUIT_REQUESTED);
                        app_info mwinfo;
                        be_app->GetAppInfo(&mwinfo);
                        be_roster->ActivateApp(mwinfo.team);
                        Activate(true);
                    }
                }
            }
            break;
        }
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

void mWindow::ResizeToScreen()
{
    BScreen screen(B_MAIN_SCREEN_ID);
    display_mode dmode;
    screen.GetMode(&dmode);
    ResizeTo(dmode.virtual_width, dmode.virtual_height);
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
        case AUTH_KEYSTORE:
        {
            status = B_NOT_SUPPORTED;
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
    switch(settings->BackgroundMode())
    {
        case BGM_STATIC:
            imgpath.SetTo(settings->BackgroundImageStaticPath());
            break;
        case BGM_FOLDER:
            imgpath.SetTo(settings->BackgroundImageFolderPath());
            break;
        case BGM_LISTFILE:
            imgpath.SetTo(settings->BackgroundImageListPath());
            break;
        default:
            imgpath.SetTo("");
            break;
    }
}

void mWindow::SystemShutdown(bool restart, bool confirm, bool sync)
{
    BRoster roster;
    BRoster::Private privroster(roster);
    privroster.ShutDown(restart, confirm, sync);
}
