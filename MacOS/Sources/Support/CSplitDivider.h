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


// Header for CSplitDivider class

#ifndef __CSPLITDIVIDER__MULBERRY__
#define __CSPLITDIVIDER__MULBERRY__


// Resources

// Messages
const	MessageT	msg_SplitDividerMove = 'SplD';

// Classes

class CSplitDivider : public LControl {

public:
	enum { class_ID = 'SplD' };

					CSplitDivider();
					CSplitDivider(LStream *inStream);
	virtual 		~CSplitDivider();
	
	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);	// Track split divider

	virtual void	AdjustMouseSelf(Point inPortPt,
									const EventRecord &inMacEvent,
									RgnHandle outMouseRgn);	// Adjust cursor

	virtual void	DrawSelf();	

	virtual	void	SetHorizontal(bool horizontal)
						{ mHorizontal = horizontal; }
	virtual bool	IsHorizontal(void)
						{ return mHorizontal; }
private:
	bool			mHorizontal;
};

#endif
