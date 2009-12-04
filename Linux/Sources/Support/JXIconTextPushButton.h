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
 JXIconTextPushButton.h

 ******************************************************************************/

#ifndef _H_JXIconTextPushButton
#define _H_JXIconTextPushButton

#include <JXTextButton.h>

class JXIconTextPushButton : public JXTextButton
{
public:

	JXIconTextPushButton(const JCharacter* label, JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h);

	virtual ~JXIconTextPushButton();

	void	SetImage(ResIDT imageID);

protected:
	ResIDT itsImageID;

	virtual void	DrawBackground(JXWindowPainter& p, const JRect& frame);
	virtual void  	DrawBorder(JXWindowPainter& p, const JRect& frame);

	virtual void	Draw(JXWindowPainter& p, const JRect& rect);

private:
	// not allowed

	JXIconTextPushButton(const JXIconTextPushButton& source);
	const JXIconTextPushButton& operator=(const JXIconTextPushButton& source);
};

#endif
