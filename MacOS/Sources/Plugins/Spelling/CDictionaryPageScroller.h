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

// Dictionary Page Scroller

#ifndef __CDICTIONARYPAGESCROLLER__MULBERRY__
#define __CDICTIONARYPAGESCROLLER__MULBERRY__

#include "CBetterScrollerX.h"
#include <LBroadcaster.h>

// Panes
const	PaneIDT		paneid_DictionaryPageScrollerPageCount = 'PAGE';

// Classes
class CSpellPlugin;
class CSpellCheckDialog;
class CStaticText;
class LScrollBar;

class CDictionaryPageScroller : public CBetterScrollerX,
								public LBroadcaster
{
public:
	enum { class_ID = 'Dscr' };

					CDictionaryPageScroller();
					CDictionaryPageScroller(LStream *inStream);
	virtual			~CDictionaryPageScroller();

			void	SetRangeCaptions(CStaticText* word_top, CStaticText* word_bottom)
				{ mWordTop = word_top; mWordBottom = word_bottom; }
			void	SetDetails(CSpellPlugin* speller, LListener* aListener)
						{ mSpeller = speller; AddListener(aListener); }
			bool	DisplayDictionary(const char* word,
									bool force_draw = false);	// Display dictionary in list

			void	SetCount(SInt32 inNewValue);
			void	SetPage(SInt32 inNewValue);
			void	DisplayPages(bool display);

	LScrollBar*	GetHorizScrollBar(void)
		{ return mHorizontalBar; }

	virtual void	HorizScroll(SInt16 inPart);
	virtual void	VertScroll(SInt16 inPart);
	virtual void	ThumbScroll(LScrollBar* inScrollBar, SInt32 inThumbValue);

	// Override these to do actual page scroll
protected:
	virtual void		FinishCreateSelf(void);					// Do odds & ends
	//virtual bool		HandleKeyPress(const EventRecord &inKeyEvent);
	//virtual void		DoKeySelection(void);						// Select from key press

	virtual void	AdjustScrollBars();

private:
	CSpellPlugin*	mSpeller;
	CStaticText*		mWordTop;
	CStaticText*		mWordBottom;
	CStaticText*		mPageCount;
	bool			mDisplayPages;
	unsigned long	mLastTyping;				// Time of last typed character
	char			mLastChars[32];				// Last characters typed
};

#endif
