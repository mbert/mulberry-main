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


// CEditIdentityAddress.h : header file
//

#ifndef __CEDITIDENTITYADDRESS__MULBERRY__
#define __CEDITIDENTITYADDRESS__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityAddress dialog

class cdstring;
class CTextInputDisplay;
class CTextInputField;
class JXDownRect;
class JXRadioGroup;
class JXScrollbarSet;
class JXStaticText;
class JXTextCheckbox;
class JXTextRadioButton;

class CEditIdentityAddress : public CTabPanel
{
public:
	CEditIdentityAddress(bool outgoing, JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) { mOutgoing = outgoing; }

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data
	virtual void SetAddressType(bool from, bool reply_to, bool sender,
									bool to, bool cc, bool bcc, bool calendar)
		{ mFrom = from; mReplyTo = reply_to; mSender = sender;
			mTo = to; mCC = cc; mBcc = bcc; mCalendar = calendar; }

protected:
// begin JXLayout1

    JXDownRect*        mActiveGroup;
    JXTextCheckbox*    mActive;
    JXRadioGroup*      mSingleGroup;
    JXTextRadioButton* mSingleBtn;
    JXTextRadioButton* mMultipleBtn;
    JXStaticText*      mNameTitle;
    CTextInputField*   mName;
    JXStaticText*      mEmailTitle;
    CTextInputField*   mEmail;
    CTextInputDisplay* mText;

// end JXLayout1

#if 0
	// This is a duplicate of the above but with a slightly
	// different layout - we ignore these
// begin JXLayout2

    JXDownRect*        mActiveGroup;
    JXTextCheckbox*    mActive;
    JXRadioGroup*      mSingleGroup;
    JXTextRadioButton* mSingleBtn;
    JXTextRadioButton* mMultipleBtn;
    JXStaticText*      mNameTitle;
    CTextInputField*   mName;
    JXStaticText*      mEmailTitle;
    CTextInputField*   mEmail;
    CTextInputDisplay* mText;

// end JXLayout2
#endif

	JXScrollbarSet*				mTextScroller;

	bool mOutgoing;
	bool mFrom;
	bool mReplyTo;
	bool mSender;
	bool mTo;
	bool mCC;
	bool mBcc;
	bool mCalendar;
	bool mSingle;

	virtual void Receive(JBroadcaster* sender, const Message& message);

	void OnActive(bool active);
	void OnSingleAddress();
	void OnMultipleAddress();

	void SetSingle(bool single);
	void SetItemData(bool active, const cdstring& address);
	void GetItemData(bool& active, cdstring& address);
};

#endif
