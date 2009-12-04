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


//	CAttachment.h

#ifndef __CATTACHMENT__MULBERRY__
#define __CATTACHMENT__MULBERRY__

#include "CMIMEContent.h"
#include "CMIMESupport.h"
#include "CSecurityPlugin.h"
#include "CStreamTypeFwd.h"

#include "SBitFlags.h"
#include "cdstring.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <PPxFSObject.h>
#endif

// Classes

class CAttachmentList;
class CFilter;
class CFullFileStream;
class CIconRef;
class CMbox;
class CMboxProtocol;
class CMessage;
class CProgress;

class CAttachment
{
	friend class CAttachmentManager;

public:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	static FSRef*		sDropLocation;
#endif

	enum EFlags
	{
		// Mbox status
		eNone = 					0,
		eSeen =						1L << 0,		// Part has been seen
		eExtracted =				1L << 1,		// Part has been extracted
		eDirectory =				1L << 2,		// File attachment is directory
		eDeleteFile =				1L << 3,		// Delete file part on attachment delete
		eFakeText =					1L << 4			// Treat as text even if content is not marked that way
	};

						CAttachment();
	explicit			CAttachment(CAttachment* parent);				// Create a child
	explicit			CAttachment(char* text,
										CAttachment* parent = NULL);		// Create text body
						CAttachment(const CAttachment& copy);
	virtual				~CAttachment();

	static CAttachment*	CopyAttachment(const CAttachment& copy);
	static CAttachment*	CreateAttachment(const CMIMEContent& content);

	static CAttachment* CloneAttachment(CMessage* owner, const CAttachment* original);		// Clone editable version of attachments

	int operator==(const CAttachment& test) const;											// Compare with same type

	// Methods relating to attachment hierarchy
	virtual void		AddPart(CAttachment* part, long pos = -1);							// Add child
	virtual void		RemovePart(CAttachment* part, bool delete_it = true);				// Remove child (with/without delete)

	virtual void		SetParent(CAttachment* part)
		{ mParent = part; }
	virtual CAttachment*	GetParent() const
		{ return mParent; }

	virtual unsigned long		CountParts() const;											// Count number of parts (not multiparts)
	virtual bool				HasUniqueTextPart(bool only_alt = true) const;				// Check number of text parts
	virtual bool				HasUniquePart() const										// Check number of text parts (only one of alternative, sigs)
		{ return HasUniqueTextPart(false); }
	virtual bool				HasAttachments() const;										// See if any cont-disp: attachment parts are present
	virtual CAttachmentList*	GetParts() const
		{ return mParts; }
	virtual void				ClearParts();

	virtual CAttachment*	GetPart(unsigned long index) const;								// Get attachment from flattened index
	virtual CAttachment*	FirstDisplayPart(bool no_style_test = false) const;				// Get first displayable part
	virtual CAttachment*	FirstEditPart() const;											// Get first editable part
	virtual CAttachment*	GetAlternative(EContentType type, EContentSubType subtype);		// Get matching alternative

	virtual void		GetPartNumber(cdstring& part,
										bool underscore = false) const;						// Return body part number
	virtual cdstring	GetURL() const;

	virtual void		ProcessSend();																// Process attachments for sending
	virtual void		ProcessSendCrypto(CSecurityPlugin::ESecureMessage mode, bool use_mime);		// Process attachments for sending with crypto

	virtual bool		TestOwnership(const CMbox* owner) const;							// Check whether any part owned by mbox

	static bool			ServerReset(CAttachment*& body, const CMboxProtocol* proto);		// Server reset
	virtual bool		ResetServer(const CMboxProtocol* proto);							// Server reset
	virtual bool		ContainsServer(const CMboxProtocol* proto) const;					// Server contained

	static bool			MailboxReset(CAttachment*& body, const CMbox* mbox);				// Mailbox reset
	virtual bool		ResetMailbox(const CMbox* mbox);									// Mailbox reset
	virtual bool		ContainsMailbox(const CMbox* mbox) const;							// Mailbox contained

	static bool			MessageRemoved(CAttachment*& body, const CMessage* msg);			// Message removed
	virtual bool		RemoveMessage(const CMessage* msg);									// Message removed
	virtual bool		ContainsMessage(const CMessage* msg) const;							// Message contained

	// Methods relating to content

	virtual void		ProcessContent();

	virtual void		SetContent(const CMIMEContent& content)
		{ mContent = content; }
							
	virtual void		SetContent(EContentType type, EContentSubType subtype)
		{ mContent.SetContent(type, subtype);}
							
	virtual void		SetContent(unsigned char* str)
		{ mContent.SetContent(str); }
	virtual void		SetContentSubtype(const char* str)
		{ mContent.SetContentSubtype(str); }
						
	virtual CMIMEContent&	GetContent()
		{ return mContent; }
	virtual const CMIMEContent&	GetContent() const
		{ return mContent; }

	virtual cdstring 	GetContentSubtypeText() const
		{ return mContent.GetContentSubtypeText(); }

	virtual ETransferMode	GetTransferMode()
		{ return mContent.GetTransferMode(); }
	virtual unsigned long	GetSize() const
		{ return mContent.GetContentSize(); }
	virtual unsigned long	GetTotalSize(bool aggregate = false) const;

	virtual void		SetName(const char* name)
		{ mName = name; }
	virtual void		SetName(const unsigned char* name)
		{ mName = name; }
	virtual const cdstring&	GetName() const
		{ return mName; }
	virtual const cdstring	GetMappedName(bool not_empty, bool force) const;

	// Flags
	virtual void SetSeen(bool seen)
		{ mFlags.Set(eSeen, seen); }
	virtual bool IsSeen() const
		{ return mFlags.IsSet(eSeen); }

	virtual void SetExtracted(bool extracted)
		{ mFlags.Set(eExtracted, extracted); }
	virtual bool IsExtracted() const
		{ return mFlags.IsSet(eExtracted); }

	virtual void SetNotCached(const CAttachment* exclude);
	virtual bool IsNotCached() const
		{ return GetContent().GetContentDisposition() == eContentDispositionMissing; }

	virtual void SetFakeText(bool fake)
		{ mFlags.Set(eFakeText, fake); }
	virtual bool IsFakeText() const
		{ return mFlags.IsSet(eFakeText); }

	// data
	virtual void		SetData(char* data);
	virtual const char*	GetData() const
		{ return mData; }
	virtual const char* GrabData()
		{ const char* temp = mData; mData = NULL; return temp; }
	virtual unsigned long	ClearAllData();

	// File related
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual bool		UniqueFile(const PPx::FSObject& spec) const;

#else
	virtual bool		UniqueFile(const cdstring& fpath) const;
#endif
	virtual const CIconRef*	GetIconRef() const;

	// Nature of part
	virtual bool		IsText() const;
	virtual bool		HasNonText() const;
	virtual bool		HasStyledText() const;
	virtual bool		HasCalendar() const;
	virtual bool		CanDisplay() const;
	virtual bool		CanEdit() const
		{ return false; }
	virtual bool		CouldEdit() const;
	virtual bool		CanChange() const
		{ return false; }

	virtual bool		IsMessage() const;
	virtual void		SetMessage(CMessage* aMessage);
	virtual CMessage*	GetMessage() const
		{ return mMessage; }

	virtual bool		IsMultipart() const;
	virtual bool		IsMixed() const;
	virtual bool		IsAlternative() const;
	virtual bool		IsDigest() const;
	virtual CAttachment*	GetFirstDigest() const;

	virtual bool		IsDSN() const;
	virtual bool		IsMDN() const;

	virtual bool		IsSigned() const;
	virtual bool		IsEncrypted() const;
	virtual bool		IsVerifiable() const;
	virtual bool		IsDecryptable() const;
	virtual bool		IsApplefile() const;
	virtual bool		IsCalendar() const;

	// Extraction/insertion
	virtual const char*	ReadPart(CMessage* owner, bool peek = false);
	virtual void		ReadAttachment(CMessage* msg,	// Read into stream
										bool peek = false,
										bool filter = true);
	virtual void		Extract(CMessage* owner, bool view);

	virtual void		WriteToStream(costream& stream,
										unsigned long& level,
										bool dummy_files,
										CProgress* progress,
										CMessage* owner = NULL,
										unsigned long count = 0,
										unsigned long start = 1) const;
	virtual void		WriteDataToStream(costream& stream,
											bool dummy_files,
											CProgress* progress,
											CMessage* owner = NULL,
											unsigned long count = 0,
											unsigned long start = 1) const;

	virtual CFilter*	GetFilter(bool decode) const
		{ return CMIMESupport::GetFilter(this, decode); }

protected:
	CAttachment*				mParent;					// Multipart parent
	CAttachmentList*			mParts;						// Multipart's contents
	CMessage*					mMessage;					// RFC822 message part
	CMIMEContent				mContent;					// Content description
	cdstring					mName;						// Name of this attachment (unencoded)
	SBitFlags					mFlags;						// Attachment flags
	char*						mData;						// Data associated with attachment

	virtual void	ReadAttachment(CMessage* msg,
									LStream* aStream,
									bool peek = false);	// Read into stream
	virtual void	ReadAttachment(CMessage* msg,						// Copy into stream
							costream* aStream,
							bool peek = false,
							unsigned long count = 0,
							unsigned long start = 1) const;

private:
			void				InitAttachment();
			CAttachment*		GetSubPart(unsigned long* index) const;				// Get part from flattened index
			const CAttachment*	FirstDisplaySubPart(bool& skip_it,					// Get first displayable attachment
													bool no_style_test = false) const;
			const CAttachment*	FirstEditSubPart() const;							// Get first editable attachment
			void				TryLaunch(CFullFileStream* aFile) const;			// Try to launch the decoded file
};

#endif
