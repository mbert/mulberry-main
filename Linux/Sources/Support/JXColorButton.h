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


#ifndef _H_JXColorButton
#define _H_JXColorButton

#include <JXButton.h>
#include <jColor.h>

class JXChooseColorDialog;
class JXWindowDirector;

class JXColorButton : public JXButton
{
public:

	JXColorButton(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h);

	void	SetColor(const JRGB& color);
	JRGB	GetColor() const
		{ return mColor; }
	void	SetDirector(JXWindowDirector* director)
		{ mDirector = director; }

protected:
	JRGB mColor;
	JXWindowDirector* mDirector;
	JXChooseColorDialog* mChooser;

	virtual void	Draw(JXWindowPainter& p, const JRect& rect);
	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	OnClick();
private:

  // not allowed

  JXColorButton(const JXColorButton& source);
  const JXColorButton& operator=(const JXColorButton& source);
};

#endif
