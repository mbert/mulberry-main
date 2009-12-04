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

/******************************************************************************
 JXTabButton.h

 ******************************************************************************/

#ifndef _H_JXTabButton
#define _H_JXTabButton

#include <JXTextRadioButton.h>

#include <JXButtonStates.h>
#include <JXKeyModifiers.h>

class JXImage;

class JXTabButton : public JXTextRadioButton
{
public:

	JXTabButton(const JIndex id, const JCharacter* label, JXRadioGroup* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h);

	virtual ~JXTabButton();

	void SetIcon(JIndex icon);
	JIndex GetIcon() const
		{ return mUseIcon ? mIconID : 0; }

protected:
	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);

	virtual void	DrawBackground(JXWindowPainter& p, const JRect& frame);
	virtual void	DrawBorder(JXWindowPainter& p, const JRect& frame);

	virtual void	Draw(JXWindowPainter& p, const JRect& rect);

private:
	bool		mUseIcon;
	JIndex		mIconID;
	JXImage*	mIcon;

	// not allowed

	JXTabButton(const JXTabButton& source);
	const JXTabButton& operator=(const JXTabButton& source);
	// JBroadcaster messages

public:
	static const JCharacter* kRightClick;

	class RightClick : public JBroadcaster::Message
	{
	public:

		RightClick(const JPoint& pt, const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers) :
			JBroadcaster::Message(kRightClick), itsPt(pt), itsButtonStates(buttonStates), itsModifiers(modifiers)
			{ }

		const JPoint& GetPt() const
			{ return itsPt; }
		const JXButtonStates& GetButtonStates() const
			{ return itsButtonStates; }
		const JXKeyModifiers& GetModifiers() const
			{ return itsModifiers; }

	private:
		JPoint 			itsPt;
		JXButtonStates	itsButtonStates;
		JXKeyModifiers	itsModifiers;
	};

	static const JCharacter* kShiftClick;

	class ShiftClick : public JBroadcaster::Message
	{
	public:

		ShiftClick() :
			JBroadcaster::Message(kShiftClick)
			{ }
	};
};

#endif
