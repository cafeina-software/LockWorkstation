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
    					mWindow(const char* mWindowTitle, const LWSettings* appSettings);
    virtual  			~mWindow();
    virtual void 		MessageReceived(BMessage* message);
    virtual bool        QuitRequested();
    virtual void        ScreenChanged(BRect frame, color_space mode);
    void                ResizeToScreen();
private:
    mBackgroundView    *mView;
    mLoginBox          *loginbox;
    mSystemInfo        *infoview;
    mSessionBar        *sessionbar;
    mClockView         *mClock;

    const LWSettings   *settings;
};

#endif
