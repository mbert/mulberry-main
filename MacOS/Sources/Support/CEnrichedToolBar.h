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


#include <LWindow.h>
#include "CBackView.h"
#include "LBroadcaster.h"
#include <LStream.h>

#include <PP_Messages.h>

const	PaneIDT		paneid_BoldBTN = 'BTNB';
const	PaneIDT		paneid_ItalicBTN = 'BTNI';
const	PaneIDT		paneid_UnderlineBTN = 'BTNU';
const	PaneIDT		paneid_AlignLeftBTN = 'BTNL';
const	PaneIDT		paneid_AlignCenterBTN = 'BTNC';
const	PaneIDT		paneid_AlignRightBTN = 'BTNR';
const	PaneIDT		paneid_AlignJustifyBTN = 'BTNJ';

const	PaneIDT		paneid_EnrichedFont = 'DFNT';
const	PaneIDT		paneid_EnrichedSize = 'DSIZ';
const	PaneIDT		paneid_EnrichedColor = 'ECOL';

class LGATextButton;
class LGAIconButton;
class CFontPopup;
class CSizePopup; 
class CBackView;
class LGAPopup;
class CColorPopup;


class CStyleToolbar : public CBackView{
	public:
		enum { class_ID = 'TENR' };
		static CStyleToolbar*	CreateFromStream(LStream *inStream);
			   						CStyleToolbar();
									CStyleToolbar(LStream *inStream);
		virtual 					~CStyleToolbar();
		virtual void FinishCreateSelf();
		virtual void EnableCmd(CommandT inCommand);
		void SetHiliteState(CommandT inCommand, Boolean state);
		void Ambiguate(CommandT inCommand);
		virtual void AddListener(LListener	*inListener);
		void getFont(Str255 name);
		Int16 getSize();
		void setFontName(Str255 fontName);
		void setFont(Int16 menuItem);
		void setSizeItem(Int16 menuItem);
		void setSize(Int16 size);
		void selectColor(Int32 color);
		void setColor(RGBColor color);
		RGBColor getColor();
		void SaveState();
		void RestoreState();
		Boolean InUse(){return here;}
		virtual void Show();
		virtual void Hide();
		void Setup(EContentSubType type);
		
	private:
		LGATextButton *mBold;
		LGATextButton *mItalic;
		LGATextButton *mUnderline;
		LGAIconButton *mAlignLeft;
		LGAIconButton *mAlignCenter;
		LGAIconButton *mAlignRight;
		LGAIconButton *mAlignJustify;
		CFontPopup *mFont;
		CSizePopup *mSize;
		CColorPopup *mColor;
		Boolean here;


};


