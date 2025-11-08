// mApp.h

#ifndef _mApp_h_
#define _mApp_h_

#define ApplicationSignature "application/x-vnd.LockWorkstation"

#include <Application.h>
#include "mSecureContext.h"
#include "mWindow.h"

class mApp : public BApplication
{
public:
						mApp();
						~mApp();
	virtual void		ReadyToRun();
    virtual void        MessageReceived(BMessage* msg);

    void                SystemShutdown(bool restart, bool confirm, bool sync);

    SecureContext*      SecretsLooper() const;
    mLogger*            Logger() const;
private:
    LWSettings*         fSettings;
    SecureContext*      fSecureCtx;
    mLogger*            fEventLogCtx;
	mWindow*			MainWindow;
};

#endif
