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


// Source for CBetterScrollerX class

#include "CBetterScrollerX.h"


#include <LScrollBar.h>
#include <UAppearance.h>
#include <UGAColorRamp.h>
#include <UGraphicUtils.h>

// __________________________________________________________________________________________________
// C L A S S __ C B E T T E R S C R O L L E R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CBetterScrollerX::CBetterScrollerX()
{
	mBackgroundColor = UGAColorRamp::GetWhiteColor();
	mReadOnly = false;
}

// Constructor from stream
CBetterScrollerX::CBetterScrollerX(LStream *inStream)
		: LScrollerView(inStream)
{
	mBackgroundColor = UGAColorRamp::GetWhiteColor();
	mReadOnly = false;
}

// Default destructor
CBetterScrollerX::~CBetterScrollerX()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup D&D
void CBetterScrollerX::FinishCreateSelf()
{
	// Do inherited
	LScrollerView::FinishCreateSelf();

	// Always add erase attachment
	//RGBColor temp = UGAColorRamp::GetWhiteColor();
	//AddAttachment(new LColorEraseAttachment(&temp, true));
}

void CBetterScrollerX::ShowVerticalScrollBar(Boolean show_it)
{
	if (show_it)
	{
		if (mTempVerticalBar && !mVerticalBar)
		{
			mVerticalBar = mTempVerticalBar;
			mVerticalBar->Show();
			mTempVerticalBar = NULL;
			
			Refresh();
		}
	}
	else
	{
		if (mVerticalBar)
		{
			mTempVerticalBar = mVerticalBar;
			mVerticalBar->Hide();
			mVerticalBar = NULL;
			
			Refresh();
		}
	}
}

void CBetterScrollerX::ApplyForeAndBackColors() const
{
	// Get rid of Appearance theme background as we always want white-on-black
	// inside this scroller
	#if PP_Uses_Appearance11_Routines

	if (UEnvironment::HasFeature(env_HasAppearance11)) {

		::NormalizeThemeDrawingState();

	}
	#endif

	RGBColor temp = (IsEnabled() && IsActive() ? UGAColorRamp::GetBlackColor() : UGAColorRamp::GetColor(7));
	::RGBForeColor(&temp);
	::RGBBackColor(&mBackgroundColor);

	//LScrollerView::ApplyForeAndBackColors();
}


void CBetterScrollerX::ResizeFrameBy(SInt16 inWidthDelta, SInt16 inHeightDelta, Boolean inRefresh)
{
	// Get the current relative scroll positions
	double scroll_vert = 0.0;
	double scroll_horiz = 0.0;

	SDimension32 imageSize;
	if (mScrollingView)
		mScrollingView->GetImageSize(imageSize);
	SDimension16 frameSize;
	if (mScrollingView)
		mScrollingView->GetFrameSize(frameSize);
	SPoint32 scrollPos;
	if (mScrollingView)
		mScrollingView->GetScrollPosition(scrollPos);
	if (mScrollingView)
	{
		double max_val = imageSize.width - frameSize.width;
		if (max_val == 0.0)
			max_val = 1.0;
		double now_val = scrollPos.h;
		scroll_horiz = now_val / max_val;
	}
	if (mScrollingView)
	{
		double max_val = imageSize.height - frameSize.height;
		if (max_val == 0.0)
			max_val = 1.0;
		double now_val = scrollPos.v;
		scroll_vert = now_val / max_val;
	}

	// Now do inherited change
	LScrollerView::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	
	// Reset scroll to original positions
	if (mScrollingView)
	{
		mScrollingView->GetImageSize(imageSize);
		mScrollingView->GetFrameSize(frameSize);
		mScrollingView->ScrollPinnedImageTo(::lround(scroll_horiz * (imageSize.width - frameSize.width)),
										::lround(scroll_vert * (imageSize.height - frameSize.height)), inRefresh);
	}
}

#if PP_Uses_Appearance11_Routines
class StThemeState
{
public:
	StThemeState()
		{ if (UEnvironment::HasFeature(env_HasAppearance11)) ::GetThemeDrawingState(&thState); }
	~StThemeState()
		{ if (UEnvironment::HasFeature(env_HasAppearance11)) ::SetThemeDrawingState(thState, true); }
private:
	ThemeDrawingState thState;
};
#endif
	
// Override base class to ensure white background
void CBetterScrollerX::Draw(RgnHandle inSuperDrawRgnH)
{
#if PP_Uses_Appearance11_Routines
	StThemeState thState;
	if (UEnvironment::HasFeature(env_HasAppearance11)) {
		::NormalizeThemeDrawingState();
	}
#endif
	LScrollerView::Draw(inSuperDrawRgnH);
}

// Override base class to ensure white background
void CBetterScrollerX::DrawSelf()
{
	// Erase entire frame
	Rect frame;
	CalcLocalFrameRect(frame);
	::RGBForeColor(&mBackgroundColor);
	::PaintRect(&frame);


	// Pretend its inactive if read only
	
	if (UEnvironment::HasFeature (env_HasAppearance))
	{
			// Adjust for the one pixel border that PowerPlant scrollers
			// have historically used
		
		::MacInsetRect(&frame, 1, 1);

										// Draw theme-compliant border
		ThemeDrawState	state = kThemeStateInactive;
		
		if (IsActive() && !mReadOnly) {
			state = kThemeStateActive;
		}

		::DrawThemeListBoxFrame(&frame, state);
	}
	else
	{
		StColorPenState::Normalize();

		// Figure out the inset pane's bounding rect
		Rect	localFrame;
		CalcLocalFrameRect ( localFrame );

		// Draw the inner frame around the edit text
		RGBColor	tempColor = Color_Black;
		if ( mReadOnly || !IsActive() ) {
			tempColor = UGAColorRamp::GetColor(colorRamp_Gray10);
		}
		::RGBForeColor ( & tempColor );
		::MacFrameRect ( &frame );			//   room for the focus ring.

		// Draw the shadows around the outside of the inner frame,
		// we only do this if we are drawing in color and when we are active and
		// enabled

		if (IsActive() && !mReadOnly)
		{
			// TOP LEFT SHADOW
			::MacInsetRect ( &frame, -1, -1 );
			RGBColor tempColor = UGAColorRamp::GetColor(5);
			::RGBForeColor ( &tempColor );
			UGraphicUtils::TopLeftSide ( frame, 0, 0, 1, 1 );

			// BOTTOM RIGHT SHADOW
			::RGBForeColor ( &Color_White );
			UGraphicUtils::BottomRightSide ( frame, 1, 1, 0, 0 );
		}
	}
}

