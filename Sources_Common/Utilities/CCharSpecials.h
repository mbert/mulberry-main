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


// Header for CCharSpecials classes

#ifndef __CCHARSPECIALS__H
#define __CCHARSPECIALS__H

// Allowable characters
extern const char cUSASCIIChar[];		// Allowable ASCII chars
extern const char cCEscapeChar[];		// Characters requiring escape in C
extern const char cCEscape[];			// Escape characters in C
extern const char cINETChar[];			// String chars (IMAP/IMSP?ACAP etc) (0 = OK, 1 = quote, 2 = escape, 3 = literal)
extern const char cINETCharBreak[];		// Atom delimiters (IMAP/IMSP?ACAP etc)
extern const char cQPChar[];			// Allowable quoted-printable chars
extern const char cNoQuoteChar1522[];	// Chars that must be converted for RFC1522
extern const char cNoQuoteChar1522Addr[];	// Chars that must be converted for RFC1522 in address phrase
extern const char cNoQuoteChar2231[];	// Chars that must be converted for RFC2231

#endif
