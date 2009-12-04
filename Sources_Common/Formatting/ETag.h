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


#ifndef __ETAG__MULBERRY__
#define __ETAG__MULBERRY__

enum ETag
{
	E_NOTHING = 0,
	E_PLAIN,
	E_BOLD,
	E_UNBOLD,
	E_UNDERLINE,
	E_UNUNDERLINE,
	E_ITALIC,
	E_UNITALIC,
	E_BIGGER,
	E_SMALLER,
	E_CENTER,
	E_FLEFT,
	E_FRIGHT,
	E_FBOTH,
	E_FONT,
	E_COLOR,
	E_FIXED,
	E_EXCERPT,
	E_SIZE,
	E_ANCHOR,
	E_H1,
	E_H2,
	E_H3,
	E_H4,
	E_H5,
	E_H6,
	E_EM,
	E_STRONG,
	E_CITE,
	E_CODE,
	E_SAMP,
	E_KBD,
	E_VAR,
	E_TT,
	E_BIG,
	E_SMALL,
	E_BLOCKQUOTE
};

enum EView
{
	eViewFormatted = 1,
	eViewPlain,
	eViewRaw,
	// On Mac, seperators are included in menu item positions so we need to add an offset
#if __framework == __powerplant
	eViewSeparator1,
#endif
	eViewAsHTML,
	eViewAsEnriched,
	eViewAsFixedFont,
	// On Mac, seperators are included in menu item positions so we need to add an offset
#if __framework == __powerplant
	eViewSeparator2,
#endif
	eViewAsRaw
};

#endif
