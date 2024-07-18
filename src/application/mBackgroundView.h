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
#include "StringList.h"
#include "../common/ThreadedClass.h"

class mBackgroundView : public BView, public ThreadedClass
{
public:
                    mBackgroundView(BRect, const char*, uint32, uint32, rgb_color,
                        uint8 bgmode, BString imgsrcpath = "", uint32 snooze = 10);
    virtual         ~mBackgroundView();
    virtual void    Draw(BRect updateRect);
private:
    void            InitUIData();
    status_t        InitImageFolder(BString imageFolderPath);
    status_t        InitImageListFile(BString imageListPath);
    status_t        InitImageStatic(BString imagePath);
    static int      CallSetBackgroundImage(void* data);
    void            SetBackgroundImage(void* data);
private:
    rgb_color       fBackgroundColor;
    uint8           fBackgroundMode;
    BString         fSrcPath;
    BStringList     imagePaths;
    BObjectList<BBitmap*> imageList;
    static BBitmap* currentimage;
    thread_id       imageLooper;
    static bool     shouldExit;
    uint32          snoozeMultiplier;
};


#endif /* _mBackgroundView_h_ */
