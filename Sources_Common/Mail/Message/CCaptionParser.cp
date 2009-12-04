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


// CCaptionParser.cp - parses tagged caption input by user

#include "CCaptionParser.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CMessage.h"
#include "CPreferences.h"
#include "CRFC822.h"

#include <strstream>

const char* cCaptions[] = {"%",
							"me-name",
							"me-first",
							"me-email",
							"me-full",
							"now",
							"smart-name",
							"smart-first",
							"smart-email",
							"smart-full",
							"from-name",
							"from-first",
							"from-email",
							"from-full",
							"to-name",
							"to-first",
							"to-email",
							"to-full",
							"cc-name",
							"cc-first",
							"cc-email",
							"cc-full",
							"subject",
							"sent-long",
							"sent-short",
							"page" };

cdstring CCaptionParser::ParseCaption(const cdstring& caption, const CMessage* msg, bool multi, unsigned long page_num)
{
	std::ostrstream out;
	const char* p = caption.c_str();
	long caption_on_line = 0;
	bool empty_caption = true;
	std::streampos line_pos = out.tellp();

	while(*p)
	{
		char c = *p++;
		switch(c)
		{
		case '%':
		  {
			size_t len = 0;
			int i = 0;
			for(i = ePercent; i <= ePage; i++)
			{
				len = ::strlen(cCaptions[i]);
				if (::strncmp(p, cCaptions[i], len) == 0)
					break;
			}

			switch(i)
			{
			case ePercent:
				out << "%";
				p++;
				break;
			case eMe:
			case eMeFirst:
			case eMeEmail:
			case eMeFull:
				{
					// Use default identity
					CAddress addr(CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom());
					PutAddressCaption(out, static_cast<EMode>(i), addr);
					p += len;
					caption_on_line = true;
					empty_caption = false;
				}
				break;
			case eDateNow:
				// Time now
				out << CRFC822::GetRFC822Date();
				p += len;
				caption_on_line = true;
				empty_caption = false;
				break;
			case eSmart:
			case eSmartFirst:
			case eSmartEmail:
			case eSmartFull:
				{
					const CEnvelope* theEnv = msg->GetEnvelope();
					cdstring smartTxt = "From: ";
					if (theEnv->GetFrom()->size())
					{
						const CAddress* addr = nil;

						// Compare address with current user
						if (CPreferences::TestSmartAddress(*theEnv->GetFrom()->front()))
						{
							// Check for to
							if (theEnv->GetTo()->size())
							{
								addr = theEnv->GetTo()->front();
								smartTxt = "To: ";
								if (!multi)
									out << smartTxt;
								PutAddressListCaption(out, static_cast<EMode>(i), *theEnv->GetTo(), multi);
							}
						}
						else
						{
							addr = theEnv->GetFrom()->front();
							if (!multi)
								out << smartTxt;
							PutAddressCaption(out, static_cast<EMode>(i), *addr);
						}
					}
					else
						out << "???";
					p += len;
					caption_on_line = true;
					empty_caption = false;
				}
				break;
			case eFrom:
			case eFromFirst:
			case eFromEmail:
			case eFromFull:
				{
					PutAddressListCaption(out, static_cast<EMode>(i), *msg->GetEnvelope()->GetFrom(), multi);
					p += len;
					caption_on_line = true;
					empty_caption = false;
				}
				break;
			case eTo:
			case eToFirst:
			case eToEmail:
			case eToFull:
				{
					PutAddressListCaption(out, static_cast<EMode>(i), *msg->GetEnvelope()->GetTo(), multi);
					p += len;
					caption_on_line = true;
					empty_caption = false;
				}
				break;
			case eCC:
			case eCCFirst:
			case eCCEmail:
			case eCCFull:
				{
					if (PutAddressListCaption(out, static_cast<EMode>(i), *msg->GetEnvelope()->GetCC(), multi))
						empty_caption = false;
					p += len;
					caption_on_line = true;
				}
				break;
			case eSubject:
				{
					out << msg->GetEnvelope()->GetSubject();
					p += len;
					caption_on_line = true;
					empty_caption = false;
				}
				break;
			case eDateSentFull:
				{
					out << msg->GetEnvelope()->GetTextDate(false, true);
					p += len;
					caption_on_line = true;
					empty_caption = false;
				}
				break;
			case eDateSentShort:
				{
					out << msg->GetEnvelope()->GetTextDate(false);
					p += len;
					caption_on_line = true;
					empty_caption = false;
				}
				break;
			case ePage:
				if (page_num > 0)
					out << page_num;
				else
					out << "%p";			// CHeadAndFoot interprets this
				p += len;
				caption_on_line = true;
				empty_caption = false;
				break;
			default:
				out << "%";
			}
			break;
		  }
		case '\r':
			// End of line processing
			if (caption_on_line && empty_caption)
			{
				// Roll back to previous line
				out.seekp(line_pos);

				// Punt past trailing LF
				if (*p == '\n')
					p++;
			}
			else
			{
				out << c;
				if (*p == '\n')
					out << *p++;
			}

			// Update start of next line ptr
			line_pos = out.tellp();
			caption_on_line = false;
			empty_caption = true;
			break;
		default:
			out << c;
		}
	}

	// End of line processing
	if (caption_on_line && empty_caption)
	{
		// Roll back to previous line
		out.seekp(line_pos);
	}

	out << std::ends;
	cdstring result;
	result.steal(out.str());
	return result;
}

bool CCaptionParser::PutAddressListCaption(std::ostream& out, EMode mode, const CAddressList& addrs, bool multi)
{
	if (addrs.size() && multi)
	{
		bool first = true;
		for(CAddressList::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
		{
			if (first)
				first = false;
			else
				out << ", ";
			PutAddressCaption(out, mode, **iter);
		}
	}
	else if (addrs.size())
		PutAddressCaption(out, mode, *addrs.front());

	return addrs.size();
}

void CCaptionParser::PutAddressCaption(std::ostream& out, EMode mode, const CAddress& addr)
{
	switch(mode)
	{
	case eMe:
	case eSmart:
	case eFrom:
	case eTo:
	case eCC:
		out << addr.GetNamedAddress();
		break;

	case eMeFirst:
	case eSmartFirst:
	case eFromFirst:
	case eToFirst:
	case eCCFirst:
		{
			// Do first part of name up to first white space
			cdstring name = addr.GetNamedAddress();
			char* start = name.c_str_mod();
			
			// Skip past comma (e.g. Last, First)
			if (::strchr(start, ',') != NULL)
				start = ::strchr(start, ',') + 1;
				
			// Punt leading white space
			while(*start && isspace(*start)) start++;

			// Get first word at this point
			char* p = start;
			while(*p)
			{
				if (isspace(*p))
				{
					*p = 0;
					break;
				}
				p++;
			}
			
			out << start;
		}
		break;

	case eMeEmail:
	case eSmartEmail:
	case eFromEmail:
	case eToEmail:
	case eCCEmail:
		out << addr.GetMailAddress();
		break;

	case eMeFull:
	case eSmartFull:
	case eFromFull:
	case eToFull:
	case eCCFull:
		out << addr.GetFullAddress();
		break;
	default:;
	}
}
