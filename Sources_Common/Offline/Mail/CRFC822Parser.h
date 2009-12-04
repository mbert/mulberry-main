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


// Header for an RFC822 Message parser

#ifndef __CRFC822PARSER__MULBERRY__
#define __CRFC822PARSER__MULBERRY__

#include "cdstring.h"

#include <istream>

class CAddress;
class CAddressList;
class CAttachment;
class CLocalAttachment;
class CEnvelope;
class CLocalMessage;
class CMessage;
class CMessageList;
class CMIMEContent;
class CProgress;

class CRFC822Parser
{
public:
	CRFC822Parser(bool stream = false, CMessage* owner = NULL);
	~CRFC822Parser();
	
	CMessageList*	ListFromStream(std::istream& in, CProgress* progress = NULL);
	CLocalMessage*	MessageFromStream(std::istream& in, CProgress* progress = NULL, CMessage* owner = NULL);
	void			EnvelopeFromStream(std::istream& in, CLocalMessage& msg);
	CAttachment*	AttachmentFromStream(std::istream& in, CLocalAttachment* parent);

	CAddressList*	ParseAddressList(char* str);

	char*			ParsePhrase(char *s);
	char*			ParseWord(char *s, const char *delimiters);

	char*			Quote(char *src);
	void			Skipws(char** s);
	char*			SkipComment(char** s, long trim);
	
protected:
	CMessageList*		mMsgList;
	CLocalMessage*		mMsg;
	CMessage*			mOwner;
	CAddressList*		mAddrList;
	cdstrvect			mBoundaries;
	CProgress*			mProgress;
	std::streamsize		mProgressStart;
	std::streamsize		mProgressTotal;
	bool				mStreamAttachment;
	bool				mValidMIMEVersion;
	
	bool				mLastEndl2;
	
	void			DefaultSubtype(CLocalAttachment* body);

	std::istream::pos_type	MessageStart(std::istream& in, bool do_progress = false);
	bool			ValidMessageStart(const cdstring& start);

	void			GetLine(std::istream& in, cdstring& line);
	void			GetHeader(std::istream& in, cdstring& hdr);

	bool			StreamDone(std::istream& in) const
		{ return in.fail(); }

	void			ParseHeader(CLocalMessage* msg, char* str);
	void			ParseHeader(CLocalAttachment* msg, char* str);
	void			ParseContent(CLocalAttachment* body, std::istream& in);
	void			ParseContentHeader(CLocalAttachment* body, char* name, char* s);
	void			ParseParameter(CLocalAttachment* body, char* text, bool disposition);
	bool			ParseMIMEVersion(char* text);

	void			ParseSinglePart(CLocalAttachment* body, std::istream& in);
	void			ParseMultipart(CLocalAttachment* body, std::istream& in);
	void			ParseMessagePart(CLocalAttachment* body, std::istream& in);

	bool			ParseToBoundary(std::istream& in, const cdstring& bdry, bool rewind);

	CAddress*		ParseAddress(char** str);
	CAddress*		ParseGroup(char** str);
	CAddress*		ParseMailbox(char** string);
	long			PhraseOnly(char* str);

	CAddress*		ParseRouteAddr(char *string, char **ret);
	CAddress*		ParseAddrSpec(char *string, char **ret);
		
	void			InitProgress(std::istream& in, CProgress* progress);
	void			UpdateProgress(std::istream& in);
	void			UpdateProgress(std::streamoff pos);

private:
	long			SafeTellg(std::istream& in, bool& has_eof) const;
};

#endif
