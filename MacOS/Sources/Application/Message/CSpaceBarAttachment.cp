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


// Source for CSpaceBarAttachment class

#include "CSpaceBarAttachment.h"

#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CTextDisplay.h"

// __________________________________________________________________________________________________
// C L A S S __ C W I N D O W S M E N U
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

CSpaceBarAttachment::CSpaceBarAttachment(CMessageWindow* msgWindow, CTextDisplay* text)
		: LAttachment(msg_KeyPress)
{
	mMsgWindow = msgWindow;
	mMsgView = NULL;
	mText = text;
}

CSpaceBarAttachment::CSpaceBarAttachment(CMessageView* msgView, CTextDisplay* text)
		: LAttachment(msg_KeyPress)
{
	mMsgWindow = NULL;
	mMsgView = msgView;
	mText = text;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void
CSpaceBarAttachment::ExecuteSelf(
	MessageT	/* inMessage */,
	void		*ioParam)
{
	mExecuteHost = false;		// We handle space bar
	SInt16	theKey = ((EventRecord*) ioParam)->message & keyCodeMask;

	switch (theKey)
	{
	case 0x3100:
	{						// Scroll down by height of Frame,
							//   but not past bottom of Image
		SPoint32		frameLoc;
		SPoint32		imageLoc;
		SDimension16	frameSize;
		SDimension32	imageSize;
		mText->GetFrameLocation(frameLoc);
		mText->GetImageLocation(imageLoc);
		mText->GetFrameSize(frameSize);
		mText->GetImageSize(imageSize);

		SInt32	downMax = imageSize.height - frameSize.height -
							(frameLoc.v - imageLoc.v);
		if (downMax > 0) {
			SPoint32		scrollUnit;
			mText->GetScrollUnit(scrollUnit);
			if (scrollUnit.v == 0)
				scrollUnit.v = 1;

			SInt32	down = (frameSize.height - 1) / scrollUnit.v;
			if (down <= 0) {
				down = 1;
			}
			down *= scrollUnit.v;
			if (down > downMax) {
				down = downMax;
			}
			mText->FocusDraw();
			mText->ScrollImageBy(0, down, true);
		}
		else
		{
			// Look for option key down
			if (((EventRecord*) ioParam)->modifiers & optionKey)
			{

				// Delete and go to next message
				if (mMsgWindow)
					mMsgWindow->OnMessageDeleteRead();
				else
					mMsgView->OnMessageDeleteRead();
			}
			else if (((EventRecord*) ioParam)->modifiers & shiftKey)
			{

				// Delete and go to next message
				if (mMsgWindow)
					mMsgWindow->OnMessageReadPrev();
				else
					mMsgView->OnMessageReadPrev();
			}
			else
			{
				// Go to next message
				if (mMsgWindow)
					mMsgWindow->OnMessageReadNext();
				else
					mMsgView->OnMessageReadNext();
			}
		}
		return;
	}

	default:;
	}

	switch(((EventRecord*) ioParam)->message & charCodeMask)
	{
	case char_Tab:
		// Only do in a preview pane
		if (mMsgView)
		{
			mMsgView->OnMessageReadNextNew();
			return;
		}
		break;

	// Delete and next
	case char_Backspace:
	case char_Clear:
		// Special case escape key
		if ((((EventRecord*) ioParam)->message & keyCodeMask) == vkey_Escape)
			return;
		else if (mMsgWindow)
			mMsgWindow->OnMessageDeleteRead();
		else if (mMsgView)
			mMsgView->OnMessageDeleteRead();
		return;

	default:;
	}

	mExecuteHost = true;	// Some other key, let host respond
}
