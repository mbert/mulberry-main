/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CMIMESupport class

#include "CMIMESupport.h"

#include "CAttachmentList.h"
#include "CBinHexFilter.h"
#include "CCharsetManager.h"
#include "CErrorHandler.h"
#include "CFileAttachment.h"
#include "CFilter.h"
#include "CFullFileStream.h"
#include "CGeneralException.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CICSupport.h"
#endif
#include "CLocalCommon.h"
#include "CMessage.h"
#include "CMIMEContent.h"
#include "CMIMEFilters.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CStringUtils.h"
#include "CUUFilter.h"
#if __dest_os == __win32_os
#include "CSDIFrame.h"
#include "CWinRegistry.h"
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "MyCFString.h"
#include <UStandardDialogs.h>
#endif


#if __dest_os == __linux_os
#include <jGlobals.h>
#include <jFileUtil.h>
#include <jDirUtil.h>
#include "CMailcapMap.h"
#include "CMIMETypesMap.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strstream>

#include <typeinfo>
// __________________________________________________________________________________________________
// C L A S S __ C M I M E S U P P O R T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMIMESupport::CMIMESupport()
{
}

// Default destructor
CMIMESupport::~CMIMESupport()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CMIMESupport::AddParameter(std::ostream& out, const cdstring& name, const cdstring& value, bool value_ok, long& line_length, EEndl for_endl)
{
	// Check for 2231 encoding
	if (CRFC822::Needs2231(value))
	{
		cdstring evalue(value);
		CRFC822::TextTo2231(evalue);

		out << ";";
		line_length++;
		if (line_length + 1 + name.length() + 2 + evalue.length() > cRFC822Wrap)
		{
			out << ::get_endl(for_endl);
			line_length = 0;
		}
		
		out << " " << name << "*=" << evalue;
		line_length += 1 + name.length() + 2 + evalue.length();
	}
	else
	{
		cdstring evalue(value_ok ? value : GenerateContentParameter(value));
		out << ";";
		line_length++;
		if (line_length + 1 + name.length() + 1 + evalue.length() > cRFC822Wrap)
		{
			out << ::get_endl(for_endl);
			line_length = 0;
		}
		
		out << " " << name << "=" << evalue;
		line_length += 1 + name.length() + 1 + evalue.length();
	}
}

// Split parameter according to 2231 style
bool CMIMESupport::SplitParamName(cdstring& name, cdstring& txt2231)
{
	cdstring::size_type pos = name.find('*');
	if (name.find('*') != cdstring::npos)
	{
		txt2231.assign(name, pos + 1);
		name.erase(pos);
		return true;
	}
	else
		return false;
}

// Generate header for attachment
cdstring CMIMESupport::GenerateContentHeader(const CAttachment* attach, bool dummy_files, EEndl for_endl, bool description)
{
	// Determine whether to use fake Content-Type
	if (dummy_files)
		dummy_files = (typeid(*attach) == typeid(CFileAttachment));

	const CMIMEContent& content = attach->GetContent();

	// If no type send back empty string
	if (content.GetContentType() == eNoContentType)
		return cdstring::null_str;

	std::ostrstream out;

	// Copy in header line
	if (description)
		out << cHDR_MIME_TYPE;

	// Copy in type
	out << (dummy_files ? cMIMEContentTypes[eContentXMulberry] : content.GetContentTypeText().c_str());

	// Copy in subtype
	out << '/';
	out << (dummy_files ? cMIMEContentSubTypes[eContentXSubMulberryFile] : content.GetContentSubtypeText().c_str());

	long line_length = out.pcount();

	// Copy in parameter if required
	if (description)
	{
		// Add original type/subtype
		if (dummy_files)
		{
			AddParameter(out, cMIMEParameter[eMulberryType], content.GetContentTypeText(), true, line_length, for_endl);

			AddParameter(out, cMIMEParameter[eMulberrySubtype], content.GetContentSubtypeText(), true, line_length, for_endl);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			cdstring temp(static_cast<const CFileAttachment*>(attach)->GetFSSpec()->GetPath());
#else
			cdstring temp(static_cast<const CFileAttachment*>(attach)->GetFilePath());
#endif
			AddParameter(out, cMIMEParameter[eMulberryFile], temp, false, line_length, for_endl);
		}

		// Add charset for text types
		if (content.GetContentType() == eContentText)
		{
			// Get charset - must be valid
			if (content.GetCharset() != i18n::eUnknown)
			{
				AddParameter(out, cMIMEParameter[eCharset], i18n::CCharsetManager::sCharsetManager.GetNameFromCode(content.GetCharset()), true, line_length, for_endl);
			}
		}

		// If name send it
		if (!content.GetMappedName().empty())
		{
			AddParameter(out, cMIMEParameter[eName], content.GetMappedName(), false, line_length, for_endl);
		}

		// Send any other parameters
		unsigned long param = 0;
		for(cdstrpairvect::const_iterator iter = content.GetContentParameters().begin(); iter != content.GetContentParameters().end(); iter++, param++)
		{
			// Must not do name, boundary or charset
			if (::strcmpnocase((*iter).first, cMIMEParameter[eName]) &&
				::strcmpnocase((*iter).first, cMIMEParameter[eCharset]) &&
				::strcmpnocase((*iter).first, cMIMEParameter[eBoundary]))
			{
				AddParameter(out, (*iter).first, (*iter).second, false, line_length, for_endl);
			}
		}

		// Generate boundary
		if (content.GetContentType() == eContentMultipart)
		{
			// Form parameter text
			AddParameter(out, cMIMEParameter[eBoundary], GenerateMultipartBoundary(attach, for_endl, content.GetMultiLevel()), true, line_length, for_endl);
		}
	}

	// Now generate string
	out << std::ends;
	const char* p = out.str();
	out.freeze(false);
	return p;
}

// Generate header for attachment
cdstring CMIMESupport::GenerateTransferEncodingHeader(const CAttachment* attach, EEndl for_endl, bool description)
{
	const CMIMEContent& content = attach->GetContent();
	cdstring txt;

	// Default encoding does not need to be specified
	if ((content.GetTransferEncoding() == eNoTransferEncoding) ||
		(content.GetTransferEncoding() == eXtokenEncoding))
		return txt;

	// Copy in header line
	if (description)
		txt += cHDR_MIME_ENCODING;
	txt += content.GetTransferEncodingText();

	// Now generate string
	return txt;
}

// Generate header for attachment
cdstring CMIMESupport::GenerateContentDescription(const CAttachment* attach, EEndl for_endl, bool description)
{
	const CMIMEContent& content = attach->GetContent();
	
	// Look for description
	if (content.GetContentDescription().empty())
		return cdstring::null_str;
	else
	{
		cdstring txt;
		if (description)
			txt += cHDR_MIME_DESCRIPTION;
		txt += GenerateContentParameter(content.GetContentDescription());
		return txt;
	}
}

// Generate header for attachment
cdstring CMIMESupport::GenerateContentId(const CAttachment* attach, EEndl for_endl, bool description)
{
	const CMIMEContent& content = attach->GetContent();
	
	// Look for description
	if (content.GetContentId().empty())
		return cdstring::null_str;
	else
	{
		cdstring txt;
		if (description)
			txt += cHDR_MIME_ID;
		txt += content.GetContentId();
		return txt;
	}
}

// Generate header for attachment
cdstring CMIMESupport::GenerateContentDisposition(const CAttachment* attach, EEndl for_endl, bool description)
{
	const CMIMEContent& content = attach->GetContent();

	// Look for description
	if (content.GetContentDisposition() == eNoContentDisposition)
		return cdstring::null_str;
	else
	{
		std::ostrstream out;
		if (description)
			out << cHDR_MIME_DISPOSITION;
		out << content.GetContentDispositionText();

		// If name send it
		if (description)
		{
			long line_length = out.pcount();

			// Generate filename= parameter
			if (!content.GetMappedName().empty())
			{
				AddParameter(out, cMIMEDispositionParameter[eContentDispositionFilename], content.GetMappedName(), false, line_length, for_endl);
			}

			// Generate size= parameter
			if (content.GetContentSize() != 0)
				AddParameter(out, cMIMEDispositionParameter[eContentDispositionSize], cdstring(content.GetContentSize()), true, line_length, for_endl);
		}

		out << std::ends;
		const char* p = out.str();
		out.freeze(false);
		return p;
	}
}

// Generarate MIME value
cdstring CMIMESupport::GenerateContentParameter(const cdstring& param)
{
	cdstring temp = param;
	CRFC822::HeaderQuote(temp, false, false, true);
	return temp;
}

// Determine main content type for message
CMIMEContent* CMIMESupport::GetMainContent(const CMessage* theMsg)
{
	CMIMEContent* content = new CMIMEContent(theMsg->GetBody()->GetContent());

	return content;
}

// Generate string boundary
cdstring CMIMESupport::GenerateMultipartBoundary(const CAttachment* attach, EEndl for_endl, unsigned long level, bool start, bool stop)
{
	cdstring txt;

	if (!start && !stop)
	{
		// Check for existing boundary and use again
		if (attach->GetData())
		{
			txt = attach->GetData();
		}
		else
		{
			// Create boundary
			txt += CONTENT_MULTIPART_BOUNDARY;

			// Boundary generate
			{
				// Generate random string
				cdstring rstr;
				rstr.reserve(256);
#if __dest_os == __linux_os
				::snprintf(rstr.c_str_mod(), 256, "%ld%ld%d%ld%p", clock(), time(NULL), rand(), level, attach);
#else
				::snprintf(rstr.c_str_mod(), 256, "%f%ld%ld%d%p", clock(), time(NULL), rand(), level, attach);
#endif
				cdstring boundary;
				rstr.md5(boundary);
				boundary[(cdstring::size_type)20] = 0;
				txt += boundary;
			}
			txt += CONTENT_MULTIPART_BOUNDARY;

			// Cache in attachment
			const_cast<CAttachment*>(attach)->SetData(::strdup(txt));
		}

		// Return quoted
		txt.quote(true);
		return txt;
	}
	else if (!stop)
	{
		// Return top of stack
		txt = "--";
		txt += attach->GetData();
		return txt;
	}
	else
	{
		// Return top of stack and pop it
		txt = "--";
		txt += attach->GetData();
		txt += "--";
		return txt;
	}
}

// Redo boundary in header of message/rfc822
void  CMIMESupport::RedoBoundary(const CAttachment* attach, cdstring& hdr, unsigned long level)
{
	// Generate new header
	const_cast<CAttachment*>(attach)->GetContent().SetMultiLevel(level);
	cdstring content = CMIMESupport::GenerateContentHeader(attach, false, eEndl_Auto);
	cdstring transfer = CMIMESupport::GenerateTransferEncodingHeader(attach, eEndl_Auto);
	cdstring description = CMIMESupport::GenerateContentDescription(attach, eEndl_Auto);

	// For empty header copy in content
	if (hdr.empty())
	{
		hdr = content + os_endl2;
		return;
	}

	// Find 'Content-Type:' in header
	const char* p = hdr.c_str();
	while(*p && (::strncmpnocase(p, cHDR_MIME_TYPE, sizeof(cHDR_MIME_TYPE) - 1) != 0))
	{
		// Step up to end of line
		while(*p && (*p != os_endl[0])) p++;

		// Bump past line end
		if (*p) p++;
		if ((os_endl_len == 2) && *p) p++;
	}
	if (*p)
	{
		// Copy first part of header and add new boundary;
		cdstring start(hdr, 0, p - (char*) hdr);
		start += content;

		// Step to end of content line
		while(*p)
		{
			if (*p == 0)
				break;
			else if (*p == os_endl[0])
			{
				// Snoop for folded lines
				if ((p[os_endl_len] == ' ') || (p[os_endl_len] == '\t'))
				{
					p += 2;
					continue;
				}
				else
					break;
			}
			else
				p++;
		}

		// Copy last part of header
		cdstring last(p);

		// Put it all together
		hdr = start + last;
	}
	else
	{
		// No existing content header - why?
		// Find start
		p = ::strstr(hdr.c_str(), os_endl2);
		if (p)
		{
			p += os_endl_len;

			cdstring start(hdr, 0, p - (char*) hdr);
			start += content;

			// Put it all together
			hdr = start + os_endl2;
		}
	}

	// Find 'Content-Transfer-Encoding:' in header
	p = hdr.c_str();
	while(*p && (::strncmpnocase(p, cHDR_MIME_ENCODING, sizeof(cHDR_MIME_ENCODING) - 1) != 0))
	{
		// Step up to end of line
		while(*p && (*p != os_endl[0])) p++;

		// Bump past line end
		if (*p) p++;
		if ((os_endl_len == 2) && *p) p++;
	}
	if (*p)
	{
		// Copy first part of header and add new boundary;
		cdstring start(hdr, 0, p - (char*) hdr);
		start += transfer;

		// Step to end of content line
		while(*p && (*p != os_endl[0]))
			p++;

		// Copy last part of header
		cdstring last(p);

		// Put it all together
		hdr = start + last;
	}
	else
	{
		// No existing content header - why?
		// Find start
		p = ::strstr(hdr.c_str(), os_endl2);
		if (p)
		{
			p += os_endl_len;

			cdstring start(hdr, 0, p - (char*) hdr);
			start += transfer;

			// Put it all together
			hdr = start + os_endl2;
		}
	}

	// Find 'Content-Description:' in header
	p = hdr.c_str();
	while(*p && (::strncmpnocase(p, cHDR_MIME_DESCRIPTION, sizeof(cHDR_MIME_DESCRIPTION) - 1) != 0))
	{
		// Step up to end of line
		while(*p && (*p != os_endl[0])) p++;

		// Bump past line end
		if (*p) p++;
		if ((os_endl_len == 2) && *p) p++;
	}
	if (*p)
	{
		// Copy first part of header and add new boundary;
		cdstring start(hdr, 0, p - (char*) hdr);
		start += description;

		// Step to end of content line
		while(*p && (*p != os_endl[0]))
			p++;

		// Copy last part of header
		cdstring last(p);

		// Put it all together
		hdr = start + last;
	}
	else if (description.length())
	{
		// No existing content description
		// Find start
		p = ::strstr(hdr.c_str(), os_endl2);
		if (p)
		{
			p += os_endl_len;

			cdstring start(hdr, 0, p - (char*) hdr);
			start += description;

			// Put it all together
			hdr = start + os_endl2;
		}
	}
}

// Determine best encoding type for text
EContentTransferEncoding CMIMESupport::DetermineTextEncoding(LFileStream& text, EContentDisposition disposition)
{
	const long cBufferSize = 8192;
	cdstring cbuffer;
	cbuffer.reserve(cBufferSize);
	unsigned char* buffer = reinterpret_cast<unsigned char*>(cbuffer.c_str_mod());
	bool more = true;
	EContentTransferEncoding encoding = e7bitEncoding;

	try
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Open the file
		text.OpenDataFork(fsRdPerm);
#else
		// Others - stream already open
#endif

		// Read in all data and test for 8-bit characters and line-wrap exceed
		unsigned long prefs_wrap = CPreferences::sPrefs->wrap_length.GetValue();

		// Make wrap safe
		if (prefs_wrap == 0)
			prefs_wrap = 1000;

		unsigned long line_length = 0;
		do
		{
			ExceptionCode ex;
			SInt32 bytes_read;

			// Read in bytes
			bytes_read = cBufferSize;
			ex = text.GetBytes(buffer, bytes_read);
			if (ex && (ex != readErr) && (ex != eofErr))
			{
				CLOG_LOGTHROW(CGeneralException, ex);
				throw CGeneralException(ex);
			}
			else if (ex || !bytes_read)
				more = false;

			// Test all bytes
			unsigned char* p = buffer;
			while(bytes_read--)
			{
				unsigned char c = *p++;
				bool qp_encoding = false;

				// If at least one 8-bit char found then use QP encoding
				if (c > 0x7F)
					qp_encoding = true;
				
				// Check line wrap for attachments and force qp if wrap exceed
				else if (disposition == eContentDispositionAttachment)
				{
					switch(c)
					{
					case '\r':
					case '\n':
						line_length = 0;
						break;
					default:
						if (++line_length >= prefs_wrap)
							qp_encoding = true;
						break;
					}
				}

				// Force qp encoding if required - no need to test anymore
				if (qp_encoding)
					encoding = eQuotedPrintableEncoding;
			}
		} while (more);

		// Close the file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		text.CloseDataFork();
#elif __dest_os == __win32_os || __dest_os == __linux_os
		text.Close();
#else
#error __dest_os
#endif
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);


		// Close the file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		text.CloseDataFork();
#elif __dest_os == __win32_os || __dest_os == __linux_os
		text.Abort();
#else
#error __dest_os
#endif
		CLOG_LOGRETHROW;
		throw;
	}

	return encoding;
}

// Determine best encoding type for text
EContentTransferEncoding CMIMESupport::DetermineTextEncoding(const char* text)
{
	if (text)
	{
		bool qp_encoding = false;
		while(*text)
		{
			unsigned char c = *text++;

			// Chars with high bit must be quoted
			if (c > 0x7F)
				qp_encoding = true;
		}

		// No need to quote
		return qp_encoding ? eQuotedPrintableEncoding : e7bitEncoding;
	}
	else
		return eNoTransferEncoding;
}

// Get a filter for an encoding type
CFilter* CMIMESupport::GetFilter(const CAttachment* attach, bool decoding)
{
	// When encoding we always use the binhex/uu filters
	// For decoding, we might have binhex/uu re-encoded as base64 so we let the caller
	// handle that case by adding a secondary filter on top of the cte filter
	if (!decoding)
	{
		// Check for binhex first
		if (attach->GetContent().IsBinHexed())
		{
			// Create a binhex filter
			return new CBinHexFilter();
		}

		// Check for uu next
		if (attach->GetContent().IsUUed())
		{
			// Create a UU filter
			return new CUUFilter();
		}
	}

	bool is_text = attach->IsText();
	bool is_flowed = is_text && attach->GetContent().IsFlowed() &&
						(decoding || (attach->GetContent().GetContentDisposition() == eContentDispositionInline));
	bool is_delsp = is_flowed && attach->GetContent().IsDelsp();
	switch(attach->GetContent().GetTransferMode())
	{

	case eNoTransferMode:
	case eTextMode:
	case eAppleSingleMode:
	case eAppleDoubleMode:
	case eMIMEMode:
	default:
		// Check to see whether don't encode is on and if so force it to use 7bit filter
		// This allows output of already encoded data using the proper CTE header value without it being re-encoded
		EContentTransferEncoding encoding = attach->GetContent().GetTransferEncoding();
		if (attach->GetContent().GetDontEncode())
			encoding = eNoTransferEncoding;
		switch(encoding)
		{
		case eNoTransferEncoding:
		case e7bitEncoding:
		case e8bitEncoding:
		default:
			return new C8bitFilter(attach->GetContent().GetCharset(), is_text, is_flowed, is_delsp);

		case eQuotedPrintableEncoding:
			return new CQPFilter(attach->GetContent().GetCharset(), is_text, is_flowed, is_delsp);

		case eBase64Encoding:
			return new CBase64Filter(attach->GetContent().GetCharset(), is_text, is_flowed, is_delsp);

		case eBinaryEncoding:
		case eXtokenEncoding:
			// Check for text/xxx encoded as binary
			if (is_text)
				return new C8bitFilter(attach->GetContent().GetCharset(), is_text, is_flowed, is_delsp);
			else
				// No filter - copy as is
				return new CFilter;

		}
		break;

	}

	return NULL;
}

// Get a file stream for an encoding type
LStream* CMIMESupport::GetFileStream(CAttachment& attach)
{
	CFullFileStream* aFile = NULL;

	// Look for BinHex or UU which will handle the file itself
	if (attach.GetContent().IsBinHexed() || attach.GetContent().IsUUed())
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Just create new stream
		PPx::FSObject aSpec;
		aFile = new CFullFileStream(aSpec);
#elif __dest_os == __win32_os || __dest_os == __linux_os
		// Just create new empty stream
		aFile = new CFullFileStream();
#else
#error __dest_os
#endif

	}

	else
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Get file spec from user
		PPx::FSObject file;

		// Get name buried in AppleDouble first part if required
		cdstring fname;
		if (attach.IsApplefile() && attach.IsMultipart())
			fname = attach.GetMappedName(true, false);
		else
			fname = attach.GetMappedName(true, false);

		if (CMIMESupport::MapToFile(fname, file))
		{

			// Create new stream
			aFile = new CFullFileStream(file);

			// Give this FSSpec to attachment
			HFSFlavor newFlavor;
			newFlavor.fileType = '****';
			newFlavor.fileCreator = '****';
			newFlavor.fdFlags = 0;

			// Create data/resource forks
			try
			{
				switch(attach.GetContent().GetTransferMode())
				{

				case eBinHex4Mode:
					// Binhex filter will create file
					break;

				case eAppleSingleMode:
				case eAppleDoubleMode:
					// No filter - copy as is
					ICMapEntry entry;
					CICSupport::ICMapFileName(attach.GetContent(), entry);
					aFile->CreateNewFile(entry.fileCreator, entry.fileType, smCurrentScript);
					newFlavor.fileType = entry.fileType;
					newFlavor.fileCreator = entry.fileCreator;
					newFlavor.fdFlags = 0;
					break;

				case eNoTransferMode:
				case eTextMode:
				case eMIMEMode:
				default:
					switch(attach.GetContent().GetTransferEncoding())
					{
					case eNoTransferEncoding:
					case e7bitEncoding:
					case eQuotedPrintableEncoding:
					case eBase64Encoding:
					case e8bitEncoding:
					case eBinaryEncoding:
					case eXtokenEncoding:
						// No filter - copy as is
						ICMapEntry entry;
						CICSupport::ICMapFileName(attach.GetContent(), entry);
						aFile->CreateNewDataFile(entry.fileCreator, entry.fileType, smCurrentScript);
						newFlavor.fileType = entry.fileType;
						newFlavor.fileCreator = entry.fileCreator;
						newFlavor.fdFlags = 0;
						break;

					}
					break;

				}

				//attach.SetHFSFlavor(newFlavor);
			}
			catch (const PP_PowerPlant::LException& ex)
			{
				CLOG_LOGCATCH(const);

				CErrorHandler::PutOSErrAlertRsrc("Alerts::Attachments::FilterNoFile", ex.GetErrorCode());
				delete aFile;
				aFile = NULL;
			}
		}
#else
		cdstring fpath;
		if (CMIMESupport::MapToFile(attach.GetMappedName(true, true), fpath))
		{
			try
			{
				// Create file object and specify it
#if __dest_os == __win32_os
				aFile = new CFullFileStream(fpath.win_str(), CFile::modeCreate | CFile::modeWrite);
#elif __dest_os == __linux_os
				aFile = new CFullFileStream(fpath, O_WRONLY | O_CREAT);
#endif
			}
			catch (CFileException* ex)
			{
				CLOG_LOGCATCH(CFileException*);

				CErrorHandler::PutFileErrAlertRsrc("Alerts::Attachments::FilterNoFile", *ex);
				aFile = NULL;
			}
		}
#endif
	}
	return aFile;
}

#if __dest_os == __win32_os || __dest_os == __linux_os
// Map a file to suitable MIME type/subtype
void CMIMESupport::MapFileToMIME(CAttachment& attach)
{
	CMIMEContent& content = attach.GetContent();

	// Set default type/subtype
	content.SetContent(eContentApplication, eContentSubOctetStream);
	content.SetTransferMode(eMIMEMode);

	// Get suffix
	const cdstring& name = content.GetMappedName();
	const char* pos = ::strrchr(name.c_str(), '.');

	if (pos)
	{
#if __dest_os == __win32_os
		cdstring suffix = pos;

		// Look in registry
		cdstring mime = CWinRegistry::GetMIMEFromFile(suffix);
#else
		cdstring suffix = pos;
		cdstring mime = CMIMETypesMap::sMIMETypesMap.GetMIMEType(suffix.c_str()); //Get from .mime.types file
#endif
		if (!mime.empty())
			content.SetContent(mime);
	}
}
#endif

#if ( __dest_os == __win32_os ) || (__dest_os == __linux_os)
// Get file extension for MIME
cdstring CMIMESupport::MapMIMEToFileExt(const CAttachment& attach)
{
	// Get type/subtype text
	const cdstring type = GenerateContentHeader(&attach, false, lendl, false);

	// Try explicit mappings first
	// Check for explicit mapping first
	if (CPreferences::sPrefs->mExplicitMapping.GetValue())
	{
		const CMIMEMap* map = CMIMEMap::Find(type);
		if (map)
			return map->GetFileSuffix();
	}

	cdstring name = attach.GetContent().GetMappedName();
	cdstring original;
	const char* p = ::strrchr(name.c_str(), '.');

	// Use existing extension (exclude files that start with '.')
	if (p && (p != name.c_str()))
		original = p;

#if __dest_os == __win32_os
	// Look in registry
	return CWinRegistry::GetSuffixFromMIME(type, original);
#else
	// Look in MIME map
	return CMIMETypesMap::sMIMETypesMap.GetExtension(type, original);
#endif
}
#endif

// Get application that will open MIME part
cdstring CMIMESupport::MapMIMEToApp(const CAttachment& attach)
{

	// Special for Launch option on Windows
#if __dest_os == __win32_os
	if (!CPreferences::sPrefs->mShellLaunch.GetValue().empty())
	{
		// Get first word
		cdstring temp = CPreferences::sPrefs->mShellLaunch.GetValue();
		if (::strchr(temp.c_str(), ' '))
			*::strchr(temp.c_str_mod(), ' ') = 0;
		return temp;
	}
#endif

	// Cannot map multipart (can do AppleDouble) or message
	if (attach.IsMultipart() && !attach.IsApplefile() || attach.IsMessage())
		return cdstring::null_str;

	// Do not map attachments which we decode
	if (attach.GetContent().IsBinHexed() || attach.GetContent().IsUUed() || !attach.IsMultipart() && attach.IsApplefile())
		return cdstring::null_str;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Lookup attachment with internet config
	ICMapEntry entry;
	bool ad = attach.IsMultipart() && attach.IsApplefile() && attach.GetParts();
	if (CICSupport::ICMapFileName(ad ? attach.GetParts()->back()->GetContent() : attach.GetContent(), entry) == noErr)
	{
		if (entry.flags & kICMapPostMask)
			return entry.postAppName;
		else
			return entry.creatorAppName;
	}
	else
		return cdstring::null_str;

#elif __dest_os == __win32_os
	// Look for suffix from MIME type/subtype first
	cdstring suffix = MapMIMEToFileExt(attach);

	// Look for suffix on existing name if no MIME map
	if (suffix.empty())
	{
		suffix = attach.GetMappedName(true, false);
		if (::strrchr(suffix.c_str(), '.'))
			suffix = cdstring(::strrchr(suffix.c_str(), '.'));
		else
			// Treat as executable
			suffix = ".exe";
	}
	// Look in registry for matching app
	return CWinRegistry::GetAppFromFile(suffix);
#elif __dest_os == __linux_os
	// Get type/subtype text
	const cdstring type = GenerateContentHeader(&attach, false, lendl, false);

	return CMailcapMap::sMailcapMap.GetAppName(type);
#else
#error __dest_os
#endif
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
// Get application creator that will open MIME part
OSType CMIMESupport::MapMIMEToCreator(const CAttachment& attach)
{
	// Cannot map multipart or message
	if (attach.IsMultipart() || attach.IsMessage())
		return 0L;

	// Do not map attachments which we decode
	if (attach.GetContent().IsBinHexed() || attach.GetContent().IsUUed() || attach.IsApplefile())
		return 0L;

	// Lookup attachment with internet config
	ICMapEntry entry;
	if (CICSupport::ICMapFileName(attach.GetContent(), entry) == noErr)
	{
		if (entry.flags & kICMapPostMask)
			return entry.postCreator;
		else
			return entry.fileCreator;
	}
	else
		return 0L;
}

// Get application creator that will open MIME part
OSType CMIMESupport::MapMIMEToCreator(const cdstring& fname, const cdstring& type)
{
	// Lookup attachment with internet config
	ICMapEntry entry;
	if (CICSupport::ICMapMIMEType(fname, type, entry) == noErr)
	{
		if (entry.flags & kICMapPostMask)
			return entry.postCreator;
		else
			return entry.fileCreator;
	}
	else
		return 0L;
}
#endif

// Get local filename for MIME part
#if __dest_os == __mac_os || __dest_os == __mac_os_x
bool CMIMESupport::MapToFile(const cdstring& name,  PPx::FSObject& file)
#elif  __dest_os == __win32_os || __dest_os == __linux_os
bool CMIMESupport::MapToFile(const cdstring& name, cdstring& fpath)
#endif
{
	// Make local copy
	cdstring fname = name;

	// Make name safe for OS
	MakeSafeFileName(fname);

#if __dest_os == __mac_os || __dest_os == __mac_os_x

	// Decide whether to use default path
	if (CPreferences::sPrefs->mAskDownload.GetValue() || CPreferences::sPrefs->mDefaultDownload.GetValue().empty())
	{
		bool	done = false;
		PPx::FSObject	fspec;
		MyCFString		cfstr(fname, kCFStringEncodingUTF8);
		bool	replacing = false;


		// Do standard save as dialog with directory Adjustment if required
		if (CAttachment::sDropLocation != NULL)
		{
			done = true;
			fspec = PPx::FSObject(*CAttachment::sDropLocation, cfstr);
			replacing = fspec.Exists();
		}
		else
		{
			// Get file spec from user
			done = PP_StandardDialogs::AskSaveFile(cfstr, '****', fspec, replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup);
		}


		if (done)
		{
			if (replacing)	// Delete existing file
			{
				fspec.Delete();
			}

			file = fspec;
			return true;
		}
		else
			return false;
	}
	else
	{
		// Concat path and name
		cdstring fpath = CPreferences::sPrefs->mDefaultDownload.GetValue() + fname;
		
		MyCFString cfstr(fpath, kCFStringEncodingUTF8);
		PPx::FSObject fspec(cfstr);

		// Make sure its unique
		cdstring nname = fname;
		int ctr = 1;
		while(fspec.Exists())
		{
			// Add number to name
			nname = fname + cdstring((long) ctr++);
			fpath = CPreferences::sPrefs->mDefaultDownload.GetValue() + nname;
			MyCFString cfstr(fpath, kCFStringEncodingUTF8);
			fspec = PPx::FSObject(cfstr);
		}

		// Alert if directory not found
		if (!fspec.IsValid())
		{
			UDesktop::Deactivate();		// Alert will swallow Deactivate event
			::SysBeep(1);
			::StopAlert(206, NULL);
			UDesktop::Activate();
		}

		return true;
	}
#elif __dest_os == __win32_os

	// Decide whether to use default path
	if (CPreferences::sPrefs->mAskDownload.GetValue() || CPreferences::sPrefs->mDefaultDownload.GetValue().empty())
	{
		const char* fext = ::strchr(fname.c_str(), '.');
		if (fext)
			fext++;
		CFileDialog dlg(false, cdstring(fext).win_str(), fname.win_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, CSDIFrame::GetAppTopWindow());

		if (dlg.DoModal() == IDOK)
		{
			fpath = dlg.GetPathName();

			// Delete any existing file
			try
			{
				if (!::DeleteFile(fpath.win_str()))
					CFileException::ThrowOsError((LONG)::GetLastError());
			}
			catch (CFileException* ex)
			{
				CLOG_LOGCATCH(CFileException*);

				// Only allow file not found
				if (ex->m_cause != CFileException::fileNotFound)
				{
					CLOG_LOGRETHROW;
					throw;
				}
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				fext = NULL;
			}

			return true;
		}
		else
			return false;
	}
	else
	{
		// Concat path and name
		fpath = CPreferences::sPrefs->mDefaultDownload.GetValue() + fname;

		// Make sure its unique
		cdstring nname = fname;
		int ctr = 1;
		while(GetFileAttributes(fpath.win_str()) != -1)
		{
			// Add number to name
			const char* p = ::strrchr(fname.c_str(), '.');
			nname = cdstring(fname, 0, p - fname.c_str());
			nname += cdstring((long) ctr++);
			nname += p;
			fpath = CPreferences::sPrefs->mDefaultDownload.GetValue() + nname;
		}

		// Alert if directory not found
		DWORD err = ::GetLastError();
		if (err == ERROR_PATH_NOT_FOUND)
			CErrorHandler::PutStopAlertRsrc("Alerts::Attachments::DownloadDirFail");
		if (err != ERROR_FILE_NOT_FOUND)
		{
			CLOG_LOGTHROW(CGeneralException, err);
			throw CGeneralException(err);
		}

		return true;
	}
#elif __dest_os == __linux_os
	// Decide whether to use default path
	if (CPreferences::sPrefs->mAskDownload.GetValue() || CPreferences::sPrefs->mDefaultDownload.GetValue().empty())
	{
		JString newfile;
		if (JGetChooseSaveFile()->SaveFile("prompt", NULL, fname.c_str(), &newfile))
		{
			// Delete any existing file
			if (JFileExists(newfile))
				JRemoveFile(newfile);
			fpath = newfile;
			return true;
		}
		else
			return false;
	}
	else
	{
		// Concat path and name
		fpath = CPreferences::sPrefs->mDefaultDownload.GetValue() + fname;
		
		// Make sure its unique
		cdstring nname = fname;
		int ctr = 1;
		while(JFileExists(fpath))
		{
			// Add number to name
			const char* p = ::strrchr(fname.c_str(), '.');
			nname = cdstring(fname, 0, p - fname.c_str());
			nname += cdstring((long) ctr++);
			nname += p;
			fpath = JCombinePathAndName(CPreferences::sPrefs->mDefaultDownload.GetValue(), nname);
		}
		
		// Alert if directory not found
		JString dir, name;
		JSplitPathAndName(fpath, &dir, &name);
		if (!JDirectoryExists(dir))
		{
			CErrorHandler::PutStopAlertRsrc("Alerts::Attachments::DownloadDirFail");
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		return true;
	}
#endif
}
