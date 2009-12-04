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


// Header for CMailViewOptions class

#ifndef __CMAILVIEWOPTIONS__MULBERRY__
#define __CMAILVIEWOPTIONS__MULBERRY__

#include "CPreferenceItem.h"

// Classes

class CMailViewOptions : public CPreferenceItem
{
public:
	enum EPreviewFlagging
	{
		eMarkSeen = 0,
		eDelayedSeen,
		eNoSeen
	};

	CMailViewOptions();
	CMailViewOptions(const CMailViewOptions& copy)
		{ _copy(copy); }
	virtual ~CMailViewOptions();

	CMailViewOptions& operator=(const CMailViewOptions& copy)			// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	int operator==(const CMailViewOptions& comp) const;					// Compare with same type

	bool	GetUseTabs() const
		{ return mUseTabs; }
	void	SetUseTabs(bool tabs)
		{ mUseTabs = tabs; }

	bool	GetRestoreTabs() const
		{ return mRestoreTabs; }
	void	SetRestoreTabs(bool tabs)
		{ mRestoreTabs = tabs; }

	bool	GetShowPreview() const
		{ return mShowPreview; }
	void	SetShowPreview(bool show)
		{ mShowPreview = show; }

	bool	GetShowAddressPane() const
		{ return mShowAddressPane; }
	void	SetShowAddressPane(bool show)
		{ mShowAddressPane = show; }

	bool	GetShowSummary() const
		{ return mShowSummary; }
	void	SetShowSummary(bool summary)
		{ mShowSummary = summary; }

	bool	GetShowParts() const
		{ return mShowPartsToolbar; }
	void	SetShowParts(bool parts)
		{ mShowPartsToolbar = parts; }

	EPreviewFlagging	GetPreviewFlagging() const
		{ return mPreviewFlagging; }
	void	SetPreviewFlagging(EPreviewFlagging flagging)
		{ mPreviewFlagging = flagging; }
	unsigned long GetPreviewDelay() const
		{ return mPreviewDelay; }
	void SetPreviewDelay(unsigned long delay)
		{ mPreviewDelay = delay; }

	virtual cdstring GetInfo() const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	bool				mUseTabs;					// Use tabs in 3-pane
	bool				mRestoreTabs;				// Restore tabs to 3-pane after quit/restart
	bool				mShowPreview;				// Show preview in separate window mode
	bool				mShowAddressPane;			// Show pane at top with header info
	bool				mShowSummary;				// Show summary headers inside text pane
	bool				mShowPartsToolbar;			// Show parts toolbar
	EPreviewFlagging	mPreviewFlagging;			// Controls change of unseen flag
	unsigned long		mPreviewDelay;				// Time for unseen preview delay in secs

private:
	void _copy(const CMailViewOptions& copy);
};

#endif
