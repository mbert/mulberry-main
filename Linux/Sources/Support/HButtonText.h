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

#ifndef __HBUTTON_TEXT_H
#define __HBUTTON_TEXT_H

#include <string>
#include <JXStaticText.h>
#include <JFontManager.h>
#include <JXButton.h>
#include "CMulberryApp.h"

template <class T> class HButtonText : public T
{
public:
	HButtonText(JXContainer* enclosure,
							const JXWidget::HSizingOption hSizing, 
							const JXWidget::VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
		: T(enclosure, hSizing, vSizing, x, y, w, h), itsText(NULL),
			itsEnclosure(enclosure){}

	void SetText(const char* newTxt);
	const std::string& GetText() const
		{ return txt; }

	virtual void	Show();					// must call inherited
	virtual void	Hide();					// must call inherited
	
private:
	std::string txt;
	JXStaticText *itsText;
	JXContainer* itsEnclosure;//this is kinda lame but need to have non-const
                           	//pointer
	
};

template <class T> void HButtonText<T>::SetText(const char* newTxt)
{
	txt = newTxt;

	if (itsText)
		delete itsText;

	JRect frame = this->GetFrame();
	JCoordinate ascent, descent;
	const JFontManager* fm = this->GetFontManager();
	itsText = new JXStaticText(newTxt, itsEnclosure, 
								this->GetHSizing(), this->GetVSizing(), 0, 0, 0, 0);
	itsText->Hide();
	itsText->SetFont(CMulberryApp::sAppSmallFont.name_.c_str(),
									 CMulberryApp::sAppSmallFont.size_,
									 CMulberryApp::sAppSmallFont.style_);

	JSize newWidth = itsText->GetMinBoundsWidth();
	itsText->SetSize(newWidth, itsText->GetFrameHeight());
	int x = frame.xcenter() - newWidth/2;
	itsText->Place(x > 0 ? x : 0, frame.bottom + 1);
	itsText->Show();
}

template <class T> void HButtonText<T>::Show()
{
	T::Show();
	if (itsText)
		itsText->Show();
}

template <class T> void HButtonText<T>::Hide()
{
	T::Hide();
	if (itsText)
		itsText->Hide();
}
#endif
