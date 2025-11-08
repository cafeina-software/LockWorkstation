#include <Catalog.h>
#include <LayoutBuilder.h>
#include "mWindow.h"
#include "mBackgroundView.h"
#include "mSessionBar.h"
#include "../common/LockWorkstationConfig.h"

const char* mDefaultPathToSelBG =			"/login_gfx";
const char* mDefaultPathToSelUI =			"/UserImage";
const char* mDefaultPathToSelNUI =			"/NoUserImage";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Main window"

/**********************************************************/
mWindow::mWindow(const char* mWindowTitle, const LWSettings* appSettings)
: BWindow(BRect(200, 200, 2000, 2000), mWindowTitle, B_NO_BORDER_WINDOW_LOOK,
    LW_WSCREEN_WINDOW_FEEL, B_WILL_ACCEPT_FIRST_CLICK |
    B_FLOATING_SUBSET_WINDOW_FEEL | B_NOT_CLOSABLE | B_NOT_ZOOMABLE |
    B_NOT_RESIZABLE, B_ALL_WORKSPACES),
  settings(appSettings)
{
    SetPulseRate(1000000);

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
    mView = new mBackgroundView(BRect(0, 0, 2000, 2000), "bg_view", B_FOLLOW_ALL,
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
    delete settings;
}

/**********************************************************/
/* da controllare */
void mWindow::MessageReceived(BMessage* message)
{
    switch(message->what)
    {
        case LOGIN_CHANGED:
            PostEventToLog(EVT_WARNING,
                "User tried to exit the locker application (Cmd+Q).");
            break;
        case BUTTON_LOGIN:
        {
            be_app->PostMessage(message);
            break;
        }
        case M_RESTART_REQUESTED:
        {
            // Notify the login box
            BMessage request(LBM_NOTIFY_SESSION_EVENT);
            request.AddUInt32("what", message->what);
            PostMessage(&request, loginbox);

            // Log event
            PostEventToLog(EVT_INFO, "Restart requested.");

            // Ask be_app to do the actual work
            be_app->PostMessage(message->what);
            break;
        }
        case M_SHUTDOWN_REQUESTED:
        {
            // Notify the login box
            BMessage request(LBM_NOTIFY_SESSION_EVENT);
            request.AddUInt32("what", message->what);
            PostMessage(&request, loginbox);

            // Log event
            PostEventToLog(EVT_INFO, "Shut down requested.");

            // Ask be_app to do the actual work
            be_app->PostMessage(message->what);
            break;
        }
        case M_BYPASS_REQUESTED:
            if(settings->KillerShortcutIsEnabled()) {
                PostEventToLog(EVT_WARNING, "Shutdown using security-bypass requested.");
                QuitRequested();
            }
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
