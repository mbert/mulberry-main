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

// Dictionary Page List

#ifndef __CDICTIONARYPAGELIST__MULBERRY__
#define __CDICTIONARYPAGELIST__MULBERRY__

// Classes
class CSpellPlugin;
class CSpellCheckDialog;
class LGACaption;
class LFocusBox;

class CDictionaryPageList : public CListBox
{
public:
					CDictionaryPageList();
	virtual			~CDictionaryPageList();

			void	SetRangeCaptions(CStatic* word_top, CStatic* word_bottom)
				{ mWordTop = word_top; mWordBottom = word_bottom; }
			void	SetDetails(CSpellPlugin* speller, CStatic* page_count)
						{ mSpeller = speller; mPageCount = page_count; }
			bool	DisplayDictionary(const char* word);		// Display dictionary in list

			void	InitPage(void);
			void	SetPage(SInt32 inNewValue);
			void	DisplayPages(bool display);

	virtual LRESULT WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

	// Override these to do actual page scroll
protected:
	afx_msg void	OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	DECLARE_MESSAGE_MAP()

private:
	CSpellPlugin*	mSpeller;
	CStatic*		mWordTop;
	CStatic*		mWordBottom;
	CStatic*		mPageCount;
	bool			mDisplayPages;
};

#endif
