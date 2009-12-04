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


// Header for CLetterDoc class

#ifndef __CLETTERDOC__MULBERRY__
#define __CLETTERDOC__MULBERRY__

#include "CNavSafeDoc.h"

#include "CMulberryCommon.h"

// Consts
const OSType kLetterDocType = 'Lett';

// Panes

// Messages

// Resources
const ResType	cLetterTraits_Type = 'Lwin';
const ResType	cLetterEnvelope_Type = 'Envl';
const ResType	cLetterAttachments_Type = 'Atch';
const ResType	cLetterEncoding_Type = 'Encd';
const ResType 	cLetterMIME_Type = 'Mime';

// Classes

class CLetterDoc : public CNavSafeDoc,
					public LPeriodical
{
private:
	static unsigned long	sTemporaryCount;
	PPx::FSObject			mTemporary;
	bool					mSaveToMailbox;
	cdstring				mMailboxSave;
	time_t					mAutoSaveTime;					// Time at which to trigger auto save

public:
					CLetterDoc(LCommander* inSuper,
								PPx::FSObject *inFileSpec);
	virtual 		~CLetterDoc();
	
public:
	virtual LWindow*	GetWindow() { return mWindow; }

	virtual PPx::CFString	GetDescriptor() const;

	virtual Boolean		IsSpecified() const;
	
	virtual void		SetModified(Boolean modified);
	virtual Boolean		IsModified();

	virtual void		AttemptClose(Boolean inRecordIt);		
	virtual Boolean		AttemptQuitSelf(SInt32 inSaveOption);		

	virtual Boolean		AskSaveAs(
								PPx::FSObject	&outFSSpec,
								Boolean			inRecordIt);
	virtual Boolean		AskFileSaveAs(
								PPx::FSObject	&outFSSpec,
								Boolean			inRecordIt);

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual	void		SpendTime(const EventRecord &inMacEvent);	// Called during idle

protected:
	virtual void		NameNewDoc();
	virtual void		OpenFile(PPx::FSObject &inFileSpec);

private:
	virtual void		ReadFile();
	virtual void		ReadEnvelope();
	virtual void		ReadAttachments();
	virtual void		ReadTraits();

public:	
	virtual void		AskImport();						// Import text
	virtual void		DoAEImport(PPx::FSObject &inFileSpec);			// Import file

	virtual void		DoAESave(PPx::FSObject &inFileSpec, OSType inFileType);
	virtual void		DoSave();
	virtual void		DoMailboxSave();
	virtual void		DoFileSave(bool auto_save = false);


private:
	virtual void		SaveEnvelope();
	virtual void		SaveAttachments();
	virtual void		SaveTraits();

public:
	virtual void		DoRevert();

	virtual void		SaveTemporary();
	virtual void		DeleteTemporary();
	static	void		ReadTemporary();
	static  void		MakeTemporary(PPx::FSObject& inFileSpec, unsigned long ctr);

	virtual void		HandlePrint();
	virtual void		DoPrint();								// Print the message
};

#endif
