/*
	File:		ATSUTextConversion.h

	Description:

	Author:		AD

	Copyright:	Copyright © 1998-2000 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Change History (most recent first):
				10/14/1999	AD				Updated			
				 7/27/1999	KG				Updated for Metrowerks Codewarror Pro 2.1				
				 7/01/1998	AD				Created
				

*/

#ifndef _ATSUTEXTCONVERSION_
#define _ATSUTEXTCONVERSION_

#ifdef __MWERKS__

// includes for MetroWerks CodeWarrior

#include <MacTypes.h>
#include <ATSUnicode.h>

#else

#ifdef __APPLE_CC__

// includes for ProjectBuilder

#include <Carbon/Carbon.h>

#else

// includes for MPW

#include <Carbon.h>
#include <CoreServices.h>

#endif

#endif

/********************************************************************************/
/*	These two #define statements are needed in an environment which isn't using	*/
/*		the Universal Headers version 3.2										*/
/********************************************************************************/

#ifndef FOUR_CHAR_CODE
#define FOUR_CHAR_CODE	UInt32
#endif

#ifndef EXTERN_API_C
#define EXTERN_API_C(a)	a
#endif
/********************************************************************************/
/*	Types and Structures														*/
/********************************************************************************/

typedef UInt16 ATSUVersionType;

//	We define "ATSUVersionType" because (a) it's good programming practice and
//		should have been done in ATSUnicode.h, and (b) to ease the transition from
//		two-byte versions in streamed ATSUI styles to four-byte versions

enum {
	kATSUustlVersion = 0,
	kATSUFlattenedLineCtrlVersion = 0,
	kATSUFlattenedStyleVersion = 0
};

//
//	Note on style format
//
//	The ATSUI documentation defines a common format for streaming
//		style information for ATSUI text.  We repeat it here:
//
//	ATSUVersionType	:	Overall stream version (currently 0)
//	UniCharCount	:	Total text length
//	ItemCount		:	Number of layout control runs
//	ByteCount		:	Offset to layout controls (from overall stream version)
//	ItemCount		:	Number of style runs
//	ByteCount		:	Offset to style runs (from overall stream version)
//	
//	For each layout control run:
//	     UniCharCount			:	Run length
//	     ByteCount				:	Size of streamed controls (from layout control stream version on)
//	     ATSUVersionType		:	Layout control stream version (currently 0)
//	     ItemCount				:	Number of controls
//	     For each control:
//	          ATSUAttributeTag	:    Tag
//	          ByteCount			:    Length
//	          [variable]		:    Value
//	
//	For each style run:
//	     UniCharCount			:	Run length
//	     ByteCount				:	Size of streamed style data (from style run stream version on)
//	     ATSUVersionType		:	Style run stream version (currently 0)
//	     ItemCount				:	Number of attributes
//	     ItemCount				:	Number of font features
//	     ItemCount				:	Number of font variations
//	     For each attribute:
//	          ATSUAttributeTag			:    Tag
//	          ByteCount					:    Length
//	          [variable]				:    Value
//	     For each font feature:
//	          ATSUFontFeatureType		:	Type
//	          ATUSFontFeatureSelector	:	Selector
//	     For each font variation:
//	          ATSUFontVariationAxis		:	Axis
//	          ATSUFontVariationValue	:	Value
//	
//	Several attributes and line controls should always be included because their default
//	values are not constant. They are kATSUFontTag, kATSUSizeTag, kATSULanguageTag and
//	kATSULineDirectionTag. Moreover, the font's unique name (kFontUniqueName name code)
//	should be recorded instead of its ATSUFontID. I.e., for a style run's font, the
//	attribute should be recorded as
//	
//			ATSUAttributeTag	:	kATSUFontTag
//			ByteCount			:	name length (in bytes)
//			[variable]			:	font unique name.
//

struct ATSUStyleStreamHeader
	{
	ATSUVersionType	fVersion;			//	Overall stream version (currently 0)
	UniCharCount	fTextLength;		//	Total text length
	ItemCount		fLayoutRunCount;	//	Number of layout control runs
	ByteCount		fLayoutRunOffset;	//	Offset to layout controls (from overall stream version)
	ItemCount		fStyleRunCount;		//	Number of style runs
	ByteCount		fStyleRunOffset;	//	Offset to style runs (from overall stream version)
	};

#ifndef __cplusplus
typedef struct ATSUStyleStreamHeader ATSUStyleStreamHeader;
#endif


const Size kATSUStyleStreamHeaderSize = sizeof( ATSUStyleStreamHeader );	
	//	Strictly speaking, this is unnecessary, but it's handy.
	//	It also gives us an excuse to mention that this code may not work
	//		if the compiler is set to use PPC (long) alignment for structs

struct ATSULayoutRunStreamHeader
	{
    UniCharCount	fRunLength;			//	Run length
    ByteCount		fRunDataSize;		//	Size of streamed controls (from layout control stream version on)
    ATSUVersionType	fRunStreamVersion;	//	Layout control stream version (currently 0)
	ItemCount		fRunControlCount;	//	Number of controls
	};		

#ifndef __cplusplus
typedef struct ATSULayoutRunStreamHeader ATSULayoutRunStreamHeader;
#endif



struct ATSUStyleRunStreamHeader 
	{
     UniCharCount		fRunLength;			//	Run length
     ByteCount			fRunDataSize;		//	Size of streamed style data (from style run stream version on)
     ATSUVersionType	fRunStreamVersion;	//	Style run stream version (currently 0)
     ItemCount			fRunAttributeCount;	//	Number of attributes
     ItemCount			fRunFeatureCount;	//	Number of font features
     ItemCount			fRunVariationCount;	//	Number of font variations
	};

#ifndef __cplusplus
typedef struct ATSUStyleRunStreamHeader ATSUStyleRunStreamHeader;
#endif



/********************************************************************************/
/*	Functions																	*/
/********************************************************************************/

//
//	NOTE:  The "Unicode text" used in these routines *MUST* be big-endian UTF-16
//		(two-byte) Unicode.  Clients of these functions are expected to check for
//		the byte-order mark (BOM, U+FEFF) at the beginning of their Unicode text
//		and do any necessary byte-swapping *before* they call into these
//		functions
//


extern OSStatus ATSUConvertToTEStyledText(		Handle iUnicodeText,
												Handle iUnicodeStyles,
												Handle oTEText,
												Handle oTEStyles
												);
/*	Converts Unicode text (optionally with style information) into runs of WorldScript	*/
/*		text (optionally with style information)										*/
/*	iUnicodeText == Unicode text to convert (this parameter cannot be NULL)				*/
/*	iUnicodeStyles == ATSUI streamed style information; this parameter may be NULL or	*/
/*		length 0, in which case we assume the text is unstyled							*/
/*	oTEText == A handle to fill with runs of WorldScript text.  This parameter cannot 	*/
/*		be NULL and must have been allocated by the caller.  							*/
/*	oTEStyles == A handle to fill with TextEdit style information in the TE scrap 		*/
/*		format.  If NULL, the function assumes that no style information is desired		*/
/*		and that the Unicode text should be converted to UTF-8, as a "plain text"		*/
/*		format that preserves the complete Unicode information.  If not NULL, its		*/
/*		current information will be overwritten											*/

extern OSStatus ATSUConvertFromTEStyledText(	Handle iTEText,
												Handle iTEStyles,
												Handle oUnicodeText,
												Handle oUnicodeStyles
												);
/*	Converts runs of WorldScript text (optionally with style information) into Unicode	*/
/*		text (optionally with style information)										*/
/*	iTEText == WorldScript text to convert (this parameter cannot be NULL)				*/
/*	iTEStyles == TextEdit streamed style information; this parameter may be NULL or		*/
/*		length 0, in which case we assume the text is unstyled UTF-8					*/
/*	oUnicodeText == A handle to fill with Unicode text.  This parameter cannot		 	*/
/*		be NULL and must have been allocated by the caller.  							*/
/*	oUnicodeStyles == A handle to fill with ATSUI streamed in the format described in 	*/
/*		the header.  This cannot be NULL												*/




#endif	/*	_ATSUTEXTCONVERSION_	*/