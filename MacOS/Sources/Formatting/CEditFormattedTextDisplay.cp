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

#include "CATSUIStyle.h"
#include "CColorPopup.h"
#include "CCommands.h"
#include "CMailControl.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CURL.h"

const ResIDT MENU_FontMain = 161;
const ResIDT MENU_SizeMain = 162;

CEditFormattedTextDisplay::CEditFormattedTextDisplay(LStream *inStream) : CFormattedTextDisplay(inStream)
{
	mHandleClick = false;
	
	// Turn on spell checking if present
	if (CPluginManager::sPluginManager.HasSpelling())
		SpellAutoCheck(CPluginManager::sPluginManager.GetSpelling()->SpellAsYouType());
}

CEditFormattedTextDisplay::~CEditFormattedTextDisplay()
{
}


bool CEditFormattedTextDisplay::getFormattingStatus(CommandT inCommand)
{
	// Is it continuous
	CATSUIStyle style;
	OSStatus err = ::ATSUGetContinuousAttributes(mTextLayout, mSelection.caret, mSelection.length, style);
	if (err == noErr)
	{
		Boolean result;
		switch(inCommand)
		{
		case cmd_Bold:
			return style.GetOneAttribute(kATSUQDBoldfaceTag, sizeof(Boolean), &result) && result;
		case cmd_Italic:
			return style.GetOneAttribute(kATSUQDItalicTag, sizeof(Boolean), &result) && result;
		case cmd_Underline:
			return style.GetOneAttribute(kATSUQDUnderlineTag, sizeof(Boolean), &result) && result;
		case cmd_Plain:
			return (!style.GetOneAttribute(kATSUQDBoldfaceTag, sizeof(Boolean), &result) || !result) &&
					 (!style.GetOneAttribute(kATSUQDItalicTag, sizeof(Boolean), &result) || !result) &&
					 (!style.GetOneAttribute(kATSUQDUnderlineTag, sizeof(Boolean), &result) || !result);
		default:
			return false;
		}
	}
	
	return false;
}


void CEditFormattedTextDisplay::doFormattingStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outEnabled = true;
	outUsesMark = true;

	mEnriched->EnableCmd(inCommand);

	StopListening();
	if (getFormattingStatus(inCommand))
	{
		outMark = checkMark;
		mEnriched->SetHiliteState(inCommand, true);
	}

	else
	{
		outMark = noMark;
		mEnriched->SetHiliteState(inCommand, false);
	}
	StartListening();

}

void CEditFormattedTextDisplay::doFontStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	// Is it continuous
	CATSUIStyle style;
	OSStatus err = ::ATSUGetContinuousAttributes(mTextLayout, mSelection.caret, mSelection.length, style);
	if (err == noErr)
	{
		StopListening();
		ATSUFontID result = 0;
		if (style.GetOneAttribute(kATSUFontTag, sizeof(ATSUFontID), &result))
		{
			Str255 fontName;
			style.GetFont(fontName);
			mEnriched->setFontName(fontName);
		}
		else
			mEnriched->Ambiguate(inCommand);
		StartListening();
	}
}

void CEditFormattedTextDisplay::doSizeStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	// Is it continuous
	CATSUIStyle style;
	OSStatus err = ::ATSUGetContinuousAttributes(mTextLayout, mSelection.caret, mSelection.length, style);
	if (err == noErr)
	{
		StopListening();
		Fixed result = 12.0;
		if (style.GetOneAttribute(kATSUSizeTag, sizeof(Fixed), &result))
		{
			mEnriched->setSize(FixedToInt(result));
		}
		else
			mEnriched->Ambiguate(inCommand);
		StartListening();
	}
}

inline int operator==(const RGBColor& a, const RGBColor& b);
inline int operator==(const RGBColor& a, const RGBColor& b)
{
	return (a.red == b.red) && (a.green == b.green) && (a.blue == b.blue);
}

void CEditFormattedTextDisplay::doColorStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outEnabled = true;
	outUsesMark = true;
	outMark = noMark;

	// Is it continuous
	CATSUIStyle style;
	OSStatus err = ::ATSUGetContinuousAttributes(mTextLayout, mSelection.caret, mSelection.length, style);
	if (err == noErr)
	{
		StopListening();
		RGBColor color = { 0, 0, 0 };
		if (style.GetOneAttribute(kATSUColorTag, sizeof(RGBColor), (void*) &color))
		{
			if (color == CColorPopup::sRed)
			{
				if (inCommand == cmd_Red)
				{
					outMark = checkMark;
					mEnriched->selectColor(ered);
				}
			}
			else if (color == CColorPopup::sGreen)
			{
				if (inCommand == cmd_Green)
				{
					outMark = checkMark;
					mEnriched->selectColor(egreen);
				}
			}
			else if (color == CColorPopup::sBlue)
			{
				if (inCommand == cmd_Blue)
				{
					outMark = checkMark;
					mEnriched->selectColor(eblue);
				}
			}
			else if (color == CColorPopup::sYellow)
			{
				if (inCommand == cmd_Yellow)
				{
					outMark = checkMark;
					mEnriched->selectColor(eyellow);
				}
			}
			else if (color == CColorPopup::sCyan)
			{
				if (inCommand == cmd_Cyan)
				{
					outMark = checkMark;
					mEnriched->selectColor(ecyan);
				}
			}
			else if (color == CColorPopup::sMagenta)
			{
				if (inCommand == cmd_Magenta)
				{
					outMark = checkMark;
					mEnriched->selectColor(emagenta);
				}
			}
			else if (color == CColorPopup::sMulberry)
			{
				if (inCommand == cmd_Mulberry)
				{
					outMark = checkMark;
					mEnriched->selectColor(emulberry);
				}
			}
			else if (color == CColorPopup::sBlack)
			{
				if (inCommand == cmd_Black)
				{
					outMark = checkMark;
					mEnriched->selectColor(eblack);
				}
			}
			else if (color == CColorPopup::sWhite)
			{
				if (inCommand == cmd_White)
				{
					outMark = checkMark;
					mEnriched->selectColor(ewhite);
				}
			}
			else if (inCommand == cmd_ColorOther)
			{
				outMark = checkMark;
				mEnriched->setColor(color);
			}
		}
		else
			mEnriched->Ambiguate(cmd_Color);
		StartListening();
	}
}

void CEditFormattedTextDisplay::doColorMessage(SInt32 color)
{
	switch(color)
	{
	case ered:
		doColor(cmd_Red);
		break;
	case eblue:
		doColor(cmd_Blue);
		break;
	case egreen:
		doColor(cmd_Green);
		break;
	case eyellow:
		doColor(cmd_Yellow);
		break;
	case ecyan:
		doColor(cmd_Cyan);
		break;
	case emagenta:
		doColor(cmd_Magenta);
		break;
	case eblack:
		doColor(cmd_Black);
		break;
	case ewhite:
		doColor(cmd_White);
		break;
	case emulberry:
		doColor(cmd_Mulberry);
		break;
	case eother:
		doColor(cmd_ColorOther);
		break;
	default:
		doColor(cmd_Mulberry);
		break;
	}
}

void CEditFormattedTextDisplay::doColor(CommandT inCommand)
{
	EColor color;

	switch(inCommand)
	{
	case cmd_Red:
		color = ered;
		break;
	case cmd_Blue:
		color = eblue;
		break;
	case cmd_Green:
		color = egreen;
		break;
	case cmd_Yellow:
		color = eyellow;
		break;
	case cmd_Cyan:
		color = ecyan;
		break;
	case cmd_Magenta:
		color = emagenta;
		break;
	case cmd_Black:
		color = eblack;
		break;
	case cmd_White:
		color = ewhite;
		break;
	case cmd_Mulberry:
		color = emulberry;
		break;
	case cmd_ColorOther:
		color = eother;
		break;
	}


	mEnriched->selectColor(color);

	FocusDraw();

	SetFontColor( mEnriched->getColor());
}


void CEditFormattedTextDisplay::doAlignmentStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outEnabled = false;
}


void CEditFormattedTextDisplay::doAlignment(CommandT inCommand)
{
}

void CEditFormattedTextDisplay::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
	{

	outEnabled = false;
	outUsesMark = false;

	ResIDT	menuID;
	SInt16	menuItem;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Always enable windows menu
		switch (menuID)
		{
		case MENU_FontMain:
		case MENU_SizeMain:
			outEnabled = true;
			break;
		default:
			CFormattedTextDisplay::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		}
	}
	else
	{
		switch (inCommand)
		{
		case cmd_Style:
			if(mEnriched->IsVisible())
				outEnabled = true;
			break;
		case cmd_Bold:
		case cmd_Italic:
		case cmd_Underline:
		case cmd_Plain:
			if(mEnriched->IsVisible())
				doFormattingStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
		case cmd_Align:
			if(mEnriched->IsVisible())
				outEnabled = true;
			break;
		case cmd_JustifyLeft:
		case cmd_JustifyCenter:
		case cmd_JustifyRight:
		case cmd_JustifyFull:
			if(mEnriched->IsVisible())
				doAlignmentStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
		case cmd_Color:
			if(mEnriched->IsVisible())
				outEnabled = true;
			break;
		case cmd_Red:
		case cmd_Blue:
		case cmd_Green:
		case cmd_Yellow:
		case cmd_Cyan:
		case cmd_Magenta:
		case cmd_Black:
		case cmd_White:
		case cmd_Mulberry:
		case cmd_ColorOther:
			if(mEnriched->IsVisible())
				doColorStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
		case cmd_Font:
			if(mEnriched->IsVisible())
			{
				outEnabled = true;
				mEnriched->EnableCmd(cmd_Font);
				doFontStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			}
			break;
		case cmd_Size:
			if(mEnriched->IsVisible())
			{
				outEnabled = true;
				mEnriched->EnableCmd(cmd_Size);
				doSizeStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			}
			break;
		default:
			CFormattedTextDisplay::FindCommandStatus(inCommand, outEnabled,
								outUsesMark, outMark, outName);
			break;
		}
	}
}

void CEditFormattedTextDisplay::doFormattingCommand(CommandT inCommand)
{
	switch(inCommand)
	{
	case cmd_Bold:
		SetFontStyle(bold, !getFormattingStatus(cmd_Bold));
		break;
	case cmd_Italic:
		SetFontStyle(italic, !getFormattingStatus(cmd_Italic));
		break;
	case cmd_Underline:
		SetFontStyle(underline, !getFormattingStatus(cmd_Underline));
		break;
	case cmd_Plain:
		SetFontStyle(normal);
		break;
	}
	Refresh();
}

// Handle commands our way
Boolean CEditFormattedTextDisplay::ObeyCommand(CommandT inCommand,
					void* ioParam)
{
	bool		cmdHandled = true;
	ResIDT	menuID;
	SInt16	menuItem;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Always enable windows menu
		switch (menuID)
		{
		case MENU_FontMain:
			mEnriched->setFont(menuItem);
			break;
		case MENU_SizeMain:
			mEnriched->setSizeItem(menuItem);
			break;

		default:
			LCommander::ObeyCommand(inCommand, ioParam);
		}
	}
	else
	{
		switch (inCommand)
		{
		case cmd_Bold:
		case cmd_Italic:
		case cmd_Underline:
		case cmd_Plain:
			doFormattingCommand(inCommand);
			break;
		case cmd_JustifyLeft:
		case cmd_JustifyCenter:
		case cmd_JustifyRight:
		case cmd_JustifyFull:
			doAlignment(inCommand);
			break;
		case cmd_Red:
		case cmd_Blue:
		case cmd_Green:
		case cmd_Yellow:
		case cmd_Cyan:
		case cmd_Magenta:
		case cmd_Black:
		case cmd_White:
		case cmd_Mulberry:
		case cmd_ColorOther:
			doColor(inCommand);
			break;
		case cmd_Style:
			break;
		default:
			cmdHandled = CFormattedTextDisplay::ObeyCommand(inCommand, ioParam);
			break;
		}
	}
	return cmdHandled;
}


void CEditFormattedTextDisplay::SetToolbar(CStyleToolbar *toolbar)
{
	mEnriched = toolbar;
	mEnriched->AddListener(this);
}

void CEditFormattedTextDisplay::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch(inMessage)
	{
	case msg_Bold:
		doFormattingCommand(cmd_Bold);
		break;
	case msg_Italic:
		doFormattingCommand(cmd_Italic);
		break;
	case msg_Underline:
		doFormattingCommand(cmd_Underline);
		break;
	case msg_AlignLeft:
		if (*(long*) ioParam)
			doAlignment(cmd_JustifyLeft);
		break;
	case msg_AlignCenter:
		if (*(long*) ioParam)
			doAlignment(cmd_JustifyCenter);
		break;
	case msg_AlignRight:
		if (*(long*) ioParam)
			doAlignment(cmd_JustifyRight);
		break;
	case msg_AlignJustify:
		if (*(long*) ioParam)
			doAlignment(cmd_JustifyFull);
		break;
	case msg_Font:
		doFontCommand();
		break;
	case msg_Size:
		doSizeCommand();
		break;
	case msg_Color:
		SInt32 pa = *((SInt32 *) ioParam);
		doColorMessage(pa);
		break;
	}
}


void CEditFormattedTextDisplay::DoPaste()
{
	// Must be editable
	if (!IsReadOnly())
	{
		// Look for url on clipboard
		Handle txt = ::NewHandle(0);
		cdstring old_txt;
		bool has_utf16 = UScrap::HasData(kScrapFlavorTypeUnicode);
		bool has_text = UScrap::HasData(kScrapFlavorTypeText);
		ScrapFlavorType flavor = has_utf16 ? kScrapFlavorTypeUnicode : kScrapFlavorTypeText;

		if (txt && CPreferences::sPrefs->mSmartURLPaste.GetValue() && UScrap::GetData(flavor, txt))
		{
			StHandleLocker lock(txt);
			cdstring utf8;
			if (has_utf16)
			{
				cdustring utf16((unichar_t*) *txt, ::GetHandleSize(txt) / sizeof(unichar_t));
				utf8 = utf16.ToUTF8();
			}
			else
			{
				utf8.assign(*txt, ::GetHandleSize(txt));
			}

			const char* p = utf8.c_str();
			unsigned long hlen = utf8.length();
			unsigned long plen = hlen;

			// Look for URL scheme at start
			unsigned long scheme_len = 0;
			for(cdstrvect::const_iterator iter = CPreferences::sPrefs->mRecognizeURLs.GetValue().begin();
				(scheme_len == 0) && (iter != CPreferences::sPrefs->mRecognizeURLs.GetValue().end()); iter++)
			{
				size_t iter_len = (*iter).length();

				// Look for URL scheme prefix of URL:scheme prefix
				if ((plen > iter_len) &&
					::strncmpnocase(p, (*iter).c_str(), iter_len) == 0)
					scheme_len = iter_len;
				else if ((plen > iter_len + cURLMainSchemeLength) &&
							(::strncmpnocase(p, cURLMainScheme, cURLMainSchemeLength) == 0) &&
						 	(::strncmpnocase(p + cURLMainSchemeLength, (*iter).c_str(), iter_len) == 0))
					scheme_len = cURLMainSchemeLength + iter_len;
			}
			// Check whether a scheme was found
			if (scheme_len != 0)
			{
				// Look for all text contain valid URL characters
				plen -= scheme_len;
				p += scheme_len;
				while(plen-- && (scheme_len != 0))
				{
					// Look for valid URL character
					if (cURLXCharacter[*(unsigned char*)p++] == 0)
						// Set scheme_len to zero to indicate failure
						scheme_len = 0;
				}
				
				// Now create a new string with delimiters
				if (scheme_len != 0)
				{
					// Cache old data so it can be restored after paste
					old_txt = utf8;

					// Create new delimited URL
					cdstring new_url = "<";
					new_url.append(utf8);
					new_url += ">";
					
					// Paste as utf16
					cdustring utf16(new_url);

					// Create new handle from URL and save to scrap
					Handle new_txt = ::NewHandle(sizeof(unichar_t) * utf16.length());
					if (new_txt)
					{
						{
							StHandleLocker lock(new_txt);
							::memcpy(*new_txt, utf16.c_str(), sizeof(unichar_t) * utf16.length());
							UScrap::SetData(kScrapFlavorTypeUnicode, new_txt);
						}
						::DisposeHandle(new_txt);
					}
				}
			}
		}

		// Done with original scrap handle
		if (txt)
			::DisposeHandle(txt);

		// Do standard paste operation
		FocusDraw();
		CFormattedTextDisplay::DoPaste();
		
		// Restore old text on the clipboard
		if (!old_txt.empty())
		{
			// Create new handle from URL and save to scrap
			Handle restore_txt = ::NewHandle(old_txt.length());
			if (restore_txt)
			{
				{
					StHandleLocker lock(restore_txt);
					::memcpy(*restore_txt, old_txt.c_str(), old_txt.length());
					UScrap::SetData(kScrapFlavorTypeText, restore_txt);
				}
				::DisposeHandle(restore_txt);
			}
		}
	}
}

void CEditFormattedTextDisplay::doFontCommand()
{
	LStr255 font;
	FocusDraw();
	mEnriched->getFont(font);
	SetFontName(font);
	ScrollImageBy(0, 0, true);
}

void CEditFormattedTextDisplay::doSizeCommand()
{
	FocusDraw();
	SetFontSize(::Long2Fix(mEnriched->getSize()));
	ScrollImageBy(0, 0, true);
}

void CEditFormattedTextDisplay::ActivateSelf()
{
	mEnriched->RestoreState();
	mEnriched->Enable();
	CFormattedTextDisplay::ActivateSelf();
}

void CEditFormattedTextDisplay::DeactivateSelf()
{
	mEnriched->SaveState();
	mEnriched->Disable();
	CFormattedTextDisplay::DeactivateSelf();
	//mEnriched->setFont(0);
	//mEnriched->setSize(0);
}

void CEditFormattedTextDisplay::BeTarget()
{
	CFormattedTextDisplay::BeTarget();

	// Allow typing during busy operation
	CMailControl::AllowBusyKeys(true);
}

void CEditFormattedTextDisplay::DontBeTarget()
{
	CFormattedTextDisplay::DontBeTarget();

	// Disallow typing during busy operation
	CMailControl::AllowBusyKeys(false);
}
