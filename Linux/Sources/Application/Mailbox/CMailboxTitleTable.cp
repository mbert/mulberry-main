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


// Source for CMailboxTitleTable class

#include "CMailboxTitleTable.h"

#include "CMessageListFwd.h"
#include "CTableView.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxTitleTable::CMailboxTitleTable(JXScrollbarSet* scrollbarSet,	
										 JXContainer* enclosure,	
										 const HSizingOption hSizing, 
										 const VSizingOption vSizing,
										 const JCoordinate x, 
										 const JCoordinate y,
										 const JCoordinate w, 
										 const JCoordinate h)
  : CTitleTableView(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mCanSort = true;
}


// Default destructor
CMailboxTitleTable::~CMailboxTitleTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMailboxTitleTable::OnCreate(void)
{
	// Do inherited
	CTitleTableView::OnCreate();

	cdstring menu = "To %r | From %r | Reply To %r | Sender %r | Cc %r | Subject %r | Thread %r |"
		" Date Sent %r | Date Received %r | Size %r | Flags %r | Number %r | Smart Address %r |"
		" Attachments %r | Parts %r | Matching %r | Disconnected %r %l |"
		" Insert Column after | Insert Column before | Delete Column";

	SetTitleInfo(true, true, "UI::Titles::Mailbox", eMboxColumnMax, menu);
}

// Draw the titles
void CMailboxTitleTable::MenuChoice(TableIndexT col, bool sort_col, JIndex menu_item)
{
	// Handle menu result
	switch (menu_item)
	{
	case cColumnTo:
		mTableView->SetColumnType(col, eMboxColumnTo);
		if (sort_col) mTableView->SetSortBy(cSortMessageTo);
		break;

	case cColumnFrom:
		mTableView->SetColumnType(col, eMboxColumnFrom);
		if (sort_col) mTableView->SetSortBy(cSortMessageFrom);
		break;

	case cColumnReplyTo:
		mTableView->SetColumnType(col, eMboxColumnReplyTo);
		if (sort_col) mTableView->SetSortBy(cSortMessageReplyTo);
		break;

	case cColumnSender:
		mTableView->SetColumnType(col, eMboxColumnSender);
		if (sort_col) mTableView->SetSortBy(cSortMessageSender);
		break;

	case cColumnCC:
		mTableView->SetColumnType(col, eMboxColumnCc);
		if (sort_col) mTableView->SetSortBy(cSortMessageCc);
		break;

	case cColumnSubject:
		mTableView->SetColumnType(col, eMboxColumnSubject);
		if (sort_col) mTableView->SetSortBy(cSortMessageSubject);
		break;

	case cColumnThread:
		mTableView->SetColumnType(col, eMboxColumnThread);
		if (sort_col) mTableView->SetSortBy(cSortMessageThread);
		break;

	case cColumnDateSent:
		mTableView->SetColumnType(col, eMboxColumnDateSent);
		if (sort_col) mTableView->SetSortBy(cSortMessageDateSent);
		break;

	case cColumnDateReceived:
		mTableView->SetColumnType(col, eMboxColumnDateReceived);
		if (sort_col) mTableView->SetSortBy(cSortMessageDateReceived);
		break;

	case cColumnSize:
		mTableView->SetColumnType(col, eMboxColumnSize);
		if (sort_col) mTableView->SetSortBy(cSortMessageSize);
		break;

	case cColumnFlags:
		mTableView->SetColumnType(col, eMboxColumnFlags);
		if (sort_col) mTableView->SetSortBy(cSortMessageFlags);
		break;

	case cColumnNumber:
		mTableView->SetColumnType(col, eMboxColumnNumber);
		if (sort_col) mTableView->SetSortBy(cSortMessageNumber);
		break;

	case cColumnSmart:
		mTableView->SetColumnType(col, eMboxColumnSmart);
		if (sort_col) mTableView->SetSortBy(cSortMessageSmart);
		break;

	case cColumnAttachments:
		mTableView->SetColumnType(col, eMboxColumnAttachments);
		if (sort_col) mTableView->SetSortBy(cSortMessageAttachment);
		break;

	case cColumnParts:
		mTableView->SetColumnType(col, eMboxColumnParts);
		if (sort_col) mTableView->SetSortBy(cSortMessageParts);
		break;

	case cColumnMatch:
		mTableView->SetColumnType(col, eMboxColumnMatch);
		if (sort_col) mTableView->SetSortBy(cSortMessageMatching);
		break;

	case cColumnDisconnected:
		mTableView->SetColumnType(col, eMboxColumnDisconnected);
		if (sort_col) mTableView->SetSortBy(cSortMessageDisconnected);
		break;

	case cColumnInsertAfter:
		mTableView->InsertColumn(col + 1);
		break;

	case cColumnInsertBefore:
		mTableView->InsertColumn(col);
		break;

	case cColumnDelete:
		mTableView->DeleteColumn(col);
		break;
	}
}


// Draw the titles
void CMailboxTitleTable::DrawItem(JPainter* pDC,SColumnInfo& col_info, const Rect &inLocalRect)
{
	unsigned int icon_id = 0;
	switch(col_info.column_type)
	{
	case eMboxColumnFlags:
		icon_id = IDI_TITLE_FLAG_TITLE;
		break;

	case eMboxColumnAttachments:
		icon_id = IDI_TITLE_FLAG_ATTACHMENTS;
		break;

	case eMboxColumnMatch:
		icon_id = IDI_TITLE_FLAG_MATCH;
		break;

	case eMboxColumnDisconnected:
		icon_id = IDI_TITLE_FLAG_DISCONNECTED;
		break;

	default:
		DrawText(pDC, col_info, inLocalRect);
		return;
	}

	DrawIcon(pDC, col_info, icon_id, inLocalRect);
}

// Check for right justification
bool CMailboxTitleTable::RightJustify(int col_type)
{
	switch(col_type)
	{
	case eMboxColumnSize:
	case eMboxColumnNumber:
	case eMboxColumnParts:
		return true;
	default:
		return false;
	}
}

// Get text for current tooltip cell
void CMailboxTitleTable::GetTooltipText(cdstring& txt, const STableCell& cell)
{
	// Determine which heading it is and draw it
	SColumnInfo col_info = GetColumns()[cell.col - 1];

	switch(col_info.column_type)
	{
	case eMboxColumnTo:
		txt = "The addresses to which the message was sent";
		break;
	case eMboxColumnFrom:
		txt = "The addresses from which the message was received";
		break;
	case eMboxColumnReplyTo:
		txt = "The addresses to which replies should be sent";
		break;
	case eMboxColumnSender:
		txt = "The addresses of the actual sender of the message";
		break;
	case eMboxColumnCc:
		txt = "The addresses of other recipients of the message";
		break;
	case eMboxColumnSubject:
		txt = "The subject of the message";
		txt += os_endl;
		txt += "Sorting will be alphabetic by subject";
		break;
	case eMboxColumnThread:
		txt = "The subject of the message";
		txt += os_endl;
		txt += "Messages will be hierarchically threaded when clicked";
		break;
	case eMboxColumnDateSent:
		txt = "The date and time the message was sent";
		break;
	case eMboxColumnDateReceived:
		txt = "The date and time the message was received";
		break;
	case eMboxColumnSize:
		txt = "The size of the message";
		break;
	case eMboxColumnFlags:
		txt = "The primary state of the message based on its flags";
		break;
	case eMboxColumnNumber:
		txt = "The message number";
		break;
	case eMboxColumnSmart:
		txt = "The addresses of the correspondents of the message";
		break;
	case eMboxColumnAttachments:
		txt = "Indicates whether attachments are present";
		break;
	case eMboxColumnParts:
		txt = "The number of parts in the message";
		break;
	case eMboxColumnMatch:
		txt = "Indicates whether the message is currently";
		txt += os_endl;
		txt += "part of the matched set of messages";
		break;
	case eMboxColumnDisconnected:
		txt = "Indicates whether the message is not locally";
		txt += os_endl;
		txt += "cached, partially cached or fully cached";
		break;

	default:;
	}
}
