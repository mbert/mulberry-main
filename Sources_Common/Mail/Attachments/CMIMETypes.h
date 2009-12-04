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


// Header for CMIMETypes class

#ifndef __CMIMETYPES__MULBERRY__
#define __CMIMETYPES__MULBERRY__

// MIME data
enum EContentType
{
	eNoContentType = 0,
	eContentText,
	eContentMultipart,
	eContentApplication,
	eContentMessage,
	eContentImage,
	eContentAudio,
	eContentVideo,
	eContentXMulberry,
	eContentXtoken
};
extern const char* cMIMEContentTypes[];

enum EContentSubType
{
	eNoContentSubType = 0,
	eContentSubPlain,
	eContentSubEnriched,
	eContentSubHTML,
	eContentSubRFC822Headers,
	eContentSubCalendar,
	eContentSubMixed,
	eContentSubParallel,
	eContentSubDigest,
	eContentSubAlternative,
	eContentSubRelated,
	eContentSubReport,
	eContentSubAppleDouble,
	eContentSubSigned,
	eContentSubEncrypted,
	eContentSubOctetStream,
	eContentSubPostScript,
	eContentSubMacBinhex4,
	eContentSubApplefile,
	eContentSubPGPSigned,
	eContentSubPGPEncrypted,
	eContentSubPGPKeys,
	eContentSubRFC822,
	eContentSubPartial,
	eContentSubExternalBody,
	eContentSubDeliveryStatus,
	eContentSubDispositionNotification,
	eContentSubGIF,
	eContentSubJPEG,
	eContentSubBasic,
	eContentSubMPEG,
	eContentSubQuicktime,
	eContentXSubMulberryFile,
	eContentXSubMulberryPart,
	eContentXSubMulberryMissing,
	eContentSubPKCS7Signature,
	eContentSubXPKCS7Signature,
	eContentSubPKCS7Mime,
	eContentSubXPKCS7Mime,
	eContentSubXtoken
};

extern const char* cMIMEContentSubTypes[];

enum EContentTransferEncoding
{
	eNoTransferEncoding = 0,
	e7bitEncoding,
	eQuotedPrintableEncoding,
	eBase64Encoding,
	e8bitEncoding,
	eBinaryEncoding,
	eXtokenEncoding
};

extern const char* cMIMETransferEncodings[];

enum ETransferMode
{
	eNoTransferMode = 0,
	eTextMode,
	eBinHex4Mode,
	eUUMode,
	eAppleSingleMode,
	eAppleDoubleMode,
	eMIMEMode
};

// Content parameters

#define CONTENT_PARAMETER_SET		"; %s=%s"
#define CONTENT_PARAMETER_SETQUOTED	"; %s=\"%s\""

enum EMIMEParameter
{
	eUnknown = 0,
	eCharset,
	eName,
	eBoundary,
	eReportType,
	eMulberryType,
	eMulberrySubtype,
	eMulberryFile,
	eFormat,
	eFixed,
	eFlowed,
	eDelsp,
	eDelspYes,
	eDelspNo,
	eCryptoProtocol,
	eMicalg,
	sSMIMEType,
	eMethod,
	eComponent
};

extern const char* cMIMEParameter[];

// Content dispositions

enum EContentDisposition
{
	eNoContentDisposition = 0,
	eContentDispositionInline,
	eContentDispositionAttachment,
	eContentDispositionMissing,
	eXtokenDisposition
};
extern const char* cMIMEDisposition[];

enum EContentDispositionParameter
{
	eContentDispositionFilename = 0,
	eContentDispositionCreation,
	eContentDispositionModification,
	eContentDispositionRead,
	eContentDispositionSize
};

extern const char* cMIMEDispositionParameter[];


enum EReportTypeParameter
{
	eReportTypeDSN = 0,
	eReportTypeMDN
};

extern const char* cMIMEReportTypeParameter[];

// smime-type parameters
enum ESMIMETypeParameter
{
	eSMIMETypeEnveloped = 0,
	eSMIMETypeSigned,
	eSMIMETypeCerts
};

extern const char* cMIMESMIMETypeParameter[];

// Encodings

// Multipart
#define CONTENT_MULTIPART_BOUNDARY		"=========="

#endif
