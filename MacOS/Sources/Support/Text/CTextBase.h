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


// Header for CTextBase class

#ifndef __CTextBase__MULBERRY__
#define __CTextBase__MULBERRY__

#include "CTextWidget.h"

#include "cdstring.h"
#include "cdustring.h"

// Classes

class CTextBase : public CTextWidget
{

public:
					CTextBase(LStream *inStream);
	virtual 		~CTextBase();
	
	virtual void	GetSelectionRange(long& selStart, long& selEnd) const
		{ GetSelection(&selStart, &selEnd); }
	virtual void	SetSelectionRange(long selStart, long selEnd)
		{ SetSelection(selStart, selEnd); }

			void	GetText(cdstring& all) const;
		cdstring	GetText() const;
			void	GetSelectedText(cdstring& selected) const;
	virtual void	SetTextPtr(const char* inTextP, SInt32 inTextLen);			// Force dirty
			void	SetText(const cdstring& all, bool scroll_to_top = true);								// Set all text
			void	SetText(const char* txt, size_t len = -1, bool scroll_to_top = true);					// Set all text
	
			void	GetText(cdustring& all) const;
			void	GetSelectedText(cdustring& selected) const;
			void	SetText(const cdustring& all, bool scroll_to_top = true);
			void	InsertText(const cdustring& utxt);
			void	InsertText(const unichar_t* utxt, size_t len = -1);

			void	InsertUTF8(const cdstring& txt);					// Insert unstyled utf8 at selection
			void	InsertUTF8(const char* txt, size_t len = -1);		// Insert unstyled utf8 at selection

	void	SetDirty(bool dirty) 								// Set dirty state
	{
		mDirty = dirty;
	}
	bool	IsDirty()											// Get dirty state
	{
		return mDirty;
	}

	virtual void	UserChangedText(void);								// Force dirty

protected:
	virtual void	DoPaste();

private:
	bool			mDirty;
};

class StStopRedraw
{
public:
	StStopRedraw(CTextBase* theTD);
	~StStopRedraw();
private:
	CTextBase*		mText;
	bool			mWasVisible;
};

#endif
