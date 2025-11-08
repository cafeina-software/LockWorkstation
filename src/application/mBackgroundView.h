/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mBackgroundView_h_
#define _mBackgroundView_h_

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <TranslationKit.h>
#include <ObjectList.h>
#include <StringList.h>
#include "../common/LockWorkstationConfig.h"
#include "../common/ThreadedClass.h"

class mBackgroundView : public BView, public ThreadedClass
{
public:
                    mBackgroundView(BRect, const char*, uint32, uint32,
                        const LWSettings* settings);
    virtual         ~mBackgroundView();

    virtual void    Draw(BRect updateRect);
private:
    void            InitUIData();
    status_t        InitImageFolder(BString imageFolderPath);
    status_t        InitImageListFile(BString imageListPath);
    status_t        InitImageStatic(BString imagePath);
    static int32    CallSetBackgroundImage(void* data);
    void            SetBackgroundImage(void* data);

    void            keep_center_view(BRect viewRect, BRect imageRect, BRect* resultRect);
    void            full_x_view(BRect viewRect, BRect imageRect, BRect* resultRect);
    void            full_y_view(BRect viewRect, BRect imageRect, BRect* resultRect);
    void            proportional_view(BRect viewRect, BRect imageRect, BRect* resultRect);
    void            fullscreen_view(BRect viewRect, BRect* resultRect);
private:
    const LWSettings* fCurrentSettings;
    rgb_color       fBackgroundColor;
    uint8           fBackgroundMode;
    uint32          snoozeMultiplier;
    ImgAdjust       fBackgroundImageAdjust;
    BString         fSrcPath;
    BStringList     imagePaths;
    static BBitmap* currentimage;
    thread_id       imageLooper;
    static bool     shouldExit;
};


#endif /* _mBackgroundView_h_ */
