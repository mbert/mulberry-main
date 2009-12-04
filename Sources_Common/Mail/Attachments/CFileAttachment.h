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


//	CFileAttachment.h

#ifndef __CFILEATTACHMENT__MULBERRY__
#define __CFILEATTACHMENT__MULBERRY__

#include "CAttachment.h"

#include "CDesktopIcons.h"

// Classes

class	CFileAttachment : public CAttachment
{
public:
						CFileAttachment();
	explicit			CFileAttachment(const cdstring& fpath);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	explicit			CFileAttachment(const PPx::FSObject& spec);
#endif
						CFileAttachment(const CFileAttachment& copy);
	virtual				~CFileAttachment();

	int operator==(const CFileAttachment& test) const;			// Compare with same type

	virtual void		ProcessSend();						// Process files for sending
	virtual void		ProcessContent();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual void					SetFSSpec(const PPx::FSObject& spec);
	virtual PPx::FSObject*			GetFSSpec()
		{ return &mFileSpec; }
	virtual const PPx::FSObject*	GetFSSpec() const
		{ return &mFileSpec; }
	virtual bool		UniqueFile(const PPx::FSObject& spec) const;

#else
	virtual void		SetFilePath(const cdstring& fpath);
	virtual cdstring&	GetFilePath()
							{ return mFileSpec; }
	virtual const cdstring&	GetFilePath() const
							{ return mFileSpec; }
	virtual bool		UniqueFile(const cdstring& fpath) const;
#endif
	virtual const CIconRef*	GetIconRef() const;
			bool		ValidFile() const;

			void		ViewFile() const;

	// Flags
	virtual bool IsDirectory() const
		{ return mFlags.IsSet(eDirectory); }

	virtual void SetDeleteFile(bool delete_it)
		{ return mFlags.Set(eDeleteFile, delete_it); }
	virtual bool IsDeleteFile() const
		{ return mFlags.IsSet(eDeleteFile); }


	// Status
	virtual bool		CanDisplay() const
							{ return IsText(); }
	virtual bool		CanEdit() const
							{ return false; }
	virtual bool		CanChange() const
							{ return true; }

	virtual void		WriteDataToStream(costream& stream,
											bool dummy_files,
											CProgress* progress,
											CMessage* owner = NULL,
											unsigned long count = 0,
											unsigned long start = 1) const;

private:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	PPx::FSObject		mFileSpec;					// File object
#else
	cdstring			mFileSpec;					// Attachment path
#endif
	CIconRef			mFileIcon;

			void		InitFileAttachment();

			void		CheckForDirectory();
			void		ResolveIfAlias();
			void		CalculateFileSize();
			void		MIMEMap();

};

#endif
