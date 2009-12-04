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


//	CFileTable.cp


#include "CFileTable.h"

#include "CAdminLock.h"
#include "CAliasAttachment.h"
#include "CAttachment.h"
#include "CAttachmentList.h"
#include "CCommands.h"
#include "CDataAttachment.h"
#include "CErrorHandler.h"
#include "CFileAttachment.h"
//#include "CFileTableAction.h"
#include "CIconLoader.h"
#include "CMessage.h"
#include "CMessageList.h"
#include "CMessagePartProp.h"
#include "CLetterPartProp.h"
#include "CLetterWindow.h"
#include "CMessageAttachment.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulSelectionData.h"
#include "CNodeVectorTree.h"
#include "CPreferences.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CXStringResources.h"

#include <UNX_LTableArrayStorage.h>

#include <jASCIIConstants.h>
#include <JXTextMenu.h>
#include <JXImage.h>
#include <JPainter.h>
#include "CMainMenu.h"
#include "CSimpleTitleTable.h"
#include <JXChooseSaveFile.h>
#include <JXDNDManager.h>
#include <jXGlobals.h>
#include <jXUtil.h>
#include <jFileUtil.h>
#include <JString.h>
#include <JPtrArray.h>

const TableIndexT cHierarchyCol = 3;

const long cColumnSpace = 398;

CFileTable::CFileTable(JXScrollbarSet* scrollbarSet, 
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h)
	: CHierarchyTableDrag(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	SetBorderWidth(0);

	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(CAttachment*));

	SetRowSelect(true);

	mBody = NULL;
	mAddAction = NULL;
	mRowShow = 0;
	mHierarchyCol = cHierarchyCol;
	mWindow = NULL;
	mTitles = NULL;
	mDirty = false;
	mAttachmentsOnly = false;
	mLocked = false;
}


CFileTable::~CFileTable()
{
}

void CFileTable::OnCreate()
{
	mWindow = dynamic_cast<CLetterWindow*>(GetWindow()->GetDirector());

	CHierarchyTableDrag::OnCreate();

	// Get keys but not Tab which is used to shift focus
	WantInput(kTrue);

	// Drag and drop
	AddDropFlavor(CMulberryApp::sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sFlavorAtchList);
	AddDropFlavor(CMulberryApp::sFlavorMsgAtchList);

	// Only if not locked out
	if (!CAdminLock::sAdminLock.mNoAttachments)
		AddDropFlavor(GetSelectionManager()->GetURLXAtom());

	AddDragFlavor(CMulberryApp::sFlavorAtchList);

	SetReadOnly(false);
	SetDropCell(true);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetAllowMove(true);
	SetSelfDrag(true);

	// Create columns and adjust flag rect
	InsertCols(6, 1);

	JCoordinate cx = GetApertureWidth();
	// Name column has variable width
	SetColWidth(cx > cColumnSpace ? cx - cColumnSpace : 32, 4, 4);
	// Remaining columns have fixed width
	SetColWidth(16, 1, 1);
	SetColWidth(16, 2, 2);
	SetColWidth(238, 3, 3);
	SetColWidth(48, 5, 5);
	SetColWidth(80, 6, 6);

	// Context menu
	CreateContextMenu(CMainMenu::eContextLetterParts);

	return;
}

// Resize columns
void CFileTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	CHierarchyTableDrag::ApertureResized(dw, dh);

	if (mCols)
	{
		JCoordinate cw = GetApertureWidth() - (mAttachmentsOnly ? 286 : 398);
		if (cw < 32)
			cw = 32;

		if (mAttachmentsOnly)
		{
			// Name column has variable width
			SetColWidth(cw, 1, 1);
			
			// Remaining columns have fixed width
			SetColWidth(48, 2, 2);
			SetColWidth(238, 3, 3);
		}
		else
		{
			// Name column has variable width
			SetColWidth(cw, 4, 4);
			
			// Remaining columns have fixed width
			SetColWidth(16, 1, 1);
			SetColWidth(16, 2, 2);
			SetColWidth(238, 3, 3);
			SetColWidth(48, 5, 5);
			SetColWidth(80, 6, 6);
		}
		
		if (mTitles != NULL)
			mTitles->SyncTable(this, true);
	}
}

void CFileTable::SetBody(CAttachment* aBody)
{
	mBody = aBody;

	// Reset table based on new body
	ResetTable();
}

void CFileTable::SetRowShow(CAttachment* attach)
{
	TableIndexT rows;
	GetWideOpenTableSize(rows);

	// Look at each cell
	for(int woRow = 1; woRow <= rows; woRow++)
	{
		STableCell	woCell(woRow, 1);
		CAttachment* row_attach = NULL;
		UInt32 dataSize = sizeof(CAttachment*);
		GetCellData(woCell, &row_attach, dataSize);
		
		if (attach == row_attach)
		{
			mRowShow = woRow;
			Refresh();
			return;
		}
	}	
}

void CFileTable::SetAttachmentsOnly(bool attachments)
{
	if (attachments != mAttachmentsOnly)
	{
		mAttachmentsOnly = attachments;

		// Change columns before resetting to ensure
		// data stored in cells is consistent
		TableIndexT new_cols = mAttachmentsOnly ? 3 : 6;
		TableIndexT old_cols = mCols;

		if (old_cols > new_cols)
			RemoveCols(old_cols - new_cols, 1, false);
		else if (old_cols < new_cols)
			InsertCols(new_cols - old_cols, 1, NULL, 0, false);
		
		ApertureResized(0, 0);
		SetOneColumnSelect(mAttachmentsOnly ? 1 : 3);
		mHierarchyCol = mAttachmentsOnly ? 1 : 3;
		
		// Change titles
		if (mTitles != NULL)
		{
			mTitles->SyncTable(this, true);
			mTitles->LoadTitles(mAttachmentsOnly ? "UI::Titles::LetterAttachments" : "UI::Titles::LetterParts", mAttachmentsOnly ? 3 : 6, true);
		}
		
		// See if reset is needed
		if (GetBody())
		{
			CAttachment* attach = GetPartShow();
			ResetTable();
			SetRowShow(attach);
		}
	}
}

// Get attachment from row
CAttachment* CFileTable::GetAttachment(TableIndexT row, bool is_worow)
{
	if (row == 0)
		return NULL;

	TableIndexT	woRow = is_worow ? row : GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	CAttachment* attach;
	UInt32 dataSize = sizeof(CAttachment*);
	GetCellData(woCell, &attach, dataSize);
	
	return attach;
}

CAttachment* CFileTable::GetPartShow()
{
	STableCell	woCell(mRowShow, 1);
	CAttachment* attach = NULL;
	UInt32 dataSize = sizeof(CAttachment*);
	if (mRowShow != 0)
		GetCellData(woCell, &attach, dataSize);
	
	return attach;
}

unsigned long CFileTable::CountParts() const
{
	if (GetAttachmentsOnly())
		return mRows;
	else if (mBody != NULL)
		return mBody->CountParts();
	else
		return 0;
}

bool CFileTable::HasAttachments() const
{
	if (GetAttachmentsOnly())
		return mRows > 0;
	else if (mBody != NULL)
		return mBody->CountParts() > 1;
	else
		return false;
}

// Reset the table from the mbox
void CFileTable::ResetTable(void)
{
	// Prevent window update during changes
	StDeferTableAdjustment changing(this);

	TableIndexT old_show = mRowShow;

	// Delete all existing rows (use inherited function to prevent attachment delete)
	Clear();

	if (mBody)
	{
		TableIndexT next_row = 0;

		// Add main part
		InsertPart(next_row, mBody, false);

		// Expand top rows only (when not in attachments mode)
		if (IsCollapsable(1) && !mAttachmentsOnly)
			ExpandRow(1);
	}

	// Restore previous row show
	mRowShow = old_show;
}

TableIndexT CFileTable::InsertPart(TableIndexT& parentRow, CAttachment* part, bool child, int pos)
{
	TableIndexT child_insert = parentRow;

	bool is_attachment = (dynamic_cast<CDataAttachment*>(part) == NULL) || !part->IsMultipart() && !part->CanEdit();

	// Insert this part
	if (!mAttachmentsOnly || is_attachment)
	{
		// Insert this part
		if (child)
		{
			// Determine position of part and how to insert
			if (pos == 0)
			{
				InsertChildRows(1, parentRow, &part, sizeof(CAttachment*), part->IsMultipart() || part->IsMessage(), false);
				child_insert = parentRow + 1;
			}
			else if (pos < 0)
			{
				AddLastChildRow(parentRow, &part, sizeof(CAttachment*), part->IsMultipart() || part->IsMessage(), false);
				child_insert = parentRow + CountAllDescendents(parentRow);
			}
			else
			{
				TableIndexT woRow = static_cast<CNodeVectorTree*>(mCollapsableTree)->SiblingIndex(parentRow, pos);
				child_insert = InsertSiblingRows(1, woRow, &part, sizeof(CAttachment*), part->IsMultipart() || part->IsMessage(), false);
			}
		}
		else
			// Update to new parent
			child_insert = InsertSiblingRows(1, parentRow, &part, sizeof(CAttachment*), part->IsMultipart() || part->IsMessage(), false);
	}

	// Add children (if there)
	if ((part->IsMultipart() || part->IsMessage()) && part->GetParts())
	{
		// Add each sibling
		for(CAttachmentList::iterator iter = part->GetParts()->begin(); iter != part->GetParts()->end(); iter++)
			InsertPart(child_insert, *iter, mRows > 0);
	}
	
	// Adjust visible part position
	if (child_insert <= mRowShow)
		mRowShow += CountAllDescendents(child_insert) + 1;
	
	return child_insert;
}

// Handle update of visible part
void CFileTable::DeleteSelection(void)
{
	// Force update of current part
	mWindow->SyncPart();

	// Do inherited action
	DoToSelection((DoToSelectionPP) &CFileTable::DeleteRow, false);

	// Force change of multipart structure
	if (mBody)
		ConvertMultipart(false);
	
	// Update row show
	UpdateRowShow();
}

// DeleteRow
bool CFileTable::DeleteRow(TableIndexT row)
{
	UInt32	woRow = GetWideOpenIndex(row);
	RemoveRows(1, woRow, true);
	return true;
}

// Remove rows and adjust parts
void CFileTable::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh)
{
	// Always called with inHowMany == 1
	
	// Look for visible row in hierarchy to be deleted
	if ((mRowShow >= inFromRow) && (mRowShow <= inFromRow + CountAllDescendents(inFromRow)))
	{
		mRowShow = 0;
		mWindow->mCurrentPart = NULL;
	}

	// Look for change in position of row show
	if (mRowShow > inFromRow + CountAllDescendents(inFromRow))
		mRowShow -= 1 + CountAllDescendents(inFromRow);

	// Get attachment for this row
	STableCell woCell(inFromRow, 1);
	CAttachment* attach;
	UInt32 dataSize = sizeof(CAttachment*);
	GetCellData(woCell, &attach, dataSize);
	
	// Remove it from its parent (will be deleted)
	if (attach->GetParent())
		attach->GetParent()->RemovePart(attach);
	else
	{
		// Must be root part so delete
		delete mBody;
		mBody = NULL;
		mWindow->SetBody(NULL);
	}
	
	// Do inherited action
	CHierarchyTableDrag::RemoveRows(inHowMany, inFromRow, inRefresh);
	
	SetDirty(true);
}

// Test for selected message deleted
bool CFileTable::TestSelectionChangeable(TableIndexT row)
{
	// Is it changeable?
	CAttachment* attach = GetAttachment(row);
	return attach->CanChange();
}

// Test for selected message deleted
bool CFileTable::TestSelectionUnchangeable(TableIndexT row)
{
	// Is it unchangeable?
	CAttachment* attach = GetAttachment(row);
	return !attach->CanChange();
}

#pragma mark ____________________________________Keyboard/Mouse

// Handle key down
bool CFileTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch(key)
	{
	case kJReturnKey:
		// Try to show it
		if (IsSelectionValid() && (!IsSingleSelection() || mAttachmentsOnly || !ShowPart(GetFirstSelectedRow())))
			OnViewParts();
		return true;

	case kJDeleteKey:
	case kJForwardDeleteKey:
		//mWindow->GetUndoer()->PostAction(new CFileTableDeleteAction(this));
		if (!mLocked)
			DeleteSelection();
		return true;

	default:
		// Did not handle key
		return CHierarchyTableDrag::HandleChar(key, modifiers);
	}
}

// Double-clicked item
void CFileTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifers)
{
	// Try to show otherwise do view
	if (mAttachmentsOnly || !ShowPart(inCell.row))
		OnViewParts();
}

// Click in the cell
void CFileTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	if (!mAttachmentsOnly && (inCell.col == 1))
		ShowPart(inCell.row);
	else
		CHierarchyTableDrag::LClickCell(inCell, modifiers);
}

#pragma mark ____________________________________Command Updaters

void CFileTable::OnUpdateNotLocked(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mLocked);
}

void CFileTable::OnUpdateNotLockedSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mLocked && IsSelectionValid());
}

void CFileTable::OnUpdateAttachFile(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoAttachments && !mLocked);
}

void CFileTable::OnUpdateAllowStyled(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllowStyledComposition && !mLocked);
}

#pragma mark ____________________________________Command Handlers

void CFileTable::OnEditProperties(void)
{
	bool changed = false;
	
	// Can only do to selection
	if (!IsSelectionValid())
		return;

	// Test for selection all changeable or all not changeable
	bool all_change = TestSelectionAnd((TestSelectionPP) &CFileTable::TestSelectionChangeable);
	bool all_unchange = TestSelectionAnd((TestSelectionPP) &CFileTable::TestSelectionUnchangeable);

	// All changeable => allow edit
	if (all_change && !mLocked)
	{
		// Get content for first item
		STableCell selCell(0, 0);
		GetNextSelectedCell(selCell);

		// Make a copy of the content
		CAttachment* attach = GetAttachment(selCell.row);
		CMIMEContent content = attach->GetContent();

		// Modify content if others selected
		while(GetNextSelectedCell(selCell))
		{
			if (selCell.col == 1)
			{
				CAttachment* other_attach = GetAttachment(selCell.row);

				content.NullDiff(other_attach->GetContent());
			}
		}

		// Let DialogHandler process events
		if (CLetterPartProp::PoseDialog(content))
		{
			// Copy back into all selected items
			selCell = STableCell(0, 0);
			while(GetNextSelectedCell(selCell))
			{
				if (selCell.col == 1)
				{
					TableIndexT woRow = GetWideOpenIndex(selCell.row);
					
					CAttachment* attach = GetAttachment(selCell.row);
					if (attach->GetContent().GetContentSubtype() != content.GetContentSubtype()){
						changed = true;
					}
					if (((attach->GetContent().GetContentSubtype() == eContentSubEnriched) ||
						(attach->GetContent().GetContentSubtype() == eContentSubHTML)) &&
						(content.GetContentSubtype() == eContentSubPlain)){
							if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Letter::WarnLoseFormatting") == CErrorHandler::Cancel) return;
						}
					
					content.NullAdd(attach->GetContent());

					// Need to force attachment name to be re-mapped
					attach->SetName(cdstring::null_str);

					if (changed && (woRow == mRowShow) && !mAttachmentsOnly) {
						mWindow->SetCurrentPart(GetAttachment(mRowShow, true));
					}
				}
			}
			RefreshSelection();

			SetDirty(true);
		}
	}
	
	// All unchangeable => no edit
	else if (all_unchange || mLocked)
	{
		// Special if multiple
		bool multi = !IsSingleSelection();
		
		bool more = multi;
		STableCell selCell(0, 0);

		do
		{
			// Exit loop if no more
			if (!GetNextSelectedCell(selCell))
				break;
			
			if (selCell.col == 1)
			{
				CAttachment* attach = GetAttachment(selCell.row);
				more = CMessagePartProp::PoseDialog(*attach, multi);
			}

		} while (more);
	}
	else
		// Warn user of mixed selection
		CErrorHandler::PutStopAlertRsrc("Alerts::Letter::MixedFileTableSelection");

	Focus();
}

void CFileTable::OnDraftAttachFile(void)
{
	// Only if not locked out
	if (CAdminLock::sAdminLock.mNoAttachments)
		return;

	// Do standard open dialog
	JPtrArray<JString> files(JPtrArrayT::kDeleteAll);
	if (JXGetChooseSaveFile()->ChooseFiles("Attachments", NULL, &files)) 
	{
		bool added = false;

		// Want to select only the new ones
		UnselectAllCells();

		JOrderedSetIterator<JString*>* filelist= files.NewIterator();
		JString* jfname;
		cdstring fname;
		TableIndexT first_added = 0;
		while (filelist->Next(&jfname))
		{
			fname = *jfname;
			// Must be unique to add
			if (!mBody || mBody->UniqueFile(fname))
			{
				// Create file attachment and add
				CFileAttachment* fattach = new CFileAttachment(fname);
				TableIndexT woRow = AddPart(fattach, NULL, 0, 0, false);
				TableIndexT exp_row = GetExposedIndex(woRow);
				
				// Select each one added
				SelectRow(exp_row);
				
				// Cache the first one added for scroll into view
				if (first_added == 0)
					first_added = exp_row;
				
				added = true;
			}
		}
		delete filelist;

		// Scroll so first one is in the view
		if (added)
			ScrollToRow(first_added, false, false, eScroll_Top);
		
		// Refresh table if change
		if (added)
			Refresh();

		// Force parts visible
		if (added)
			ExposePartsList();
	}
}

void CFileTable::OnDraftNewPlainPart(void)
{
	DoNewTextPart(eContentSubPlain);
}

void CFileTable::OnDraftNewEnrichedPart(void)
{
	DoNewTextPart(eContentSubEnriched);
}

void CFileTable::OnDraftNewHTMLPart(void)
{
	DoNewTextPart(eContentSubHTML);
}

void CFileTable::DoNewTextPart(EContentSubType subType)
{
	// Must always be in full parts mode if we have more than one inline text part
	SetAttachmentsOnly(false);

	TableIndexT old_show = mRowShow;

	CAttachment* tattach = new CDataAttachment((char*) NULL);
	tattach->GetContent().SetContent(eContentText, subType);
	TableIndexT woRow = AddPart(tattach, NULL, 0, 0, true);

	// Refresh old one
	TableIndexT old_row = GetExposedIndex(old_show);
	RefreshRow(old_row);

	// Change value
	mRowShow = woRow;

	// Find window in super view chain and show new part
	mWindow->SetCurrentPart(GetAttachment(mRowShow, true));
	mWindow->UpdatePartsCaption();

	// Refresh new one
	TableIndexT row = GetExposedIndex(mRowShow);
	RefreshRow(row);
	
	// Select only the new one
	UnselectAllCells();
	SelectRow(row);
	ScrollToRow(row, false, false, eScroll_Top);
	
	// Force parts visible
	ExposePartsList();
}

void CFileTable::OnDraftMultipartMixed(void)
{
	DoMultipart(eContentSubMixed);
}

void CFileTable::OnDraftMultipartParallel(void)
{
	DoMultipart(eContentSubParallel);
}

void CFileTable::OnDraftMultipartDigest(void)
{
	DoMultipart(eContentSubDigest);
}

void CFileTable::OnDraftMultipartAlternative(void)
{
	DoMultipart(eContentSubAlternative);
}

void CFileTable::DoMultipart(EContentSubType subType)
{
	// Must always be in full parts mode if we add multiparts
	SetAttachmentsOnly(false);

	CDataAttachment* mattach = new CDataAttachment();
	mattach->GetContent().SetContent(eContentMultipart, subType);
	TableIndexT woRow = AddPart(mattach, NULL, 0, 0, true);
	TableIndexT expRow = GetExposedIndex(woRow);
	mWindow->UpdatePartsCaption();
	
	// Select only the new one
	UnselectAllCells();
	SelectRow(expRow);
	ScrollToRow(expRow, false, false, eScroll_Top);
	
	// Force parts visible
	ExposePartsList();
}

void CFileTable::ActionDeleted(void)
{
	if (mAddAction)
		mAddAction = NULL;
}

// View selected parts
void CFileTable::OnViewParts()
{
	if (!TestSelectionAnd((TestSelectionPP) &CFileTable::CheckViewPart))
		CErrorHandler::PutStopAlertRsrc("Alerts::Message::CannotDownload");
	else
	{
		DoToSelection((DoToSelectionPP) &CFileTable::ViewPart);
		RefreshSelection();
	}
}

// Check for valid view of specified part
bool CFileTable::CheckViewPart(TableIndexT row)
{
	// Check against admin locks
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;

	// Look for local file attachment - we always allow the user to view these
	// iirespective of admin locks since they can view it via the file system anyway
	CFileAttachment* fattach = dynamic_cast<CFileAttachment*>(attach);

	return (fattach != NULL) || CAdminLock::sAdminLock.CanDownload(attach->GetContent());
}

// View the part
bool CFileTable::ViewPart(TableIndexT row)
{
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;
	CFileAttachment* fattach = dynamic_cast<CFileAttachment*>(attach);

	// Check part size first (only if its not on disk already)
	if ((fattach == NULL) &&
		CPreferences::sPrefs->mDoSizeWarn.GetValue() &&
		(CPreferences::sPrefs->warnMessageSize.GetValue() > 0) &&
		(attach->GetSize() > CPreferences::sPrefs->warnMessageSize.GetValue() * 1024L))
	{
		bool dontshow = false;
		short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Message::WarnSize", 0, &dontshow);
		if (dontshow)
			CPreferences::sPrefs->mDoSizeWarn.SetValue(false);
		if (answer == CErrorHandler::Cancel)
			return false;
	}

	// View chosen part if cached
	if (!attach->IsNotCached())
	{
		// Behaviour:
		//   file attachments are viewed directly off disk
		//   message attachments are viewed via their owning message
		if (fattach)
			fattach->ViewFile();
		else
		{
		}
	}
	return true;
}

// Make sure list is visible in draft window
void CFileTable::ExposePartsList()
{
	// Check whether twisted or not
	if (!mWindow->IsPartsTwist())
		// twist without changing focus
		mWindow->DoPartsTwist(true, false);
}

// Add whole message as part
void CFileTable::ForwardMessages(CMessageList* msgs, EForwardOptions forward)
{
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		// Check that message has more than one alternative text part
		if (!(*iter)->HasUniqueTextPart() || (forward & eForwardAttachment))
		{
			// Check for forward using message/rfc822 part
			CAttachment* new_attach = NULL;
			if (forward & eForwardRFC822)
				new_attach = new CMessageAttachment(*iter, *iter);
			else
			{
				// Make aliases of original body
				if ((*iter)->GetBody()->IsMessage())
					new_attach = new CMessageAttachment(*iter, (*iter)->GetBody()->GetMessage());
				else
					new_attach = new CAliasAttachment(*iter, (*iter)->GetBody());
			}
			AddPart(new_attach, NULL, 0, 0, true);
		}
	}
	
	// Expose parts if items were added
	if (mRows > 1)
		ExposePartsList();
}

// Add whole message as total
void CFileTable::BounceMessages(CMessageList* msgs)
{
	// Dump existing
	delete mBody;
	SetBody(NULL);
	mWindow->SetBody(NULL);

	// Must always be in full parts mode when bouncing
	SetAttachmentsOnly(false);

	CAttachment* new_attach = NULL;

	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		// Make aliases of original body
		if ((*iter)->GetBody()->IsMessage())
			new_attach = new CMessageAttachment(*iter, (*iter)->GetBody()->GetMessage());
		else
			new_attach = new CAliasAttachment(*iter, (*iter)->GetBody());
		
		AddPart(new_attach, NULL, 0, 0, true);
	}
	
	// Expose parts if items were added
	if (mRows > 1)
		ExposePartsList();
}

// Add whole message as total
void CFileTable::RejectMessages(CMessageList* msgs, bool return_msg)
{
	// Dump existing
	delete mBody;
	SetBody(NULL);
	mWindow->SetBody(NULL);

	// Must always be in full parts mode when bouncing
	SetAttachmentsOnly(false);

	// Can only do this to one message
	if (msgs->size() != 1)
		return;
	
	// Create the DSN message
	CAttachment* dsn = msgs->front()->CreateRejectDSNBody(return_msg);
	
	// Give this to the table
	if (dsn)
		AddPart(dsn, NULL, 0, 0, true);

	UpdateRowShow();
	
	// Lock it to prevent changes to reject
	mLocked = true;
	
	// Prevent drag and drop when locked
	SetReadOnly(true);
}

// Add whole message as total with editable bits
void CFileTable::SendAgainMessages(CMessageList* msgs)
{
	// Dump existing
	delete mBody;
	SetBody(NULL);
	mWindow->SetBody(NULL);

	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		CAttachment* new_attach = NULL;

		// Clone entire body to editable parts
		// This will take care of signed/encrypted/alternative types
		new_attach = CAttachment::CloneAttachment(*iter, (*iter)->GetBody());

		// Add to list and update visible part
		AddPart(new_attach, NULL, 0, 0, true);
	}

	UpdateRowShow();
	
	// Expose parts if items were added
	if (mRows > 1)
		ExposePartsList();
	
	// We need to reset the display of the current part
	mWindow->SetCurrentPart(mWindow->GetCurrentPart());
}

// Add whole message as part
void CFileTable::DigestMessages(CMessageList* msgs)
{
	// Must always be in full parts mode when rejecting
	SetAttachmentsOnly(false);

	// Add multipart digest
	CDataAttachment* mattach = new CDataAttachment();
	mattach->GetContent().SetContent(eContentMultipart, eContentSubDigest);
	TableIndexT parent_row = AddPart(mattach, NULL, 0, 0, true);

	// Add all messages into digest
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		CMessageAttachment* new_attach = new CMessageAttachment(*iter, *iter);
		AddPart(new_attach, mattach, parent_row, -1, true);
	}
	
	// Expose parts if items were added
	if (mRows > 1)
		ExposePartsList();
}

// Add file from path
void CFileTable::AddFile(const cdstring& file)
{
	// Create new file attachment and add to list
	CFileAttachment* attached = new CFileAttachment(file);

	// Must be unique to add
	if (attached->ValidFile() &&
		(!mBody || mBody->UniqueFile(attached->GetFilePath())))
	{
		// Create file attachment and add
		AddPart(attached, NULL, 0, 0, true);

		// Force parts visible
		ExposePartsList();
	}
	else
		delete attached;
}

// Add attachment
void CFileTable::AddAttachment(CDataAttachment* attach)
{
	// Add attachment
	AddPart(attach, NULL, 0, 0, true);

	// Force parts visible
	ExposePartsList();
}

// Server reset
void CFileTable::ServerReset(const CMboxProtocol* proto)
{
	// Find any parts dependent on this message and remove them
	if (mBody && CAttachment::ServerReset(mBody, proto))
	{
		ResetTable();
	}
}

// Mailbox reset
void CFileTable::MailboxReset(const CMbox* mbox)
{
	// Find any parts dependent on this message and remove them
	if (mBody && CAttachment::MailboxReset(mBody, mbox))
	{
		ResetTable();
	}
}

// Message removed
void CFileTable::MessageRemoved(const CMessage* msg)
{
	// Find any parts dependent on this message and remove them
	if (mBody && CAttachment::MessageRemoved(mBody, msg))
	{
		ResetTable();
	}
}

// Add attachment to table
// CAttachment* attach	: attachment to add
// CAttachment* parent	: parent of new attachment. If NULL new attachment goes to current selection
// int parent_row		: wide-open index of parent if not NULL
// int pos				: position within parent if not NULL. If -1 put at end of parent
// bool refresh		: force redraw of window
// returns int			: wide-open index of newly inserted attachment
TableIndexT CFileTable::AddPart(CAttachment* attach, CAttachment* parent, TableIndexT parent_row, int pos, bool refresh)
{
	bool force_reset = false;
	TableIndexT new_woRow = 0;

	if (mBody)
	{
		// Check for existing multipart
		force_reset = ConvertMultipart(true);
		
		// If changed bump up positions
		if (force_reset)
		{
			// Reset to new parent
			parent = mBody;
			parent_row = 1;
			pos = -1;
			new_woRow = -1;
		}

		// Get default parent
		if (!parent || !parent->CanChange())
		{
			parent = mBody;
			parent_row = 1;
			pos = -1;

			// Check whether only one selected
			if (!mAttachmentsOnly && IsSingleSelection())
			{
				// Check for multipart
				STableCell aCell(0, 0);
				GetFirstSelection(aCell);
				UInt32 woRow = GetWideOpenIndex(aCell.row);

				CAttachment* attach = GetAttachment(aCell.row);

				// Set parent to single multipart selected item
				if (attach->IsMultipart())
				{
					parent = attach;
					parent_row = woRow;
				}
			}
		}

		// Add to parts
		parent->AddPart(attach, pos);
	}
	else
	{
		mBody = attach;
		mWindow->SetBody(mBody);
		parent_row = 0;
		pos = 0;
		new_woRow = 1;
		force_reset = true;
	}

	// Reset table if required
	if (force_reset)
	{
		ResetTable();
		
		// Must adjust new row to the last one that could possibly be
		TableIndexT temp;
		GetWideOpenTableSize(temp);
		new_woRow = temp;
	}
	else
	{
		// Manually insert part as child or sibling (if flat attachment only list)
		TableIndexT temp = parent_row;
		if (mAttachmentsOnly)
		{
			TableIndexT sibling = ((CNodeVectorTree*) mCollapsableTree)->GetSiblingIndex(temp);
			while(sibling != 0)
			{
				temp = sibling;
				sibling = ((CNodeVectorTree*) mCollapsableTree)->GetSiblingIndex(temp);
			}
		}
		new_woRow = InsertPart(temp, attach, !mAttachmentsOnly, pos);
	}
	
	// Force redraw if required
	if (refresh)
		Refresh();
	
	SetDirty(true);
	
	return new_woRow;
}

// Convert between single/multipart
// bool add		: indicates whether a part is about to be added, or has been removed
// returns bool	: true if root part changed (i.e. reset of whole table required
bool CFileTable::ConvertMultipart(bool add)
{
	// Must have a valid body to do this
	if (!mBody)
		return false;

	// Check for single (or unchangeable) part when adding a new part
	if (add && (!mBody->IsMultipart() || !mBody->CanChange()))
	{
		// Create new multipart entity
		CDataAttachment* root = new CDataAttachment();
		root->GetContent().SetContent(eContentMultipart, eContentSubMixed);
		
		// Add original part to new root
		CAttachment* child = mBody;
		root->AddPart(child);
		
		// Make body start at new root
		mBody = root;
		
		// Force window to update
		mWindow->SetBody(mBody, true);
	
		// Bump up row show if not zero and reset window's current part back to original
		if (mRowShow)
		{
			mRowShow++;
			mWindow->mCurrentPart = child;
		}
	
		// Force table reset
		return true;
	}
	
	// Check for multipart with a single or no child part
	else if (!add && mBody->IsMultipart() && (!mBody->GetParts() || (mBody->GetParts()->size() < 2)))
	{
		// Get old root
		CAttachment* old_root = mBody;
		
		// Get new root from old
		CAttachment* new_root = mBody->GetPart(2);
		
		// Remove new from old without delete and delete old
		if (new_root)
			old_root->RemovePart(new_root, false);
		delete old_root;

		// Make body start at new root
		mBody = new_root;
		
		// Reset table in case of draw
		ResetTable();

		// Force window to update
		mWindow->SetBody(mBody, true);
	
		// Bump down row show if not zero (window's current part remians the same)
		if (mRowShow)
		{
			mRowShow--;
		}

		// Iterate again
		ConvertMultipart(false);

		// Force table reset
		return true;
	}
	
	return false;
}

// Current part changed
void CFileTable::ChangedCurrent(void)
{
	// Refresh new one
	if (!mAttachmentsOnly)
	{
		TableIndexT row = GetExposedIndex(mRowShow);
		RefreshRow(row);
		mWindow->SetCurrentPart(GetAttachment(mRowShow, true));
	}
	else
		mWindow->SetCurrentPart(mWindow->GetCurrentPart());
}

// Find first editable part
void CFileTable::UpdateRowShow(void)
{
	// Reset row show with first viewable if changed
	if (!mAttachmentsOnly && !mRowShow)
	{
		for(unsigned long selrow = 1; selrow <= mCollapsableTree->CountNodes(); selrow++)
		{
			STableCell woCell(selrow, 1);
			CAttachment* attach;
			UInt32 dataSize = sizeof(CAttachment*);
			GetCellData(woCell, &attach, dataSize);
			if (attach->CanEdit())
			{
				mRowShow = selrow;
				break;
			}
		}
		
		// Force window text reset
		mWindow->SetCurrentPart(GetAttachment(mRowShow, true));
		
		// Refresh new one
		TableIndexT row = GetExposedIndex(mRowShow);
		if (row)
			RefreshRow(row);
	}

	mWindow->UpdatePartsCaption();
}

// Click in the cell
bool CFileTable::ShowPart(TableIndexT row)
{
	// Do click in different text show
	TableIndexT	woRow = GetWideOpenIndex(row);
	
	CAttachment* attach = GetAttachment(row);
	if ((woRow != mRowShow) && attach->CanEdit())
	{
		// Refresh old one to force row show off
		TableIndexT old_row = GetExposedIndex(mRowShow);
		mRowShow = woRow;
		RefreshRow(old_row);

		// Find window in super view chain and show new part
		mWindow->SetCurrentPart(GetAttachment(mRowShow, true));

		// Refresh new one
		RefreshRow(row);
		
		return true;
	}
	else
		return false;
}

// Draw the items
void CFileTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	UInt32	woRow = GetWideOpenIndex(inCell.row);
	
	CAttachment* attach = GetAttachment(inCell.row);

	unsigned long col_type = 0;
	
	switch(inCell.col)
	{
	case 1:
		col_type = mAttachmentsOnly ? eColType_NameIcon : eColType_Diamond;
		break;
	case 2:
		col_type = mAttachmentsOnly ? eColType_Size : eColType_RW;
		break;
	case 3:
		col_type = mAttachmentsOnly ? eColType_MIME : eColType_MIMEIcon;
		break;
	case 4:
		col_type = eColType_Name;
		break;
	case 5:
		col_type = eColType_Size;
		break;
	case 6:
		col_type = eColType_Encoding;
		break;
	}

	JXImage* icon;
	switch(col_type)
	{
	
	case eColType_Diamond:
		// Button if not multipart
		if (!attach->IsMultipart())
		{
			int	plot_flag = 0;

			// Check for diamond type
			if (woRow == mRowShow)
				plot_flag = IDI_DIAMONDTICKED;
			else if (attach->CanEdit())
				plot_flag = (attach->IsSeen() ? IDI_DIAMONDHIGHLIGHT : IDI_DIAMOND);
			else
				plot_flag = IDI_DIAMONDDISABLE;

			icon = CIconLoader::GetIcon(plot_flag, this, 16, 0x00FFFFFF);
			pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		}
		break;

	case eColType_RW:
		icon = CIconLoader::GetIcon((attach->CanChange() && !mLocked) ? IDI_MAILBOXREADWRITE : IDI_MAILBOXREADONLY, this, 16, 0x00FFFFFF);
		// Button if not multipart
		pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		break;

	case eColType_MIME:
		{
			cdstring content = CMIMESupport::GenerateContentHeader(attach, false, lendl, false);
			::DrawClippedStringUTF8(pDC, content, JPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Left);
		}
		break;
	
	case eColType_MIMEIcon:

		// Draw icon followed by encoding
		icon = GetAttachIcon(attach);
			
		// Get encoding mode
		{
			cdstring content = CMIMESupport::GenerateContentHeader(attach, false, lendl, false);
			DrawHierarchyRow(pDC, inCell.row, inLocalRect, content, icon);
		}
		break;
	
	case eColType_Name:
		if (!attach->IsMultipart())
		{
			cdstring at_name = attach->GetMappedName(true, false);
			if (!at_name.empty())
			{
				::DrawClippedStringUTF8(pDC, at_name, JPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Left);
			}
		}
		break;
	
	case eColType_NameIcon:
		// Draw icon followed by encoding
		icon = GetAttachIcon(attach);
			
		// Get encoding mode
		{
			cdstring at_name = attach->GetMappedName(true, false);
			DrawHierarchyRow(pDC, inCell.row, inLocalRect, at_name, icon);
		}
		break;
	
	case eColType_Size:
		// Draw size
		if (!attach->IsMultipart())
		{
			long size;
			cdstring theTxt;
			theTxt.reserve(32);
			
			size = attach->GetSize();
			theTxt = ::GetNumericFormat(size);
			::DrawClippedStringUTF8(pDC, theTxt, JPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Right);
		}
		break;
	
	case eColType_Encoding:
		// Get encoding mode
		if (!attach->IsMultipart() && !attach->IsMessage())
		{
			cdstring theTxt;
			theTxt = rsrc::GetIndexedString("UI::Letter::EncodeMode", attach->GetTransferMode());
			::DrawClippedStringUTF8(pDC, theTxt, JPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Left);
		}
		break;

	default:
		break;
	}
}

// Plot appropriate icon for attachment
JXImage* CFileTable::GetAttachIcon(CAttachment* attach)
{
	resType nID = 0;
	if (attach->IsApplefile())
		nID = IDI_APPLEFILEPART;
	else if (attach->IsMultipart())
		nID = IDI_MULTIPART;
	else if (attach->IsMessage())
		nID = IDI_MESSAGEPART;
	else if (attach->IsCalendar())
		nID = IDI_CALENDARPART;
	else if (attach->GetIconRef()->GetIconRef())
		return attach->GetIconRef()->GetIconRef();
	else
		nID = IDI_UNKNOWNFILE;
	
	return CIconLoader::GetIcon(nID, this, 16, 0x00FFFFFF);
}

#pragma mark ____________________________________Drag&Drop

void CFileTable::HandleDNDDidDrop(const Atom& action)
{
	// Handle move operation by deleting selection
	JXDNDManager* dndMgr = GetDNDManager();
	Atom move = dndMgr->GetDNDActionMoveXAtom();
	if ((action == move) && mAllowMove)
		DeleteSelection();
}

// Test drop into cell
bool CFileTable::IsDropCell(JArray<Atom>& typeList, const STableCell& cell)
{
	if (IsValidCell(cell))
	{
		// Look for drop on self
		if (sTableDragSource == this)
		{
			// Make sure row is not selected
			if (CellIsSelected(cell))
				return false;
			
			// Make sure parent row is not selected
			TableIndexT woRow = GetWideOpenIndex(cell.row);
			while((woRow = GetParentIndex(woRow)) > 0)
			{
				STableCell parent_cell(GetExposedIndex(woRow), 1);
				if (CellIsSelected(parent_cell))
					return false;
			}
		}

		// Only drop into changable multipart part
		TableIndexT woRow = GetWideOpenIndex(cell.row);
		CAttachment* attach = GetAttachment(cell.row);
		return IsCollapsable(woRow) && attach->CanChange();
	}
	else
		return false;
}

// Test drop at cell
bool CFileTable::IsDropAtCell(JArray<Atom>& typeList, STableCell& cell)
{
	// Do not drop above top row
	if (cell.row == 1)
		return false;

	TableIndexT parent_row = 0;
	CAttachment* parent = NULL;
	int pos = 0;
	GetDropAtParent(cell.row, parent_row, parent, pos);
	if (!parent && (cell.row <= mRows))
		return false;

	// Look for drop on self 
	if ((sTableDragSource == this) && parent_row)
	{
		// Make sure parent row is not selected
		TableIndexT woRow = parent_row;
		do
		{
			STableCell parent_cell(GetExposedIndex(woRow), 1);
			if (CellIsSelected(parent_cell))
				return false;
		} while((woRow = GetParentIndex(woRow)) > 0);
	}

	// Must be OK if we get here
	return true;
}

bool CFileTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	// Force sync of current part
	mWindow->SyncPart();

	if (type == CMulberryApp::sFlavorAtchList)
	{
		// Create list to hold dragged items
		std::auto_ptr<CAttachmentList> atchs(new CAttachmentList);
		atchs->set_delete_data(false);

		// Add selected attachments at same level
		TableIndexT parent = 0;
		int nest_level = -1;
		STableCell selCell(0, 0);
		while(GetNextSelectedCell(selCell))
		{
			// Once per row only
			if (selCell.col != 1)
				continue;

			TableIndexT	woRow = GetWideOpenIndex(selCell.row);
			
			// Get nesting level of first selection
			if (parent == 0)
			{
				parent = GetParentIndex(woRow);
				nest_level = GetNestingLevel(woRow);
			}
			
			// Add only those at same level in same parent
			if ((parent == GetParentIndex(woRow)) && (nest_level == GetNestingLevel(woRow)))
			{
				STableCell	woCell(woRow, 1);
				CAttachment* attach;
				UInt32 dataSize = sizeof(CAttachment*);
				GetCellData(woCell, &attach, dataSize);
				atchs->push_back(attach);
			}
		}

		// Allocate global memory for the text if not already
		seldata->SetData(type, reinterpret_cast<unsigned char*>(atchs.release()), sizeof(CAttachmentList*));
		rendered = true;
	}
	
	return rendered;
}

// Drop data into cell
bool CFileTable::DropDataIntoCell(Atom theFlavor, unsigned char* drag_data,
										 unsigned long data_size, const STableCell& cell)
{
	bool added = false;

	// Only allow drag to group
	TableIndexT	woRow = GetWideOpenIndex(cell.row);
	if (!IsCollapsable(woRow))
		return false;

	// Get attachment to drop into
	CAttachment* parent = GetAttachment(cell.row);

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		CMessageList* msgs = reinterpret_cast<CMessageList*>(drag_data);
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			CMessageAttachment* new_attach = new CMessageAttachment(*iter, *iter);
			AddPart(new_attach, parent, woRow, -1, false);
			added = true;
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorAtchList)
	{
		CAttachmentList* atchs = reinterpret_cast<CAttachmentList*>(drag_data);
		for(CAttachmentList::const_iterator iter = atchs->begin(); iter != atchs->end(); iter++)
		{
			CAttachment* new_attach = CAttachment::CopyAttachment(**iter);
			AddPart(new_attach, parent, woRow, -1, false);
			added = true;
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorMsgAtchList)
	{
		CMessage* owner = *((CMessage**) drag_data);
		drag_data += sizeof(CMessage*);
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CAttachment* theAtch = ((CAttachment**) drag_data)[i];
			if (theAtch->IsMessage())
			{
				CMessageAttachment* new_attach = new CMessageAttachment(owner, theAtch->GetMessage());		// Make alias, not copy
				AddPart(new_attach, parent, woRow, -1, false);
			}
			else
			{
				CAliasAttachment* new_attach = new CAliasAttachment(owner, theAtch);		// Make alias, not copy
				AddPart(new_attach, parent, woRow, -1, false);
			}
			added = true;
		}
	}
	// Only if not locked out
	else if ((theFlavor == GetSelectionManager()->GetURLXAtom()) && !CAdminLock::sAdminLock.mNoAttachments)
	{
		// Unpack into filenames and URLs (which are other files on different hosts)
		// URLs are ignored - we only handle files on this system
		JPtrArray<JString> fileNameList(JPtrArrayT::kDeleteAll);
		JPtrArray<JString> urlList(JPtrArrayT::kDeleteAll);
		JXUnpackFileNames((char*) drag_data, data_size, &fileNameList, &urlList);

		for(unsigned long i = 1; i <= fileNameList.GetElementCount(); i++)
		{
			// Get file name and check it can be read from
			const JString* fname = fileNameList.NthElement(i);
			if (JFileReadable(*fname))
			{
				// Add file as attachment

				// Create new file attachment and add to list
				CFileAttachment* theAtch = new CFileAttachment(fname->GetCString());
				AddPart(theAtch, parent, woRow, -1, false);
			}
		}

		for(unsigned long i = 1; i <= urlList.GetElementCount(); i++)
		{
			// Get file name and check it can be read from
			const JString* fname = urlList.NthElement(i);
			const char* p = fname->GetCString();

			// Look for file:/ URL
			if (!::strncmp(p, "file:", 5))
			{
				p += 5;
				if ((*p == '/') && (*(p+1) != '/'))
				{
					if (JFileReadable(p))
					{
						// Add file as attachment

						// Create new file attachment and add to list
						CFileAttachment* theAtch = new CFileAttachment(p);
						AddPart(theAtch, parent, woRow, -1, false);
					}
				}
			}
		}

		fileNameList.DeleteAll();
		urlList.DeleteAll();
	}
	
	// Refresh table if change
	if (added)
	{
		UpdateRowShow();
		Refresh();
	}

	return added;
}

// Drop data at cell
bool CFileTable::DropDataAtCell(Atom theFlavor, unsigned char* drag_data,
										unsigned long data_size, const STableCell& cell)
{
	bool added = false;

	// Get location of drop cell or first cell if beyond the start
	int	woRow = GetWideOpenIndex(cell.row - 1);
	if (!woRow)
		woRow = GetWideOpenIndex(1);

	TableIndexT actual_row = cell.row;
	TableIndexT parent_row = 0;
	CAttachment* parent = NULL;
	int pos = 0;
	GetDropAtParent(actual_row, parent_row, parent, pos);

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		CMessageList* msgs = reinterpret_cast<CMessageList*>(drag_data);
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			CMessageAttachment* new_attach = new CMessageAttachment(*iter, *iter);
			AddPart(new_attach, parent, parent_row, pos, false);
			added = true;
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorAtchList)
	{
		CAttachmentList* atchs = reinterpret_cast<CAttachmentList*>(drag_data);
		for(CAttachmentList::const_iterator iter = atchs->begin(); iter != atchs->end(); iter++)
		{
			CAttachment* new_attach = CAttachment::CopyAttachment(**iter);
			AddPart(new_attach, parent, parent_row, pos, false);
			added = true;
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorMsgAtchList)
	{
		CMessage* owner = *((CMessage**) drag_data);
		drag_data += sizeof(CMessage*);
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CAttachment* theAtch = ((CAttachment**) drag_data)[i];
			if (theAtch->IsMessage())
			{
				CMessageAttachment* new_attach = new CMessageAttachment(owner, theAtch->GetMessage());		// Make alias, not copy
				AddPart(new_attach, parent, parent_row, pos, false);
			}
			else
			{
				CAliasAttachment* new_attach = new CAliasAttachment(owner, theAtch);		// Make alias, not copy
				AddPart(new_attach, parent, parent_row, pos, false);
			}
			added = true;
		}
	}
	// Only if not locked out
	else if ((theFlavor == GetSelectionManager()->GetURLXAtom()) && !CAdminLock::sAdminLock.mNoAttachments)
	{
		// Unpack into filenames and URLs (which are other files on different hosts)
		// URLs are ignored - we only handle files on this system
		JPtrArray<JString> fileNameList(JPtrArrayT::kDeleteAll),
					urlList(JPtrArrayT::kDeleteAll);
		JXUnpackFileNames((char*) drag_data, data_size, &fileNameList, &urlList);

		for(unsigned long i = 1; i <= fileNameList.GetElementCount(); i++)
		{
			// Get file name and check it can be read from
			const JString* fname = fileNameList.NthElement(i);
			if (JFileReadable(*fname))
			{
				// Add file as attachment

				// Create new file attachment and add to list
				CFileAttachment* theAtch = new CFileAttachment(fname->GetCString());
				AddPart(theAtch, parent, parent_row, pos, false);
			}
		}

		for(unsigned long i = 1; i <= urlList.GetElementCount(); i++)
		{
			// Get file name and check it can be read from
			const JString* fname = urlList.NthElement(i);
			const char* p = fname->GetCString();

			// Look for file:/ URL
			if (!::strncmp(p, "file:", 5))
			{
				p += 5;
				if ((*p == '/') && (*(p+1) != '/'))
				{
					if (JFileReadable(p))
					{
						// Add file as attachment

						// Create new file attachment and add to list
						CFileAttachment* theAtch = new CFileAttachment(p);
						AddPart(theAtch, parent, parent_row, pos, false);
					}
				}
			}
		}

		fileNameList.DeleteAll();
		urlList.DeleteAll();
	}
	
	// Refresh table if change
	if (added)
	{
		UpdateRowShow();
		Refresh();
	}

	return added;
}

bool CFileTable::DropData(Atom theFlavor, unsigned char* drag_data, unsigned long data_size)
{
	STableCell cell(1, 1);
	bool added = false;

	TableIndexT parent_row = 0;
	CAttachment* parent = NULL;
	int pos = -1;

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		CMessageList* msgs = reinterpret_cast<CMessageList*>(drag_data);
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			CMessageAttachment* new_attach = new CMessageAttachment(*iter, *iter);
			AddPart(new_attach, parent, parent_row, pos, false);
			added = true;
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorAtchList)
	{
		CAttachmentList* atchs = reinterpret_cast<CAttachmentList*>(drag_data);
		for(CAttachmentList::const_iterator iter = atchs->begin(); iter != atchs->end(); iter++)
		{
			CAttachment* new_attach = CAttachment::CopyAttachment(**iter);
			AddPart(new_attach, parent, parent_row, pos, false);
			added = true;
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorMsgAtchList)
	{
		CMessage* owner = *((CMessage**) drag_data);
		drag_data += sizeof(CMessage*);
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CAttachment* theAtch = ((CAttachment**) drag_data)[i];
			if (theAtch->IsMessage())
			{
				CMessageAttachment* new_attach = new CMessageAttachment(owner, theAtch->GetMessage());		// Make alias, not copy
				AddPart(new_attach, parent, parent_row, pos, false);
			}
			else
			{
				CAliasAttachment* new_attach = new CAliasAttachment(owner, theAtch);		// Make alias, not copy
				AddPart(new_attach, parent, parent_row, pos, false);
			}
			added = true;
		}
	}
	// Only if not locked out
	else if ((theFlavor == GetSelectionManager()->GetURLXAtom()) && !CAdminLock::sAdminLock.mNoAttachments)
	{
		// Unpack into filenames and URLs (which are other files on different hosts)
		// URLs are ignored - we only handle files on this system
		JPtrArray<JString> fileNameList(JPtrArrayT::kDeleteAll),
					urlList(JPtrArrayT::kDeleteAll);
		JXUnpackFileNames((char*) drag_data, data_size, &fileNameList, &urlList);

		for(unsigned long i = 1; i <= fileNameList.GetElementCount(); i++)
		{
			// Get file name and check it can be read from
			const JString* fname = fileNameList.NthElement(i);
			if (JFileReadable(*fname))
			{
				// Add file as attachment

				// Create new file attachment and add to list
				CFileAttachment* theAtch = new CFileAttachment(fname->GetCString());
				AddPart(theAtch, parent, parent_row, pos, false);
			}
		}

		for(unsigned long i = 1; i <= urlList.GetElementCount(); i++)
		{
			// Get file name and check it can be read from
			const JString* fname = urlList.NthElement(i);
			const char* p = fname->GetCString();

			// Look for file:/ URL
			if (!::strncmp(p, "file:", 5))
			{
				p += 5;
				if ((*p == '/') && (*(p+1) != '/'))
				{
					if (JFileReadable(p))
					{
						// Add file as attachment

						// Create new file attachment and add to list
						CFileAttachment* theAtch = new CFileAttachment(p);
						AddPart(theAtch, parent, parent_row, pos, false);
					}
				}
			}
		}

		fileNameList.DeleteAll();
		urlList.DeleteAll();
	}
	
	// Refresh table if change
	if (added)
	{
		UpdateRowShow();
		Refresh();
	}

	return added;
}

// Get parent of cell for drop at
// returns bool	: indicates whether original drop parent is used or not
void CFileTable::GetDropAtParent(TableIndexT& at_row, TableIndexT& parent_row, CAttachment*& parent, int& pos)
{
	// Get location of drop cell or first cell if beyond the start
	TableIndexT	woRow = GetWideOpenIndex(at_row - 1);	// This is row before cursor
	if (!woRow)
		woRow = GetWideOpenIndex(1);

	parent_row = 0;
	parent = NULL;
	pos = 0;
	if (IsCollapsable(woRow) && IsExpanded(woRow))
	{
		parent_row = woRow;
		STableCell woCell(woRow, 1);
		UInt32 dataSize = sizeof(CAttachment*);
		GetCellData(woCell, &parent, dataSize);
		pos = 0;
	}
	else
	{
		// Get attachment to drop into
		// Note if there is only a single part it will be magically converted into a multipart
		parent_row = GetParentIndex(woRow);
		if (parent_row)
		{
			STableCell woCell(parent_row, 1);
			UInt32 dataSize = sizeof(CAttachment*);
			GetCellData(woCell, &parent, dataSize);
			pos = static_cast<CNodeVectorTree*>(mCollapsableTree)->SiblingPosition(woRow);
		}
		else
		{
			parent_row = 0;
			parent = NULL;
			pos = -1;
		}
	}
	
	// Now check that parent is changeable
	if (parent && !parent->CanChange())
	{
		// Find parent's parent that is changeable
		CAttachment* next = parent->GetParent();
		TableIndexT next_row = GetParentIndex(parent_row);
		while(next && !next->CanChange())
		{
			parent = next;
			parent_row = next_row;
			next = parent->GetParent();
			next_row = GetParentIndex(parent_row);
		}
		
		// Found a parent that will accept new attachment
		if (next)
		{
			// New insert position is at child parent
			at_row = parent_row;

			// Find position for insert
			pos = static_cast<CNodeVectorTree*>(mCollapsableTree)->SiblingPosition(parent_row);
			parent = next;
			parent_row = next_row;
		}
		else
		{
			parent_row = 0;
			parent = NULL;
			pos = -1;
			
			// Force to last row
			at_row = mRows + 1;
		}
	}
}

void CFileTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eEditDelete:
		OnUpdateNotLockedSelection(cmdui);
		return;

	case CCommand::eEditProperties:
		OnUpdateSelection(cmdui);
		return;

	case CCommand::eDraftNewTextPart:
	case CCommand::eNewTextPlain:
		OnUpdateNotLocked(cmdui);
		return;

	case CCommand::eNewTextEnriched:
	case CCommand::eNewTextHTML:
		OnUpdateAllowStyled(cmdui);
		return;

	case CCommand::eDraftNewMultipart:
	case CCommand::eNewMultiMixed:
	case CCommand::eNewMultiParallel:
	case CCommand::eNewMultiDigest:
	case CCommand::eNewMultiAlt:
		OnUpdateNotLocked(cmdui);
		return;
	default:;
	}

	CHierarchyTableDrag::UpdateCommand(cmd, cmdui);
}


bool CFileTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eEditDelete:
		if (!mLocked)
			DeleteSelection();
		return true;

	case CCommand::eEditProperties:
		OnEditProperties();
		return true;

	case CCommand::eDraftAttachFile:
		// Only if not locked out
		if (!CAdminLock::sAdminLock.mNoAttachments)
			OnDraftAttachFile();
		return true;

	case CCommand::eNewTextPlain:
		OnDraftNewPlainPart();
		return true;
	case CCommand::eNewTextEnriched:
		OnDraftNewEnrichedPart();
		return true;
	case CCommand::eNewTextHTML:
		OnDraftNewHTMLPart();
		return true;

	case CCommand::eNewMultiMixed:
		OnDraftMultipartMixed();
		return true;
	case CCommand::eNewMultiParallel:
		OnDraftMultipartParallel();
		return true;
	case CCommand::eNewMultiDigest:
		OnDraftMultipartDigest();
		return true;
	case CCommand::eNewMultiAlt:
		OnDraftMultipartAlternative();
		return true;
	}

	return CHierarchyTableDrag::ObeyCommand(cmd, menu);
}
