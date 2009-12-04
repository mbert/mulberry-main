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


// Header for CServerViewPopup class

#ifndef __CSERVERVIEWPOPUP__MULBERRY__
#define __CSERVERVIEWPOPUP__MULBERRY__

#include "CToolbarButton.h"

#include "CListener.h"

// Consts

// Classes

class CServerViewPopup : public CToolbarButton,
							public CListener
{

public:
	enum { class_ID = 'Vpop' };

	enum
	{
		eServerView_New = 1,
		eServerView_Separator,
		eServerView_First
	};

					CServerViewPopup(LStream *inStream);
					CServerViewPopup(
							const SPaneInfo	&inPaneInfo,
							MessageT		inValueMessage,
							SInt16			inBevelProc,
							ResIDT			inMenuID,
							SInt16			inMenuPlacement,
							SInt16			inContentType,
							SInt16			inContentResID,
							ResIDT			inTextTraits,
							ConstStringPtr	inTitle,
							SInt16			inInitialValue,
							SInt16			inTitlePlacement,
							SInt16			inTitleAlignment,
							SInt16			inTitleOffset,
							SInt16			inGraphicAlignment,
							Point			inGraphicOffset,
							Boolean			inCenterPopupGlyph);
	virtual 		~CServerViewPopup();

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual	void	SetValue(SInt32 inValue);
	virtual	void	SetupCurrentMenuItem(MenuHandle inMenuH,
											SInt16 inCurrentItem);
	virtual void	SyncMenu(void);

	virtual Boolean		TrackHotSpot(
								SInt16			inHotSpot,
								Point			inPoint,
								SInt16			inModifiers);

protected:
	virtual void	FinishCreateSelf(void);

private:
	bool mDirty;
};

#endif
