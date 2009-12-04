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


// Source for CHelpAttach class

#include "CContextMenu.h"

#include "CCopyToMenu.h"
#include "CLog.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"
#include "CToolbar.h"

//#include <ContextualMenu.h>

#pragma mark ____________________________CContextMenuProcessAttachment

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

bool CContextMenuProcessAttachment::sHasCMM = false;
EventRecord CContextMenuProcessAttachment::sCMMEvent;
bool CContextMenuProcessAttachment::sProcessing = false;

std::map<ResIDT, LMenu*> CContextMenuAttachment::sContextMenus;

// Default constructor
CContextMenuProcessAttachment::CContextMenuProcessAttachment(LApplication* itsApp)
		: LAttachment(msg_Event, true)
{
	mApp = itsApp;

	InitCMM();
}

CContextMenuProcessAttachment::CContextMenuProcessAttachment(LStream *inStream)
		: LAttachment(inStream)
{
	InitCMM();
}

void CContextMenuProcessAttachment::InitCMM(void)
{
	// Check for CMM
	sHasCMM = UEnvironment::HasGestaltAttribute(gestaltContextualMenuAttr, gestaltContextualMenuUnusedBit) &&
				UEnvironment::HasGestaltAttribute(gestaltContextualMenuAttr, gestaltContextualMenuTrapAvailable);

	// Register with CMM
	if (sHasCMM)
	{
		OSErr theErr = InitContextualMenus();
		if (theErr != noErr)
			sHasCMM = false;
	}
}

// O T H E R  M E T H O D S ____________________________________________________________________________

bool CContextMenuProcessAttachment::IsCMMEvent(const EventRecord& event)
{
	if (sHasCMM)
		return IsShowContextualMenuClick(&event);
	else
		return (event.what == mouseDown) && (event.modifiers & controlKey);
}

// Do context
void CContextMenuProcessAttachment::ExecuteSelf(MessageT inMessage, void *ioParam)
{
	bool do_cmm = false;

	// Determine if this event should show a contextual menu or not
	do_cmm = IsCMMEvent(*(EventRecord*) ioParam);

	if (do_cmm)
	{
		// cache this event
		sCMMEvent = *static_cast<EventRecord*>(ioParam);

		StValueChanger<bool> _change(sProcessing, true);

										// Find out where the mouse is
		WindowPtr	macWindowP;
		Point		globalMouse = static_cast<EventRecord*>(ioParam)->where;
		SInt16		thePart = ::FindWindow(globalMouse, &macWindowP);

		if (macWindowP != NULL) {		// Mouse is inside a Window

			LWindow	*theWindow = LWindow::FetchWindowObject(macWindowP);
			if ((theWindow != NULL) &&
				theWindow->IsActive() &&
				theWindow->IsEnabled()) {
										// Mouse is inside an active and enabled
										//   PowerPlant Window. Let the Window
										//   adjust the cursor shape.

										// Get mouse location in Port coords
				Point	portMouse = globalMouse;
				theWindow->GlobalToPortPoint(portMouse);

				// Find the top pane under the mouse
				LPane* hitPane = theWindow->FindDeepSubPaneContaining(portMouse.h, portMouse.v);

				// Execute the CContextMenuAttachment if the pane exists
				while(hitPane)
				{
					// Note CContextMenuAttachment will return execute host false when it is present
					if (hitPane->ExecuteAttachments(msg_DoContextualMenu, (void*) hitPane))
						// Go to next pane
						hitPane = hitPane->GetSuperView();
					else
					{
						// No more processing of this event - context menu handled it
						SetExecuteHost(false);
						break;
					}
				}
			}
		}
	}
	else
		// Carry on with processing
		SetExecuteHost(true);
}

#pragma mark ____________________________CContextMenuAttachment

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CContextMenuAttachment::CContextMenuAttachment(ResIDT menu_id, LCommander* cmdr, bool execute_click)
		: LAttachment(msg_DoContextualMenu, true)
{
	mMenuID = menu_id;
	mCmdr = cmdr;
	mExecuteClick = execute_click;
}

CContextMenuAttachment::CContextMenuAttachment(LStream *inStream)
		: LAttachment(inStream)
{
	// Always force to our message
	mMessage = msg_DoContextualMenu;

	// Cache the resource info
	*inStream >> mMenuID;

	mCmdr = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CContextMenuAttachment::AddUniqueContext(LAttachable* attach_to, ResIDT menu_id, LCommander* cmdr, bool execute_click)
{
	// Send message to delete any existing context menu attachments
	attach_to->ExecuteAttachments(msg_DoContextualMenu, reinterpret_cast<void*>(-1L));

	// Now add it
	attach_to->AddAttachment(new CContextMenuAttachment(menu_id, cmdr, execute_click));
}

void CContextMenuAttachment::AddUniqueContext(LAttachable* attach_to, LAttachment* attach)
{
	// Send message to delete any existing context menu attachments
	attach_to->ExecuteAttachments(msg_DoContextualMenu, reinterpret_cast<void*>(-1L));

	// Now add it
	attach_to->AddAttachment(attach);
}

// Do context
void CContextMenuAttachment::ExecuteSelf(MessageT inMessage, void *ioParam)
{
	// To make it unqiue may need to delete
	if (reinterpret_cast<long>(ioParam) == -1L)
	{
		delete this;
		return;
	}

	SetExecuteHost(false);

	// No menu id => do not do any context menu at all even if superview has one
	if (!mMenuID)
		return;

	// Intercept toolbar clicks before anything else
	LPane* hitPane = static_cast<LPane*>(ioParam);
	if (dynamic_cast<CToolbar*>(hitPane) != NULL)
	{
		CToolbar* toolbar = dynamic_cast<CToolbar*>(hitPane);
		
		// Pass to text object first to see if it wants to override default behaviour
		toolbar->HandleContextMenuEvent(CContextMenuProcessAttachment::CMMEvent());
		return;
	}

	LCommander* cmdr = NULL;
	if (mCmdr)
		cmdr = mCmdr;
	else
		cmdr = LCommander::GetDefaultCommander();

	// If it was a click allow it to go through normal processing
	if (mExecuteClick && (CContextMenuProcessAttachment::CMMEvent().what == mouseDown))
		LEventDispatcher::GetCurrentEventDispatcher()->DispatchEvent(CContextMenuProcessAttachment::CMMEvent());
	else if ((CContextMenuProcessAttachment::CMMEvent().what == mouseDown) && !cmdr->IsTarget())
		LCommander::SwitchTarget(cmdr);

	// Determine if text pane
	if (dynamic_cast<CTextDisplay*>(hitPane) != NULL)
	{
		CTextDisplay* tdisplay = dynamic_cast<CTextDisplay*>(hitPane);
		
		// Pass to text object first to see if it wants to override default behaviour
		if (tdisplay->HandleContextMenuEvent(CContextMenuProcessAttachment::CMMEvent()))
			return;
	}

	// Now copy items from actual menu into this menu
	try
	{
		// Get menu to use from static map
		LMenu* menuH2 = GetMenu(mMenuID);

		// Process the command status for this entire menu
		LEventDispatcher::GetCurrentEventDispatcher()->UpdateMenus();

		// Now determine whether we do Sys 7 popup menu or proper Contextual Menu
		short menu_hit = 0;
		unsigned short item_hit = 0;
		if (CContextMenuProcessAttachment::HasCMM())
		{
			bool use_ae = false;
			AEDesc ae_desc;
			Handle text_handle = NULL;

			// Determine if text pane
			if (dynamic_cast<CTextDisplay*>(hitPane))
			{
				CTextDisplay* tdisplay = dynamic_cast<CTextDisplay*>(hitPane);
				
				use_ae = true;

				// Get current selection and text
				SInt32 sel_start;
				SInt32 sel_end;
				tdisplay->GetSelectionRange(sel_start, sel_end);

				cdstring txt;
				if (sel_start == sel_end)
				{
					tdisplay->GetText(txt);
				}
				else
				{
					tdisplay->GetSelectedText(txt);
				}
				text_handle = ::NewHandle(txt.length());
				StHandleLocker _lock(text_handle);
				::memcpy(*text_handle, txt.c_str(), txt.length());
			}
			else if (dynamic_cast<CTextFieldX*>(hitPane))
			{
				CTextFieldX* tfield = dynamic_cast<CTextFieldX*>(hitPane);
				use_ae = true;
				
				// Get current selection and text
				SInt32 sel_start;
				SInt32 sel_end;
				tfield->GetSelection(&sel_start, &sel_end);

				cdstring txt;
				if (sel_start == sel_end)
				{
					tfield->GetText(txt);
				}
				else
				{
					tfield->GetSelectedText(txt);
				}
				text_handle = ::NewHandle(txt.length());
				StHandleLocker _lock(text_handle);
				::memcpy(*text_handle, txt.c_str(), txt.length());
			}

			// Create AEDec if in use
			if (use_ae)
			{
#if ACCESSOR_CALLS_ARE_FUNCTIONS
				OSErr	err = ::AECreateDesc(typeText, *text_handle, ::GetHandleSize(text_handle), &ae_desc);
				ThrowIfOSErr_(err);
#else
				ae_desc.descriptorType = typeText;
				ae_desc.dataHandle = text_handle;
#endif
			}

			Str255 help_str = "\p";
			unsigned long result_type;
			if (::ContextualMenuSelect(menuH2->GetMacMenuH(),
										CContextMenuProcessAttachment::CMMEvent().where,
										false, kCMHelpItemNoHelp, help_str, use_ae ? &ae_desc : NULL,
										&result_type, &menu_hit, &item_hit) == noErr)
			{
				switch(result_type)
				{
				case kCMNothingSelected:
				case kCMShowHelpSelected:
					item_hit = 0;
					break;
				default:;
				}
			}
			else
				item_hit = 0;

			// Dispose of text handle
			if (text_handle)
				::DisposeHandle(text_handle);
		}
		else
		{
			long result = ::PopUpMenuSelect(menuH2->GetMacMenuH(),
											CContextMenuProcessAttachment::CMMEvent().where.v,
											CContextMenuProcessAttachment::CMMEvent().where.h, 0);
			menu_hit = HiWord(result);
			item_hit = LoWord(result);
		}

		if (item_hit)
		{
			// Get command number
			CommandT cmd;

			// Check for hit in this menu
			if (menu_hit == mMenuID)
				cmd = menuH2->CommandFromIndex(item_hit);
			else
				// Must try and get menu from menu bar
				cmd = LMenuBar::GetCurrentMenuBar()->FindCommand(menu_hit, item_hit);

			// Look for intercept
			Point portMouse = CContextMenuProcessAttachment::CMMEvent().where;
			hitPane->GlobalToPortPoint(portMouse);
			LPane* deepPane = hitPane->FindDeepSubPaneContaining(portMouse.h, portMouse.v);
			if (deepPane == NULL)
				deepPane = hitPane;
			if ((dynamic_cast<CContextCommandIntercept*>(deepPane) == NULL) ||
				!dynamic_cast<CContextCommandIntercept*>(deepPane)->ObeyContextCommand(cmd, CContextMenuProcessAttachment::CMMEvent()))
				cmdr->ObeyCommand(cmd);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}
}

LMenu* CContextMenuAttachment::GetMenu(ResIDT resid)
{
	LMenu* result = NULL;
	std::map<ResIDT, LMenu*>::const_iterator found = sContextMenus.find(resid);
	if (found == sContextMenus.end())
	{
		result = new LMenu(resid);
		sContextMenus[resid] = result;
		LMenuBar::GetCurrentMenuBar()->InstallMenu(result, -1);
	}
	else
		result = (*found).second;
	
	return result;
}

#pragma mark ____________________________CCopyToContextMenuAttachment


// Do context
void CCopyToContextMenuAttachment::ExecuteSelf(MessageT inMessage, void *ioParam)
{
	// Always do update of copy to menu - this will only be done if its dirty
	CCopyToMenu::ResetMenuList();

	// Now do default
	CContextMenuAttachment::ExecuteSelf(inMessage, ioParam);
}


#pragma mark ____________________________CNULLContextMenuAttachment


// Do context
void CNULLContextMenuAttachment::ExecuteSelf(MessageT inMessage, void *ioParam)
{
	// Process the event normally - ignore context
	SetExecuteHost(false);
	LEventDispatcher::GetCurrentEventDispatcher()->DispatchEvent(CContextMenuProcessAttachment::CMMEvent());
}