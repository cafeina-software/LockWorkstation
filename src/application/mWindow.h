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
#include "../common/LockWorkstationConfig.h"

class mWindow : public BWindow
{
public:
    					mWindow(const char* mWindowTitle);
    virtual  			~mWindow();
    virtual void 		MessageReceived(BMessage* message);
    virtual bool        QuitRequested();
    void                ResizeToScreen();
    status_t            Login(AuthMethod mthd, const char* usr, const char* pwd);
private:
    void                InitUIData();
    void                SystemShutdown(bool restart, bool confirm, bool sync);
private:
    mBackgroundView    *mView;
    mLoginBox          *loginbox;
    mSystemInfo        *infoview;
    mSessionBar        *sessionbar;
    mClockView         *mClock;

    LWSettings         *settings;
    mLogger            *logger;
    BString             imgpath;
};

#endif
