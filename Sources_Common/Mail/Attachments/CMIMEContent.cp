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


// Source for CMIMEContent class

#include "CMIMEContent.h"

#include "CAttachmentList.h"
#include "CCharsetManager.h"
#include "CMIMESupport.h"
#include "CMIMETypes.h"
#include "CRFC822.h"
#include "CStringUtils.h"

#include "cdiomanip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <istream>
#include <ostream>

// __________________________________________________________________________________________________
// S T R U C T __ C M I M E C O N T E N T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMIMEContent::CMIMEContent()
{
	InitMIMEContent();
}

// Constructor
CMIMEContent::CMIMEContent(ETransferMode mode, EContentType type, EContentSubType subtype, EContentTransferEncoding encoding)
{
	InitMIMEContent();

	mTransferMode = mode;
	mType = type;
	mSubtype = subtype;
	mEncoding = encoding;

	if (mEncoding == eNoTransferEncoding)
		SetDefaultEncoding();
}

// Constructor
CMIMEContent::CMIMEContent(unsigned long level)
{
	InitMIMEContent();

	mType = eContentMultipart;
	mSubtype = eContentSubMixed;
	mEncoding = eNoTransferEncoding;

	mMultiLevel = level;
}

CMIMEContent::~CMIMEContent()
{
	delete mContentDispositionParams;
	mContentDispositionParams = NULL;
}

// Assignment with same type
void CMIMEContent::_copy(const CMIMEContent& content)
{
	// Copy fields - duplicate strings, delete existing ones
	mTransferMode = content.mTransferMode;
	mType = content.mType;
	mXtype = content.mXtype;
	mSubtype = content.mSubtype;
	mXsubtype = content.mXsubtype;
	mContentParams = content.mContentParams;
	mCharset = content.mCharset;
	mEncoding = content.mEncoding;
	mDontEncode = content.mDontEncode;
	mContentId = content.mContentId;
	mContentDescription = content.mContentDescription;
	mContentDisposition = content.mContentDisposition;
	mContentDispositionParams = (content.mContentDispositionParams ? new cdstrpairvect(*content.mContentDispositionParams) : NULL);
	mMappedName = content.mMappedName;
	mMultiLevel = content.mMultiLevel;
	mContentSize = content.mContentSize;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Common init
void CMIMEContent::InitMIMEContent()
{
	mTransferMode = eNoTransferMode;
	mType = eNoContentType;
	mSubtype = eNoContentSubType;
	mCharset = i18n::eAutomatic;
	mEncoding = eNoTransferEncoding;
	mDontEncode = false;
	mContentDisposition = eNoContentDisposition;
	mContentDispositionParams = NULL;
	mMultiLevel = 0;
	mContentSize = 0;
}

// Do reconstruct recovery test
bool CMIMEContent::RecoverCompare(const CMIMEContent& comp) const
{
	return (mTransferMode == comp.mTransferMode) &&
			(mType == comp.mType) &&
			(mXtype == comp.mXtype) &&
			(mSubtype == comp.mSubtype) &&
			(mXsubtype == comp.mXsubtype) &&
			(mContentParams == comp.mContentParams) &&
			(mCharset == comp.mCharset) &&
			(mEncoding == comp.mEncoding) &&
			(mContentId == comp.mContentId) &&
			(mContentDescription == comp.mContentDescription) &&
			(mContentDisposition == comp.mContentDisposition) &&
			!((mContentDispositionParams != NULL) ^ (comp.mContentDispositionParams != NULL)) &&
			(!mContentDispositionParams || (*mContentDispositionParams == *comp.mContentDispositionParams)) &&
			(mMappedName == comp.mMappedName) &&
			(mMultiLevel == comp.mMultiLevel) &&
			(mContentSize == comp.mContentSize);
}

// Null out fields which are different
void CMIMEContent::NullDiff(CMIMEContent& content)
{
	if (content.GetTransferMode() != GetTransferMode())
		mTransferMode = eNoTransferMode;

	if (content.GetContentTypeText() != GetContentTypeText())
	{
		mType = eNoContentType;
		mXtype = cdstring::null_str;
	}

	if (content.GetContentSubtypeText() != GetContentSubtypeText())
	{
		mSubtype = eNoContentSubType;
		mXsubtype = cdstring::null_str;
	}

	if (content.GetCharset() != GetCharset())
		mCharset = i18n::eUnknown;

	if (content.GetTransferEncoding() != GetTransferEncoding())
		mEncoding = eNoTransferEncoding;

	if (content.GetContentDescription() != GetContentDescription())
		mContentDescription = cdstring::null_str;

	if (content.GetContentDisposition() != GetContentDisposition())
		mContentDisposition = eNoContentDisposition;

	if (content.GetMappedName() != GetMappedName())
		mMappedName = cdstring::null_str;
}

// Copy from fields which are not empty
void CMIMEContent::NullAdd(CMIMEContent& content)
{
	// Do transfer mode first as this may change type & subtype
	ETransferMode mode = GetTransferMode();
	if (mode != eNoTransferMode)
		content.SetTransferMode(mode);

	switch(mode)
	{
	case eNoTransferMode:
	case eTextMode:
	case eMIMEMode:
		// Change to user specified type/subtype
		if ((GetContentType() != eNoContentType) ||
			(GetContentSubtype() != eNoContentSubType))
		{
			cdstring type_subtype = GetContentTypeText() + "/" + GetContentSubtypeText();
			content.SetContent(type_subtype);
		}
		break;
	default:;
		// Other modes always set their own content type/subtype
	}

	if (GetCharset() != i18n::eUnknown)
		content.SetCharset(GetCharset());

	if (GetTransferEncoding() != eNoTransferEncoding)
		content.SetTransferEncoding(GetTransferEncoding());

	if (!GetContentDescription().empty())
		content.SetContentDescription(GetContentDescription());

	if (GetContentDisposition() != eNoContentDisposition)
		content.SetContentDisposition(GetContentDisposition());

	if (!GetMappedName().empty())
		content.SetMappedName(GetMappedName());
}

void CMIMEContent::SetTransferMode(ETransferMode mode)
{
	mTransferMode = mode;

	// Must reset content for particular types
	switch(mTransferMode)
	{
	case eTextMode:
		SetContent(eContentText, eContentSubPlain);
		mEncoding = e7bitEncoding;
		break;

	case eMIMEMode:
	default:
		// Do NOT do anything here - disaster could ensue!
		break;

	case eBinHex4Mode:
		SetContent(eContentApplication, eContentSubMacBinhex4);
		mEncoding = e7bitEncoding;
		break;

	case eUUMode:
		SetContent(eContentApplication, eContentSubOctetStream);
		mEncoding = e7bitEncoding;
		break;

	case eAppleSingleMode:
	case eAppleDoubleMode:
		SetContent(eContentApplication, eContentSubApplefile);
		mEncoding = eBase64Encoding;
		break;

	}

	// Reset just in case SetContent changed it
	mTransferMode = mode;

}

void CMIMEContent::ConvertDummy()
{
	// Get type parameter
	for(cdstrpairvect::iterator iter = mContentParams.begin(); iter != mContentParams.end(); )
	{
		if (!::strcmpnocase((*iter).first, cMIMEParameter[eMulberryType]))
		{
			SetContentType((*iter).second);
			iter = mContentParams.erase(iter);
			continue;
		}
		else if (!::strcmpnocase((*iter).first, cMIMEParameter[eMulberrySubtype]))
		{
			SetContentSubtype((*iter).second);
			iter = mContentParams.erase(iter);
			continue;
		}
		else if (!::strcmpnocase((*iter).first, cMIMEParameter[eMulberryFile]))
		{
			iter = mContentParams.erase(iter);
			continue;
		}
		
		iter++;
	}
}

bool CMIMEContent::IsDummy() const
{
	return (GetContentType() == eContentXMulberry) &&
			(GetContentSubtype() == eContentXSubMulberryFile);
}

#pragma mark ____________________________type & subtype

// Set with type and subtype
void CMIMEContent::SetContent(EContentType type, EContentSubType subtype)
{
	// Get type without string
	mType = type;
	mXtype = cdstring::null_str;

	// Get subtype without string
	mSubtype = subtype;
	mXsubtype = cdstring::null_str;

	// Reset encoding
	SetDefaultEncoding();
}

// Set with type string and subtype
void CMIMEContent::SetContent(const char* xtype, EContentSubType subtype)
{
	if (xtype && *xtype)
	{
		// Get type with string
		mType = eContentXtoken;

		// Copy xtype string as lowercase
		mXtype = xtype;
		::strlower(mXtype.c_str_mod());
	}
	else
	{
		mType = eNoContentType;
		mXtype = cdstring::null_str;
	}

	// Get subtype without string
	mSubtype = subtype;
	mXsubtype = cdstring::null_str;

	// Reset encoding
	SetDefaultEncoding();
}

// Set with type and subtype string
void CMIMEContent::SetContent(EContentType type, const char* xsubtype)
{
	// Get type without string
	mType = type;
	mXtype = cdstring::null_str;

	if (xsubtype && *xsubtype)
	{
		// Get subtype with string
		mSubtype = eContentSubXtoken;

		// Copy xtype string as lowercase
		mXsubtype = xsubtype;
		::strlower(mXsubtype.c_str_mod());
	}
	else
	{
		mSubtype = eNoContentSubType;
		mXsubtype = cdstring::null_str;
	}

	// Reset encoding
	SetDefaultEncoding();
}

// Set with strings for type and subtype
void CMIMEContent::SetContent(const char* xtype, const char* xsubtype)
{
	if (xtype && *xtype)
	{
		// Get type with string
		mType = eContentXtoken;

		// Copy xtype string as lowercase
		mXtype = xtype;
		::strlower(mXtype.c_str_mod());
	}
	else
	{
		mType = eNoContentType;
		mXtype = cdstring::null_str;
	}

	if (xsubtype && *xsubtype)
	{
		// Get subtype with string
		mSubtype = eContentSubXtoken;

		// Copy xtype string as lower
		mXsubtype = xsubtype;
		::strlower(mXsubtype.c_str_mod());
	}
	else
	{
		mSubtype = eNoContentSubType;
		mXsubtype = cdstring::null_str;
	}

	// Reset encoding
	SetDefaultEncoding();
}

// Set from mime type/subtype description
void CMIMEContent::SetContent(const unsigned char* mime)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	p2cstr((unsigned char*) mime);
	SetContent((char*) mime);
	c2pstr((char*) mime);
#endif
}

// Set from mime type/subtype description
void CMIMEContent::SetContent(const char* mime)
{
	// Make local c-string copy
	char* txt = ::strdup(mime);

	// Get first token and convert to lower
	char* type_token = txt ? ::strtok(txt, " /") : NULL;
	if (!type_token || !*type_token)
	{
		SetContent(eNoContentType, eNoContentSubType);
		return;
	}
	::strlower(type_token);

	// Determine content type
	EContentType type = eNoContentType;
	short i = 1;
	while(i < eContentXtoken)
	{
		if (::strcmp(type_token, cMIMEContentTypes[i]) == 0)
			break;

		i++;
	}
	type = (EContentType) i;

	// Get subtype
	EContentSubType subtype = eNoContentSubType;
	char* subtype_token = ::strtok(NULL, " /");
	if (subtype_token)
	{
		::strlower(subtype_token);

		// Determine content subtype
		i = 1;
		while(i < eContentSubXtoken)
		{
			if (::strcmp(subtype_token, cMIMEContentSubTypes[i]) == 0)
				break;

			i++;
		}
		subtype = (EContentSubType) i;
	}

	// Set it based on what is defined
	if ((type != eContentXtoken) && (subtype != eContentSubXtoken))
		SetContent(type, subtype);
	else if ((type == eContentXtoken) && (subtype != eContentSubXtoken))
		SetContent(type_token, subtype);
	else if ((type != eContentXtoken) && (subtype == eContentSubXtoken))
		SetContent(type, subtype_token);
	else
		SetContent(type_token, subtype_token);

	delete txt;
}

#pragma mark ____________________________type

// Set specific value from text
void CMIMEContent::SetContentType(const char* type_txt)
{
	// Determine content type
	mType = eNoContentType;
	mXtype = cdstring::null_str;
	if (type_txt && *type_txt)
	{
		short i = 1;
		while(i < eContentXtoken)
		{
			if (::strcmpnocase(type_txt, cMIMEContentTypes[i]) == 0)
				break;

			i++;
		}

		// Set new values (duplicate string as lowercase if X-token)
		mType = (EContentType) i;

		if (mType == eContentXtoken)
		{
			mXtype = type_txt;
			::strlower(mXtype.c_str_mod());
		}
	}

	// Reset encoding
	SetDefaultEncoding();
}

// Set with type
void CMIMEContent::SetContentType(EContentType type)
{
	// Get type without string
	mType = type;
	mXtype = cdstring::null_str;
}

// Depends on mode
EContentType CMIMEContent::GetContentType() const
{
	switch(mTransferMode)
	{
		// Force to binhex content type if that mode
		case eBinHex4Mode:
			return eContentApplication;

		default:
			return mType;
	}
}

// Get type based on current content
cdstring CMIMEContent::GetContentTypeText() const
{
	// Get in type
	if (mType != eContentXtoken)
		return cMIMEContentTypes[mType];
	else
		return mXtype;
}

#pragma mark ____________________________subtype

// Set specifc value from text
void CMIMEContent::SetContentSubtype(const char* subtype_txt)
{
	// Determine content subtype
	mSubtype = eNoContentSubType;
	mXsubtype = cdstring::null_str;

	if (subtype_txt && *subtype_txt)
	{
		short i = 1;
		while(i < eContentSubXtoken)
		{
			if (::strcmpnocase(subtype_txt, cMIMEContentSubTypes[i]) == 0)
				break;

			i++;
		}

		// Set new values (duplicate string as lowercase if X-token)
		mSubtype = (EContentSubType) i;

		if (mSubtype == eContentSubXtoken)
		{
			mXsubtype = subtype_txt;
			::strlower(mXsubtype.c_str_mod());
		}
	}

	// Reset encoding
	SetDefaultEncoding();
}

// Set with subtype
void CMIMEContent::SetContentSubtype(EContentSubType subtype)
{
	// Get subtype without string
	mSubtype = subtype;
	mXsubtype = cdstring::null_str;
}

// Depends on mode
EContentSubType CMIMEContent::GetContentSubtype() const
{
	switch(mTransferMode)
	{
		// Force to binhex content subtype if that mode
		case eBinHex4Mode:
			return eContentSubMacBinhex4;

		default:
			return mSubtype;
	}
}

// Get subtype based on current content
cdstring CMIMEContent::GetContentSubtypeText() const
{
	// Get in subtype
	if (mSubtype != eContentSubXtoken)
		return cMIMEContentSubTypes[mSubtype];
	else
		return mXsubtype;
}

#pragma mark ____________________________parameter

// Copy parameter text (c-string)
void CMIMEContent::SetContentParameter(const char* param1, const char* param2)
{
	// Only process if something there
	if (!param1 || !param2)
		return;

	// Copy into list - but lowercase param2
	mContentParams.push_back(cdstrpair(param1, param2));

	// Look for comparisons
	cdstring name(param1);
	cdstring txt2231;
	bool is2231 = CMIMESupport::SplitParamName(name, txt2231);
	if (name.compare(cMIMEParameter[eCharset], true) == 0)
	{
		mCharset = i18n::CCharsetManager::sCharsetManager.GetCodeFromName(param2);
	}
	else if (name.compare(cMIMEParameter[eName], true) == 0)
	{
		cdstring name_param(param2);
		if (is2231)
		{
			// Do 2231 decode of parameter value
			CRFC822::TextFrom2231(name_param);
		}
		else
		{
			// Do 1522 decode of parameter value to cope with MS bug
			CRFC822::TextFrom1522(name_param);
		}
		SetMappedName(name_param);
	}
	else
		// Ignore others for now
		;
}

// Remove a parameter
void CMIMEContent::RemoveContentParameter(const char* param)
{
	// Only process if something there
	if (!param || !*param)
		return;

	for(cdstrpairvect::iterator iter = mContentParams.begin(); iter != mContentParams.end(); )
	{
		if (!::strcmpnocase(iter->first, param))
		{
			mContentParams.erase(iter);
			continue;
		}
		
		iter++;
	}
}

// Copy parameter text (with
cdstring CMIMEContent::GetContentParameterText(short index) const
{
	// Check for quoting
	return mContentParams[index].first + "=" + CMIMESupport::GenerateContentParameter(mContentParams[index].second);
}


// Get parameter by name if present
const cdstring&	CMIMEContent::GetContentParameter(const char* param) const
{
	for(cdstrpairvect::const_iterator iter = mContentParams.begin(); iter != mContentParams.end(); iter++)
	{
		if (!::strcmpnocase((*iter).first, param))
			return (*iter).second;
	}

	return cdstring::null_str;
}

// Get boundary parameter
const cdstring& CMIMEContent::GetMultipartBoundary() const
{
	return GetContentParameter(cMIMEParameter[eBoundary]);
}

// Get boundary parameter
const cdstring& CMIMEContent::GetXMulberryFile() const
{
	return GetContentParameter(cMIMEParameter[eMulberryFile]);
}

#pragma mark ____________________________transfer encoding

// Set specifc value from text
void CMIMEContent::SetTransferEncoding(const char* encoding_txt)
{
	// Check for valid encoding string
	if (!encoding_txt || !*encoding_txt)
	{
		mEncoding = e7bitEncoding;
		mTransferMode = eMIMEMode;
		return;
	}

	// Determine content encoding
	short i = 1;
	while(i < eXtokenEncoding)
	{
		if (::strcmpnocase(encoding_txt, cMIMETransferEncodings[i]) == 0)
			break;

		i++;
	}

	// Set new value
	mEncoding = (EContentTransferEncoding) i;
	mTransferMode = eMIMEMode;

}

// Just return encoding
EContentTransferEncoding CMIMEContent::GetTransferEncoding() const
{
	return mEncoding;
}

// Get transfer encoding based on current content
cdstring CMIMEContent::GetTransferEncodingText() const
{
	// Get in type
	if ((mEncoding != eNoTransferEncoding) && (mEncoding != eXtokenEncoding))
		return cMIMETransferEncodings[mEncoding];
	else
		return cdstring::null_str;

}

#pragma mark ____________________________disposition

// Set specifc value from text
void CMIMEContent::SetContentDisposition(const char* disposition)
{
	// Only process if something there
	if (!disposition)
		return;

	// Determine content encoding
	short i = 0;
	while(i < eXtokenDisposition)
	{
		if (::strcmpnocase(disposition, cMIMEDisposition[i]) == 0)
			break;

		i++;
	}

	// Set new value
	mContentDisposition = (EContentDisposition) i;
}

// Get disposition
cdstring CMIMEContent::GetContentDispositionText() const
{
	// Get disposition
	if (mContentDisposition != eXtokenDisposition)
		return cMIMEDisposition[mContentDisposition];
	else
		return cdstring::null_str;

}

// Copy parameter text (c-string)
void CMIMEContent::SetContentDispositionParameter(const char* param1, const char* param2)
{
	// Only process if something there
	if (!param1 || !param2)
		return;

	// Create list if required
	if (!mContentDispositionParams)
		mContentDispositionParams = new cdstrpairvect;

	// Copy into list
	mContentDispositionParams->push_back(cdstrpair(param1, param2));

	// Look for comparisons
	cdstring name(param1);
	cdstring txt2231;
	bool is2231 = CMIMESupport::SplitParamName(name, txt2231);
	if (name.compare(cMIMEDispositionParameter[eContentDispositionFilename], true) == 0)
	{
		cdstring name_param(param2);
		if (is2231)
		{
			// Do 2231 decode of parameter value
			CRFC822::TextFrom2231(name_param);
		}
		else
		{
			// Do 1522 decode of parameter value to cope with MS bug
			CRFC822::TextFrom1522(name_param);
		}
		SetMappedName(name_param);
	}
	else
		// Ignore others for now
		;
}


#pragma mark ____________________________size

// Copy from string
void CMIMEContent::SetContentSize(const char* size)
{
	if (size)
		mContentSize = ::atol(size);
	else
		mContentSize = 0;
}

#pragma mark ____________________________others

// Check for a binhexed part
bool	CMIMEContent::IsBinHexed() const
{
	return ((GetTransferMode() == eBinHex4Mode) ||
			(GetContentSubtype() == eContentSubMacBinhex4));
}

// Check for a UUed part
bool CMIMEContent::IsUUed() const
{
	return (GetTransferMode() == eUUMode);
}

// Is it 'format=flowed'?
bool CMIMEContent::IsFlowed() const
{
	for(cdstrpairvect::const_iterator iter = mContentParams.begin(); iter != mContentParams.end(); iter++)
	{
		if (!::strcmpnocase((*iter).first, cMIMEParameter[eFormat]))
			return !::strcmpnocase((*iter).second, cMIMEParameter[eFlowed]);
	}

	return false;
}

// Is it 'delsp=yes'?
bool CMIMEContent::IsDelsp() const
{
	for(cdstrpairvect::const_iterator iter = mContentParams.begin(); iter != mContentParams.end(); iter++)
	{
		if (!::strcmpnocase((*iter).first, cMIMEParameter[eDelsp]))
			return !::strcmpnocase((*iter).second, cMIMEParameter[eDelspYes]);
	}

	return false;
}

// Is text content flowed?
bool CMIMEContent::IsTextFlowed() const
{
	// Enriched and HTML always flowed
	switch (mSubtype)
	{
	case eContentSubPlain:
	default:
		// Check for format=flowed parameter
		return IsFlowed();
	case eContentSubEnriched:
	case eContentSubHTML:
		return true;
	}
}

// Common init
void CMIMEContent::SetDefaultEncoding()
{
	switch (mType)
	{
	case eContentText:
	case eContentMessage:
		mEncoding = e7bitEncoding;
		mTransferMode = eTextMode;
		break;

	case eContentMultipart:
	case eNoContentType:
	default:
		mEncoding = eNoTransferEncoding;
		mTransferMode = eNoTransferMode;
		break;

	case eContentApplication:
		switch(mSubtype)
		{
		case eContentSubMacBinhex4:
			mEncoding = e7bitEncoding;
			mTransferMode = eBinHex4Mode;
			break;

		case eContentSubApplefile:
			mEncoding = eBase64Encoding;
			mTransferMode = eAppleSingleMode;
			break;

		case eContentSubPGPSigned:
		case eContentSubPGPEncrypted:
		case eContentSubPGPKeys:
			mEncoding = e7bitEncoding;
			mTransferMode = eTextMode;
			break;

		default:
			mEncoding = eBase64Encoding;
			mTransferMode = eMIMEMode;
			break;

		}
		break;

	case eContentImage:
	case eContentAudio:
	case eContentVideo:
	case eContentXtoken:
		mEncoding = eBase64Encoding;
		mTransferMode = eMIMEMode;
		break;
	}
}


void CMIMEContent::WriteCacheToStream(std::ostream& out) const
{

	out << GetContentTypeText() << cd_endl;
	out << GetContentSubtypeText() << cd_endl;
	out << CountContentParameters() << cd_endl;
	for(int i = 0; i < CountContentParameters() ; i++)
	{
		out << mContentParams[i].first << cd_endl;
		out << mContentParams[i].second << cd_endl;
	}
	out << (long) mCharset << cd_endl;
	out << GetTransferEncodingText() << cd_endl;
	out << GetContentId() << cd_endl;
	out << GetContentDescription() << cd_endl;
	out << GetContentDispositionText() << cd_endl;
	out << (mContentDispositionParams ? mContentDispositionParams->size() : 0) << cd_endl;
	if (mContentDispositionParams)
	{
		for(unsigned long i = 0; i < mContentDispositionParams->size() ; i++)
		{
			out << (*mContentDispositionParams)[i].first << cd_endl;
			out << (*mContentDispositionParams)[i].second << cd_endl;
		}
	}
}

void CMIMEContent::ReadCacheFromStream(std::istream& in, unsigned long vers)
{
	cdstring temp;
	getline(in, temp);
	SetContentType(temp);
	getline(in, temp);
	SetContentSubtype(temp);

	int num = 0;
	in >> num;
	in.ignore();
	for(int i = 0; i < num ; i++)
	{
		cdstring param1;
		cdstring param2;
		getline(in, param1);
		getline(in, param2);
		SetContentParameter(param1, param2);
	}

	long templ;
	in >> templ;
	mCharset = (i18n::ECharsetCode) templ;
	in.ignore();
	getline(in, temp);
	SetTransferEncoding(temp);
	getline(in, temp);
	SetContentId(temp);
	getline(in, temp);
	SetContentDescription(temp);
	getline(in, temp);
	SetContentDisposition(temp);

	num = 0;
	in >> num;
	in.ignore();
	for(int i = 0; i < num ; i++)
	{
		cdstring param1;
		cdstring param2;
		getline(in, param1);
		getline(in, param2);
		SetContentDispositionParameter(param1, param2);
	}
}
