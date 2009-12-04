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


// Header for CMIMESupport class

#ifndef __CMIMESUPPORT__MULBERRY__
#define __CMIMESUPPORT__MULBERRY__

#include "cdstring.h"
#include "CMIMETypes.h"

// Classes
class CAttachment;
class CFileAttachment;
class CFilter;
class CMessage;
class CMIMEContent;

class LStream;
class LFileStream;

class CMIMESupport
{
public:
	static bool SplitParamName(cdstring& name, cdstring& txt2231);				// Split parameter according to 2231 style

	static cdstring	GenerateContentHeader(const CAttachment* attach,			// Generate header for attachment
											bool dummy_files,
											EEndl for_endl,
											bool description = true);
	static cdstring	GenerateTransferEncodingHeader(const CAttachment* attach,	// Generate header for attachment
											EEndl for_endl,
											bool description = true);
	static cdstring	GenerateContentDescription(const CAttachment* attach,		// Generate header for attachment
											   EEndl for_endl,
											   bool description = true);
	static cdstring	GenerateContentId(const CAttachment* attach,				// Generate header for attachment
											   EEndl for_endl,
											   bool description = true);
	static cdstring	GenerateContentDisposition(const CAttachment* attach,		// Generate header for attachment
											EEndl for_endl,
											bool description = true);

	static cdstring GenerateContentParameter(const cdstring& param);			// Generarate MIME value

	static cdstring	GenerateMultipartBoundary(const CAttachment* attach,		// Generate string boundary
												EEndl for_endl,
												unsigned long level,
												bool start = false,
												bool stop = false);
	static void		RedoBoundary(const CAttachment* attach,						// Redo boundary in header of message/rfc822
									cdstring& hdr,
									unsigned long level);

	static EContentTransferEncoding DetermineTextEncoding(LFileStream& text,	// Determine best encoding type for text file
									EContentDisposition disposition);
	static EContentTransferEncoding DetermineTextEncoding(const char* text);	// Determine best encoding type for text

	static CFilter*	GetFilter(const CAttachment* attach, bool decoding);		// Get a filter for some content

	static LStream*	GetFileStream(CAttachment& attach);							// Get a file stream for some content

#if (__dest_os == __win32_os) || (__dest_os == __linux_os)
	static void MapFileToMIME(CAttachment& attach);								// Map file extension to MIME content
	static cdstring MapMIMEToFileExt(const CAttachment& attach);				// Get file extension for MIME
#endif
	static cdstring MapMIMEToApp(const CAttachment& attach);					// Get application that will open MIME part
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	static OSType MapMIMEToCreator(const CAttachment& attach);						// Get application that will open MIME part
	static OSType MapMIMEToCreator(const cdstring& fname, const cdstring& type);	// Get application that will open MIME part
	static bool MapToFile(const cdstring& name, PPx::FSObject& file);				// Get local filename for MIME part
#else
	static bool MapToFile(const cdstring& name, cdstring& fpath);					// Get local filename for MIME part
#endif

private:
					CMIMESupport();
					~CMIMESupport();
	static CMIMEContent*	GetMainContent(const CMessage* theMsg);					// Get the content type used in the message header
	
	static void AddParameter(std::ostream& out, const cdstring& name, const cdstring& value, bool value_ok, long& line_length, EEndl for_endl);

};

#endif
