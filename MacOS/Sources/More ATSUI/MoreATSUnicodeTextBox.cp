/*
	File:		MoreATSUnicodeTextBox.cp

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
				10/14/1999	AD				Macro names changed
											use ATSUGetGlyphBounds to compute line height
				 7/27/1999	KG				Updated for Metrowerks Codewarror Pro 2.1				
				 7/01/1998	AD				Created
				

*/



/*________ Includes ________________________________________________________________*/

#ifdef __MWERKS__

// includes for MetroWerks CodeWarrior

#include <Fonts.h>	// GetSysFont(), GetAppFont()
#include <Script.h>	// GetSysDirection()
#include <FixMath.h>
#include <TextEdit.h>

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

#include "MoreATSUnicode.h"
#include "MoreATSUnicodeInternal.h"

// 	converts a TextEdit based alignment to 
//	an ATSUI flush factor
static Fract atsuFlushFactorFromTEAlignment(short alignment)
{
	Fract flushFactor = kATSUStartAlignment;
	
	switch (alignment)
	{
		case teFlushRight:
			flushFactor = kATSUEndAlignment;
			break;
			
		case teCenter:
			flushFactor = kATSUCenterAlignment;
			break;

		case teFlushLeft:
			flushFactor = kATSUStartAlignment;
			break;
		default:
			// teFlushDefault
			flushFactor = (GetSysDirection() == 0) ? kATSUStartAlignment : kATSUEndAlignment;
			break;
	}
	return flushFactor;
}


enum {
		kVerticalTextAngle = IntToFixed(-90)
	};

/*
	Draw the text layout within the specified rect.
*/
OSStatus atsuDrawTextInBox(	ATSUTextLayout 		iTextLayout,
							const Rect *			iBox)
{
	OSStatus 			theStatus = noErr;
	UniCharArrayOffset	textOffset = 0;
	UniCharCount		textLength = 0;
	UniCharArrayOffset*	lineEndOffsets = NULL;
	
	require (iBox != NULL, EXIT);
	
	// the the range of text to be drawn
	theStatus = ATSUGetTextLocation (iTextLayout, NULL, NULL, &textOffset, &textLength, NULL);
	if (theStatus == noErr) {
		UniCharArrayOffset		lineStartOffset = textOffset;
		UniCharArrayOffset		lineEndOffset = 0;
		
		//	assume horizontal text values
		ATSUTextMeasurement		xPos = IntToFixed(iBox->left);
		ATSUTextMeasurement		yPos = IntToFixed(iBox->top);
		ATSUTextMeasurement		lineStart = xPos;
		ATSUTextMeasurement		lineEnd = IntToFixed(iBox->right);
		ATSUTextMeasurement		lineWidth = 0;
		
		Fixed					textAngle = 0;
		ItemCount				lineCount = 0;
		ItemCount				softBreakCount = 0;
		ATSUTextMeasurement		maxAscent = 0, maxDescent = 0;
		
		int ln = 0;

		//	need to get the text rotation for drawing
		theStatus = atsuGetLayoutRotation(iTextLayout, &textAngle);
		require(theStatus == noErr, EXIT);
		
		// is the text not horizontal?
		if (textAngle != 0) {
			// if it is not horizontal, we only know how to draw vertical
			require (textAngle == kVerticalTextAngle, EXIT);
			xPos = 	IntToFixed(iBox->right);
			lineStart = yPos;
			lineEnd = IntToFixed(iBox->bottom);
		}

		// check for linewidth set as a layout control
		theStatus = atsuGetLayoutWidth(iTextLayout, &lineWidth);
		require(theStatus == noErr, EXIT);
		
		//	if there is no layout control set for width
		//	then set it using the box bounds
		if (lineWidth == 0)
			lineWidth = lineEnd - lineStart;
		
		//	Break and measure each line to determine the max ascent and descent;
		//	This is needed because things that end up on different lines
		//	could interact to affect the line height when they're on the same line.
		//	To get the best value we break the lines first, then measure each
		//	line and find the max ascent and descent.
		while (lineStartOffset < textOffset + textLength) {
			ATSUTextMeasurement		ascent = 0, descent = 0;
			
			// set the soft breaks, we will use them later
			theStatus = ATSUBreakLine(iTextLayout, lineStartOffset, lineWidth, true, &lineEndOffset);
			require(theStatus == noErr, EXIT);
			
			// @@@ If you want leave lines that are terminated with a hard break unjustified, this is a
			// good place to set the individual line control to do that.

#if (ATSU_TARG_VERSION >= ATSU_1_1)
			// ATSUGetGlyphBounds is better than ATSUMeasureText if you've got any interesting feature
			// such as justification turned on. The former will do the layout exactly as it will be drawn and cache
			// the result, so things will be faster overall. ATSUMeasureText will turn off justification and end up doing
			// an "extra" layout operation.
			{
				ATSTrapezoid	glyphBounds;	// one should be enough when we're asking for the whole line.
				
				theStatus = ATSUGetGlyphBounds( iTextLayout, 0, 0, lineStartOffset, lineEndOffset - lineStartOffset, kATSUseFractionalOrigins,
												1, &glyphBounds, NULL );
				require(theStatus == noErr, EXIT);
				
				// The top and bottom of the bounds should be parallel to the baseline. You might want to check that.
				if (textAngle == 0) {
					ascent = -glyphBounds.upperLeft.y;
					descent = glyphBounds.lowerLeft.y;
				} else {
					// This looks strange, but the box you get is rotated, so "upper left" relative to the line is "upper right" in absolute terms.
					ascent = glyphBounds.upperLeft.x;
					descent = -glyphBounds.lowerLeft.x;
				}
			}
#else
			theStatus = ATSUMeasureText(iTextLayout, lineStartOffset, lineEndOffset - lineStartOffset, NULL, NULL, &ascent, &descent);
			require(theStatus == noErr, EXIT);
#endif

			if (ascent > maxAscent)
				maxAscent = ascent;
			if (descent > maxDescent)
				maxDescent = descent;
				
			lineStartOffset = lineEndOffset;
			lineCount++;
		}
		
		lineEndOffsets = (UniCharArrayOffset*) NewPtr(lineCount * sizeof(UniCharArrayOffset));
		
		theStatus = MemError();
		require(theStatus == noErr, EXIT);
		
		theStatus = ATSUGetSoftLineBreaks(
			iTextLayout, textOffset, textLength, lineCount, lineEndOffsets, &softBreakCount);
		//	assert that the number of soft breaks is always one less than the number of lines
		//	since ATSUBreakLine does not insert a softbreak at the end of the text.
		require((theStatus == noErr && softBreakCount == lineCount - 1), EXIT);

		lineEndOffsets[softBreakCount] = textOffset + textLength;
		lineStartOffset = textOffset;

		// @@@ If maxAscent or maxDescent are not integers, this code may produce uneven line spacing. It will also
		// not necessarily match the line height used by ATSUI when highlighting. ATSUI highlighting uses
		// FixedToInt( baseline - ascent ) for the top of the box and FixedToInt( baseline + descent ) for the bottom.
		// Any other combination of reflection and rounding can result in one-pixel gaps and overlaps.
		
		// If you're using ATSUGetGlyphBounds above, you could ask for kATSUseDeviceOrigins to have ATSUI do this transformation for you.
		
		//	draw each line
		for (ln = 0; ln < lineCount; ln++) {
			lineEndOffset = lineEndOffsets[ln];
			
			// predecrement to account for the line height
			if (textAngle == kVerticalTextAngle)
				xPos -= maxAscent;
			else
				yPos += maxAscent;

			theStatus = ATSUDrawText(iTextLayout, lineStartOffset, lineEndOffset - lineStartOffset, xPos,  yPos);
			require(theStatus == noErr, EXIT);

			lineStartOffset = lineEndOffset;
			
			if (textAngle == kVerticalTextAngle)
				xPos -= maxDescent;
			else
				yPos += maxDescent;
		}
	}
	
EXIT:
	if (lineEndOffsets != NULL) 
		DisposePtr((char*)lineEndOffsets);	
	return theStatus;
}


/*
	atsuTextBox expands the functionality of TETextBox by adding
	justification and vertical text.
	
	atsuTextBox first sets up a ATSUTextLayout. It then draws the text
	using atsuDrawTextInBox
	
*/

enum {
	kInitialStyleAllocation = 10
	};

OSStatus atsuTextBox(	ConstUniCharArrayPtr 		iText,
						UniCharCount 				iTextLength,
						const Rect *				iBox,
						short 						alignment,
						Fract						justification,
						short						textOrientation,
						Boolean						useFontFallBacks)
{
	OSStatus		theStatus = 0;
	ATSUStyle		globalStyle = NULL;
	ATSUTextLayout	txLayout = NULL;
	
	int				substituteStyleCount = 0;
	int				maxStyleCount = kInitialStyleAllocation;
	ATSUStyle**		substituteStyles = NULL;
	
	theStatus = ATSUCreateStyle(&globalStyle);
	if (theStatus == noErr) 
	{
		//	Initialize the style with the current port attributes
		theStatus = atsuSetStyleFromGrafPtr(globalStyle, NULL, false);
		require(theStatus == noErr, EXIT);

		//	Don't want to have hanging punctuation drawn outside of the rectangle,
		//	so inhibit that feature. (It's allowed by default)
		theStatus = atsuSetHangingInhibitFactor(globalStyle, ((Fract) 0x00000001L));
		
		if (theStatus == noErr) 
		{
		
			theStatus = ATSUCreateTextLayoutWithTextPtr( iText, 0, iTextLength, iTextLength,
													1, &iTextLength, &globalStyle,
													&txLayout);
													
			if (theStatus == noErr) //	Set layout controls and draw
			{
				UniCharArrayOffset 	replaceOffset = 0;
				UniCharCount		replaceLength = iTextLength;
				
				//	Could have had atsuTextBox use the ATSUI flushFactors directly
				//	This is just to illustrate how to convert from the
				//	TE equivalents
				Fract	flushFactor = atsuFlushFactorFromTEAlignment(alignment);
				
				//	a slight optimization since we know that this
				//	text is display only
				theStatus = atsuSetLayoutOptions(txLayout, kATSLineIsDisplayOnly);
				require(theStatus == noErr, EXIT);

				//	Set the flush and justification if they are other than the 
				//	default
				if (flushFactor != kATSUStartAlignment || justification != kATSUNoJustification)
				{
					Fixed	lineWidth = IntToFixed(iBox->right-iBox->left);
					
					if (textOrientation == katsuVerticalText)
						lineWidth = IntToFixed(iBox->bottom-iBox->top);
									
					//	line width must be set for flush and justification to work
					theStatus = atsuSetLayoutWidth(txLayout, lineWidth);				
					require(theStatus == noErr, EXIT);

					theStatus = atsuSetLayoutFlushFactor(txLayout, flushFactor);		
					require(theStatus == noErr, EXIT);
							
					theStatus = atsuSetLayoutJustFactor(txLayout, justification);
					require(theStatus == noErr, EXIT);
					
					
					// don't justify the last line
					theStatus = atsuSetLayoutOptions(txLayout, kATSLineLastNoJustification);
					require(theStatus == noErr, EXIT);
				}
				
				if (textOrientation == katsuVerticalText)
				{
					// set the character orientation
					theStatus = atsuSetVerticalCharacter(globalStyle, kATSUStronglyVertical);
					require(theStatus == noErr, EXIT);
					
					//	rotate the text
					theStatus = atsuSetLayoutRotation(txLayout, kVerticalTextAngle);
					require(theStatus == noErr, EXIT);
				}

				//	If the caller specifies useFontFallBacks
				//	Check to see if there are any characters that can't be drawn
				//	with the font that came in from the port.
				//	Use ATSUMatchFontsToText to find problem subranges
				//	Explicitly replacing the font will be less than the overhead
				//	of setting transient font matching for the entire layout.
				if (useFontFallBacks)
				{
					Boolean	needTransientMatching = false;
					Boolean	doneWithSubstitution = false;
					
					while (!doneWithSubstitution)
					{
						ATSUFontID	replaceFont;
						
						theStatus = ATSUMatchFontsToText(
										txLayout, replaceOffset, replaceLength,
										&replaceFont, &replaceOffset, &replaceLength);
										
						switch (theStatus)
						{
							case kATSUFontsMatched:
							{
								int			i = 0;
								ATSUStyle	replacementStyle = NULL;
								
								//	"kATSUFontsMatched" means that some of the characters cannot
								//	be drawn with the font, but a suitable subsitute font
								//	is active in the system
								
								//	before we create a new style, see if we already have a style with this font
								for (i = 0; i < substituteStyleCount && !replacementStyle; i++)
								{
									ATSUFontID	thisFont;
									atsuGetFont((*substituteStyles)[i], &thisFont);
									if (thisFont == replaceFont)
									{
										replacementStyle = (*substituteStyles)[i];
									}
								}
								
								if (!replacementStyle)
								{
									//	need to create a new one, cloning the global style
									theStatus = ATSUCreateAndCopyStyle(globalStyle, &replacementStyle);
									require(theStatus == noErr, EXIT);
									
									theStatus = atsuSetFont(replacementStyle, replaceFont);
									require(theStatus == noErr, EXIT);
									
									//	add it to the list
									if (substituteStyles == NULL)
									{
										maxStyleCount = kInitialStyleAllocation;
										substituteStyles = (ATSUStyle**) NewHandle(maxStyleCount * sizeof(ATSUStyle));
										HLock((Handle) substituteStyles);
										require ((substituteStyles != NULL && MemError() == noErr), EXIT);
									} else {
										// do we need to expand our array of styles?
										if (substituteStyleCount > maxStyleCount)
										{
											maxStyleCount += kInitialStyleAllocation;
											HUnlock((Handle) substituteStyles);
											SetHandleSize((Handle) substituteStyles, maxStyleCount * sizeof(ATSUStyle));
											HLock((Handle) substituteStyles);
											theStatus = MemError();
											require(theStatus == noErr, EXIT);
										}
									}
									(*substituteStyles)[substituteStyleCount++] = replacementStyle;
								}

								theStatus = ATSUSetRunStyle(txLayout, replacementStyle, replaceOffset, replaceLength);
								require (theStatus == noErr, EXIT);
								break;
							}
							case kATSUFontsNotMatched:
							{
								//	kATSUFontsNotMatched" means that some of the characters cannot
								//	be drawn with any currently active font.
								//	this will tell us to turn on transient font matching 
								//	and pick up glyphs from the last resort font 
								needTransientMatching = true;
								break;
							}
							default:
								//	done with any other status
								doneWithSubstitution = true;
								break;
						}
						replaceOffset += replaceLength;
						replaceLength = iTextLength - replaceOffset;
					} 
					require(theStatus == noErr, EXIT);
					
					if (needTransientMatching)
					{
						theStatus = ATSUSetTransientFontMatching(txLayout, true);
						require(theStatus == noErr, EXIT);
					}
				}
				
				//	Finally, this is where the drawing happens
				theStatus = atsuDrawTextInBox(txLayout, iBox);
			}
		}
	}

EXIT:
	if (globalStyle)
	{
		ATSUDisposeStyle(globalStyle);
	}
	
	if (substituteStyles)
	{
		int i;
		for (i = 0; i < substituteStyleCount; i++)
		{
			ATSUDisposeStyle((*substituteStyles)[i]);
		}
		HUnlock((Handle) substituteStyles);
		DisposeHandle((Handle) substituteStyles);	

	}
	if (txLayout) {
		ATSUDisposeTextLayout(txLayout);
	}
	return theStatus;
}


