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


#ifndef _H_JXMultiImageButton
#define _H_JXMultiImageButton

#include <JXButton.h>
#include <jXConstants.h>

class JXMultiImageButton : public JXButton
{
public:

	JXMultiImageButton(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h);

	virtual ~JXMultiImageButton();

	void	SetImage(ResIDT imageID);
	void	SetImages(ResIDT imageID, ResIDT pushedImageID);

protected:
	virtual void	DrawBackground(JXWindowPainter& p, const JRect& frame);
	virtual void	DrawBorder(JXWindowPainter& p, const JRect& frame);

	virtual void	Draw(JXWindowPainter& p, const JRect& rect);

private:
	ResIDT itsImageID;
	ResIDT itsPushedImageID;

	// not allowed

	JXMultiImageButton(const JXMultiImageButton& source);
	const JXMultiImageButton& operator=(const JXMultiImageButton& source);
};

#endif
