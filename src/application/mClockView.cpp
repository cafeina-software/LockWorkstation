// mWindow.cpp
#include <DateFormat.h>
#include <TimeFormat.h>
#include <ctime>
#include "mClockView.h"

/*************************************************/
/******************* Start ***********************/
/*************************************************/
mClockView::mClockView(BRect rect, LWSettings* settings)
: BView(rect, "Mariux", B_FOLLOW_LEFT_TOP, B_WILL_DRAW | B_PULSE_NEEDED),
  mSettings(settings)
{
    ThreadedCall(UserThread, UserThreadChange_static, "Change Image Thread",
        B_URGENT_DISPLAY_PRIORITY, this);

    _InitUIData();

    SetHighColor(mColor);
    SetFont(be_bold_font);
    SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    ResizeToPreferred();
}

/*********************************************/
/***************** Draw **********************/
/*********************************************/
void mClockView::Draw(BRect updateRect)
{
    SetFontSize(mFontSize * 8);
    float timewidth = StringWidth(mTimer.String());
    MovePenTo((Frame().Width() / 2) - (timewidth / 2),
        (Frame().Height() * 0.5f));
    DrawString(mTimer.String());

    SetFontSize(mFontSize * 4);
    float datewidth = StringWidth(mDater.String());
    MovePenTo((Frame().Width() / 2) - (datewidth / 2),
        (Frame().Height()) * 0.9f);
    DrawString(mDater.String());

    Invalidate();
}

void mClockView::Pulse()
{
    _TimeUpdate();
    Invalidate();
}

int32
mClockView::UserThreadChange_static(void *data)
{
	mClockView *changeimage = (mClockView *)data;
	changeimage->UserThreadChangeImage();

	return 0;
}

void mClockView::UserThreadChangeImage()
{
    while(mShowClock) {
        _TimeUpdate();

        //mTimer.SetTo("");
        BTimeFormat tfmt;
        tfmt.Format(mTimer, tmptodaytime, B_MEDIUM_TIME_FORMAT, NULL);

        //mDater.SetTo("");
        BDateFormat fmt;
        fmt.Format(mDater, tmptodaytime, B_LONG_DATE_FORMAT, NULL);

        sleep(1);
    }
}

void mClockView::_InitUIData()
{
    mShowClock = mSettings->ClockIsEnabled();
    mColor = mSettings->ClockColor();
    mPlace = mSettings->ClockLocation();
    mFontSize = mSettings->ClockSize();
}

void mClockView::_TimeUpdate()
{
    std::time(&tmptodaytime);
}
