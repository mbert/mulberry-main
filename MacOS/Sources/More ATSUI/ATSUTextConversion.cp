/*
	File:		ATSUTextConversion.cp

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
#include "ATSUTextConversion.h"
#endif

#include <string.h>

#ifdef __MWERKS__

// includes for MetroWerks CodeWarrior

#include <MacErrors.h>
#include <FixMath.h>
#include <Fonts.h>
#include <QuickDraw.h>
#include <TextEdit.h>
#include <TextEncodingConverter.h>
//#include <Unicode.h>
#include <Memory.h>
#include <Script.h>

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
/*	Internal constants															*/
/********************************************************************************/

const ItemCount kMaximumTextRunCount = 64;
const Size kMaximumNameLength = 256;

/********************************************************************************/
/*	Internal function declarations												*/
/********************************************************************************/

static OSStatus	ATSUConvertUnicodeToUTF8(	Handle iText, 
											Handle oText );
static OSStatus	ATSUConvertUnstyledUnicodeToTEStyledText(	Handle iText, 
															Handle oText, 
															Handle oStyles );
static OSStatus ATSUConvertAndAppendUnicodeStyleData(	StScrpRec* iTEStyleTable, 
														const Ptr& iUnicodeStyleData,
														const ScriptCodeRun& iRunData,
														ByteCount iInitialOffset );
static OSStatus ATSUConvertUTF8ToUnicode(	Handle iText, 
											Handle oText, 
											Handle oStyles );
static OSStatus ATSUConvertAndAppendScrpSTElement(	Handle iStream, 
													const ScrpSTElement& iSTElement, 
													UniCharCount iRunLength );
static void		ATSUCopyAndMovePtr(	ATSUAttributeTag iTag, 
									Ptr iSource, 
									Ptr* iDest, 
									ByteCount iLength );

/********************************************************************************/
/*	External function definitions												*/
/********************************************************************************/

OSStatus 
ATSUConvertToTEStyledText(		Handle iUnicodeText,
								Handle iUnicodeStyles,
								Handle oTEText,
								Handle oTEStyles
								)
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
{
	OSStatus returnValue = noErr;
	if ( iUnicodeText == NULL || oTEText == NULL )
		returnValue = paramErr;
	else if ( oTEStyles == NULL )
		returnValue = ATSUConvertUnicodeToUTF8( iUnicodeText, oTEText );
	else if ( iUnicodeStyles == NULL || GetHandleSize( iUnicodeStyles ) == 0 )
		ATSUConvertUnstyledUnicodeToTEStyledText( iUnicodeText, oTEText, oTEStyles );
	else {
	
		//	This is the most unpleasant conversion problem
		//		since a single run of Unicode text may
		//		change to multiple runs of Macintosh text
		//
		//	Fortunately, the TEC can be set up to do the bulk
		//		of the grunt work
		//
		//	Variable declarations
		
		Ptr p;
		ATSUVersionType version;
		UniCharCount unicodeCount;
		ItemCount numLayoutRuns;
		ByteCount layoutRunOffset;
		ItemCount numStyleRuns;
		ByteCount styleRunOffset;
		ByteCount iOutputBufLen;
		ByteCount totalMacLength;
		Ptr oOutputStr;
		StScrpRec* theStScrpRec;
		UnicodeToTextRunInfo unicodeToTextInfo;
		ItemCount totalStyleCount = 0;
		
		//	Lock all our input handles

		SInt8 iTextState = HGetState( iUnicodeText );
		SInt8 iStylesState = HGetState( iUnicodeStyles );
		SInt8 oTextState = HGetState( oTEText );
		SInt8 oStylesState = HGetState( oTEStyles );

		MoveHHi( iUnicodeText );
		HLock( iUnicodeText );
		MoveHHi( iUnicodeStyles );
		HLock( iUnicodeStyles );		
		
		//	Read in the first part of the ATSUI style data

		p = Ptr( *iUnicodeStyles );
		version = *((ATSUVersionType*) p);
		p += sizeof( ATSUVersionType );
		unicodeCount = *((UniCharCount*) p);
		p += sizeof( UniCharCount );
		numLayoutRuns = *((ItemCount*) p);
		p += sizeof( ItemCount );
		layoutRunOffset = *((ByteCount*) p);
		p += sizeof( ByteCount );
		numStyleRuns = *((ItemCount*) p);
		p += sizeof( ItemCount );
		styleRunOffset = *((ByteCount*) p);
		p += sizeof( ByteCount );
		
		//	Skip the layout controls in the input styles
		
		p = Ptr( *iUnicodeStyles ) + styleRunOffset;
		
		//	Set up TEC variables
		
		iOutputBufLen = 6 * GetHandleSize( iUnicodeText );		//	Worst case scenario	
		totalMacLength = 0;
		HUnlock( oTEText );
		SetHandleSize( oTEText, iOutputBufLen );
		MoveHHi( oTEText );
		HLock( oTEText );
		oOutputStr = Ptr( *oTEText );
		
		HUnlock( oTEStyles );
		SetHandleSize( oTEStyles, sizeof( ScrpSTTable ) + sizeof( short ) );
			//	Worst case scenario -- styled TextEdit allows the text to be up to
			//		32K in size and allows up to 1601 style runs
		MoveHHi( oTEStyles );
		HLock( oTEStyles );
		theStScrpRec = (StScrpRec*) *oTEStyles;
		theStScrpRec->scrpNStyles = 0;	//	This will be updated as we go

		returnValue = CreateUnicodeToTextRunInfoByScriptCode( 0, NULL, &unicodeToTextInfo );
			//
			//	This will automatically set things up so that we can convert Unicode
			//		text to runs of Macintosh script runs with any *installed* Mac
			//		script as a potential target
			//
			//	EXERCISE LEFT TO THE READER:  The converted text may end up on someone
			//		else's Macintosh, who may have more scripts installed than the
			//		caller of this routine (or fewer).  Set up the parameters to 
			//		CreateUnicodeToTextRunInfoByScriptCode() to allow conversion to
			//		any supported Macintosh script (hint:  not all scripts from smRoman
			//		through smKlingon are actually supported by the TEC).  
		
		if ( returnValue == noErr ) {
			ConstUniCharArrayPtr iUnicodeStr = ConstUniCharArrayPtr( *iUnicodeText );		
			while ( returnValue == noErr && numStyleRuns-- > 0 ) {
				
				//	Set up the remaining parameters we'll need for the TEC 
				//	Note that the one thing TEC doesn't do for us is honor our style run boundaries
				//		when setting up its script runs.  We therefore have to go through
				//		one style run at a time
				
				UniCharCount runUnicodeCount = *((UniCharCount*) p);
				p += sizeof( UniCharCount );
				ByteCount runDataSize = *((ByteCount*) p );
				p += sizeof( ByteCount );
				Ptr q = p;
				p += runDataSize;
				
				ByteCount iUnicodeLen = runUnicodeCount * sizeof( UniChar );
				OptionBits iControlFlags = kUnicodeUseFallbacksMask 		| 
											kUnicodeDefaultDirectionMask 	|	//	Dangerous!  It might be
																				//		better to do something
																				//		a bit smarter for bidi here
											kUnicodeLooseMappingsMask 		|
											kUnicodeTextRunMask 			|
											kUnicodeKeepSameEncodingMask	|
											kUnicodeKeepInfoMask
											;
				ByteCount oInputRead;
				ByteCount oOutputLen;
				ItemCount iEncodingRunBufLen = kMaximumTextRunCount;
				ItemCount oScriptRunOutLen;
				ScriptCodeRun oScriptCodeRuns[ kMaximumTextRunCount ];
				
				returnValue = ConvertFromUnicodeToScriptCodeRun( unicodeToTextInfo,
								iUnicodeLen,
								iUnicodeStr,
								iControlFlags,
								0,						//	iOffsetCount
								NULL,					//	iOffsetArray
								NULL,					//	&oOffsetCount
								NULL,					//	oOffsetArray
								iOutputBufLen,
								&oInputRead,
								&oOutputLen,
								oOutputStr,
								kMaximumTextRunCount,	//	iEncodingRunBufLen
								&oScriptRunOutLen,
								oScriptCodeRuns
								);
				
				for ( ItemCount i = 0; returnValue == noErr && i < oScriptRunOutLen; i++ ) 
					ATSUConvertAndAppendUnicodeStyleData( theStScrpRec, q, oScriptCodeRuns[ i ], totalMacLength );
				
				totalMacLength += oOutputLen;				
				if ( totalMacLength > 32000 )
					returnValue = paramErr;			//	Ack!  The dreaded 32K limit!

				totalStyleCount += oScriptRunOutLen;
				if ( totalStyleCount > 1600 )
					returnValue = paramErr;			//	A lesser-known but equally dreaded artificial limit

				iUnicodeStr += runUnicodeCount;
				iOutputBufLen -= oOutputLen;
				oOutputStr += oOutputLen;

				}
			returnValue = DisposeUnicodeToTextRunInfo( &unicodeToTextInfo );
			}
		
		//	Restore handle states and do general clean-up
		
		HUnlock( oTEText );
		SetHandleSize( oTEText, totalMacLength );
		HSetState( oTEText, oTextState );
		
		HUnlock( oTEStyles );
		SetHandleSize( oTEStyles, sizeof( short ) + sizeof( ScrpSTElement ) * totalStyleCount );
		HSetState( oTEStyles, oStylesState );

		HSetState( iUnicodeStyles, iStylesState );
		HSetState( iUnicodeText, iTextState );
			
		}
	return returnValue;
}





OSStatus 
ATSUConvertFromTEStyledText(	Handle iTEText,
								Handle iTEStyles,
								Handle oUnicodeText,
								Handle oUnicodeStyles
								)
/*	Converts runs of WorldScript text (optionally with style information) into Unicode	*/
/*		text (optionally with style information)										*/
/*	iTEText == WorldScript text to convert (this parameter cannot be NULL)				*/
/*	iTEStyles == TextEdit streamed style information; this parameter may be NULL or		*/
/*		length 0, in which case we assume the text is unstyled UTF-8					*/
/*	oUnicodeText == A handle to fill with Unicode text.  This parameter cannot		 	*/
/*		be NULL and must have been allocated by the caller.  							*/
/*	oUnicodeStyles == A handle to fill with ATSUI streamed in the format described in 	*/
/*		the header.  This cannot be NULL												*/
{

	//	Various variables needed for the TEC
	//	For each style run, we check to see if we already have a TEC converter, and if we
	//		do, if it will work for the current run's script.  If not, we delete it and
	//		allocate a new one.  We then delete any TEC converter we have left at the
	//		end
	//	An alternative way to do this would be to preallocate a TEC converter for each
	//		possible Mac script and then delete them all *en masse* at the end.  This
	//		alternative is going to be less efficient, however, except in cases where
	//		there is extensive intermixing of two or more Mac scripts

	UniCharCount totalUCCount = 0;
	TextEncoding ucs2Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicode16BitFormat );
	TextEncoding macEncoding;
	TECObjectRef encodingConverter = NULL;
	ScriptCode macScript = smKlingon+1;
		//	This is guaranteed to be an invalid script ID

	OSStatus returnValue = noErr;
	if ( iTEText == NULL || oUnicodeText == NULL )
		returnValue = paramErr;
	else if ( iTEStyles == NULL || GetHandleSize( iTEStyles ) == 0 )
		ATSUConvertUTF8ToUnicode( iTEText, oUnicodeText, oUnicodeStyles );
	else {
	
		SInt8 iTextState = HGetState( iTEText );
		SInt8 iStylesState = HGetState( iTEStyles );
		SInt8 oTextState = HGetState( oUnicodeText );
		SInt8 oStylesState = HGetState( oUnicodeStyles );
		
		SetHandleSize( oUnicodeText, 4 * sizeof( UniChar ) * GetHandleSize( iTEText ) );	
			//	Worse case scenario -- allows enough room for formatting
			//		codes and surrogate conversions
		
		MoveHHi( iTEText );
		HLock( iTEText );
		Ptr macText = Ptr( *iTEText );
		MoveHHi( iTEStyles );
		HLock( iTEStyles );
		StScrpPtr macStyles = StScrpPtr( *iTEStyles );
		
		//	We'll fill the text run by run as if it were a pointer, then unlock
		//		and resize it when we're done
		
		MoveHHi( oUnicodeText );
		HLock( oUnicodeText );
		UniCharArrayPtr uc = UniCharArrayPtr( *oUnicodeText );
		
		HUnlock( oUnicodeStyles );
		SetHandleSize( oUnicodeStyles, kATSUStyleStreamHeaderSize );
		returnValue = MemError();
		if ( returnValue == noErr ) {
			ATSUStyleStreamHeader* header;
			MoveHHi( oUnicodeStyles );
			HLock( oUnicodeStyles );
			header = (ATSUStyleStreamHeader*) *oUnicodeStyles;
			header->fVersion = 0;
			header->fTextLength = 0;
			header->fLayoutRunCount = 0;
			header->fLayoutRunOffset = 0;
			header->fStyleRunCount = 0;
			header->fStyleRunOffset = kATSUStyleStreamHeaderSize;
			HUnlock( oUnicodeStyles );
			}
			
		//	Initial information for the style handle
		
		for ( ItemCount i = 0; returnValue == noErr && i < macStyles->scrpNStyles; i++ ) {
		
			//	Go through the text a style run at a time and convert each run
			//	NOTE:  The TEC provides a conversion from valid Mac scripts which is
			//		guaranteed to succeed for every character in the script.  This means
			//		that a single run of Mac script can *always* be converted into
			//		exactly one Unicode run, and we take advantage of the fact

			UniCharCount ucCount = 0;
			
			//	1.  Check that our current encoding converter is appropriate for 
			//		this style run; if not, delete it and allocate a new one
			
			if ( encodingConverter == NULL || macScript != FontToScript( macStyles->scrpStyleTab[ i ].scrpFont ) ) {
				if ( encodingConverter != NULL )
					TECDisposeConverter( encodingConverter );
				macScript = FontToScript( macStyles->scrpStyleTab[ i ].scrpFont );
				LangCode macLang = LangCode( GetScriptVariable( macScript, smScriptLang ) );
				RegionCode macRegion = RegionCode( GetScriptManagerVariable( smRegionCode ) );
				returnValue = UpgradeScriptInfoToTextEncoding( macScript, macLang, macRegion, NULL, &macEncoding );
				if ( returnValue == noErr ) 
					returnValue = TECCreateConverter( &encodingConverter, macEncoding, ucs2Encoding );
				}

			//	2.  Nasty calculation for the length of the run; we can't assume that
			//		the STScrpRec has correctly stuck the total text length after
			//		the last valid run the way we'd like

			ByteCount inputBufferLength = macStyles->scrpStyleTab[ i+1 ].scrpStartChar;
			if ( i == macStyles->scrpNStyles - 1 )
				inputBufferLength = GetHandleSize( iTEText );
			inputBufferLength -= macStyles->scrpStyleTab[ i ].scrpStartChar;
			TextPtr inputBuffer = TextPtr( macText );
			
			//	3.  Set up the conversion for the TEC
			
			ByteCount outputBufferLength = 4 * inputBufferLength * sizeof( UniChar );	//	Worst case scenario;
																						//	Allows for TEC bidi handling and surrogates
			ByteCount actualInputLength;
			ByteCount actualOutputLength;
			
			TextPtr outputBuffer = TextPtr( uc );
			if ( returnValue == noErr ) 
				returnValue = TECConvertText(	encodingConverter,
											inputBuffer, inputBufferLength, &actualInputLength,
											outputBuffer, outputBufferLength, &actualOutputLength );

			ucCount = actualOutputLength / sizeof( UniChar );

			//	EXERCISE LEFT TO THE READER:  The TEC is known to insert directinonality overrides
			//		in situations when we may not actually want them.  The TEC also doesn't
			//		normalize line returns to U+2029 PARAGRAPH SEPARATOR, which we may (or 
			//		may not) want -- ATSUI per se doesn't require it.  Do a quick scan-and-
			//		normalize for the text the TEC returns
			
			//	4.  Update our information
			
			macText += inputBufferLength;	//	NOT actualInputLength, although they should be the same
											//		if we succeeded
			uc += ucCount;
			totalUCCount += ucCount;
			if ( returnValue == noErr )
				returnValue = ATSUConvertAndAppendScrpSTElement( oUnicodeStyles, macStyles->scrpStyleTab[ i ], ucCount );
			}
		
		if ( encodingConverter != NULL )
			TECDisposeConverter( encodingConverter );
		HUnlock( oUnicodeText );
		SetHandleSize( oUnicodeText, totalUCCount * sizeof( UniChar ) );
		
		//	Restore handle states
		
		HSetState( oUnicodeStyles, oStylesState );
		HSetState( oUnicodeText, oTextState );
		HSetState( iTEStyles, iStylesState );
		HSetState( iTEText, iTextState );
	
		}
	return returnValue;
}



/********************************************************************************/
/*	Internal function definitions												*/
/********************************************************************************/

OSStatus 
ATSUConvertUnstyledUnicodeToTEStyledText(
								Handle iUnicodeText,
								Handle oTEText,
								Handle oTEStyles
								)
//	The name says it all
{
	OSStatus returnValue = noErr;
	Handle iDummyStyles = NewHandle( kATSUStyleStreamHeaderSize + sizeof( ATSUStyleRunStreamHeader ) );
	if ( iDummyStyles == NULL )
		returnValue = MemError();
	else {
		//	We solve the problem the lazy way -- make up a dummy Unicode streamed style
		//		record and call back into ATSUConvertToTEStyledText
		ATSUStyleStreamHeader* streamHeader;
		ATSUStyleRunStreamHeader* runHeader;
		MoveHHi( iDummyStyles );
		HLock( iDummyStyles );
		streamHeader = (ATSUStyleStreamHeader*) *iDummyStyles;
		streamHeader->fVersion = 0;
		streamHeader->fTextLength = GetHandleSize( iUnicodeText ) / sizeof( UniChar );
		streamHeader->fLayoutRunCount = 0;
		streamHeader->fLayoutRunOffset = 0;
		streamHeader->fStyleRunCount = 1;
		streamHeader->fStyleRunOffset = kATSUStyleStreamHeaderSize;
		runHeader = (ATSUStyleRunStreamHeader*) ( *iDummyStyles + streamHeader->fStyleRunOffset );
		runHeader->fRunLength = streamHeader->fTextLength;
		runHeader->fRunDataSize = sizeof( ATSUVersionType ) + 3 * sizeof( ItemCount );
		runHeader->fRunStreamVersion = 0;
		runHeader->fRunAttributeCount = 0;
		runHeader->fRunFeatureCount = 0;
		runHeader->fRunVariationCount = 0;
		HUnlock( iDummyStyles );
		returnValue = ATSUConvertToTEStyledText( iUnicodeText, iDummyStyles, oTEText, oTEStyles );
		DisposeHandle( iDummyStyles );
		}
	return returnValue;
}





OSStatus
ATSUConvertUnicodeToUTF8( Handle iText, Handle oText )
//	The name says it all -- just call through to the TEC
{
	SInt8 iTextState;
	SInt8 oTextState;
	
	UniCharCount ucCount;
	TextEncoding ucs2Encoding;
	TextEncoding utf8Encoding;
	TECObjectRef encodingConverter;
	OSStatus returnValue = noErr;

	if ( iText == NULL || oText == NULL )
		return paramErr;
	
	iTextState = HGetState( iText );
	oTextState = HGetState( oText );
	
	ucCount = 0;
	ucs2Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicode16BitFormat );
	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	returnValue = TECCreateConverter( &encodingConverter, ucs2Encoding, utf8Encoding );
	if ( returnValue == noErr ) {
		TextPtr outputBuffer;
		ByteCount inputBufferLength = GetHandleSize( iText );
		TextPtr inputBuffer = TextPtr( *iText );
		
		ByteCount outputBufferLength = 4 * inputBufferLength;	//	Worst case scenario;
																//	Allows for TEC bidi handling and surrogates
		ByteCount actualInputLength;
		ByteCount actualOutputLength;
		
		MoveHHi( iText );
		HLock( iText );
		HUnlock( oText );
		SetHandleSize( oText, outputBufferLength );
		MoveHHi( oText );
		HLock( oText );
		outputBuffer = TextPtr( *oText );
		returnValue = TECConvertText(	encodingConverter,
										inputBuffer, inputBufferLength, &actualInputLength,
										outputBuffer, outputBufferLength, &actualOutputLength );
		HUnlock( oText );
		SetHandleSize( oText, actualOutputLength );
		
		TECDisposeConverter( encodingConverter );
		}
	
	HSetState( oText, oTextState );
	HSetState( iText, iTextState );
	return returnValue;
}




OSStatus
ATSUConvertAndAppendUnicodeStyleData(
								StScrpRec* iTEStyleTable, 
								const Ptr& iUnicodeStyleData,
								const ScriptCodeRun& iRunData,
								ByteCount iInitialOffset )
//	Takes the ATSUI streamed style information pointed to by iUnicodeStyleData,
//		converts it to TextEdit style data and appends the result in 
//		iTEStyleTable
//	NOTE:  This function parses the ATSUI streamed style data itself.  Alternatively,
//		it could allocate an ATSUStyle, use ATSUPasteFromHandle to parse the data,
//		and then ATSUGetAttribute to pick out the TextEdit equivalent.  
{

	ATSUVersionType version;
	ItemCount numAttributes;
	ItemCount numFeatures;
	ItemCount numVariations;
	ScrpSTElement element;
	Ptr styleData = iUnicodeStyleData;
	OSStatus returnValue = noErr;

	element.scrpStartChar = iInitialOffset + iRunData.offset;
	element.scrpFont = SInt16( GetScriptVariable( iRunData.script, smScriptAppFond ) );
	element.scrpFace = normal;
	element.scrpSize = SInt16( GetScriptVariable( iRunData.script, smScriptAppFondSize ) & 0x0FFFF );
	element.scrpColor.red = 0;
	element.scrpColor.green = 0;
	element.scrpColor.blue = 0;
	
	version = *((ATSUVersionType*) styleData);
	styleData += sizeof( ATSUVersionType );
	numAttributes = *((ItemCount*) styleData);
	styleData += sizeof( ItemCount );
	numFeatures = *((ItemCount*) styleData);
	styleData += sizeof( ItemCount );
	numVariations = *((ItemCount*) styleData);
	styleData += sizeof( ItemCount );
	
	while ( numAttributes-- > 0 ) {
		ATSUAttributeTag theTag = *((ATSUAttributeTag*) styleData );
		styleData += sizeof( ATSUAttributeTag );
		ByteCount theSize = *((ByteCount*) styleData );
		styleData += sizeof( ByteCount );
		
		if ( theTag == kATSUFontTag ) {
			char fontName[ kMaximumNameLength ];
			BlockMoveData( styleData, fontName, theSize );
			fontName[ theSize ] = '\0';
			ATSUFontID theFontID;
			returnValue = ATSUFindFontFromName(	fontName,
													theSize,
													kFontUniqueName,
													kFontNoPlatformCode,
													kFontNoScriptCode,
													kFontNoLanguageCode,
								 					&theFontID );
			if ( returnValue == noErr && theFontID != kATSUInvalidFontID ) {
				short macFont;
				Style macStyle;
				returnValue = ATSUFontIDtoFOND( theFontID, &macFont, &macStyle );
				if ( returnValue == noErr && FontToScript( macFont ) == iRunData.script ) {
					element.scrpFont = macFont;
					element.scrpFace |= macStyle;
					}
				}
			}
		else if ( theTag == kATSUSizeTag ) {
			Fixed ATSUISize = *((Fixed*) styleData);
			element.scrpSize = short( Fix2Long( ATSUISize ) );
			}
		else if ( theTag == kATSUColorTag ) {
			element.scrpColor = *((RGBColor*) styleData);
			}
		else if ( theTag == kATSUQDBoldfaceTag ) {
			if ( *((Boolean*) styleData) )
				element.scrpFace |= bold;
			}
		else if ( theTag == kATSUQDItalicTag ) {
			if ( *((Boolean*) styleData) )
				element.scrpFace |= italic;
			}
		else if ( theTag == kATSUQDUnderlineTag ) {
			if ( *((Boolean*) styleData) )
				element.scrpFace |= underline;
			}
		else if ( theTag == kATSUQDCondensedTag ) {
			if ( *((Boolean*) styleData) )
				element.scrpFace |= condense;
			}
		else if ( theTag == kATSUQDExtendedTag ) {
			if ( *((Boolean*) styleData) )
				element.scrpFace |= extend;
			}
		
		styleData += theSize;
		}

	FMetricRec metrics;
	TextFont( element.scrpFont );
	TextSize( element.scrpSize );
	FontMetrics( &metrics );
	element.scrpHeight = short( Fix2Long( metrics.ascent + metrics.descent + metrics.leading ) );
	element.scrpAscent = short( Fix2Long( metrics.ascent ) );
	
	iTEStyleTable->scrpStyleTab[ iTEStyleTable->scrpNStyles++ ] = element;
	return returnValue;
	
}




 
OSStatus
ATSUConvertUTF8ToUnicode( Handle iText, Handle oText, Handle oStyles )
//	Converts the Unicode text in iText from UTF-8 to UTF-16
//	This includes style information
{
	if ( iText == NULL || oText == NULL || oStyles == NULL ) 
		return paramErr;
	
	OSStatus returnValue = noErr;
	SInt8 iTextState = HGetState( iText );
	SInt8 oTextState = HGetState( oText );
	SInt8 oStylesState = HGetState( oStyles );
	
	UniCharCount ucCount = 0;
	TextEncoding ucs2Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicode16BitFormat );
	TextEncoding utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	TECObjectRef encodingConverter;
	returnValue = TECCreateConverter( &encodingConverter, utf8Encoding, ucs2Encoding );
	if ( returnValue == noErr ) {
		ByteCount inputBufferLength = GetHandleSize( iText );
		MoveHHi( iText );
		HLock( iText );
		TextPtr inputBuffer = TextPtr( *iText );
		
		ByteCount outputBufferLength = 4 * inputBufferLength * sizeof( UniChar );	//	Worst case scenario;
																					//	Allows for TEC bidi handling and surrogates
		ByteCount actualInputLength;
		ByteCount actualOutputLength;
		
		HUnlock( oText );
		SetHandleSize( oText, outputBufferLength );
		MoveHHi( oText );
		HLock( oText );
		TextPtr outputBuffer = TextPtr( *oText );
		returnValue = TECConvertText(	encodingConverter,
										inputBuffer, inputBufferLength, &actualInputLength,
										outputBuffer, outputBufferLength, &actualOutputLength );
		HUnlock( oText );
		SetHandleSize( oText, actualOutputLength );
		ucCount = actualOutputLength / sizeof( UniChar );

		//	EXERCISE LEFT TO THE READER:  The TEC is known to insert directinonality overrides
		//		in situations when we may not actually want them.  The TEC also doesn't
		//		normalize line returns to U+2029 PARAGRAPH SEPARATOR, which we may (or 
		//		may not) want -- ATSUI per se doesn't require it.  Do a quick scan-and-
		//		normalize for the text the TEC returns
		
		TECDisposeConverter( encodingConverter );
		}
	
	//	We've got the text; now convert the styles
	
	HUnlock( oStyles );
	SetHandleSize( oStyles, kATSUStyleStreamHeaderSize );
	returnValue = MemError();
	if ( returnValue == noErr ) {
		ATSUStyleStreamHeader* header;
		MoveHHi( oStyles );
		HLock( oStyles );
		header = (ATSUStyleStreamHeader*) *oStyles;
		header->fVersion = 0;
		header->fTextLength = 0;
		header->fLayoutRunCount = 0;
		header->fLayoutRunOffset = 0;
		header->fStyleRunCount = 0;
		header->fStyleRunOffset = kATSUStyleStreamHeaderSize;
		HUnlock( oStyles );
		}
	
	ScrpSTElement stElement;
	stElement.scrpStartChar = 0;
	stElement.scrpFont = SInt16( GetScriptVariable( smSystemScript, smScriptAppFond ) );
	stElement.scrpFace = normal;
	stElement.scrpSize = SInt16( ( GetScriptVariable( smSystemScript, smScriptAppFondSize ) & 0x0FFFF ) );
	stElement.scrpColor.red = 0;
	stElement.scrpColor.green = 0;
	stElement.scrpColor.blue = 0;
	ATSUConvertAndAppendScrpSTElement( oStyles, stElement, ucCount );
	
	HSetState( oStyles, oStylesState );
	HSetState( oText, oTextState );
	HSetState( iText, iTextState );
	return returnValue;
}





OSStatus
ATSUConvertAndAppendScrpSTElement(	Handle iStream, 
									const ScrpSTElement& iSTElement, 
									UniCharCount iRunLength )
//	Takes a single ScrpSTElement and adds it to the end of
//		the ATSUI style stream being accumulated in iStream
//	Note, this function generates the ATSUI style stream information.
//		An alternative approach would have been to allocate an
//		ATSUStyle, set the appropriate attributes in it, 
//		then use ATSUCopyToHandle to create the streamed data.
{

	//	We need to know in advance how long the font name will be
	//		so that we can make sure the total length of the
	//		streamed style is correct
	
	ByteCount oldHandleSize = GetHandleSize( iStream );
	SInt8 streamState = HGetState( iStream );
	ByteCount newHandleSize;
	
	//	Determine what the font is
	
	unsigned char fontName[ kMaximumNameLength ];
	ByteCount fontNameLength = 0;
	ATSUFontID theFontID;
	OSStatus returnValue = ATSUFONDtoFontID( iSTElement.scrpFont, iSTElement.scrpFace, &theFontID );
	if ( returnValue != kATSUNoCorrespondingFontErr && theFontID != kATSUInvalidFontID ) 
		returnValue = ATSUFindFontName( theFontID, 
						kFontUniqueName,
						kFontNoPlatformCode,
						kFontNoScriptCode,
						kFontNoLanguageCode,
						kMaximumNameLength, 
						Ptr( fontName ), 
						&fontNameLength, 
						NULL );
	fontName[ fontNameLength ] = '\0';
		//	We need a total of fontNameLength bytes to stream the font name

	ATSUVersionType version = 0;
	ItemCount numAttributes = 3;	//	At least font, size, and color
	ItemCount numFeatures = 0;
	ItemCount numVariations = 0;
	ByteCount dataLength = sizeof( ATSUVersionType ) + 3 * sizeof( ItemCount );	
								//	room needed for version, 
								//	room needed for number of attributes, 
								//	room needed for number of features, 
								//	room needed for number of variations

	dataLength += sizeof( ATSUAttributeTag ) + sizeof( ByteCount ) + fontNameLength;
								//	room needed for font
	dataLength += sizeof( ATSUAttributeTag ) + sizeof( ByteCount ) + sizeof( Fixed );
								//	room needed for point size
	dataLength += sizeof( ATSUAttributeTag ) + sizeof( ByteCount ) + sizeof( RGBColor );
								//	room needed for color
	
	//	Look for QD styles; make sure we allow for them to be streamed, too
	
	if ( iSTElement.scrpFace & bold ) {
		numAttributes++;
		dataLength += sizeof( ATSUAttributeTag ) + sizeof( ByteCount ) + sizeof( Boolean );
		}
	if ( iSTElement.scrpFace & italic ) {
		numAttributes++;
		dataLength += sizeof( ATSUAttributeTag ) + sizeof( ByteCount ) + sizeof( Boolean );
		}
	if ( iSTElement.scrpFace & underline ) {
		numAttributes++;
		dataLength += sizeof( ATSUAttributeTag ) + sizeof( ByteCount ) + sizeof( Boolean );
		}
	if ( iSTElement.scrpFace & condense ) {
		numAttributes++;
		dataLength += sizeof( ATSUAttributeTag ) + sizeof( ByteCount ) + sizeof( Boolean );
		}
	if ( iSTElement.scrpFace & extend ) {
		numAttributes++;
		dataLength += sizeof( ATSUAttributeTag ) + sizeof( ByteCount ) + sizeof( Boolean );
		}
	
	newHandleSize = oldHandleSize + dataLength + sizeof( ByteCount ) + sizeof( UniCharCount );
	HUnlock( iStream );
	SetHandleSize( iStream, newHandleSize );
	returnValue = MemError();
	if ( returnValue == noErr ) {
		Ptr p;
		MoveHHi( iStream );
		HLock( iStream );
		ATSUStyleStreamHeader* streamHeader = (ATSUStyleStreamHeader*) *iStream;
		ATSUStyleRunStreamHeader* runData = (ATSUStyleRunStreamHeader*) ( *iStream + oldHandleSize );
		streamHeader->fTextLength += iRunLength;
		streamHeader->fStyleRunCount++;
		runData->fRunLength = iRunLength;
		runData->fRunDataSize = dataLength;
		runData->fRunStreamVersion = 0;
		runData->fRunAttributeCount = numAttributes;
		runData->fRunFeatureCount = 0;
		runData->fRunVariationCount = 0;
		p = *iStream + oldHandleSize + sizeof( ATSUStyleRunStreamHeader );

		//	Output the attribute data
		
		Fixed fontSize = Long2Fix( iSTElement.scrpSize );
		ATSUCopyAndMovePtr( kATSUSizeTag, (Ptr) &fontSize, &p, sizeof( Fixed ) );
		ATSUCopyAndMovePtr( kATSUColorTag, (Ptr) &iSTElement.scrpColor, &p, sizeof( RGBColor ) );
		ATSUCopyAndMovePtr( kATSUFontTag, (Ptr) fontName, &p, fontNameLength );

		Boolean tagValue = true;
		if ( iSTElement.scrpFace & bold ) 
			ATSUCopyAndMovePtr( kATSUQDBoldfaceTag, (Ptr) &tagValue, &p, sizeof( Boolean ) );
		if ( iSTElement.scrpFace & italic ) 
			ATSUCopyAndMovePtr( kATSUQDItalicTag, (Ptr) &tagValue, &p, sizeof( Boolean ) );
		if ( iSTElement.scrpFace & underline ) 
			ATSUCopyAndMovePtr( kATSUQDUnderlineTag, (Ptr) &tagValue, &p, sizeof( Boolean ) );
		if ( iSTElement.scrpFace & condense ) 
			ATSUCopyAndMovePtr( kATSUQDCondensedTag, (Ptr) &tagValue, &p, sizeof( Boolean ) );
		if ( iSTElement.scrpFace & extend ) 
			ATSUCopyAndMovePtr( kATSUQDExtendedTag, (Ptr) &tagValue, &p, sizeof( Boolean ) );
		}
	
	return returnValue;

}




void		
ATSUCopyAndMovePtr( ATSUAttributeTag iTag, Ptr iSource, Ptr* iDest, ByteCount iLength )
//	Appends the ATSUI attribute data indicated at the location pointed to by
//		*iDest and updates iDest appropriately
{
	BlockMoveData( &iTag, *iDest, sizeof( ATSUAttributeTag ) );
	*iDest += sizeof( ATSUAttributeTag );
	BlockMoveData( &iLength, *iDest, sizeof( ByteCount ) );
	*iDest += sizeof( ByteCount );
	BlockMoveData( iSource, *iDest, iLength );
	*iDest += iLength;
}




