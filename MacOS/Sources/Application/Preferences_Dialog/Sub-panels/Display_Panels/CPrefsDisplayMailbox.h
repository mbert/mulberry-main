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


// Header for CPrefsDisplayMailbox class

#ifndef __CPREFSDISPLAYMAILBOX__MULBERRY__
#define __CPREFSDISPLAYMAILBOX__MULBERRY__

#include "CPrefsDisplayPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsDisplayMailbox = 5041;
const	PaneIDT		paneid_DMBRecentColour = 'RCOL';
const	PaneIDT		paneid_DMBUnseenColour = 'UCOL';
const	PaneIDT		paneid_DMBOpenColour = 'OCOL';
const	PaneIDT		paneid_DMBFavouriteColour = 'FCOL';
const	PaneIDT		paneid_DMBClosedColour = 'CCOL';
const	PaneIDT		paneid_DMBRecentBold = 'RBLD';
const	PaneIDT		paneid_DMBUnseenBold = 'UBLD';
const	PaneIDT		paneid_DMBOpenBold = 'OBLD';
const	PaneIDT		paneid_DMBFavouriteBold = 'FBLD';
const	PaneIDT		paneid_DMBClosedBold = 'CBLD';
const	PaneIDT		paneid_DMBRecentItalic = 'RITL';
const	PaneIDT		paneid_DMBUnseenItalic = 'UITL';
const	PaneIDT		paneid_DMBOpenItalic = 'OITL';
const	PaneIDT		paneid_DMBFavouriteItalic = 'FITL';
const	PaneIDT		paneid_DMBClosedItalic = 'CITL';
const	PaneIDT		paneid_DMBRecentStrike = 'RSTK';
const	PaneIDT		paneid_DMBUnseenStrike = 'USTK';
const	PaneIDT		paneid_DMBOpenStrike = 'OSTK';
const	PaneIDT		paneid_DMBFavouriteStrike = 'FSTK';
const	PaneIDT		paneid_DMBClosedStrike = 'CSTK';
const	PaneIDT		paneid_DMBRecentUnderline = 'RUND';
const	PaneIDT		paneid_DMBUnseenUnderline = 'UUND';
const	PaneIDT		paneid_DMBOpenUnderline = 'OUND';
const	PaneIDT		paneid_DMBFavouriteUnderline = 'FUND';
const	PaneIDT		paneid_DMBClosedUnderline = 'CUND';
const	PaneIDT		paneid_DMBIgnoreRecent = 'NORE';

// Mesages

// Resources

// Classes
class LCheckBox;

class	CPrefsDisplayMailbox : public CPrefsDisplayPanel
{
private:
	SFullStyleItems		mRecent;
	SFullStyleItems		mUnseen;
	SFullStyleItems		mOpen;
	SFullStyleItems		mFavourite;
	SFullStyleItems		mClosed;
	LCheckBox*			mIgnoreRecent;

public:
	enum { class_ID = 'Dmbx' };

					CPrefsDisplayMailbox();
					CPrefsDisplayMailbox(LStream *inStream);
	virtual 		~CPrefsDisplayMailbox();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
