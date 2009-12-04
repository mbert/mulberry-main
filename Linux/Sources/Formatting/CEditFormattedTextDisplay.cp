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


#include "CEditFormattedTextDisplay.h"

#include "CColorPopup.h"
#include "CCommands.h"
#include "CFontNameMenu.h"
#include "CFontSizeMenu.h"
#include "CMainMenu.h"
#include "CMulberryApp.h"
#include "CStyleToolbar.h"

#include "JXColormap.h"
#include "JXMultiImageCheckbox.h"
#include <JXTextMenu.h>
//#include <JXFontSizeMenu.h>

CEditFormattedTextDisplay::CEditFormattedTextDisplay(
		      JXContainer* enclosure,
		      const HSizingOption hSizing, const VSizingOption vSizing,
		      const JCoordinate x, const JCoordinate y,
		      const JCoordinate w, const JCoordinate h)
	: CFormattedTextDisplay(enclosure, hSizing, vSizing, x, y, w, h)
{
	mHandleClick = false;
	
	mEnriched = NULL;
}

CEditFormattedTextDisplay::~CEditFormattedTextDisplay()
{
}

void CEditFormattedTextDisplay::UpdateStyledToolbar()
{
	if (!mEnriched)
		return;

	JBroadcaster::StStopListening _no_listen(this);

	const JFontStyle style = GetCurrentFontStyle();

	mEnriched->SetHiliteState(CCommand::eStyleBold, style.bold);
	mEnriched->SetHiliteState(CCommand::eStyleItalic, style.italic);
	mEnriched->SetHiliteState(CCommand::eStyleUnderline, style.underlineCount != 0);

	const AlignmentType align = GetCurrentFontAlign();

	mEnriched->SetHiliteState(CCommand::eAlignLeft, align == kAlignLeft);
	mEnriched->SetHiliteState(CCommand::eAlignRight, align == kAlignRight);
	mEnriched->SetHiliteState(CCommand::eAlignCenter, align == kAlignCenter);
	mEnriched->SetHiliteState(CCommand::eAlignJustified, false);

	mEnriched->mFont->SetFontName(GetCurrentFontName());

	mEnriched->mSize->SetFontSize(GetCurrentFontSize());

	if (style.color == mBlackIndex)
		mEnriched->SelectColor(CColorPopup::eColorIndex_Black);
	else if (style.color == mRedIndex)
		mEnriched->SelectColor(CColorPopup::eColorIndex_Red);
	else if (style.color == mGreenIndex)
		mEnriched->SelectColor(CColorPopup::eColorIndex_Green);
	else if (style.color == mBlueIndex)
		mEnriched->SelectColor(CColorPopup::eColorIndex_Blue);
	else if (style.color == mYellowIndex)
		mEnriched->SelectColor(CColorPopup::eColorIndex_Yellow);
	else if (style.color == mCyanIndex)
		mEnriched->SelectColor(CColorPopup::eColorIndex_Cyan);
	else if (style.color == mMagentaIndex)
		mEnriched->SelectColor(CColorPopup::eColorIndex_Magenta);
	else if (style.color == mMulberryIndex)
		mEnriched->SelectColor(CColorPopup::eColorIndex_Mulberry);
	else if (style.color == mWhiteIndex)
		mEnriched->SelectColor(CColorPopup::eColorIndex_White);
	else
		mEnriched->SetColor(mColorList.GetColormap()->GetRGB(style.color));
}

bool CEditFormattedTextDisplay::GetFormattingStatus(unsigned long cmd)
{
	const JFontStyle style = GetCurrentFontStyle();

	switch(cmd)
	{
	case CCommand::eStyleBold:
		return style.bold;
	case CCommand::eStyleItalic:
		return style.italic;
	case CCommand::eStyleUnderline:
		return (style.underlineCount != 0) && (style.underlineType != JFontStyle::redwavy_Underline);
	case CCommand::eStylePlain:
		return !style.bold && !style.italic && ((style.underlineCount == 0) || (style.underlineType == JFontStyle::redwavy_Underline));
	default:
		return false;
	}
}


void CEditFormattedTextDisplay::DoFormattingStatus(unsigned long cmd, CCmdUI* cmdui)
{
	if (!mEnriched)
		return;

	JBroadcaster::StStopListening _no_listen(this);

	cmdui->Enable(mEnriched->HasCommand(cmd));
		
	if (GetFormattingStatus(cmd))
	{
		cmdui->SetCheck(true);
		mEnriched->SetHiliteState(cmd, true);
	}
	else
	{
		cmdui->SetCheck(false);
		mEnriched->SetHiliteState(cmd, false);
	}
}

void CEditFormattedTextDisplay::DoAlignmentStatus(unsigned long cmd, CCmdUI* cmdui)
{
	if (!mEnriched)
		return;

	JBroadcaster::StStopListening _no_listen(this);

	const AlignmentType align = GetCurrentFontAlign();

	cmdui->Enable(mEnriched->HasCommand(cmd));

	switch(cmd)
	{
	case CCommand::eAlignLeft:
	default:
		cmdui->SetCheck(align == kAlignLeft);
		mEnriched->SetHiliteState(cmd, align == kAlignLeft);
		break;
	case CCommand::eAlignRight:
		cmdui->SetCheck(align == kAlignRight);
		mEnriched->SetHiliteState(cmd, align == kAlignRight);
		break;
	case CCommand::eAlignCenter:
		cmdui->SetCheck(align == kAlignCenter);
		mEnriched->SetHiliteState(cmd, align == kAlignCenter);
		break;
	case CCommand::eAlignJustified:
		cmdui->SetCheck(false);
		mEnriched->SetHiliteState(cmd, false);
		break;
	}
}

void CEditFormattedTextDisplay::DoColorStatus(unsigned long cmd, CCmdUI* cmdui)
{
	if (!mEnriched)
		return;

	JBroadcaster::StStopListening _no_listen(this);

	const JFontStyle style = GetCurrentFontStyle();

	cmdui->Enable(mEnriched->HasCommand(cmd));

	switch(cmd)
	{
	case CCommand::eColourBlack:
		if (style.color == mBlackIndex)
		{
			cmdui->SetCheck(true);
			mEnriched->SelectColor(CColorPopup::eColorIndex_Black);
		}
		else
			cmdui->SetCheck(false);
		break;
	case CCommand::eColourRed:
		if (style.color == mRedIndex)
		{
			cmdui->SetCheck(true);
			mEnriched->SelectColor(CColorPopup::eColorIndex_Red);
		}
		else
			cmdui->SetCheck(false);
		break;
	case CCommand::eColourGreen:
		if (style.color == mGreenIndex)
		{
			cmdui->SetCheck(true);
			mEnriched->SelectColor(CColorPopup::eColorIndex_Green);
		}
		else
			cmdui->SetCheck(false);
		break;
	case CCommand::eColourBlue:
		if (style.color == mBlueIndex)
		{
			cmdui->SetCheck(true);
			mEnriched->SelectColor(CColorPopup::eColorIndex_Blue);
		}
		else
			cmdui->SetCheck(false);
		break;
	case CCommand::eColourYellow:
		if (style.color == mYellowIndex)
		{
			cmdui->SetCheck(true);
			mEnriched->SelectColor(CColorPopup::eColorIndex_Yellow);
		}
		else
			cmdui->SetCheck(false);
		break;
	case CCommand::eColourCyan:
		if (style.color == mCyanIndex)
		{
			cmdui->SetCheck(true);
			mEnriched->SelectColor(CColorPopup::eColorIndex_Cyan);
		}
		else
			cmdui->SetCheck(false);
		break;
	case CCommand::eColourMagenta:
		if (style.color == mMagentaIndex)
		{
			cmdui->SetCheck(true);
			mEnriched->SelectColor(CColorPopup::eColorIndex_Magenta);
		}
		else
			cmdui->SetCheck(false);
		break;
	case CCommand::eColourMulberry:
		if (style.color == mMulberryIndex)
		{
			cmdui->SetCheck(true);
			mEnriched->SelectColor(CColorPopup::eColorIndex_Mulberry);
		}
		else
			cmdui->SetCheck(false);
		break;
	case CCommand::eColourWhite:
		if (style.color == mWhiteIndex)
		{
			cmdui->SetCheck(true);
			mEnriched->SelectColor(CColorPopup::eColorIndex_White);
		}
		else
			cmdui->SetCheck(false);
		break;
	case CCommand::eColourOther:
		if ((style.color == mBlackIndex) ||
			(style.color == mRedIndex) ||
			(style.color == mGreenIndex) ||
			(style.color == mBlueIndex) ||
			(style.color == mYellowIndex) ||
			(style.color == mCyanIndex) ||
			(style.color == mMagentaIndex) ||
			(style.color == mMulberryIndex) ||
			(style.color == mWhiteIndex))
		{
			cmdui->SetCheck(false);
		}
		else
		{
			cmdui->SetCheck(true);
			mEnriched->SetColor(mColorList.GetColormap()->GetRGB(style.color));
		}
		break;
	}
}

void CEditFormattedTextDisplay::DoFontStatus(CFontNameMenu* menu)
{
	JBroadcaster::StStopListening _no_listen(this);

	menu->SetFontName(GetCurrentFontName());
}

void CEditFormattedTextDisplay::DoSizeStatus(CFontSizeMenu* menu)
{
	JBroadcaster::StStopListening _no_listen(this);

	menu->SetFontSize(GetCurrentFontSize());
}

void CEditFormattedTextDisplay::SetToolbar(CStyleToolbar *toolbar)
{
	mEnriched = toolbar;
	mEnriched->AddListener(this);

	mBlackIndex = mColorList.Add(CColorPopup::sBlack);
	mRedIndex = mColorList.Add(CColorPopup::sRed);
	mGreenIndex = mColorList.Add(CColorPopup::sGreen);
	mBlueIndex = mColorList.Add(CColorPopup::sBlue);
	mYellowIndex = mColorList.Add(CColorPopup::sYellow);
	mCyanIndex = mColorList.Add(CColorPopup::sCyan);
	mMagentaIndex = mColorList.Add(CColorPopup::sMagenta);
	mMulberryIndex = mColorList.Add(CColorPopup::sMulberry);
	mWhiteIndex = mColorList.Add(CColorPopup::sWhite);
}

void CEditFormattedTextDisplay::ShowToolbar(bool show)
{
	if (show)
		UpdateStyledToolbar();
}

void CEditFormattedTextDisplay::DoFormattingCommand(unsigned long cmd)
{
	// Toggle style
	ETag tag;
	switch(cmd)
	{
	case CCommand::eStyleBold:
		tag = GetFormattingStatus(cmd) ? E_UNBOLD : E_BOLD;
		break;
	case CCommand::eStyleItalic:
		tag = GetFormattingStatus(cmd) ? E_UNITALIC : E_ITALIC;
		break;
	case CCommand::eStyleUnderline:
		tag = GetFormattingStatus(cmd) ? E_UNUNDERLINE : E_UNDERLINE;
		break;
	case CCommand::eStylePlain:
	default:
		tag = E_PLAIN;
		break;
	}

	// For now always turn it on
	FaceFormat(tag);
}

void CEditFormattedTextDisplay::DoAlignmentCommand(unsigned long cmd)
{
	// Toggle style
	ETag tag;
	switch(cmd)
	{
	case CCommand::eAlignLeft:
	default:
		tag = E_FLEFT;
		break;
	case CCommand::eAlignCenter:
		tag = E_CENTER;
		break;
	case CCommand::eAlignRight:
		tag = E_FRIGHT;
		break;
	}

	AlignmentFormat(tag);
	
	// Update toolbar state to make sure radio button behaviour occurs
	UpdateStyledToolbar();
}


void CEditFormattedTextDisplay::DoColorCommand(unsigned long cmd)
{
	JIndex color;
	
	switch(cmd)
	{
	case CCommand::eColourBlack:
		color = CColorPopup::eColorIndex_Black;
		break;
	case CCommand::eColourRed:
		color = CColorPopup::eColorIndex_Red;
		break;
	case CCommand::eColourBlue:
		color = CColorPopup::eColorIndex_Blue;
		break;
	case CCommand::eColourGreen:
		color = CColorPopup::eColorIndex_Green;
		break;
	case CCommand::eColourYellow:
		color = CColorPopup::eColorIndex_Yellow;
		break;
	case CCommand::eColourCyan:
		color = CColorPopup::eColorIndex_Cyan;
		break;
	case CCommand::eColourMagenta:
		color = CColorPopup::eColorIndex_Magenta;
		break;
	case CCommand::eColourMulberry:
		color = CColorPopup::eColorIndex_Mulberry;
		break;
	case CCommand::eColourWhite:
		color = CColorPopup::eColorIndex_White;
		break;
	case CCommand::eColourOther:
		color = CColorPopup::eColorIndex_Other;
		break;	
	}
	
	DoColorItem(color);
}
			
void CEditFormattedTextDisplay::DoColorItem(JIndex item)
{
	if (!mEnriched)
		return;

	mEnriched->SelectColor(item);
	JRGB color = mEnriched->GetColor();

	ColorFormat(color);
}
			
void CEditFormattedTextDisplay::DoFontItem(CFontNameMenu* menu)
{
	cdstring font(menu->GetFontName());
	FontFormat(font);
}

void CEditFormattedTextDisplay::DoSizeItem(CFontSizeMenu* menu)
{
	FontSizeFormat(menu->GetFontSize(), false);
}

void CEditFormattedTextDisplay::Activate()
{
	if (mEnriched)
		mEnriched->Activate();
	CFormattedTextDisplay::Activate();
}

void CEditFormattedTextDisplay::Deactivate()
{
	if (mEnriched)
		mEnriched->Deactivate();
	CFormattedTextDisplay::Deactivate();
}

void CEditFormattedTextDisplay::StyledToolbarListen(JXWidget* listen_to)
{
	ListenTo(listen_to);
}

void CEditFormattedTextDisplay::Receive(JBroadcaster* sender, const Message& message)
{
	// Look for cursor/text changes to trigger spell-as-you-type
	if ((sender == this) &&
		(message.Is(JTextEditor16::kTextChanged) ||
		 message.Is(JTextEditor16::kCaretLocationChanged)))
		UpdateStyledToolbar();
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mEnriched->mBold)
		{
			DoFormattingCommand(CCommand::eStyleBold);
			return;
		}
		else if (sender == mEnriched->mItalic)
		{
			DoFormattingCommand(CCommand::eStyleItalic);
			return;
		}
		else if (sender == mEnriched->mUnderline)
		{
			DoFormattingCommand(CCommand::eStyleUnderline);
			return;
		}
		else if (sender == mEnriched->mAlignLeft)
		{
			DoAlignmentCommand(CCommand::eAlignLeft);
			return;
		}
		else if (sender == mEnriched->mAlignCenter)
		{
			DoAlignmentCommand(CCommand::eAlignCenter);
			return;
		}
		else if (sender == mEnriched->mAlignRight)
		{
			DoAlignmentCommand(CCommand::eAlignRight);
			return;
		}
	}
	else if (message.Is(CFontNameMenu::kNameChanged))
	{
		CFontNameMenu* menu = dynamic_cast<CFontNameMenu*>(sender);
		if (menu != NULL)
		{
			DoFontItem(menu);
			return;
		}
	}
	else if (message.Is(CFontSizeMenu::kSizeChanged))
	{
		CFontSizeMenu* menu = dynamic_cast<CFontSizeMenu*>(sender);
		if (menu != NULL)
		{
			DoSizeItem(menu);
			return;
		}
	}
	else if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
		if (sender == mEnriched->mColor)
		{
			DoColorItem(index);
			return;
		}
	}

	CFormattedTextDisplay::Receive(sender, message);
}

// Handle commands our way
bool CEditFormattedTextDisplay::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	// Special check for dynamic font/size menus
	if (menu)
	{
		if (dynamic_cast<CFontNameMenu*>(menu->mMenu) != NULL)
		{
			// Make sure we are listening to the menu so that we get the change notification
			ListenTo(static_cast<CFontNameMenu*>(menu->mMenu));
			return true;
		}
		else if (dynamic_cast<CFontSizeMenu*>(menu->mMenu) != NULL)
		{
			// Make sure we are listening to the menu so that we get the change notification
			ListenTo(static_cast<CFontSizeMenu*>(menu->mMenu));
			return true;
		}
	}

	bool cmd_handled = true;

	switch (cmd)
	{
	case CCommand::eStyleBold:	
	case CCommand::eStyleItalic:
	case CCommand::eStyleUnderline:
	case CCommand::eStylePlain:
		DoFormattingCommand(cmd);
		break;
	case CCommand::eAlignLeft:
	case CCommand::eAlignCenter:
	case CCommand::eAlignRight:
	case CCommand::eAlignJustified:
		DoAlignmentCommand(cmd);
		break;
	case CCommand::eColourRed:
	case CCommand::eColourBlue:
	case CCommand::eColourGreen:
	case CCommand::eColourYellow:
	case CCommand::eColourCyan:
	case CCommand::eColourMagenta:
	case CCommand::eColourBlack:
	case CCommand::eColourWhite:
	case CCommand::eColourMulberry:
	case CCommand::eColourOther:
		DoColorCommand(cmd);
		break;
	default:
		cmd_handled = CFormattedTextDisplay::ObeyCommand(cmd, menu);
		break;
	}
	return cmd_handled;
}

void CEditFormattedTextDisplay::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	// Special check for dynamic font/size menus
	if (cmdui->mMenu)
	{
		if (dynamic_cast<CFontNameMenu*>(cmdui->mMenu) != NULL)
		{
			// Only do for first item as it will update state of all items
			if (cmdui->mMenuIndex == 1)
				DoFontStatus(static_cast<CFontNameMenu*>(cmdui->mMenu));
			OnUpdateAlways(cmdui);
			return;
		}
		else if (dynamic_cast<CFontSizeMenu*>(cmdui->mMenu) != NULL)
		{
			// Only do for first item as it will update state of all items
			if (cmdui->mMenuIndex == 1)
				DoSizeStatus(static_cast<CFontSizeMenu*>(cmdui->mMenu));
			OnUpdateAlways(cmdui);
			return;
		}
	}

	switch (cmd)
	{
	case CCommand::eDraftStyle:
		if (mEnriched && mEnriched->IsVisible())
			OnUpdateAlways(cmdui);
		else
			OnUpdateNever(cmdui);
		return;
	case CCommand::eStyleBold:
	case CCommand::eStyleItalic:
	case CCommand::eStyleUnderline:
	case CCommand::eStylePlain:	
		if (mEnriched && mEnriched->IsVisible())
			DoFormattingStatus(cmd, cmdui);
		else
			OnUpdateNever(cmdui);
		return;
	case CCommand::eDraftAlignment:
		if (mEnriched && mEnriched->IsVisible())
			OnUpdateAlways(cmdui);
		else
			OnUpdateNever(cmdui);
		return;
	case CCommand::eAlignLeft:
	case CCommand::eAlignCenter:
	case CCommand::eAlignRight:
	case CCommand::eAlignJustified:
		if (mEnriched && mEnriched->IsVisible())
			DoAlignmentStatus(cmd, cmdui);
		else
			OnUpdateNever(cmdui);
		return;
	case CCommand::eDraftFont:
	case CCommand::eDraftSize:
	case CCommand::eDraftColour:
		if (mEnriched && mEnriched->IsVisible())
			OnUpdateAlways(cmdui);
		else
			OnUpdateNever(cmdui);
		return;
	case CCommand::eColourRed:
	case CCommand::eColourBlue:
	case CCommand::eColourGreen:
	case CCommand::eColourYellow:
	case CCommand::eColourCyan:
	case CCommand::eColourMagenta:
	case CCommand::eColourBlack:
	case CCommand::eColourWhite:
	case CCommand::eColourMulberry:
	case CCommand::eColourOther:
		if(mEnriched && mEnriched->IsVisible())
			DoColorStatus(cmd, cmdui);
		else
			OnUpdateNever(cmdui);
		return;
	default:;
	}

	CFormattedTextDisplay::UpdateCommand(cmd, cmdui);
}
