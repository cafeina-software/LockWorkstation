// mWindow.h

#ifndef _mClockView_h_
#define _mClockView_h_

#include "mConstant.h"
#include "../common/LockWorkstationConfig.h"
#include "../common/ThreadedClass.h"

class mClockView
		: public BView, ThreadedClass
{
public:
						mClockView(BRect rect, LWSettings* settings);
	void				Draw(BRect updateRect);
	 virtual void		Pulse();
private:
    void                _InitUIData();
    void                _TimeUpdate();
private:
    // From settings
    LWSettings         *mSettings;
	int					mFontSize;
	bool				mShowClock;
    rgb_color           mColor;
    BPoint              mPlace;

    // Date & time
	time_t				tmptodaytime;
	BString 			mTimer,
                        mDater;

	//threads
	thread_id			UserThread;
	static int32 		UserThreadChange_static(void *data);
	void 				UserThreadChangeImage();
};

#endif
