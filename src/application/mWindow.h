// mWindow.h

#ifndef _mWindow_h_
#define _mWindow_h_

#include <SupportKit.h>

#include "mBackgroundView.h"
#include "mBitmap.h"
#include "mConstant.h"
#include "mClockView.h"
#include "mLogger.h"
#include "mLoginBox.h"
#include "mSessionBar.h"
#include "mSystemInfo.h"

class mWindow : public BWindow
{
public:
    					mWindow(const char* mWindowTitle);
    virtual  			~mWindow();
    virtual void 		MessageReceived(BMessage* message);
    virtual bool        QuitRequested();
    void                ResizeToScreen();
private:
    void                InitUIData();
    // status_t            LoadSettings();
    // status_t            SaveSettings();
    void                SystemShutdown(bool restart, bool confirm, bool sync);
private:
    mBackgroundView     *mView;
    mLoginBox           *loginbox;
    mSystemInfo         *infoview;
    mSessionBar         *sessionbar;

    BMessage            savemessage;

    BString             mTheRightUserName;
    BString             mTheRightPassword;
    rgb_color           mBackgroundColor;
    uint8               mBackgroundImageMode;
    BString             mBackgroundImageFolderPath;
    uint32              mBackgroundListSnooze;
    rgb_color           mClockColor;
    BPoint              mClockLocation;
    uint32              mClockSize;
    bool                mClockShown;
    BString             mStringLanguage;
    bool                mSessionBarShown;
    bool                mSysInfoPanelShown;
    bool                mKillerShortcutEnabled;
    bool                mLoggingEnabled;

    BString				mStringPathToBG;
	BString				mStringPathToCU;
	BString				mStringPathToNOCU;

    mLogger*            logger;
};

#endif
