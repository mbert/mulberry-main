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


#ifndef __CFONTNAMEMENU__MULBERRY__
#define __CFONTNAMEMENU__MULBERRY__

#include "CMenu.h"

class CFontNameMenu : public CMenu
{
public:
	CFontNameMenu(const JCharacter* title, JXContainer* enclosure,
				   const HSizingOption hSizing, const VSizingOption vSizing,
				   const JCoordinate x, const JCoordinate y,
				   const JCoordinate w, const JCoordinate h);

	CFontNameMenu(JXImage* image, const JBoolean menuOwnsImage,
			   JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h);

	CFontNameMenu(JXMenu* owner, const JIndex itemIndex, JXContainer* enclosure);

	virtual ~CFontNameMenu();

	void OnCreate(bool screen = true, bool mono = false, bool fancy = false)
		{ mForScreen = screen; mForPrinter = !screen;
		  mMonoSpaced = mono; mDisplayWithFont = fancy; BuildMenu(); }

			void	BuildMenuNow(bool quick = false);

	JString	GetFontName() const;
	void	SetFontName(const JCharacter* name);

	void	SetToPopupChoice(const JBoolean isPopup = kTrue);

protected:
	JIndex		itsFontIndex;
	JBoolean	itsBroadcastNameChangeFlag;

	bool	mForScreen;
	bool	mForPrinter;
	bool	mMonoSpaced;
	bool	mDisplayWithFont;

	static JPtrArray<JString>*	itsXFontNames;	// can be NULL
	static JPtrArray<JString>*	itsXMonoFontNames;	// can be NULL

	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual void	BuildMenu();

private:

	void	CFontNameMenuX();

	JString	GetFontName(const JIndex index) const;
	void	SetFontName1(const JIndex index);

	// not allowed

	CFontNameMenu(const CFontNameMenu& source);
	const CFontNameMenu& operator=(const CFontNameMenu& source);

public:

	// JBroadcaster messages

	static const JCharacter* kNameNeedsUpdate;
	static const JCharacter* kNameChanged;

	class NameNeedsUpdate : public JBroadcaster::Message
		{
		public:

			NameNeedsUpdate()
				:
				JBroadcaster::Message(kNameNeedsUpdate)
				{ };
		};

	class NameChanged : public JBroadcaster::Message
		{
		public:

			NameChanged()
				:
				JBroadcaster::Message(kNameChanged)
				{ };

		// call broadcaster's GetFont() to get fontName
		};
};


/******************************************************************************
 GetFontName

 ******************************************************************************/

inline JString
CFontNameMenu::GetFontName()
	const
{
	return GetFontName(itsFontIndex);
}

#endif
