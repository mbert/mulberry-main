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


// Header for CFontSizeMenu class

#ifndef __CFONTSIZEMENU__MULBERRY__
#define __CFONTSIZEMENU__MULBERRY__

#include "CMenu.h"

class CFontNameMenu;
class JXChooseFontSizeDialog;

class CFontSizeMenu : public CMenu
{
public:
	CFontSizeMenu(const JCharacter* title,
				   JXContainer* enclosure,
				   const HSizingOption hSizing, const VSizingOption vSizing,
				   const JCoordinate x, const JCoordinate y,
				   const JCoordinate w, const JCoordinate h);

	CFontSizeMenu(JXImage* image, const JBoolean menuOwnsImage,
			   JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h);

	CFontSizeMenu(JXMenu* owner, const JIndex itemIndex, JXContainer* enclosure);

	virtual ~CFontSizeMenu() {}

	void	OnCreate(CFontNameMenu* menu);

	void	SetFontName(const JCharacter* name);

	JSize	GetFontSize() const;
	void	SetFontSize(const JSize size);

	void	SetToPopupChoice(const JBoolean isPopup = kTrue);

protected:
	virtual void	Receive(JBroadcaster* sender, const Message& message);

//private:	cd: need to access these in drived class

	CFontNameMenu*	itsFontNameMenu;
	JSize			itsFontSize;
	JIndex			itsCurrIndex;
	JIndex			itsVarSizeIndex;	// 0 if font not rescalable
	JBoolean		itsBroadcastSizeChangeFlag;

	// used when selecting arbitrary font size

	JXChooseFontSizeDialog*	itsChooseSizeDialog;	// can be NULL

//private:	cd: need to access these in drived class

	void	CFontSizeMenuX(CFontNameMenu* fontMenu);
	void	BuildMenu(const JCharacter* fontName);
	void	AdjustVarSizeItem(const JSize fontSize);


private:

	void	DoSizeChoice(const JIndex index);

	// not allowed

	CFontSizeMenu(const CFontSizeMenu& source);
	const CFontSizeMenu& operator=(const CFontSizeMenu& source);

public:

	// JBroadcaster messages

	static const JCharacter* kSizeNeedsUpdate;
	static const JCharacter* kSizeChanged;

	class SizeNeedsUpdate : public JBroadcaster::Message
		{
		public:

			SizeNeedsUpdate()
				:
				JBroadcaster::Message(kSizeNeedsUpdate)
				{ };
		};

	class SizeChanged : public JBroadcaster::Message
		{
		public:

			SizeChanged(const JSize size)
				:
				JBroadcaster::Message(kSizeChanged),
				itsSize(size)
				{ };

			JSize
			GetSize() const
			{
				return itsSize;
			};

		private:

			JSize itsSize;
		};
};

/******************************************************************************
 GetFontSize

 ******************************************************************************/

inline JSize
CFontSizeMenu::GetFontSize()
	const
{
	return itsFontSize;
}

/******************************************************************************
 SetFontName

 ******************************************************************************/

inline void
CFontSizeMenu::SetFontName
	(
	const JCharacter* name
	)
{
	const JSize currSize = GetFontSize();
	BuildMenu(name);
	SetFontSize(currSize);
}

#endif
