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


// Header for CAddressBookDoc class

#ifndef __CADDRESSBOOKDOC__MULBERRY__
#define __CADDRESSBOOKDOC__MULBERRY__

#include "CNavSafeDoc.h"

#include "CAddressListFwd.h"
#include "CGroupList.h"
#include "CMulberryCommon.h"
#include "CWindowStates.h"

// Consts
const OSType kAddressBookDocType = 'AdBk';

#define	QUOTE_SPACE			"\" "
#define	COMMA_SPACE			", "
#define	COMMA_SPACE_CRLF	", \r\n"
#define ANON				"Anonymous"
#define	GROUP_HEADER		"Group:"
#define	NEW_GROUP_HEADER	"Grp:"

// Panes

// Messages

// Resources
const ResType	cAddrTraits_Type = 'Awin';

// Classes

class CAddressBook;
class CAddressBookWindow;
class LStream;

class CAddressBookDoc : public CNavSafeDoc
{
private:
	NumVersion					mVersion;
	CAddressBookWindowState		mTraits;
	bool						mIsLocal;
	static unsigned long		sAddressBookCount;

public:
					CAddressBookDoc(CAddressBook* mAdbk, PPx::FSObject* inFileSpec);
	virtual 		~CAddressBookDoc();
	
public:
	virtual LWindow*		GetWindow(void)
		{ return mWindow; }
	virtual CAddressBook*	GetAddressBook(void);
	virtual void		GetSpecifier(PPx::FSObject& spec)
							{ mFile->GetSpecifier(spec); }

	virtual void		SetModified(Boolean modified);
	virtual Boolean		IsModified(void);

	virtual void		Close(void);

	virtual bool		IsVisible(void)
							{ return (mWindow ? mWindow->IsVisible() : false); }

	virtual void		AttemptClose(Boolean inRecordIt);		
	virtual Boolean		AttemptQuitSelf(SInt32 inSaveOption);		

	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

protected:
	virtual void		NameNewDoc(void);
	virtual void		OpenFile(PPx::FSObject &inFileSpec);

private:
	virtual void		ReadFile(void);
	virtual void		ExportTabbedAddresses(LStream& out);

public:
	virtual void		DoAESave(PPx::FSObject &inFileSpec, OSType inFileType);
	virtual void		DoSave(void);

	virtual void		DoRevert(void);
};

#endif
