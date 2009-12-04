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


// CMessageDoc.cp : implementation of the CMessageDoc class
//


#include "CMessageDoc.h"

#include "CLocalCommon.h"
#include "CMessage.h"
#include "CMessageList.h"
#include "CMessageWindow.h"
#include "CPreferences.h"
#include "CStringUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageDoc

IMPLEMENT_DYNCREATE(CMessageDoc, CSDIDoc)

BEGIN_MESSAGE_MAP(CMessageDoc, CSDIDoc)
	//{{AFX_MSG_MAP(CMessageDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageDoc construction/destruction

CMessageDoc::CMessageDoc()
{
	// TODO: add one-time construction code here
	mMsgWnd = NULL;
}

// Handle illegal file name
BOOL CMessageDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
	// Make file name safe for file system
	cdstring fname(m_strTitle);
	MakeSafeFileName(fname);

	// Save copy of current title
	m_strPathName = fname.win_str();
	
	// Clip length to 15
	if (m_strPathName.GetLength() > 15)
		m_strPathName.ReleaseBuffer(15);
	
	// Add default suffix
	m_strPathName += _T(".");
	m_strPathName += CPreferences::sPrefs->mSaveCreator.GetValue().win_str();

	// Do inherited call - NB always use NULL for file name to ensure user is prompted for a name
	BOOL result = CSDIDoc::DoSave(NULL, false);
	m_strPathName = _T("");

	return result;
}

// Save data to file
void CMessageDoc::Serialize(CArchive& ar)
{
	// Can only save incoming messages
	if (!ar.IsLoading())
	{
		// Get its message
		CMessage* theMsg = mMsgWnd->GetMessage();

		if (theMsg)
		{
			// Add header if required or in raw mode
			if (CPreferences::sPrefs->saveMessageHeader.GetValue() || (mMsgWnd->GetViewAs() == eViewAsRaw))
				ar.WriteString(cdstring(theMsg->GetHeader()).win_str());

			// Write the text
			const unichar_t* txt =  mMsgWnd->GetSaveText();
			if (txt)
			{
				// RichEdit 2.0 fix: Need to add in LFs as saved text comes back with CRs only
				cdustring temp(txt);
				temp.ConvertEndl();
				cdstring utf8 = temp.ToUTF8();
				ar.WriteString(utf8.win_str());
			}
		}
		else
		{
			CMessageList* msgs = mMsgWnd->GetMessageList();

			// Iterate over each message and add to file
			bool first = true;
			for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
			{
				if (first)
					first = false;
				else
					ar.Write("\r\n", 2);

				// Write header if required
				if (CPreferences::sPrefs->saveMessageHeader.GetValue())
					ar.WriteString(cdstring((*iter)->GetHeader()).win_str());

				// Write text to file stream
				bool has_data = (*iter)->HasData(0);
				const char* data = (*iter)->ReadPart(0);

				if (data)
					ar.WriteString(cdstring(data).win_str());
				
				if (!has_data)
					(*iter)->ClearMessage();
			}
			
		}
	}
}