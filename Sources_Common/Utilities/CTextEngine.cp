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


// CTextEngine.cp - various text processing actions

#include "CTextEngine.h"

#include "CStringUtils.h"

#include "cdstring.h"

#include <strstream>
#include <algorithm>

extern const char* cSpace;

long CTextEngine::sSpacesPerTab = 8;
cdstring CTextEngine::sCurrentPrefix;

const char* CTextEngine::WrapLines(const char* text, unsigned long length, unsigned long wrap_len, bool flowed)
{
	// Create stream to handle writing
	std::ostrstream out;

	// Make wrap length safe
	if (wrap_len == 0)
		wrap_len = 1000;

	// Loop over all text and wrap
	long remaining = length;

	// Loop over all input chars
	long lastSpace = -1;
	long count = 0;
	const char* endLine = text;

	// Loop while waiting for line break or exceed of wrap length
	while(true)
	{
		if (remaining - count == 0)
		{
			// Output remainder of entire text
			out.write(text, count);
			remaining = 0;
			break;
		}
		else if (*endLine == lendl1)
		{
			// If flowed don't output trailing spaces
			long write_count = count;
			if (flowed)
			{
				// Special case usenet signature
				if ((count == 3) &&
					(text[0] == '-') &&
					(text[1] == '-') &&
					(text[2] == ' '))
				{
					// Allow this to be passed through
				}
				else
				{
					const char* trail = endLine - 1;
					while(write_count && (*trail == ' '))
					{
						trail--;
						write_count--;
					}
				}
			}

			// Output remainder of line
			out.write(text, write_count);
			remaining -= count;
			text += count;
			count = 0;

			// Punt over line end
#if __line_end != __crlf
			text++;
			remaining--;
#else
			text += 2;
			remaining -= 2;
#endif

			// Add CRLF to force wrap
			out.write(os_endl, os_endl_len);

			// Reset line counters
			endLine = text;
			lastSpace = -1;
		}
		
		// Now allow lines without spaces upto 500 characters long to prevent wrapping
		// of long URLs
		else if ((count >= wrap_len) && (lastSpace > 0) || (count > 500))
		{
			// Output upto last space
			if (lastSpace > 0)
			{
				// Adjust count
				count = lastSpace;
				if (count < 0)
					count = 0;

				// Strip trailing spaces
				long non_space_count = count;
				const char* s = text + count - 1;
				while(non_space_count && ((*s == ' ') || (*s == '\t')))
				{
					s--;
					non_space_count--;
				}
				// Copy line to buffer without trailing SP & then add CR
				out.write(text, non_space_count);

				// Skip spaces
				s = text + count;

				// Check that next line does not start with spaces
				while((*s == ' ') && (remaining - count > 0))
				{
					s++;
					count++;
				}
				
				// Add flowed space before CRLF
				if (flowed)
					out.put(' ');
			}
			else
			{
				// Copy whole line to buffer & bump down to not loose characters at end
				out.write(text, --count);
			}

			remaining -= count;
			text += count;
			count = 0;

			// Add CRLF to force wrap
			out.write(os_endl, os_endl_len);

			// Reset line counters
			endLine = text;
			lastSpace = -1;
		}
		else
		{
			if (isspace((unsigned char)*endLine))
				lastSpace = count;
			endLine++;
			count++;
		}
	}

	// Now create the buffer for the header
	out << std::ends;
	return out.str();
}

const char* CTextEngine::UnwrapLines(const char* text, unsigned long length)
{
	std::ostrstream out;

	const char* s = text;
	char last_char = 0;
	unsigned long count = 0;

	while(count < length)
	{
		// Check for single CRLF
		if (*s == lendl1)
		{
#if __line_end != __crlf
			// Punt past it
			s++;
			count++;
#else
			// Punt past CRLF
			s += 2;
			count += 2;
#endif
			// Check for paragraph or line end
			if (*s == lendl1)
			{
				// Output the original CRLF
				out.write(os_endl, os_endl_len);

				// Copy remaining CRLFs to stream
				while((count < length) && ((*s == '\r') || (*s == '\n')))
				{
					out << *s++;
					count++;
				}
			}

			// If end do nothing - will fall through loop
			else if (!*s)
				;
			
			// Look for a space or tab which could imply formatted text (e.g. a table, bullet list etc)
			else if (isspace(*s))
				// Output the original CRLF
				out.write(os_endl, os_endl_len);
			
			// Add space if not flowed
			else if (last_char != ' ')
				// Insert space
				out.put(' ');

			// Pretend last char is an end of line				
			last_char = lendl1;
		}
		else
		{
			last_char = *s++;
			out.put(last_char);
			count++;
		}
	}

	// Must end the entire block with a CRLF
	out.write(os_endl, os_endl_len);

	// Now create the buffer for the header
	out << std::ends;
	return out.str();
}

const char* CTextEngine::QuoteLines(const char* text, unsigned long length,
									unsigned long wrap_len, const cdstring& prefix,
									const cdstrvect* recognise, bool original_flowed)
{
	// Make wrap length safe
	if (wrap_len == 0)
		wrap_len = 1000;

	// Fake input parameter for now...
	cdstrvect prefix_matches;
	if (recognise)
		prefix_matches = *recognise;
	bool found = false;
	for(cdstrvect::const_iterator iter = prefix_matches.begin(); (iter != prefix_matches.end()) && !found; iter++)
		found = (*iter == prefix);
	if (!found)
		prefix_matches.push_back(prefix);

	// Create stream to handle writing
	std::ostrstream out;

	// Loop over all text and wrap
	bool first = true;
	long remaining = length;
	long prefix_length = prefix.length();
	long last_prefix_depth = -1;

	// Adjust prefix length for tabs
	if (prefix_length)
	{
		long tab_cnt = std::count(prefix.c_str(), prefix.c_str() + prefix_length, '\t');
		prefix_length += (sSpacesPerTab - 1) * tab_cnt;
	}

	// Loop over all input chars
	long lastSpace = -1;
	long lastCRLF = -1;
	long count = 0;
	const char* endLine = text;
	long prefixed = 0;
	long partial_line = 0;
	long prefix_depth = 0;
	bool crlf_run = false;
	bool rewrap = false;
	bool add_space = false;

	// Loop while waiting for line break or exceed of wrap length
	while(true)
	{
		if (remaining - count == 0)
		{
			// Output remainder of entire text
			if (add_space)
			{
				out.put(' ');
				add_space = false;
			}
			out.write(text, count);
			break;
		}
		else if (first || (*endLine == lendl1))
		{
			long current_line_length = count;

			if (!first)
			{
				// Output remainder of line
				if (add_space)
				{
					out.put(' ');
					add_space = false;
				}
				out.write(text, count);
				remaining -= count;
				text += count;
				count = 0;

				// Punt over line end
				if (*text == lendl1)
				{
					text++;
					remaining--;
				}
#if __line_end == __crlf
				if (remaining && (*text == lendl2))
				{
					text++;
					remaining--;
				}
#endif
				if (!remaining)
				{
					// Output last line end
					out.write(os_endl, os_endl_len);
					break;
				}
			}

			// Determine next prefix depth
			prefix_depth = GetPrefixDepth(text, remaining, prefix_matches);

			if (!remaining)
				break;

			// Check distance between last CRLF and this and force rewrap off if it might be end of para
			if (lastCRLF - remaining < wrap_len - 10)
				rewrap = false;
			lastCRLF = remaining;

			// Check current prefix depth against last prefix depth
			// If the text is not already quoted and flowed, turn off rewrap behaviour
			if ((prefix_depth != last_prefix_depth) || !rewrap || (prefix_depth == 0) && original_flowed)
			{
				// Output line end if not first line
				if (!first)
					out.write(os_endl, os_endl_len);
				partial_line = 0;
				rewrap = false;

				// Update last depth
				last_prefix_depth = prefix_depth;

				// Add prefix + 1
				AddPrefix(out, prefix, prefix_length, 1, prefix_depth, prefixed);

				// Adjust prefix counter length for prefixes longer than wrap length
				if (prefixed >= wrap_len)
					prefixed = wrap_len - 16;

				crlf_run = (*text == lendl1);
			}
			else
			{
				// Check for run of line ends
				if (*text == lendl1)
				{
			 		// Recover last line end
					out.write(os_endl, os_endl_len);
					partial_line = 0;

					// Add prefix + 1
					AddPrefix(out, prefix, prefix_length, 1, prefix_depth, prefixed);

					// Adjust prefix counter length for prefixes longer than wrap length
					if (prefixed >= wrap_len)
						prefixed = wrap_len - 16;

					crlf_run = true;
					rewrap = false;
				}
				else if (crlf_run)
				{
			 		// Recover last line end
					out.write(os_endl, os_endl_len);
					partial_line = 0;

					// Add prefix + 1
					AddPrefix(out, prefix, prefix_length, 1, prefix_depth, prefixed);

					// Adjust prefix counter length for prefixes longer than wrap length
					if (prefixed >= wrap_len)
						prefixed = wrap_len - 16;

					crlf_run = false;
				}
				else
				{
					// Output space
					add_space = true;
					partial_line += current_line_length + 1;
				}
			}

			// Reset line counters
			endLine = text;
			lastSpace = -1;
			first = false;
		}
		else if (prefixed + partial_line + count >= wrap_len)
		{
			// Output upto last space
			if ((lastSpace > 0) || partial_line)
			{
				// Adjust count
				count = lastSpace;
				if (count < 0)
					count = 0;

				// Strip trailing spaces
				long non_space_count = count;
				const char* s = text + count - 1;
				while(non_space_count && ((*s == ' ') || (*s == '\t')))
				{
					s--;
					non_space_count--;
				}
				// Copy line to buffer without trailing SP & then add CR
				if (add_space && non_space_count)
					out.put(' ');
				add_space = false;
				out.write(text, non_space_count);

				// Check that next line does not start with spaces
				if (lastSpace > 0)
				{
					s = text + count;
					while((*s == ' ') && (remaining - count > 0))
					{
						s++;
						count++;
					}
				}
			}
			else
			{
				// Copy whole line to buffer & bump down twice to not loose characters at end
				if (add_space)
				{
					out.put(' ');
					add_space = false;
				}
				
				// Take care => count may be 1 so force it out
				out.write(text, count > 1 ? --count : count);
			}

			// Add line end to force wrap
			out.write(os_endl, os_endl_len);
			partial_line = 0;

			// Force into wrap mode
			rewrap = true;

			// Reset line counters
			remaining -= count;
			text += count;
			count = 0;
			endLine = text;
			lastSpace = -1;

			// Add prefix + 1
			if (remaining)
			{
				AddPrefix(out, prefix, prefix_length, 1, prefix_depth, prefixed);

				// Adjust prefix counter length for prefixes longer than wrap length
				if (prefixed >= wrap_len)
					prefixed = wrap_len - 16;
			}
		}
		else
		{
			if (isspace((unsigned char)*endLine))
				lastSpace = count;
			endLine++;
			count++;
		}
	}

	// Now create the buffer for the header
	out << std::ends;
	return out.str();
}

const char* CTextEngine::UnquoteLines(const char* text, unsigned long length, const char* prefix)
{
	// Check for empty prefix
	if (!prefix || !*prefix)
		return ::strndup(text, length);

	std::ostrstream out;
	long prefix_length = (prefix ? ::strlen(prefix) : 0);
	const char* s = text;
	unsigned long count = 0;
	bool space_punt = (prefix[prefix_length - 1] != ' ');

	while(count < length)
	{
		// Check for prefix
		if (::strncmp(prefix, s, prefix_length) == 0)
		{
			s += prefix_length;
			count += prefix_length;

			// Punt over space
			if (space_punt && (count < length) && (*s == ' '))
			{
				s++;
				count++;
			}
		}

		// Punt to end of line
		while((count < length) && (*s != lendl1))
		{
			out.put(*s++);
			count++;
		}

		// Punt to start of next line
		while((count < length) && ((*s == '\r') || (*s == '\n')))
		{
			out.put(*s++);
			count++;
		}
	}

	// Now create the buffer for the header
	out << std::ends;
	return out.str();
}

// Remove lines that start with a quote character
const char* CTextEngine::StripQuotedLines(const char* text, const cdstrvect& quotes)
{
	std::ostrstream out;
	
	// Start of text => start of line
	bool line_start = true;
	const char* p = text;
	while(*p)
	{
		// Look for line end
		if ((*p == '\r') || (*p == '\n'))
		{
			out << *p++;
			line_start = true;
			continue;
		}

		// Punt whitespace at start of line
		else if (line_start && isspace(*p))
		{
			out << *p++;
			continue;
		}
		
		// Look for quotes at start of line
		else if (line_start)
		{
			// Turn off line start at this point so we can cycle back through this line and copy it out
			// if it is not found to be quoted
			line_start = false;
			
			// Look for quotes
			bool got_quote = false;
			for(cdstrvect::const_iterator iter = quotes.begin(); !got_quote && (iter != quotes.end()); iter++)
				got_quote = (::strncmp(p, *iter, (*iter).length()) == 0);
			
			// Punt line if quote found
			if (got_quote)
			{
				while(*p)
				{
					if ((*p == '\r') || (*p == '\n'))
						break;
					p++;
				}
			}
		}
		
		// Just output the char of the unquoted line
		else
			out << *p++;
	}
	
	out << std::ends;
	return out.str();
}

long CTextEngine::GetPrefixDepth(const char*& text, long& remaining, const cdstrvect& matches)
{
	// Determine next prefix depth
	bool loop_more = true;
	long prefix_depth = 0;
	const char* prefix_start = text;
	sCurrentPrefix = cdstring::null_str;

	// Policy must start with a prefix character
	// Trailing spaces/tabs are merged into prefix

	while(loop_more && remaining)
	{
		loop_more = false;

		// Compare with possible matches
		for(cdstrvect::const_iterator iter = matches.begin(); iter != matches.end(); iter++)
		{
			size_t str_len = (*iter).length();
			if (str_len && (::strncmp(text, (*iter).c_str(), str_len) == 0))
			{
				loop_more = true;
				prefix_depth++;

				// Bump past prefix
				text += str_len;
				remaining -= str_len;

				// Bump past any space or tabs
				while(remaining && ((*text == ' ') || (*text == '\t')))
				{
					text++;
					remaining--;
				}
				break;
			}
		}
	}

	// Bump past any trailing space or tabs if prefix was found
	if (prefix_start != text)
	{
		while(remaining && ((*text == ' ') || (*text == '\t')))
		{
			text++;
			remaining--;
		}
	}

	sCurrentPrefix.assign(prefix_start, text - prefix_start);

	return prefix_depth;
}

void CTextEngine::AddPrefix(std::ostream& out, const cdstring& prefix, long prefix_length, long num_add, long depth, long& prefixed)
{
	bool add_space = false;

	prefixed = 0;

	if (prefix_length && num_add)
	{
		// Add upto requested depth
		size_t out_len = prefix.length();
		for(long i = 0; i < num_add; i++)
		{
			out.write(prefix.c_str(), out_len);
			prefixed += prefix_length;
		}

		// Look for terminating space or tab
		char c = prefix.c_str()[out_len - 1];
		if ((c != ' ') && (c != '\t'))
			add_space = true;
	}

	// Now add previous prefix
	size_t prev_length = sCurrentPrefix.length();
	if (prev_length)
	{
		out.write(sCurrentPrefix.c_str(), prev_length);
		prefixed += prev_length;

		// Look for terminating space or tab
		char c = sCurrentPrefix.c_str()[--prev_length];
		if ((c == ' ') || (c == '\t'))
			add_space = !depth;
		else
			add_space = true;
	}

	// Add space between prefix and start of line (always add if no real previous prefix)
	if (add_space)
	{
		out.put(' ');
		prefixed++;
	}
}

void CTextEngine::RemoveSigDashes(char* text)
{
	char* p = text;
	char* start_dashes = NULL;
	char* start_endlrun = NULL;
	enum ESigDashState
	{
		eSigDash_None = 0,
		eSigDash_Endl,
		eSigDash_Dash1,
		eSigDash_Dash2,
		eSigDash_DashSpace,
		eSigDash_Found
	};
	unsigned long sig_dash_state = eSigDash_None;

	// Scan for last occurrence of sig dashes
	while(*p)
	{
		switch(*p)
		{
		case lendl1:
#if __dest_os == __win32_os
		case lendl2:
#endif
			if (sig_dash_state == eSigDash_DashSpace)
			{
				sig_dash_state++;
				start_dashes = start_endlrun;
			}
			else
			{
				if (sig_dash_state != eSigDash_Endl)
					start_endlrun = p;
				sig_dash_state = eSigDash_Endl;
			}
			break;
		
		case '-':
			if (sig_dash_state == eSigDash_Endl)
				sig_dash_state++;
			else if (sig_dash_state == eSigDash_Dash1)
				sig_dash_state++;
			else
				sig_dash_state = eSigDash_None;
			break;
		case ' ':
			if (sig_dash_state == eSigDash_Dash2)
				sig_dash_state++;
			else
				sig_dash_state = eSigDash_None;
			break;
		default:
			sig_dash_state = eSigDash_None;
			break;
		}
		p++;
	}
	
	// Now terminate string at start of sig dashes if found
	if (start_dashes != NULL)
		*start_dashes = 0;
}
