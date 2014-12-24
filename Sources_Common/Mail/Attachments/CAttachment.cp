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


//	CAttachment.cp

#include "CAttachment.h"

#include "CAFFilter.h"
#include "CAliasAttachment.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CAppLaunch.h"
#endif
#include "CAttachmentList.h"
#include "CAttachmentManager.h"
#include "CCharsetManager.h"
#include "CDataAttachment.h"
#include "CDesktopIcons.h"
#include "CErrorHandler.h"
#include "CFileAttachment.h"
#include "CFilter.h"
#include "CFullFileStream.h"
#include "CGeneralException.h"
#include "CHTMLTransformer.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CICSupport.h"
#endif
#if __dest_os == __linux_os
#include "CMailcapMap.h"
#endif
#include "CMbox.h"
#include "CMessage.h"
#include "CMessageAttachment.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMessageWindow.h"
#endif
#include "CMIMESupport.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CProgress.h"
#include "CStreamType.h"
#include "CStringUtils.h"
#include "CTextEngine.h"

#if __dest_os == __win32_os
#include <WIN_LDataStream.h>
#include <WIN_LFileStream.h>
#include <WIN_LMemFileStream.h>
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <Finder.h>
#endif


#if __dest_os == __linux_os
#include "UNX_LDataStream.h"
#include "UNX_LFileStream.h"
#include "UNX_LMemFileStream.h"
#include <jProcessUtil.h>
#include <jFileUtil.h>
#endif

#include <stdio.h>
#include "CParserEnriched.h"
#include "CParserHTML.h"
#include "CEnrichedTransformer.h"

#include "CRFC822.h"
#include "cdstring.h"

#include <typeinfo>

#pragma mark ____________________________CAttachment

#if __dest_os == __mac_os || __dest_os == __mac_os_x
FSRef* CAttachment::sDropLocation = NULL;
#endif

// Default constructor
CAttachment::CAttachment()
{
	// Init to default values
	InitAttachment();
}

// Construct as child
CAttachment::CAttachment(CAttachment* parent)
{
	// Init to default values
	InitAttachment();

	// Add this part to its parent
	if (parent)
		parent->AddPart(this);
}

// Create text body
CAttachment::CAttachment(char* text, CAttachment* parent)
{
	// Init to default values
	InitAttachment();

	// Set data
	mData = text;

	// Set type and sub-type
	mContent.SetContent(eContentText, eContentSubPlain);

	// Add this part to its parent
	if (parent)
		parent->AddPart(this);
}

// Copy constructor
CAttachment::CAttachment(const CAttachment& copy)
	: mContent(copy.mContent),
	  mName(copy.mName)
{
	// Init to default values
	InitAttachment();

	// Copy submesssage first
	if (copy.mMessage)
		SetMessage(new CMessage(*copy.mMessage));

	// Duplicate all parts and add to this one (if not message)
	else if (copy.mParts)
	{
		// For all entries
		for(CAttachmentList::iterator iter = copy.mParts->begin(); iter != copy.mParts->end(); iter++)
		{
			// Copy items
			CAttachment* copy_attach = CAttachment::CopyAttachment(**iter);
			AddPart(copy_attach);
		}
	}

	mFlags = copy.mFlags;

	// Never copy data. Use CDataAttachment instead.
}

// Destructor
CAttachment::~CAttachment()
{
	// Remove from parent and delete all subparts
	//if (mParent) mParent->RemovePart(this);
	delete mParts;
	delete mMessage;	// This may be dangerous if message is in use by someone else (possibly reply letter)
	delete mData;

	mParent = NULL;
	mParts = NULL;
	mMessage = NULL;
	mData = NULL;
}

// Common init
void CAttachment::InitAttachment()
{
	// Init to default values
	mParent = NULL;
	mParts = NULL;
	mMessage = NULL;

	mFlags = eNone;

	mData = NULL;
}

CAttachment* CAttachment::CopyAttachment(const CAttachment& copy)
{
	//THIS IS REALLY BAD
	//why not just have a virtual method CAttachment::copy that returns
	//a copy of itself?

	// Determine which type to create from existing class
	if (typeid(copy) == typeid(CAttachment))
		return new CAttachment(copy);
	else if (typeid(copy) == typeid(CDataAttachment))
		return new CDataAttachment(*((CDataAttachment*) &copy));
	else if (typeid(copy) == typeid(CFileAttachment))
		return new CFileAttachment(*((CFileAttachment*) &copy));
	else if (typeid(copy) == typeid(CMessageAttachment))
		return new CMessageAttachment(*((CMessageAttachment*) &copy));
	else if (typeid(copy) == typeid(CAliasAttachment))
		return new CAliasAttachment(*((CAliasAttachment*) &copy));
	else
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}
	return NULL;
}

// Create from content
CAttachment* CAttachment::CreateAttachment(const CMIMEContent& content)
{
	CAttachment* new_attach = NULL;

	// Look for dummy files
	if (content.IsDummy())
	{
		// Get file path parameter
		const cdstring& file_path = content.GetXMulberryFile();

		// Create a file attachment
		new_attach = new CFileAttachment(file_path);

		const_cast<CMIMEContent&>(content).ConvertDummy();
	}
	else
		new_attach = new CAttachment();
	new_attach->SetContent(content);

	return new_attach;
}

// Clone editable version of attachments
CAttachment* CAttachment::CloneAttachment(CMessage* owner, const CAttachment* original)
{
	// If sub-message, make uneditable clone
	if (original->IsMessage())
		return new CMessageAttachment(owner, original->GetMessage());

	// Handle multipart/signed
	if (original->IsVerifiable() && original->GetPart(2))
	{
		// Clone only the first child part - ignore the signature
		return CAttachment::CloneAttachment(owner, original->GetPart(2));
	}
	
	// Handle multipart/encrypted - but only if its the top-level item
	else if (original->IsDecryptable() && (owner->GetBody() == original))
	{
		// Decrypt the message and then clone the decrypted parts

		// Look for security plugin
		if (CPluginManager::sPluginManager.HasSecurity())
		{
			CAttachment* old_body = owner->GetBody();
			CMessageCryptoInfo info;
			if (CSecurityPlugin::VerifyDecryptPart(owner, NULL, info))
			{
				// Clone the decrypted part
				return CAttachment::CloneAttachment(owner, owner->GetBody());
			}
		}
		
		// Clone only the second child part - failed to decrypt
		return CAttachment::CloneAttachment(owner, original->GetPart(3));
	}
	
	// Handle multipart/alternative
	else if (original->IsAlternative() && original->FirstDisplayPart())
	{
		// Clone only the alternative part that would be displayed
		return CAttachment::CloneAttachment(owner, original->FirstDisplayPart());
	}

	// Handle other multipart
	else if (original->GetParts())
	{
		// Create new data attachment and copy content
		CDataAttachment* new_attach = new CDataAttachment;
		new_attach->GetContent() = original->GetContent();

		// Clone each part
		for(CAttachmentList::const_iterator iter = original->GetParts()->begin();
			iter != original->GetParts()->end(); iter++)
		{
			// Copy items
			CAttachment* copy_attach = CAttachment::CloneAttachment(owner, *iter);
			new_attach->AddPart(copy_attach);
		}

		return new_attach;
	}

	// Handle editable single part
	else if (original->CouldEdit())
	{
		// Create new data attachment and copy content
		CDataAttachment* new_attach = new CDataAttachment;
		new_attach->GetContent() = original->GetContent();

		// Read in attachment data and copy
		bool delete_after = (original->GetData() == NULL);

		// Make sure attachment read in
		owner->ReadAttachment((CAttachment*) original);

		// Copy data to new part
		new_attach->SetData(::strdup(original->GetData()));

		// Clear data if not required
		if (delete_after)
			((CAttachment*) original)->SetData(NULL);

		return new_attach;
	}
	else
		// Uneditable single part
		return new CAliasAttachment(owner, original);
}

// Compare with same type
int CAttachment::operator==(const CAttachment& test) const
{
	// Compare class id's first
	if (typeid(*this) != typeid(test)) return false;

	// Same id's so determine which operator== to call
	if (typeid(*this) == typeid(CFileAttachment))
		// Do operator== comparision
		return ( (*(CFileAttachment*) this) == *((CFileAttachment*) &test));
	else
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	return false;
}

// Add a part
// CAttachment* part	: part to add
// short pos			: position to add it at in list. If -1 at to end
void CAttachment::AddPart(CAttachment* part, long pos)
{
	// Only if multipart or message and part exists
	if ((!IsMultipart() && !IsMessage()) || !part)
		return;

	// Create list if required
	if (!mParts)
		mParts = new CAttachmentList();

	// Insert item
	if (pos < 0)
		mParts->push_back(part);
	else
		mParts->insert(mParts->begin() + pos, part);

	// Own child
	part->SetParent(this);
}

// Remove part (with/without delete)
void CAttachment::RemovePart(CAttachment* part, bool delete_it)
{
	// Only if multipart or message, part exists and list exists
	if ((!IsMultipart() && !IsMessage()) || !part || !mParts)
		return;

	// Remove item and reset parent if it still exists
	mParts->RemoveAttachment(part, delete_it);
	if (!delete_it)
		part->SetParent(NULL);

	// Now remove the parts list if empty
	if (mParts->empty())
	{
		delete mParts;
		mParts = NULL;
	}
}

// Count non-multiparts
unsigned long CAttachment::CountParts() const
{
	unsigned long count = 0;

	// Count sub-parts
	if (mParts)
	{
		// Iterate over each part, recurse to get total
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			count += (*iter)->CountParts();
		}
	}

	// Otherwise just this part (including message/rfc822's) if not empty
	else if (mContent.GetContentType() != eNoContentType)
		count = 1;

	return count;
}

void CAttachment::ClearParts()
{
	delete mParts;
	mParts = NULL;
}

// Check number of text parts (only one of alternative)
bool CAttachment::HasUniqueTextPart(bool only_alt) const
{
	// Count sub-parts
	if (mParts)
	{
		// Check for alternative
		if (mContent.GetContentSubtype() == eContentSubAlternative)
		{
			// Check each part - first one that returns false terminates
			for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
			{
				if (!(*iter)->HasUniqueTextPart())
					return false;
			}

			return true;
		}
		else if (!only_alt)
		{
			if (mContent.GetContentSubtype() == eContentSubSigned)
			{
				if (mParts->size() != 2)
					return false;
				else if (!mParts->front()->HasUniqueTextPart())
					return false;
				else
					return true;
			}
		}

		// Any other multipart => false
		return (mParts->size() == 1) ? (*mParts->begin())->HasUniqueTextPart() : false;
	}

	// Otherwise check if this part can be displayed
	else if (CanDisplay())
		return true;

	// Some other part
	else
		return false;
}

// See if any cont-disp: attachment parts are present
bool CAttachment::HasAttachments() const
{
	// Count sub-parts
	if (mParts)
	{
		// Check each part - first one that returns false terminates
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			if ((*iter)->HasAttachments())
				return true;
		}
	}

	// Otherwise check if this part can be displayed
	else if (GetContent().GetContentDisposition() != eContentDispositionInline)
		return true;

	// Some other part
	return false;
}

// Look for part counting down
CAttachment* CAttachment::GetPart(unsigned long index) const
{
	// If got this one return
	index--;
	if (!index) return (CAttachment*) this;

	if (mParts)
	{
		// For all parts
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			CAttachment* found = (*iter)->GetSubPart(&index);
			if (found) return found;
		}
	}
	else if (mMessage)
		return mMessage->GetBody()->GetSubPart(&index);

	return NULL;
}

// Look for part counting down
CAttachment* CAttachment::GetSubPart(unsigned long* index) const
{
	// If got this one return
	(*index)--;
	if (!*index) return (CAttachment*) this;

	if (mParts)
	{
		// For all parts
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			CAttachment* found = (*iter)->GetSubPart(index);
			if (found) return found;
		}
	}
	else if (mMessage)
		return mMessage->GetBody()->GetSubPart(index);

	return NULL;
}

// Look for first part that can be displayed
CAttachment* CAttachment::FirstDisplayPart(bool no_style_test) const
{
	bool ignore = false;
	return const_cast<CAttachment*>(FirstDisplaySubPart(ignore, no_style_test));
}

// Look for first part that can be displayed
const CAttachment* CAttachment::FirstDisplaySubPart(bool& skip_it, bool no_style_test) const
{
	if (mParts)
	{
		const CAttachment* firstFound = NULL;
		EContentSubType subtype = (EContentSubType) (CPreferences::sPrefs->showStyled.GetValue() ? eContentSubPlain - 1 : (eContentSubHTML + 1));
		bool alternative = (mContent.GetContentSubtype() == eContentSubAlternative);

		// For all parts
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			// Find first display in current subpart
			const CAttachment* found = (*iter)->FirstDisplaySubPart(skip_it, no_style_test);

			// Special process if found
			if (found)
			{
				// Look for alternatives
				if (alternative)
				{
					switch((*iter)->mContent.GetContentType())
					{
					case eContentText:
					{
						// Get the part's subtype and restrict to proper range to avoid displaying text parts we're not sure of
						EContentSubType part_subtype = (*iter)->mContent.GetContentSubtype();
						if (part_subtype > eContentSubHTML)
							part_subtype = (CPreferences::sPrefs->showStyled.GetValue() ? eNoContentSubType : eContentSubXtoken);

                        // Skip text with no content
                        if ((*iter)->mContent.GetContentSize() <= 4)
                            break;

						// Get higher type if showing styled or style test is turned off
						if ((no_style_test || CPreferences::sPrefs->showStyled.GetValue()) && (part_subtype > subtype) ||
							// Get lower type if not showing styled
							!CPreferences::sPrefs->showStyled.GetValue() && (part_subtype < subtype))
						{
							subtype = (*iter)->mContent.GetContentSubtype();
							firstFound = found;
						}
						break;
					}
					default:;
					}
				}

				// If not skipping return it
				else if (!skip_it)
					return found;

				// Mark it as first found if one not already present
				if (!firstFound)
					firstFound = found;
			}
		}

		// Now return the one it thinks is best
		if (firstFound)
			return firstFound;
	}
	else if (mMessage)
		return mMessage->GetBody()->FirstDisplaySubPart(skip_it, no_style_test);

	else if (CanDisplay())
	{
		// Check size - special fix for dumb AOL message/rfc822 forwarding
		skip_it = (GetContent().GetContentSize() <= 4);

		// If got this one return it
		return this;
	}

	return NULL;
}

// Look for first part that can be edited
CAttachment* CAttachment::FirstEditPart() const
{
	return const_cast<CAttachment*>(FirstEditSubPart());
}

// Look for first part that can be edited
const CAttachment* CAttachment::FirstEditSubPart() const
{
	if (mParts)
	{
		// For all parts
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			const CAttachment* found = (*iter)->FirstEditSubPart();
			if (found) return found;
		}
	}
	else if (mMessage)
		return mMessage->GetBody()->FirstEditSubPart();

	// If got this one return
	else if (CanEdit())
		return this;

	return NULL;
}

// Get matching alternative
CAttachment* CAttachment::GetAlternative(EContentType type, EContentSubType subtype)
{
	// Look for multipart/alternative parent
	if (!GetParent() || !GetParent()->IsAlternative())
		return NULL;
	
	// Look at each sibling for one matching the type/subtype
	for(CAttachmentList::iterator iter = GetParent()->GetParts()->begin(); iter != GetParent()->GetParts()->end(); iter++)
	{
		if (((*iter)->GetContent().GetContentType() == type) &&
			((*iter)->GetContent().GetContentSubtype() == subtype))
			return *iter;
	}
	
	return NULL;
}

// Return body part number
void CAttachment::GetPartNumber(cdstring& part, bool underscore) const
{
	// Check for sub-part
	if (mParent)
	{

		// Ask parent for its part number
		mParent->GetPartNumber(part, underscore);

		// Do not add multipart at top level of message
		if (mParts && mParent->IsMessage())
		{
			// Do not add
		}
		// Add this parts level
		else
		{
			// Get position of this part in the parents list
			unsigned long pos = mParent->mParts->FetchIndexOf(this);

			// Append this parts position to string (prepend with '_'/'.' if parent has parent)
			char str[256];
			if (mParent->mParent || mParent->IsMessage())
			{
				::snprintf(str, 256, underscore ? "_%ld" : ".%ld", pos);
				part += str;
			}
			else
			{
				::snprintf(str, 256, "%ld", pos);
				part = str;
			}
		}
	}

	// Check for single part
	else
		part = "1";
}

cdstring CAttachment::GetURL() const
{
	cdstring url = ";section=";
	cdstring partnum;
	GetPartNumber(partnum);
	url += partnum;
	
	return url;
}

// Process files for sending
void CAttachment::ProcessSend()
{
	// If multipart, iterate over parts (not if already AppleDouble)
	if (mParts && !IsApplefile())
	{
		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)

			// Convert each sub-part
			(*iter)->ProcessSend();

	}

	// Check for AppleDouble encoding
	else if ((mContent.GetTransferMode() == eAppleDoubleMode) && CanChange())
	{
		// Duplicate this part twice
		CAttachment* mSinglePart = CopyAttachment(*this);
		CAttachment* mDataPart = CopyAttachment(*this);

		// Set appropriate mode for each
		mSinglePart->GetContent().SetTransferMode(eAppleSingleMode);
		mDataPart->GetContent().SetTransferMode(eMIMEMode);

		// Recalculate mapping for data which must be text
		mDataPart->GetContent().SetContent(eContentText, eContentSubPlain);
		mDataPart->ProcessContent();

		// Make this into multipart
		mContent.SetContent(eContentMultipart, eContentSubAppleDouble);

		// Add sub-parts
		AddPart(mSinglePart);
		AddPart(mDataPart);

	}
	else if ((mContent.GetContentType() == eContentText) && (mContent.GetContentSubtype() == eContentSubEnriched) &&
			 (CPreferences::sPrefs->enrMultiAltPlain.GetValue() || CPreferences::sPrefs->enrMultiAltHTML.GetValue()) &&
			 CanChange())
	{
		char *temp = mData;
		mData = NULL;
		CAttachment* mPlainPart = NULL;
		CAttachment* mEnrichedPart = NULL;
		CAttachment* mHTMLPart = NULL;


		mEnrichedPart = CopyAttachment(*this);
		mEnrichedPart->GetContent().SetContent(eContentText, eContentSubEnriched);
		mEnrichedPart->SetData(temp);
		mEnrichedPart->ProcessContent();


		if (CPreferences::sPrefs->enrMultiAltPlain.GetValue())
		{
			mPlainPart = CopyAttachment(*this);
			mPlainPart->GetContent().SetContent(eContentText, eContentSubPlain);

			// Do utf8->utf16->utf8 conversion
			cdustring utf16(temp);
			CParserEnriched parser(utf16, NULL);
			utf16.steal(const_cast<unichar_t*>(parser.Parse(0, false)));
			mPlainPart->SetData(utf16.ToUTF8().release());
			mPlainPart->ProcessContent();
			mPlainPart->ProcessSend();
		}


		if (CPreferences::sPrefs->enrMultiAltHTML.GetValue())
		{
			mHTMLPart = CopyAttachment(*this);
			mHTMLPart->GetContent().SetContent(eContentText, eContentSubHTML);
			mHTMLPart->SetData(CEnrichedUtils::ConvertToHTML(temp));
			mHTMLPart->ProcessContent();
		}

		mContent.SetContent(eContentMultipart, eContentSubAlternative);

		if (mPlainPart)
			AddPart(mPlainPart);
		if (mEnrichedPart)
			AddPart(mEnrichedPart);
		if(mHTMLPart)
			AddPart(mHTMLPart);

	}
	else if ((mContent.GetContentType() == eContentText) && (mContent.GetContentSubtype() == eContentSubHTML) &&
			 (CPreferences::sPrefs->htmlMultiAltPlain.GetValue() || CPreferences::sPrefs->htmlMultiAltEnriched.GetValue()) &&
			 CanChange())
	{
		char *temp = mData;
		mData = NULL;
		CAttachment* mPlainPart = NULL;
		CAttachment* mEnrichedPart = NULL;
		CAttachment* mHTMLPart = NULL;


		mHTMLPart = CopyAttachment(*this);
		mHTMLPart->GetContent().SetContent(eContentText, eContentSubHTML);
		mHTMLPart->SetData(temp);
		mHTMLPart->ProcessContent();


		if (CPreferences::sPrefs->htmlMultiAltPlain.GetValue())
		{
			mPlainPart = CopyAttachment(*this);
			mPlainPart->GetContent().SetContent(eContentText, eContentSubPlain);
			
			// Do utf8->utf16->utf8 conversion
			cdustring utf16(temp);
			CParserHTML parser(utf16, NULL, NULL, NULL);
			utf16.steal(const_cast<unichar_t*>(parser.Parse(0, false)));
			mPlainPart->SetData(utf16.ToUTF8().release());
			mPlainPart->ProcessContent();
			mPlainPart->ProcessSend();
		}


		if (CPreferences::sPrefs->htmlMultiAltEnriched.GetValue())
		{
			mEnrichedPart = CopyAttachment(*this);
			mEnrichedPart->GetContent().SetContent(eContentText, eContentSubEnriched);
			mEnrichedPart->SetData(CHTMLUtils::ConvertToEnriched(temp));
			mEnrichedPart->ProcessContent();
		}

		mContent.SetContent(eContentMultipart, eContentSubAlternative);

		if(mPlainPart)
			AddPart(mPlainPart);
		if(mEnrichedPart)
			AddPart(mEnrichedPart);
		if(mHTMLPart)
			AddPart(mHTMLPart);

	}
	else if ((mContent.GetContentType() == eContentText) && (mContent.GetContentSubtype() == eContentSubPlain) && CanChange())
	{
		// Force line wrap on this attachment
		if (mData)
		{
			// Wrap it in flowed mode if requested and inline disposition
			bool flowed = CPreferences::sPrefs->mFormatFlowed.GetValue() &&
							(mContent.GetContentDisposition() == eContentDispositionInline);
			const char* wrapped = CTextEngine::WrapLines(mData, ::strlen(mData), CRFC822::GetWrapLength(), flowed);
			SetData(const_cast<char*>(wrapped));
			
			// Add flowed parameter if not already
			if (flowed && !mContent.IsFlowed())
				mContent.SetContentParameter(cMIMEParameter[eFormat], cMIMEParameter[eFlowed]);
		}
		ProcessContent();
	}
	else
		ProcessContent();
}

// Process attachments for sending with crypto
void CAttachment::ProcessSendCrypto(CSecurityPlugin::ESecureMessage mode, bool use_mime)
{
	// Iterate over parts
	if (mParts)
	{
		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)

			// Convert each sub-part
			(*iter)->ProcessSendCrypto(mode, use_mime);
	}
	else
	{
		switch(mode)
		{
		case CSecurityPlugin::eSign:
		case CSecurityPlugin::eEncryptSign:
			// Special processing for format=flowed due to bugs in PGP
			if (GetContent().IsFlowed())
			{
				if (use_mime)
				{
					// Force QP encoding on
					switch(GetContent().GetTransferEncoding())
					{
					case eQuotedPrintableEncoding:
					case eBase64Encoding:
						// These are OK
						break;
					default:
						// Force QP
						GetContent().SetTransferEncoding(eQuotedPrintableEncoding);
						break;
					}
				}
				else
				{
					// Turn off QP
					GetContent().RemoveContentParameter(cMIMEParameter[eFormat]);
				}
			}
			break;
		default:
			break;
		}
	}
}

void CAttachment::ProcessContent()
{
	// Process text to determine encoding and charset
	if (IsText() && (mData != NULL))
	{
		EContentTransferEncoding encoding;
		bool non_ascii = CMIMESupport::DetermineTextEncoding(mData) != e7bitEncoding;

		// enriched & html goes as q-p
		if ((mContent.GetContentType() == eContentText) &&
			((mContent.GetContentSubtype() == eContentSubEnriched) ||(mContent.GetContentSubtype() == eContentSubHTML)))
			encoding = eQuotedPrintableEncoding;
		else
			// Determine encoding from data
			encoding = non_ascii ? eQuotedPrintableEncoding : e7bitEncoding;

		mContent.SetTransferEncoding(encoding);
		mContent.SetTransferMode(eMIMEMode);

		// Set charset: use US Ascii if no chars > 0x7F
		if (!non_ascii)
		{
			// Use US Ascii if no chars > 0x7F
			mContent.SetCharset(i18n::eUSASCII);
		}
		else
		{
			i18n::ECharsetCode charset = i18n::CCharsetManager::sCharsetManager.CharsetForText(mData, CPreferences::sPrefs->mAlwaysUnicode.GetValue());
			
			// Current data is UTF8 so conversion to other charset may be needed
			if (charset != i18n::eUTF8)
			{
				const char* new_data = NULL; 
				if (i18n::CCharsetManager::sCharsetManager.Transcode(i18n::eUTF8, charset,	mData, ::strlen(mData), new_data))
				{
					delete mData;
					mData = const_cast<char*>(new_data);
				}
				else
					charset = i18n::eUTF8;
			}

			mContent.SetCharset(charset);
		}
	}
	else
		// Always have US-ASCII
		mContent.SetCharset(i18n::eUSASCII);

	// Do not do any more
}

// Check whether any part owned by mbox
bool CAttachment::TestOwnership(const CMbox* owner) const
{
	// Test this part
	if (mMessage && (mMessage->GetMbox() == owner))
		return true;

	// Test any subparts
	if (mParts)
		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			if ((*iter)->TestOwnership(owner))
				return true;
		}

	// All done
	return false;
}

// Server reset
bool CAttachment::ServerReset(CAttachment*& body, const CMboxProtocol* proto)
{
	// Test contents
	if (body->ContainsServer(proto))
	{
		delete body;
		body = NULL;
		return true;
	}
	else
		return body->ResetServer(proto);
}

// Server reset
bool CAttachment::ResetServer(const CMboxProtocol* proto)
{
	if (mParts)
	{
		bool changed = false;

		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); )
		{
			// Test contents
			if ((*iter)->ContainsServer(proto))
			{
				delete *iter;
				iter = mParts->erase(iter);
				changed = true;
				continue;
			}
			else
				changed = changed || (*iter)->ResetServer(proto);
			
			iter++;
		}

		return changed;
	}
	else
		// Always return false as this part has already been tested for message contents
		return false;
}

// Server contained
bool CAttachment::ContainsServer(const CMboxProtocol* proto) const
{
	return (mMessage != NULL) &&
			((mMessage->GetMbox()->GetProtocol() == proto) ||
				(mMessage->GetMbox()->GetMsgProtocol() == proto));
}

// Mailbox reset
bool CAttachment::MailboxReset(CAttachment*& body, const CMbox* mbox)
{
	// Test contents
	if (body->ContainsMailbox(mbox))
	{
		delete body;
		body = NULL;
		return true;
	}
	else
		return body->ResetMailbox(mbox);
}

// Mailbox reset
bool CAttachment::ResetMailbox(const CMbox* mbox)
{
	if (mParts)
	{
		bool changed = false;

		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); )
		{
			// Test contents
			if ((*iter)->ContainsMailbox(mbox))
			{
				delete *iter;
				iter = mParts->erase(iter);
				changed = true;
				continue;
			}
			else
				changed = changed || (*iter)->ResetMailbox(mbox);
			
			iter++;
		}

		return changed;
	}
	else
		// Always return false as this part has already been tested for message contents
		return false;
}

// Mailbox contained
bool CAttachment::ContainsMailbox(const CMbox* mbox) const
{
	return (mMessage != NULL) && (mMessage->GetMbox() == mbox);
}

// Message removed
bool CAttachment::MessageRemoved(CAttachment*& body, const CMessage* msg)
{
	// Test contents
	if (body->ContainsMessage(msg))
	{
		delete body;
		body = NULL;
		return true;
	}
	else
		return body->RemoveMessage(msg);
}

// Message removed
bool CAttachment::RemoveMessage(const CMessage* msg)
{
	if (mParts)
	{
		bool changed = false;

		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); )
		{
			// Test contents
			if ((*iter)->ContainsMessage(msg))
			{
				delete *iter;
				iter = mParts->erase(iter);
				changed = true;
				continue;
			}
			else
				changed = changed || (*iter)->RemoveMessage(msg);
			
			iter++;
		}

		return changed;
	}
	else
		// Always return false as this part has already been tested for message contents
		return false;
}

// Message contained
bool CAttachment::ContainsMessage(const CMessage* msg) const
{
	return (mMessage != NULL) && (mMessage == msg);
}

// Get size of this and all subparts
unsigned long CAttachment::GetTotalSize(bool aggregate) const
{
	// Get size of this part
	unsigned long size = GetSize();

	// Add size of all sub-parts if not AppleDouble
	if (mParts && (aggregate || !IsApplefile()))
		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)

			// Get total size of each sub-part
			size += (*iter)->GetTotalSize();

	return size;
}

// Get suite of icons from icon cache
const cdstring CAttachment::GetMappedName(bool not_empty, bool force) const
{
	// Return existing name if present
	if (mName.empty())
	{
		cdstring name;
		const CAttachment* part_to_map = this;

		// special for AppleDouble
		if (IsMultipart() && IsApplefile() && GetParts())
		{
			// Generate a name if none is present
			if (mContent.GetMappedName().empty())
			{
				// Look for name in data part
				cdstring rsrc_name = GetParts()->front()->GetContent().GetMappedName();
				cdstring data_name = GetParts()->back()->GetContent().GetMappedName();

				// Force data part name to name of rsrc part (if it exists)
				if (data_name.empty() && !rsrc_name.empty())
				{
					GetParts()->back()->GetContent().SetMappedName(rsrc_name);
				}

				// Name for this part is name of data part
				cdstring mapped_name = GetParts()->back()->GetMappedName(false, force);
				const_cast<CAttachment*>(this)->mContent.SetMappedName(mapped_name);	// Cast away const of this
			}
			
			// Use the data part to determine file mapping of the multipart/applefile
			part_to_map = GetParts()->back();
		}

		name = mContent.GetMappedName();

		// If it does not currently exist create temporary default
		if (not_empty && name.empty())
		{
			cdstring part_num;
			GetPartNumber(part_num, true);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			name = "Unnamed part #";
			name += part_num;
#elif __dest_os == __win32_os
			name = "Unnamed part ";
			name += part_num;
#elif __dest_os == __linux_os
			name = "UnnamedPart";
			name += part_num;
#endif
		}
		
		// Must have proper file extension
		if (!::strchr(name.c_str(), '.'))
			name += CMIMESupport::MapMIMEToFileExt((CAttachment&) *part_to_map);
		else if (force)
		{
			*(::strrchr(name.c_str_mod(), '.')) = 0;
			name += CMIMESupport::MapMIMEToFileExt((CAttachment&) *part_to_map);
		}

		// If still no suffix, use any suffix present in MIME headers
		if (!::strchr(name.c_str(), '.'))
		{
			cdstring result = mContent.GetMappedName();
			if (::strrchr(result.c_str(), '.'))
				name += cdstring(::strrchr(result.c_str(), '.'));
		}

		const_cast<CAttachment*>(this)->SetName(name);
	}

	return mName;
}

// Mark as not cached if not exclude
void CAttachment::SetNotCached(const CAttachment* exclude)
{
	// Set this one to missing if not excluded
	if (this != exclude)
		GetContent().SetContentDisposition(eContentDispositionMissing);

	// Do the same for all subparts
	if (mParts)
	{
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
			(*iter)->SetNotCached(exclude);
	}
}

// Set new data
void CAttachment::SetData(char* data)
{
	delete mData;
	mData = data;
	mContent.SetContentSize(mData ? ::strlen(mData) : 0);
}

// Clear all data
unsigned long CAttachment::ClearAllData()
{
	unsigned long freed = 0;

	if (mParts)
	{
		// Delete data for all subparts
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			freed += (*iter)->ClearAllData();
		}
	}
	else
	{
		freed = (mData ? ::strlen(mData) + 1 : 0);

		// Delete data for this attachment
		delete mData;
		mData = NULL;
	}

	return freed;
}

#pragma mark ____________________________File Related

#if __dest_os == __mac_os || __dest_os == __mac_os_x

// Check hierarchy for unique file
bool CAttachment::UniqueFile(const PPx::FSObject& spec) const
{
	if (mParts)
	{
		// For all sub-parts
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			if (!(*iter)->UniqueFile(spec))
				return false;
		}

		// No match found
		return true;
	}
	else
		// This is not a file so its unique
		return true;
}

#else

// Check hierarchy for unique path
bool CAttachment::UniqueFile(const cdstring& fpath) const
{
	if (mParts)
	{
		// For all sub-parts
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			if (!(*iter)->UniqueFile(fpath))
				return false;
		}

		// No match found
		return true;
	}
	else
		// This is not a file so its unique
		return true;
}
#endif

// Get suite of icons from icon cache
const CIconRef* CAttachment::GetIconRef() const
{
#if 0
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	ICMapEntry entry;
	if (CICSupport::ICMapFileName(mContent, entry, true) == noErr)
		return CDesktopIcons::GetDesktopIcons(entry.fileCreator, entry.fileType);
#endif
#endif
	return CDesktopIcons::GetDesktopIconsFromMIME(mContent.GetContentTypeText(), mContent.GetContentSubtypeText());
}

#pragma mark ____________________________Nature of Part

// Check for text
bool CAttachment::IsText() const
{
	// Check for fake
	if (IsFakeText())
		return true;

	// Various types of MIME types that Mulberry treats as text
	switch(mContent.GetContentType())
	{
	// Always do text/xxx
	case eContentText:
		return true;

	// Allow signed and encrypted parts to be displayed
	case eContentApplication:
		switch(mContent.GetContentSubtype())
		{
		case eContentSubPGPSigned:
		case eContentSubPGPEncrypted:
		case eContentSubPGPKeys:
			return true;
		default:
			return false;
		};

	// Allow some message/xxx parts
	case eContentMessage:
		switch(mContent.GetContentSubtype())
		{
		case eContentSubExternalBody:
		case eContentSubDeliveryStatus:
		case eContentSubDispositionNotification:
			return true;
		default:
			return false;
		};

	default:
		return false;
	}
}

// Check for any non-text parts other than multipart
bool CAttachment::HasNonText() const
{
	if (mParts)
	{
		// Check for signed and have first part process this
		if (IsSigned() && (mParts->size() == 2))
			return mParts->front()->HasNonText();

		// Check multipart/alternative parts for non-text
		else if (IsAlternative())
		{
			// For all sub-parts
			for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
			{
				if ((*iter)->HasNonText())
					return true;
			}
			
			return false;
		}
		
		// Special check for multipart/mixed with single part - check the single part
		else if (IsMixed() && (mParts->size() == 1))
			return mParts->front()->HasNonText();

		// Other multiple part items are treated as attachments
		else
			return true;
	}
	// Just look for non-text in this part
	else
		return !IsText();
}

bool CAttachment::HasStyledText() const
{
	// Check for multiple parts
	if (mParts)
	{
		// Get first part that will be displayed - always force the check to look for
		// the highest level of styling irrespecting of show styled option in preferences
		CAttachment* display = FirstDisplayPart(true);
		return display ? display->HasStyledText() : false;
	}
	else
		// Just look for text/enriched or text/html
		return (mContent.GetContentType() == eContentText) &&
				((mContent.GetContentSubtype() == eContentSubEnriched) ||
				 (mContent.GetContentSubtype() == eContentSubHTML));
}

bool CAttachment::HasCalendar() const
{
	// Check for multiple parts
	if (mParts)
	{
		// Check for signed and have first part process this
		if (IsSigned() && (mParts->size() == 2))
			return mParts->front()->HasCalendar();

		// For all sub-parts
		bool result = false;
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			if ((*iter)->IsCalendar())
				result = true;
			else if ((*iter)->IsText())
				continue;
			else
				// Has some other type of content at the top level in addition to text & calendars
				return false;
		}
		
		return result;
	}
	else
		// Just look for text/calendar
		return IsCalendar();
}

// Check for content that Mulberry can display
bool CAttachment::CanDisplay() const
{
	switch(mContent.GetContentType())
	{
	// Always do text/xxx
	case eContentText:
		return true;

	// Allow signed and encrypted parts to be displayed
	case eContentApplication:
		switch(mContent.GetContentSubtype())
		{
		case eContentSubPGPSigned:
		case eContentSubPGPEncrypted:
		case eContentSubPGPKeys:
			return true;
		default:
			return false;
		};

	// Allow some message/xxx parts
	case eContentMessage:
		switch(mContent.GetContentSubtype())
		{
		case eContentSubExternalBody:
		case eContentSubDeliveryStatus:
		case eContentSubDispositionNotification:
			return true;
		default:
			return false;
		};

	default:
		return false;
	}
}

// Check for content that Mulberry could edit
// only text/xxx at the moment
bool CAttachment::CouldEdit() const
{
	return (mContent.GetContentType() == eContentText);
}

// Check for multipart
bool CAttachment::IsMultipart() const
{
	// Check for sub-parts first
	return (mContent.GetContentType() == eContentMultipart);
}

// Check for multipart/mixed
bool CAttachment::IsMixed() const
{
	// Check for sub-parts first
	return (mContent.GetContentType() == eContentMultipart) &&
			(mContent.GetContentSubtype() == eContentSubMixed);
}

// Check for multipart/alternative
bool CAttachment::IsAlternative() const
{
	// Check for sub-parts first
	return (mContent.GetContentType() == eContentMultipart) &&
			(mContent.GetContentSubtype() == eContentSubAlternative);
}

// Check for multipart/digest
bool CAttachment::IsDigest() const
{
	// Check for sub-parts first
	return (mContent.GetContentType() == eContentMultipart) &&
			(mContent.GetContentSubtype() == eContentSubDigest);
}

// Look for first message/rfc822 in a multipart/digest
CAttachment* CAttachment::GetFirstDigest() const
{
	// Only if subparts exist
	if (mParts && mParts->size())
	{
		// Check for top-level digest
		if (IsDigest())
		{
			// Scan parts for first message/rfc822
			for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
			{
				if ((*iter)->IsMessage())
					return *iter;
			}
		}
		else
		{
			// Scan each subpart for a digest
			for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
			{
				if ((*iter)->IsDigest())
					return (*iter)->GetFirstDigest();
			}
		}
	}

	return NULL;
}

// Check for multipart/report; report-type=delivery-status
bool CAttachment::IsDSN() const
{
	// Check for sub-parts first
	return IsMultipart() && (mContent.GetContentSubtype() == eContentSubReport) &&
			!::strcmpnocase(mContent.GetContentParameter(cMIMEParameter[eReportType]), cMIMEReportTypeParameter[eReportTypeDSN]);
}

// Check for multipart/report; report-type=disposition-notification
bool CAttachment::IsMDN() const
{
	// Check for sub-parts first
	return IsMultipart() && (mContent.GetContentSubtype() == eContentSubReport) &&
			!::strcmpnocase(mContent.GetContentParameter(cMIMEParameter[eReportType]), cMIMEReportTypeParameter[eReportTypeMDN]);
}

// Check for multipart/signed
bool CAttachment::IsSigned() const
{
	// Check for multipart/signed
	return IsMultipart() && (mContent.GetContentSubtype() == eContentSubSigned);
}

// Check for multipart/encrypted
bool CAttachment::IsEncrypted() const
{
	// Check for multipart/signed
	return IsMultipart() && (mContent.GetContentSubtype() == eContentSubEncrypted);
}

// Check for multipart/signed
bool CAttachment::IsVerifiable() const
{
	// Check for multipart/signed
	if (IsSigned())
		return true;
	
	// Check for application/(x-)pkcs7-mime; smime-type=signed-data
	if ((mContent.GetContentType() == eContentApplication) &&
		((mContent.GetContentSubtype() == eContentSubPKCS7Mime) || (mContent.GetContentSubtype() == eContentSubXPKCS7Mime)) &&
		!::strcmpnocase(mContent.GetContentParameter(cMIMEParameter[sSMIMEType]), cMIMESMIMETypeParameter[eSMIMETypeSigned]))
		return true;
	
	return false;
}

// Check for multipart/encrypted
bool CAttachment::IsDecryptable() const
{
	// Check for multipart/encrypted
	if (IsEncrypted())
		return true;
	
	// Check for application/(x-)pkcs7-mime (smime-type parameter is optional)
	if ((mContent.GetContentType() == eContentApplication) &&
		((mContent.GetContentSubtype() == eContentSubPKCS7Mime) || (mContent.GetContentSubtype() == eContentSubXPKCS7Mime)))
		return true;
	
	return false;
}

// Check for applefile
bool CAttachment::IsApplefile() const
{
	// Check for appledouble/applesingle
	return (((mContent.GetContentType() == eContentMultipart) &&
			 (mContent.GetContentSubtype() == eContentSubAppleDouble)) ||
			((mContent.GetContentType() == eContentApplication) &&
			 (mContent.GetContentSubtype() == eContentSubApplefile)));
}

// Check for calendar part
bool CAttachment::IsCalendar() const
{
	return ((mContent.GetContentType() == eContentText) &&
			(mContent.GetContentSubtype() == eContentSubCalendar));
}

// Check for message part
bool CAttachment::IsMessage() const
{
	return ((mContent.GetContentType() == eContentMessage) &&
			(mContent.GetContentSubtype() == eContentSubRFC822));
}

// Set sub-message
void  CAttachment::SetMessage(CMessage* aMessage)
{
	delete mMessage;
	mMessage = aMessage;

	// Make sub-message's parts owned by this one
	mMessage->GetBody()->SetParent(this);

	// Make sub-message main part a child of this one
	AddPart(mMessage->GetBody());
}

#pragma mark ____________________________Reading in

const char* CAttachment::ReadPart(CMessage* owner, bool peek)
{
	// Just get owner to do it
	return owner->ReadPart(this, peek);
}

// Read into stream
void CAttachment::ReadAttachment(CMessage* msg, bool peek, bool filter)
{
	// Make its mailbox read it in if not already here or a different part
	if (!GetData())
	{
		// Create text stream
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		LHandleStream data;
#elif __dest_os == __win32_os || __dest_os == __linux_os
		LMemFileStream data;
#else
#error __dest_os
#endif

		// Get appropriate filter
		CFilter* aFilter = (filter ? CMIMESupport::GetFilter(this, true) : new CFilterEndls());
		aFilter->SetStream(&data);
		aFilter->SetForNetwork(lendl);

		try
		{
			// Tell mbox to read this part
			CMessage* top_message = msg;
			while(top_message->GetOwner())
				top_message = top_message->GetOwner();
			ReadAttachment(top_message, aFilter, peek);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			if (data.GetMarker())
#elif __dest_os == __win32_os || __dest_os == __linux_os
			if (data.GetLength())
#else
#error __dest_os
#endif
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				StHandleLocker lock(data.GetDataHandle());

				// Make a copy of the text
				char* txt = ::strndup(*data.GetDataHandle(), data.GetMarker());
#elif __dest_os == __win32_os || __dest_os == __linux_os
				// Must write c-string terminator
				char end = 0;
				data.WriteBlock(&end, 1);

				// Get original data
				char* txt = data.DetachData();
				if (!txt || !*txt)
				{
					delete txt;
					txt = NULL;
				}
#else
#error __dest_os
#endif

				// Set it in part
				SetData(txt);
				SetSeen(true);
			}

		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up and throw up
			delete aFilter;
			CLOG_LOGRETHROW;
			throw;
		}

		// Clean up
		delete aFilter;
	}
}

#pragma mark ____________________________Extraction

// Extract part from server
void CAttachment::Extract(CMessage* owner, bool view)
{
	// Does it have sub-parts (not apple double)?
	if (mParts && !IsApplefile())
	{
		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			// Write each sub-part to stream
			(*iter)->Extract(owner, view);
		}

	}

	else if (view)
		CAttachmentManager::sAttachmentManager.ViewAttachment(owner, this);
#if 1
	else
		CAttachmentManager::sAttachmentManager.ExtractAttachment(owner, this, true, false);
#else

	// Handle apple double as special case
	else if (mParts && IsApplefile())
	{
		CFullFileStream* aFile = NULL;
		CFilter* aFilter = NULL;
		CAFFilter* aAFFilter = NULL;

		// Check validity of this part
		if (mParts->size() != 2)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		CAttachment* applefile = mParts->at(0);
		CAttachment* datafile = mParts->at(1);

		if ((applefile->GetContent().GetContentType() != eContentApplication) ||
			(applefile->GetContent().GetContentSubtype() != eContentSubApplefile))
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		try
		{
			// Create full file stream
			aFile = (CFullFileStream*) CMIMESupport::GetFileStream(*this);

			// Might have been cancelled
			if (aFile)
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				aFile->OpenDataFork(fsWrPerm);
				aFile->OpenResourceFork(fsWrPerm);
#elif __dest_os == __win32_os || __dest_os == __linux_os
				// Assume opened in GetFileStream
#else
#error __dest_os
#endif

				// Do application/applefile part first

				// Get appropriate filter for decoding
				aFilter = CMIMESupport::GetFilter(applefile, true);

				// Get filter for applefile
				aAFFilter = new CAFFilter;

				// Set filters
				aFilter->SetStream(aAFFilter);
				aAFFilter->SetStream(aFile, false);
				aFilter->SetForNetwork(lendl);

				// Tell mbox to read this part
				applefile->ReadAttachment(owner, aFilter);

				applefile->SetExtracted(true);

				delete aFilter;
				aFilter = NULL;
				delete aAFFilter;
				aAFFilter = NULL;

				// Do data next

				// Get appropriate filter for encoding
				aFilter = CMIMESupport::GetFilter(datafile, true);

				// Set filters
				aFilter->SetStream(aFile);
				aFilter->SetForNetwork(lendl);

				// Tell mbox to read this part
				datafile->ReadAttachment(owner, aFilter);

				datafile->SetExtracted(true);

				SetExtracted(true);


				// Try to launch it
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				if (CAttachment::sDropLocation == NULL)
#endif
					TryLaunch(aFile);
			}

		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up and throw up

			// Delete existing file
			if (aFile)
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				PPx::FSObject spec;
				aFile->GetSpecifier(spec);
				spec.Delete();
#elif __dest_os == __win32_os
				CString fpath = aFile->GetFilePath();	// Cache path as Abort empties it
				aFile->Abort();
				CFile::Remove(fpath);
#elif __dest_os == __linux_os
				cdstring fpath = aFile->GetFilePath();
				aFile->Abort();
				JRemoveFile(fpath.c_str());
#endif
			}
			delete aFile;
			delete aFilter;
			delete aAFFilter;
			CLOG_LOGRETHROW;
			throw;
		}

		// Clean up
		delete aFile;
		delete aFilter;
		delete aAFFilter;
	}

	// Handle single part
	else
	{
		CFullFileStream* aFile = NULL;
		CFilter* aFilter = NULL;
		CAFFilter* aAFFilter = NULL;

		try
		{
			// Create full file stream
			aFile = (CFullFileStream*) CMIMESupport::GetFileStream(*this);

			// Might have been cancelled
			if (aFile)
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				if (!mContent.IsBinHexed() && !mContent.IsUUed())
					aFile->OpenDataFork(fsWrPerm);
#elif __dest_os == __win32_os || __dest_os == __linux_os
				// Already open in GetFileStream
#else
#error __dest_os
#endif
				if (IsApplefile())
				{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					aFile->OpenResourceFork(fsWrPerm);
#elif __dest_os == __win32_os || __dest_os == __linux_os
					// No resource fork for Windows or Linux or ...
#else
#error __dest_os
#endif

					aAFFilter = new CAFFilter;
				}

				// Get appropriate filter
				aFilter = CMIMESupport::GetFilter(this, true);

				// Set filter chain
				if (IsApplefile())
				{
					aFilter->SetStream(aAFFilter);
					aAFFilter->SetStream(aFile, true);
				}
				else
					aFilter->SetStream(aFile);
				aFilter->SetForNetwork(lendl);

				// Tell mbox to read this part
				ReadAttachment(owner, aFilter);

				SetExtracted(true);

				// Try to launch it
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				if (CAttachment::sDropLocation == NULL)
#endif
					TryLaunch(aFile);
			}

		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up and throw up

			// Delete existing file
			if (aFile)
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				PPx::FSObject spec;
				aFile->GetSpecifier(spec);
				spec.Delete();
#elif __dest_os == __win32_os
				CString fpath = aFile->GetFilePath();	// Cache path as Abort empties it
				aFile->Abort();
				CFile::Remove(fpath);
#elif __dest_os == __linux_os
				cdstring fpath = aFile->GetFilePath();
				aFile->Abort();
				JRemoveFile(fpath.c_str());
#else
#error __dest_os
#endif
			}
			delete aFile;
			delete aFilter;
			delete aAFFilter;
			CLOG_LOGRETHROW;
			throw;
		}

		// Clean up
		delete aFile;
		delete aFilter;
		delete aAFFilter;
	}
#endif
}

// Try to launch the decoded file
void CAttachment::TryLaunch(CFullFileStream* aFile) const
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	PPx::FSObject fspec;
	aFile->GetSpecifier(fspec);
#endif

	// Check that user wants to launch and there is something to launch
	if ((CMIMEMap::FindAppLaunch(*this) != eAppLaunchNever) &&
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		fspec.Exists() &&
#elif __dest_os == __win32_os
		aFile->GetFilePath().GetLength() &&
#elif __dest_os == __linux_os
			aFile->GetFilePath().length() &&
#else
#error __dest_os
#endif
		(!CanDisplay() || CPreferences::sPrefs->mLaunchText.GetValue()))
	{
		bool launch = true;

		// Ask user first?
		if (CMIMEMap::FindAppLaunch(*this) == eAppLaunchAsk)
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			cdstring fname(fspec.GetName());
#elif __dest_os == __win32_os || __dest_os == __linux_os
			cdstring fname = aFile->GetFileName();
#endif
			cdstring appName = CMIMESupport::MapMIMEToApp(*this);

			// If no default app name try to get it from decoded file's creator
			if (appName.empty())
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				FinderInfo finfo;
				fspec.GetFinderInfo(&finfo, NULL, NULL);
				OSType appCreator = finfo.file.fileCreator;
				appName.reserve(64);
				CDesktopIcons::GetAPPLName(appCreator, (unsigned char*) (char*) appName);
				p2cstr((unsigned char*) (char*) appName);
#endif
			}

			// Must have name
			if (!appName.empty())

				launch = (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Attachments::DownloadLaunchAsk", fname, appName.c_str(), NULL, NULL, true) == CErrorHandler::Ok);
			else
				launch = false;
		}


		if (launch)
		{
			// Close file prior to launch
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			aFile->CloseDataFork();
			aFile->CloseResourceFork();
#elif __dest_os == __win32_os
			CString fpath = aFile->GetFilePath();
			aFile->Close();
#elif __dest_os == __linux_os
			cdstring fpath = aFile->GetFilePath();
			aFile->Close();
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Try default map first
			OSType appCreator = CMIMESupport::MapMIMEToCreator(*this);

			// If not found or decoded file use file's creator
			if (!appCreator)
			{
				FinderInfo finfo;
				fspec.GetFinderInfo(&finfo, NULL, NULL);
				appCreator = finfo.file.fileCreator;
			}

            cdstring mimeType = CMIMESupport::GenerateContentHeader(this, false, lendl, false);
			CAppLaunch::OpenDocumentWithApp(&fspec, mimeType, appCreator);
#elif __dest_os == __win32_os
			TCHAR dir[MAX_PATH];
			if (::GetCurrentDirectory(MAX_PATH, dir))
			{
				// Look for a shell launch spec
				if (CPreferences::sPrefs->mShellLaunch.GetValue().length())
				{
					// Get file part of launch spec
					cdstring temp = CPreferences::sPrefs->mShellLaunch.GetValue();
					char* p = temp.c_str_mod();
					cdstring file = ::strgetquotestr(&p);

					// Insert parameter
					cdstring params;
					size_t params_reserve = ::strlen(p) + fpath.GetLength() + 1;
					params.reserve(params_reserve);
					::snprintf(params.c_str_mod(), params_reserve, p, fpath);

					::ShellExecute(*::AfxGetMainWnd(), _T("open"), file.win_str(), params.win_str(), dir, SW_SHOWNORMAL);
				}
				else
					::ShellExecute(*::AfxGetMainWnd(), _T("open"), fpath, NULL, dir, SW_SHOWNORMAL);
			}
#elif __dest_os == __linux_os
			// Get type/subtype text
			const cdstring type = CMIMESupport::GenerateContentHeader(this, false, lendl, false);

			// Get view-command from mailcap
			cdstring cmd = CMailcapMap::sMailcapMap.GetCommand(type);

			// Do file name substitution
			cdstring buf;
			size_t buf_reserve = cmd.length() + fpath.length() + 1;
			buf.reserve(buf_reserve);
			::snprintf(buf.c_str_mod(), buf_reserve, cmd.c_str(), fpath.c_str());

			// Execute the command
			pid_t childPID;
			JExecute(buf.c_str(), &childPID);
#endif
		}
	}
}

// Write attachment to stream
void CAttachment::WriteToStream(costream& stream, unsigned long& level, bool dummy_files,
								CProgress* progress, CMessage* owner, unsigned long count, unsigned long start) const
{
	// If this part has no parent then it is the first part in a message.
	// This means that the content header/encoding have already been generated and appear
	// in the message header fields. Therefore do not generate these again.

	// Does it have sub-parts?
	if (mParts)
	{
		{
			// Bump up level for each new header (not the first which must remain at 0)
			level++;

			// Send content type header (multipart)
			const_cast<CAttachment*>(this)->mContent.SetMultiLevel(level);
			cdstring content = CMIMESupport::GenerateContentHeader(this, dummy_files, stream.GetEndlType());
			if (!content.empty())
				stream.Stream() << content << stream.endl();

			// Send description
			cdstring description = CMIMESupport::GenerateContentDescription(this, stream.GetEndlType());
			if (!description.empty())
				stream.Stream() << description << stream.endl();
			
			// Send id
			cdstring id = CMIMESupport::GenerateContentId(this, stream.GetEndlType());
			if (!id.empty())
				stream.Stream() << id << stream.endl();
			
			// Send disposition
			cdstring disposition = CMIMESupport::GenerateContentDisposition(this, stream.GetEndlType());
			if (!disposition.empty())
				stream.Stream() << disposition << stream.endl();

			// Boundary between header and body
			stream.Stream() << stream.endl();
		}

		unsigned long this_level = level;

		// Get multipart boundary text
		cdstring boundary = CMIMESupport::GenerateMultipartBoundary(this, stream.GetEndlType(), this_level, true);

		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			// Send start boundary
			stream.Stream() << boundary << stream.endl();

			// Write each sub-part to stream + CRLF at end
			(*iter)->WriteToStream(stream, level, dummy_files, progress, owner, count, start);
			stream.Stream() << stream.endl();
		}

		// Send ending boundary + CRLF first
		cdstring end_boundary = CMIMESupport::GenerateMultipartBoundary(this, stream.GetEndlType(), this_level, true, true);
		stream.Stream() << end_boundary << stream.endl();
	}

	// Generate data
	else
	{
		// Only do this if not multipart/signed or multipart/encrypted
		if ((mContent.GetContentSubtype() != eContentSubSigned) &&
			(mContent.GetContentSubtype() != eContentSubEncrypted))
		{
			// Send content type header
			cdstring content = CMIMESupport::GenerateContentHeader(this, dummy_files, stream.GetEndlType());
			if (!content.empty())
				stream.Stream() << content << stream.endl();

			// Only have encoding for real file
			if (!dummy_files || (typeid(*this) == typeid(CDataAttachment)))
			{
				// Send transfer encoding header
				cdstring encoding = CMIMESupport::GenerateTransferEncodingHeader(this, stream.GetEndlType());
				if (!encoding.empty())
					stream.Stream() << encoding << stream.endl();
			}

			// Send description
			cdstring description = CMIMESupport::GenerateContentDescription(this, stream.GetEndlType());
			if (!description.empty())
				stream.Stream() << description << stream.endl();
			
			// Send id
			cdstring id = CMIMESupport::GenerateContentId(this, stream.GetEndlType());
			if (!id.empty())
				stream.Stream() << id << stream.endl();
			
			// Send disposition
			cdstring disposition = CMIMESupport::GenerateContentDisposition(this, stream.GetEndlType());
			if (!disposition.empty())
				stream.Stream() << disposition << stream.endl();

			// Boundary between header and body
			stream.Stream() << stream.endl();
		}

		// Send actual data
		WriteDataToStream(stream, dummy_files, progress, owner, count, start);
	}
}

// Write data to stream
void  CAttachment::WriteDataToStream(costream& stream, bool dummy_files, CProgress* progress, CMessage* owner,
										unsigned long count, unsigned long start) const
{
	// Bump progress counter
	if (progress)
		progress->BumpCount();

	// Only do this if cached
	if (IsNotCached())
		return;

	// If owned message read from message
	if (owner)
	{
		// Bump up to root message
		while (owner->IsSubMessage())
			owner = owner->GetOwner();

		// Tell mbox to read this part
		ReadAttachment(owner, &stream, true, count, start);
	}

	// Add message data to stream if there
	else if (mData)
	{
		LDataStream theText(mData, ::strlen(mData));
		CFilter* aFilter = CMIMESupport::GetFilter(this, false);
		aFilter->SetStream(&theText);
		aFilter->SetProgress(progress);
		aFilter->SetForNetwork(stream.GetEndlType());
		{
			char* buffer = new char[8192];
			while(!aFilter->Complete())
			{
				long read = 8192;
				aFilter->GetBytes(buffer, read);
				stream.Stream().write(buffer, read);
			}
			delete buffer;
		}
		delete aFilter;
	}
}

// Read into stream
void CAttachment::ReadAttachment(CMessage* msg, LStream* aStream, bool peek)
{
	msg->GetMbox()->ReadAttachment(msg, this, aStream, peek);
}

// Copy into stream
void CAttachment::ReadAttachment(CMessage* msg, costream* aStream, bool peek, unsigned long count, unsigned long start) const
{
	// Tell mbox to read this part
	msg->GetMbox()->CopyAttachment(msg, const_cast<CAttachment*>(this), aStream, peek, count, start);
}
