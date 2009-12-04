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

#include "CMailboxTable.h"
#include "CTable.h"
#include "CTableView.h"

BEGIN_MESSAGE_MAP(CMailboxTitleTable, CTitleTableView)
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxTitleTable::CMailboxTitleTable()
{
	// Load strings if not already
	SetTitleInfo(true, true, "UI::Titles::Mailbox", eMboxColumnMax, IDR_POPUP_MAILBOX_HEADER);

	EnableToolTips();
}

// Default destructor
CMailboxTitleTable::~CMailboxTitleTable()
{
}

// Draw the titles
void CMailboxTitleTable::DrawItem(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect)
{
	switch(col_info.column_type)
	{
	case eMboxColumnFlags:
		DrawIcon(pDC, col_info, IDI_TITLE_FLAG_TITLE, cellRect);
		break;

	case eMboxColumnAttachments:
		DrawIcon(pDC, col_info, IDI_TITLE_FLAG_ATTACHMENTS, cellRect);
		break;

	case eMboxColumnMatch:
		DrawIcon(pDC, col_info, IDI_TITLE_FLAG_MATCH, cellRect);
		break;

	case eMboxColumnDisconnected:
		DrawIcon(pDC, col_info, IDI_TITLE_FLAG_DISCONNECTED, cellRect);
		break;
	
	default:
		DrawText(pDC, col_info, cellRect);
		break;
	}
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
