/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


// Header for CHelpTags class

#ifndef __CHELPTAGS__MULBERRY__
#define __CHELPTAGS__MULBERRY__

#include <LPopupButton.h>

#include "MyCFString.h"

// Consts

// Classes

// This is a mix-in that adds help tagging to a window. The SetupTags method
// must be called in the window's FinishCreateSelf to setup the callback

class CHelpTagWindow
{
public:
	CHelpTagWindow(LWindow* window) :
		mWindow(window), mHelpCallback(NULL) {}
	virtual ~CHelpTagWindow();

	void	SetupHelpTags();

private:
	LWindow*			mWindow;
	HMWindowContentUPP	mHelpCallback;

	static pascal OSStatus	TagCallback(WindowRef inWindow, Point inGlobalMouse,
									HMContentRequest inRequest, HMContentProvidedType *outContentProvided, HMHelpContentPtr ioHelpContent);
};

// This is a mix-in to a pane/view etc that provides the help content. The pane can
// override SetupTagText and provide the static text in mHelpTag, or it can override
// GetHelpContent to provide dynamic text each time the tag is displayed.

class CHelpTagPane
{
	friend class CHelpTagWindow;

public:
	CHelpTagPane()
	{
		mHelpEnabled = true;
	}
	virtual ~CHelpTagPane() {}

protected:
	MyCFString		mHelpTag;
	bool			mHelpEnabled;

	void EnableHelp(bool enable)
	{
		mHelpEnabled = enable;
	}

	virtual bool SetupHelp(LPane* pane, Point inPortMouse, HMHelpContentPtr ioHelpContent);
	virtual void SetTagText(const char* txt);
	virtual void SetupTagText();
	virtual const MyCFString& GetHelpContent() const;
};

#endif
