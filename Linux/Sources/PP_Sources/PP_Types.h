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


#ifndef _H_PP_Types
#define _H_PP_Types
#ifndef __PP_TYPES__MULBERRY__
#define __PP_TYPES__MULBERRY__

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

	// Integer types for when it's important to know the number
	// of bits occupied by the value. These should be changed
	// depending on the compiler.

typedef		char			SInt8;
typedef		short			SInt16;
typedef		long			SInt32;

typedef		unsigned char	UInt8;
typedef		unsigned short	UInt16;
typedef		unsigned long	UInt32;

typedef		UInt16			Char16;

typedef		unsigned char	Uchar;
typedef		const unsigned char *ConstStringPtr;


enum	ETriState {
	triState_Off,
	triState_Latent,
	triState_On
};


typedef		SInt32			CommandT;
typedef		SInt32			MessageT;

typedef		SInt16			ResIDT;
typedef		SInt32			PaneIDT;
typedef		SInt32			ClassIDT;
typedef		SInt32			DataIDT;


#ifndef topLeft
#define topLeft(r)	(((Point *) &(r))[0])
#endif

#ifndef botRight
#define botRight(r)	(((Point *) &(r))[1])
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif

#endif

#endif
