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


// Source for CMIMETypes class

#include "CMIMETypes.h"

const char* cMIMEContentTypes[] = {"", "text", "multipart", "application",
									"message", "image", "audio", "video", "x-mulberry"};
const char* cMIMEContentSubTypes[] = {"",
									 "plain", "enriched", "html", "rfc822-headers", "calendar",
									 "mixed", "parallel", "digest", "alternative", "related", "report", "appledouble", "signed", "encrypted",
									 "octet-stream", "postscript", "mac-binhex40", "applefile", "pgp-signature", "pgp-encrypted", "pgp-keys",
									 "rfc822", "partial", "external-body", "delivery-status", "disposition-notification",
									 "gif", "jpeg",
									 "basic",
									 "mpeg", "quicktime",
									 "file", "part", "missing",
									 "pkcs7-signature", "x-pkcs7-signature",
									 "pkcs7-mime", "x-pkcs7-mime"};
const char* cMIMETransferEncodings[] = {"", "7bit", "quoted-printable",
										"base64", "8bit", "binary"};
const char* cMIMEParameter[] = {"unknown",
								"charset",
								"name",
								"boundary",
								"report-type",
								"mulberry_type", "mulberry-subtype", "mulberry-file",
								"format", "fixed", "flowed",
								"delsp", "yes", "no",
								"protocol", "micalg", "smime-type",
								"method", "component"};
const char* cMIMEDisposition[] = {"unknown", "inline", "attachment", "x-mulberry-partial"};
const char* cMIMEDispositionParameter[] = {"filename", "creation-date", "modification-date", "read-date", "size"};

const char* cMIMEReportTypeParameter[] = {"delivery-status", "disposition-notification"};

const char* cMIMESMIMETypeParameter[] = {"enveloped-data", "signed-data", "certs-only"};

const char* cMIMEParameterSpecials = " ()<>@,;:\\\"/[]?=";
