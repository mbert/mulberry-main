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

#include "CToDoItem.h"

#include "CCommands.h"
#include "CContextMenu.h"
#include "CNewToDoDialog.h"
#include "CCalendarTableBase.h"
#include "CCalendarViewBase.h"
#include "CGUtils.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarComponentExpanded.h"

#include <LCheckBox.h>
#include "MyCFString.h"

const float cItemHeight = 32.0;
const float cCheckboxLeftOffset = 2.0;
const float cCheckboxTopOffset = 4.0;
const float cCheckboxWidth = 16.0;
const float cCheckboxHeight = 12.0;
const float cTextTopOffset = 2.0;

const float body_transparency = 1.0;
const float line_transparency = 1.0;
const float text_transparency = 1.0;

// ---------------------------------------------------------------------------
//	CToDoItem														  [public]
/**
	Default constructor */

CToDoItem::CToDoItem(const SPaneInfo &inPaneInfo, const SViewInfo &inViewInfo, LCommander* inSuper) :
	LView(inPaneInfo, inViewInfo), LCommander(inSuper)
{
	mCompleted = NULL;
	mType = eToDo;
	mTable = NULL;
	mIsSelected = false;
	mIsCompleted = false;
	mColour = 0;
}


// ---------------------------------------------------------------------------
//	~CToDoItem														  [public]
/**
	Destructor */

CToDoItem::~CToDoItem()
{
}

CToDoItem* CToDoItem::Create(LView* parent, const HIRect& frame, LCommander* inSuper)
{
	SPaneInfo pane;
	pane.visible = true;
	pane.enabled = true;
	pane.userCon = 0L;
	pane.superView = parent;
	pane.bindings.left = true;
	pane.bindings.right = false;
	pane.bindings.top = false;
	pane.bindings.bottom = false;
	pane.paneID = 0;
	pane.width = frame.size.width;
	pane.height = frame.size.height;
	pane.left = frame.origin.x;
	pane.top = frame.origin.y;
	SViewInfo view;
	view.imageSize.width = frame.size.width;
	view.imageSize.height = frame.size.height;
	view.scrollPos.h = 0;
	view.scrollPos.v = 0;
	view.scrollUnit.h = 0;
	view.scrollUnit.v = 0;
	view.reconcileOverhang = 0;
	CToDoItem* result = new CToDoItem(pane, view, inSuper);
	try
	{
		result->FinishCreate();
	}
	catch (...)
	{									// FinishCreate failed. View is
		delete result;					//   in an inconsistent state.
		throw;							//   Delete it and rethrow.
	}

	return result;
}

#pragma mark -

void CToDoItem::FinishCreateSelf()
{
	LView::FinishCreateSelf();

	SDimension16 frame;
	GetFrameSize(frame);

	// Create the checkbox - top-left corner of control
	SPaneInfo pane;
	pane.visible = true;
	pane.enabled = true;
	pane.userCon = 0L;
	pane.superView = this;
	pane.bindings.left =
	pane.bindings.top =
	pane.bindings.right =
	pane.bindings.bottom = false;
	pane.paneID = eCheckbox_ID;
	pane.width = cCheckboxWidth;
	pane.height = cCheckboxHeight;
	pane.left = cCheckboxLeftOffset;
	pane.top = cCheckboxTopOffset;

	mCompleted = new LCheckBox(pane, eCheckbox_ID);
	SInt16 ctrl_size = kControlSizeSmall;
	mCompleted->SetDataTag(kControlEntireControl, kControlSizeTag, sizeof(SInt16), &ctrl_size);
	mCompleted->AddListener(this);

	// If currently visible, redo state setting to force checkboxes into proper state
	if (IsVisible())
	{
		Activate();
		Show();
	}
	Enable();

	CContextMenuAttachment::AddUniqueContext(this, 1831, this);
}

// Respond to clicks
void CToDoItem::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case eCheckbox_ID:
		OnCompleted();
		break;
	}
}

//	Respond to commands
Boolean CToDoItem::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_EditCalendarItem:
		OnEditToDo();
		break;

	case cmd_DuplicateCalendarItem:
		OnDuplicateToDo();
		break;

	case cmd_DeleteCalendarItem:
		OnDeleteToDo();
		break;

	case cmd_CompletedToDo:
		OnChangeCompleted();
		break;

	default:
		cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CToDoItem::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_EditCalendarItem:
	case cmd_DuplicateCalendarItem:
	case cmd_DeleteCalendarItem:
		// Always enabled if real to do item
		outEnabled = (mType == eToDo);
		break;

	case cmd_CompletedToDo:
		// Always enabled if real to do item
		outEnabled = (mType == eToDo);
		outUsesMark = true;
		outMark = mIsCompleted ? (UInt16)checkMark : (UInt16)noMark;
		break;

	default:
		LCommander::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

void CToDoItem::SetDetails(iCal::CICalendarComponentExpandedShared& todo, CCalendarTableBase* table)
{
	mVToDo = todo;
	mType = eToDo;
	mTable = table;
	mSummary = todo->GetMaster<iCal::CICalendarVToDo>()->GetSummary();
	mStatus = todo->GetMaster<iCal::CICalendarVToDo>()->GetStatusText();
	
	// Setup a help tag
	SetupTagText();

	// Set the completed status (do without handling the change of value)
	{
		StopListening();
		switch(todo->GetMaster<iCal::CICalendarVToDo>()->GetStatus())
		{
		default:
			mCompleted->SetValue(0);
			break;
		case iCal::eStatus_VToDo_Completed:
			mIsCompleted = true;
			mCompleted->SetValue(1);
			break;
		case iCal::eStatus_VToDo_Cancelled:
			mCompleted->SetVisible(false);
			break;
		}
		StartListening();
	}
	
	// Determine colour
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mVToDo->GetMaster<iCal::CICalendarVToDo>()->GetCalendar());
	if (cal)
	{
		mColour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(cal);
	}
}

void CToDoItem::SetDetails(EType type)
{
	mVToDo.reset();
	mType = type;
	mTable = NULL;
	
	mSummary = rsrc::GetIndexedString("CToDoItem::When", type);
	
	// Hide checkbox
	mCompleted->SetVisible(false);
	
	// Turn off tooltip
	EnableHelp(false);
}

Boolean CToDoItem::HandleKeyPress(const EventRecord &inKeyEvent)
{
	// Return => edit
	switch(inKeyEvent.message & charCodeMask)
	{
	case char_Enter:
	case char_Return:
		// Only if real to do
		if (mType == eToDo)
			OnEditToDo();
		return true;
	}
	
	return false;
}

// Click
void CToDoItem::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// Only if real to do
	if (mType == eToDo)
	{
		bool was_selected = mIsSelected;
		
		// Always change focus to this one
		SwitchTarget(this);

		// Look for toggle of state
		if ((inMouseDown.macEvent.modifiers & (shiftKey | cmdKey)) != 0)
			SelectToDo(!was_selected);

		// Broadcast double-click if in month area
		if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		{
			OnEditToDo();
		}
	}
}

void CToDoItem::BeTarget()
{
	Activate();

	// Only if real to do
	if (mType == eToDo)
		SelectToDo(true);
}

void CToDoItem::DontBeTarget()
{
	Deactivate();

	// Only if real to do
	if (mType == eToDo)
		SelectToDo(false);
}

void CToDoItem::ApplyForeAndBackColors() const
{
	Pattern	whitePat;
	UQDGlobals::GetWhitePat(&whitePat);
	::BackPat(&whitePat);

	::RGBForeColor(&Color_Black);

	float red = CGUtils::GetCGRed(mColour);
	float green = CGUtils::GetCGGreen(mColour);
	float blue = CGUtils::GetCGBlue(mColour);
	if (!mIsSelected)
	{
		CGUtils::LightenColours(red, green, blue, true);
	}

	RGBColor back = CGUtils::GetQDColor(red, green, blue);
	::RGBBackColor(&back);
}

void CToDoItem::DrawSelf()
{
	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	Rect qdrect;
	CalcLocalFrameRect(qdrect);
	HIRect rect;
	CGUtils::QDToHIRect(qdrect, rect);
	rect = ::CGRectInset(rect, 1.0, 1.0);

	if (mType == eToDo)
	{
		// Use unsaturated colour for selected item
		float red = CGUtils::GetCGRed(mColour);
		float green = CGUtils::GetCGGreen(mColour);
		float blue = CGUtils::GetCGBlue(mColour);
		if (mIsSelected)
		{
			::CGContextSetRGBFillColor(inContext, red, green, blue, 1.0);
		}
		else
		{
			CGUtils::LightenColours(red, green, blue, true);
			::CGContextSetRGBFillColor(inContext, red, green, blue, body_transparency);
		}
		
		::CGContextFillRect(inContext, rect);
	}
	else
	{
		ThemeButtonDrawInfo theme;
		theme.state = (IsActive() && IsEnabled()) ? kThemeStateActive : kThemeStateInactive;
		theme.value = kThemeButtonOff;
		theme.adornment = kThemeAdornmentNone;
		::DrawThemeButton(&qdrect, kThemeSmallBevelButton, &theme, NULL, NULL, NULL, NULL);
	}

	// Draw summary text
	rect = ::CGRectInset(rect, 3.0, 0.0);
	Rect box;
	CGUtils::HIToQDRect(rect, box);
	if (mType == eToDo)
		box.left += cCheckboxLeftOffset + cCheckboxWidth;
	box.top += cTextTopOffset;
	box.bottom = box.top + cItemHeight / 2.0;
	float red = CGUtils::GetCGRed(mColour);
	float green = CGUtils::GetCGGreen(mColour);
	float blue = CGUtils::GetCGBlue(mColour);
	if (mIsSelected)
	{
		::CGContextSetGrayFillColor(inContext, (red + green + blue > 2.5) ? 0.0 : 1.0, 1.0);
	}
	else
	{
		CGUtils::DarkenColours(red, green, blue);
		::CGContextSetRGBFillColor(inContext, red, green, blue, text_transparency);
	}
	MyCFString trunc1(mSummary, kCFStringEncodingUTF8);
	::TruncateThemeText(trunc1, kThemeSmallSystemFont, kThemeStateActive, box.right - box.left, truncEnd, NULL);
	::DrawThemeTextBox(trunc1, kThemeSmallSystemFont, kThemeStateActive, false, &box, (mType == eToDo) ? teJustLeft : teJustCenter, inContext);

	// Draw status text
	CGUtils::HIToQDRect(rect, box);
	box.top +=cItemHeight / 2.0;

	MyCFString trunc2(mStatus, kCFStringEncodingUTF8);
	::TruncateThemeText(trunc2, kThemeSmallSystemFont, kThemeStateActive, box.right - box.left, truncEnd, NULL);
	::DrawThemeTextBox(trunc2, kThemeSmallSystemFont, kThemeStateActive, false, &box, (mType == eToDo) ? teJustLeft : teJustCenter, inContext);
}

void CToDoItem::OnChangeCompleted()
{
	// Toggle checkbox to trigger action
	mCompleted->SetValue(1 - mCompleted->GetValue());
}

void CToDoItem::OnCompleted()
{
	// Only if real to do
	if (mType != eToDo)
		return;

	// Change the completed status on the event but do not force an update to the view right now
	mVToDo->GetMaster<iCal::CICalendarVToDo>()->EditStatus(mCompleted->GetValue() == 1 ? iCal::eStatus_VToDo_Completed : iCal::eStatus_VToDo_NeedsAction);
	mVToDo->GetMaster<iCal::CICalendarVToDo>()->Changed();
	mIsCompleted = (mCompleted->GetValue() == 1);
	mStatus = mVToDo->GetMaster<iCal::CICalendarVToDo>()->GetStatusText();
	
	// Redo tooltip for change
	SetupTagText();
	
	// Refresh as status may have changed
	FRAMEWORK_REFRESH_WINDOW(this);
}

