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


// Header for CPrefsMailboxAdvanced class

#ifndef __CPREFSMAILBOXADVANCED__MULBERRY__
#define __CPREFSMAILBOXADVANCED__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsMailboxAdvanced = 5061;
const	PaneIDT		paneid_MAUnseenNew = 'UNSN';
const	PaneIDT		paneid_MARecentNew = 'RCNT';
const	PaneIDT		paneid_MARecentUnseenNew = 'REUN';
const	PaneIDT		paneid_MAFavouriteCopyTo = 'COPY';
const	PaneIDT		paneid_MAFavouriteAppendTo = 'APPN';
const	PaneIDT		paneid_MAMaximumMRU = 'MAXR';
const	PaneIDT		paneid_MARLoCache = 'MINC';
const	PaneIDT		paneid_MARNoCacheLimit = 'NLIM';
const	PaneIDT		paneid_MARUseHiCache = 'MMAX';
const	PaneIDT		paneid_MARHiCache = 'MAXC';
const	PaneIDT		paneid_MARAutoResize = 'AUTO';
const	PaneIDT		paneid_MARUseIncrement = 'INCB';
const	PaneIDT		paneid_MARCacheIncrement = 'INCC';
const	PaneIDT		paneid_MARSortCache = 'SORT';
const	PaneIDT		paneid_MALLoCache = 'LINC';
const	PaneIDT		paneid_MALNoCacheLimit = 'LLIM';
const	PaneIDT		paneid_MALUseHiCache = 'LMAX';
const	PaneIDT		paneid_MALHiCache = 'LAXC';
const	PaneIDT		paneid_MALAutoResize = 'LUTO';
const	PaneIDT		paneid_MALUseIncrement = 'LNCB';
const	PaneIDT		paneid_MALCacheIncrement = 'LNCC';
const	PaneIDT		paneid_MALSortCache = 'LORT';

// Mesages
const	MessageT	msg_MAFavouriteCopyTo = 'COPY';
const	MessageT	msg_MAFavouriteAppendTo = 'APPN';
const	MessageT	msg_MARNoCacheLimit = 'NLIM';
const	MessageT	msg_MARUseHiCache = 'MMAX';
const	MessageT	msg_MARAutoResize = 'AUTO';
const	MessageT	msg_MARUseIncrement = 'INCB';
const	MessageT	msg_MALNoCacheLimit = 'LLIM';
const	MessageT	msg_MALUseHiCache = 'LMAX';
const	MessageT	msg_MALAutoResize = 'LUTO';
const	MessageT	msg_MALUseIncrement = 'LNCB';

// Resources
const	ResIDT		RidL_CPrefsMailboxAdvancedBtns = 5061;

// Classes
class LCheckBox;
class LRadioButton;
class CTextFieldX;

class	CPrefsMailboxAdvanced : public CPrefsTabSubPanel,
									public LListener
{
private:
	LRadioButton*		mUnseenNew;
	LRadioButton*		mRecentNew;
	LRadioButton*		mRecentUnseenNew;
	LCheckBox*			mFavouriteCopyTo;
	LCheckBox*			mFavouriteAppendTo;
	CTextFieldX*		mMaximumMRU;
	CTextFieldX*		mRLoCache;
	LRadioButton*		mRNoLimit;
	LRadioButton*		mRUseHiCache;
	CTextFieldX*		mRHiCache;
	LRadioButton*		mRAutoResize;
	LRadioButton*		mRUseIncrement;
	CTextFieldX*		mRCacheIncrement;
	CTextFieldX*		mRSortCache;
	CTextFieldX*		mLLoCache;
	LRadioButton*		mLNoLimit;
	LRadioButton*		mLUseHiCache;
	CTextFieldX*		mLHiCache;
	LRadioButton*		mLAutoResize;
	LRadioButton*		mLUseIncrement;
	CTextFieldX*		mLCacheIncrement;
	CTextFieldX*		mLSortCache;

public:
	enum { class_ID = 'Madv' };

					CPrefsMailboxAdvanced();
					CPrefsMailboxAdvanced(LStream *inStream);
	virtual 		~CPrefsMailboxAdvanced();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
