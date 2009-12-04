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


// Header for CBetterScrollerX class

#ifndef __CBETTERSCROLLERX__MULBERRY__
#define __CBETTERSCROLLERX__MULBERRY__

#include <LScrollerView.h>
 
// Classes

class CBetterScrollerX : public LScrollerView
{
public:
	enum { class_ID = 'Xscr' };

					CBetterScrollerX();
					CBetterScrollerX(LStream *inStream);
	virtual 		~CBetterScrollerX();
	
			void	SetBackgroundColor(const RGBColor& color)
		{ mBackgroundColor = color; Refresh(); }

			void	SetReadOnly(bool read_only)
		{ mReadOnly = read_only; }

	virtual	void	ShowVerticalScrollBar(Boolean show_it);

	virtual void		ResizeFrameBy(
								SInt16		inWidthDelta,
								SInt16		inHeightDelta,
								Boolean		inRefresh);
								

protected:
	virtual void	FinishCreateSelf();								// Setup D&D
	virtual void	ApplyForeAndBackColors() const;

	virtual void	Draw(RgnHandle	inSuperDrawRgnH);
	virtual void	DrawSelf();

private:
	LScrollBar*		mTempVerticalBar;
	RGBColor		mBackgroundColor;
	bool			mReadOnly;
};

#endif
