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


// Header for CSelectPopup class

#ifndef __CSELECTPOPUP__MULBERRY__
#define __CSELECTPOPUP__MULBERRY__

#include "CToolbarButton.h"

// Consts

// Classes

class CSelectPopup : public CToolbarButton
{

public:
	enum { class_ID = 'Spop' };

					CSelectPopup(LStream *inStream);
					CSelectPopup(
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
	virtual 		~CSelectPopup();

	virtual	void	SetValue(SInt32 inValue);
	virtual	void	SetupCurrentMenuItem(MenuHandle inMenuH,
											SInt16 inCurrentItem);

	virtual Boolean		TrackHotSpot(
								SInt16			inHotSpot,
								Point			inPoint,
								SInt16			inModifiers);
};

#endif
