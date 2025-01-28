// mApp.h

#ifndef _mApp_h_
#define _mApp_h_

#define ApplicationSignature "application/x-vnd.LockWorkstation"

#include <Application.h>
#include "mWindow.h"

class mApp : public BApplication
{
public:
						mApp();
						~mApp();
	virtual void		ReadyToRun();
    virtual void        MessageReceived(BMessage* msg);
private:
	mWindow*			MainWindow;
};

#endif
