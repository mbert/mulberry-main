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


#ifndef __CEDITFORMATTEDTEXTDISPLAY__MULBERRY__
#define __CEDITFORMATTEDTEXTDISPLAY__MULBERRY__

#include "CEnrichedToolbar.h"
#include "CLetterTextEditView.h"


class CEnrichedToolbar;
class CLetterTextEditView;



class CEditFormattedTextDisplay : public CContainerView{
	DECLARE_DYNCREATE(CEditFormattedTextDisplay)

public:
						CEditFormattedTextDisplay();
	virtual 			~CEditFormattedTextDisplay();
	CRichEditCtrl&		GetRichEditCtrl(){
							return mTextDisplay.GetRichEditCtrl();}
	
	void 				SetContextMenuID(unsigned int mID){
							mTextDisplay.SetContextMenuID(mID);}
							
	void				SetSpacesPerTab(short spt){
							mTextDisplay.SetSpacesPerTab(spt);}
							
	void 				SetTabSelectAll(bool all){
							mTextDisplay.SetTabSelectAll(all);}
							
	void				SetLetterWindow(CLetterWindow *window){
							mTextDisplay.SetLetterWindow(window);}
							
	void				SetTabOrder(CWnd* next, CWnd* prev, CView* viewNext, CView* viewPrev){
							mTextDisplay.SetTabOrder(next, prev, viewNext, viewPrev);}
							
	void				ResetFont(CFont* pFont){
							mTextDisplay.ResetFont(pFont);}
							
	void				ShowNoToolbar(){
							mTextDisplay.ShowNoToolbar();
						}
	
	void 				ShowEnrichedToolbar(){
							mTextDisplay.ShowEnrichedToolbar();
						}	
							
	
protected:
	int 				OnCreate(LPCREATESTRUCT lpCreateStruct);
	
	
private:
	CEnrichedToolbar 			mEnrichedToolbar;
	CLetterTextEditView 		mTextDisplay;
	CDisplayFormatter			*mFormatter;
	
	bool 						mEnrichedShowing;

protected:
	
	DECLARE_MESSAGE_MAP()
};

#endif
