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

#ifndef _H_CTextWidget
#define _H_CTextWidget
#pragma once

#include <LView.h>
#include <LString.h>
#include <UATSUI.h>

#include "CTextWidgetEventHandler.h"
#include "CTextWidgetDragAndDrop.h"

// ---------------------------------------------------------------------------

class CTWTypingAction;
class CMultiUndoer;

class	CTextWidget : public LView,
				  		public LCommander,
				  		public CTextWidgetEventHandler,
				  		public CTextWidgetDragAndDrop
{
	friend class CTWStyleTextAction;
	friend class CTWCutAction;
	friend class CTWPasteAction;
	friend class CTWInsertAction;
	friend class CTWClearAction;
	friend class CTWTypingAction;
	friend class CTWStyleAction;
	friend class CTextWidgetDragAndDrop;
	friend class CTextWidgetDragAction;

public:
	enum { class_ID = FOUR_CHAR_CODE('mltv') };
	
						CTextWidget();
						
						CTextWidget(
								const SPaneInfo&	inPaneInfo,
								const SViewInfo&	inViewInfo);
								
						CTextWidget( LStream* inStream );
								
	virtual				~CTextWidget();

	const LATSUITextLayout&	GetTextLayout() const
	{
		return mTextLayout;	
	}
	
	virtual void		SetTextPtr(
								const void*			inTextP,
								ByteCount			inTextLength);
	
	#if PP_Uses_CFDescriptor
	
	virtual CFStringRef	CopyCFDescriptor() const;
	
	virtual void		SetCFDescriptor( CFStringRef inStringRef );
	
	#endif
	
	virtual Boolean		ObeyCommand(
								CommandT			inCommand,
								void*				ioParam);
								
	virtual void		FindCommandStatus(
								CommandT			inCommand,
								Boolean&			outEnabled,
								Boolean&			outUsesMark,
								UInt16&				outMark,
								Str255				outName);
								
	virtual Boolean		HandleKeyPress(const EventRecord& inKeyEvent);
	virtual Boolean		HandleKeyPress(const EventRecord& inKeyEvent, const UniChar* uc, UniCharCount ucount);
	virtual OSStatus	ProcessKeyEvent(
								EventHandlerCallRef	inCallRef,
								EventRef			inEventRef);
	virtual OSStatus	HandleKeyEvent(
								EventHandlerCallRef	inCallRef,
								EventRef			inEventRef);

	void				SetMargins(const Rect& margins)
	{
		mMargins = margins;
	}

	void				SetFontNum( short inName );
	void				SetFontName( ConstStringPtr inName );
	
	void				SetFontSize( Fixed inSize );
	
	void				SetFontStyle( Style inStyle, bool add = true );
	void				ModifyFontInfo( SInt32 selStart, SInt32 selEnd, Style inStyle, bool add, const RGBColor& inColor );
	
	void				SetFontColor( const RGBColor& inColor );

	void				SetSpellHighlight(UniCharArrayOffset sel_start, UniCharArrayOffset sel_end, bool isSpelling);

	void				SetAlignment(int align);

	virtual void		SelectAll();

	unsigned long	GetLineCount() const;
	virtual long	GetTextLength() const;

	unsigned long	GetLineOffset(unsigned long line) const;
	unsigned long	GetLineHeight(unsigned long line) const;
	unsigned long	GetFullWidth() const;
	unsigned long	GetFullHeight() const;
	
	Point			GetPoint(SInt32 offset) const;
	SInt32			GetOffset(Point localpt) const;

	void			GetSelection(SInt32* selStart, SInt32* selEnd) const;
	void			SetSelection(SInt32 selStart, SInt32 selEnd);
	void			MoveSelection(SInt32 selStart, SInt32 selEnd);
	Boolean			PointOverSelection(Point localPt);					// See if point over selection

	void SetPasswordMode();

	void SetReadOnly(bool read_only)
	{
		mReadOnly = read_only;
	}
	bool IsReadOnly() const
	{
		return mReadOnly;
	}
	
	void SetRedraw(bool redraw);
	bool IsRedrawOn() const
	{
		return mRedrawOn;
	}

	bool IsSelectable() const
	{
		return mSelectable;
	}
	
	bool IsWordWrap() const
	{
		return mWordWrap;
	}
	
	void SetAllowStyles(bool allow)
	{
		mAllowStyles = allow;
	}
	bool IsStyled() const
	{
		return mAllowStyles;
	}
	
	void			SetDefaultBackground(const RGBColor& color)
	{
		mBackground = color;
		SetBackground(mBackground);
	}
	void			SetBackground(const RGBColor& color);

	void			ClearUndo();

	virtual void		ResizeFrameBy(
								SInt16				inWidthDelta,
								SInt16				inHeightDelta,
								Boolean				inRefresh);

	virtual void		ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta,
									Boolean inRefresh);

	virtual void		MoveBy(	SInt32				inHorizDelta,
								SInt32				inVertDelta,
								Boolean				inRefresh);
	

protected:
	virtual void		FinishCreateSelf();

	virtual void		ActivateSelf();
	virtual void		DeactivateSelf();

	virtual void		BeTarget();
	virtual void		DontBeTarget();
	

	virtual void		AdjustMouseSelf(
								Point				inPortPt,
								const EventRecord&	inMacEvent,
								RgnHandle			outMouseRgn);

	virtual void		CalcPortOrigin();

	virtual Boolean		FocusDraw( LPane* inSubPane = nil );

	virtual void		PortToLocalPoint( Point &ioPoint ) const;
	
	virtual void		LocalToPortPoint( Point &ioPoint) const;
	
	virtual void		DrawSelf();
			void		HighlightSelection();

	virtual void		CountPanels(
								UInt32&				outHorizPanels,
								UInt32&				outVertPanels);
								
	virtual Boolean		ScrollToPanel( const PanelSpec& inPanel );

	virtual void		PrintPanelSelf( const PanelSpec& inPanel );
								
	virtual void		PrepareTextLayout();
	virtual void		AdjustTextLayout();
	virtual void		NeedTextLayout();
	
	virtual void		Click(SMouseDownEvent &inMouseDown);		// Allow background window D&D
	virtual void		ClickSelf( const SMouseDownEvent& inMouseDown );

	virtual void		UserChangedText();
	virtual void		AdjustImageToText();

	virtual void		DoCut();
	virtual void		DoCutAction();
	virtual void		DoCopy();
	virtual void		DoCopyAction();
	virtual void		DoPaste();
	virtual void		DoPasteAction();
	virtual void		DoClear();
	virtual void		DoClearAction();

protected:
	typedef struct SSelectedBlock
	{
		UniCharArrayOffset		selStart;		// start of the selected block in the line
		UniCharCount			selLength;		// length of the selected block
		SPoint32				imageLineSel;	// image coords of the line start at x
		ItemCount				lineNumber;		// which line is this?
	};
	typedef struct SSelectionInfo
	{
		bool				withHilight;
		UniCharArrayOffset	anchor;
		UniCharArrayOffset	caret;
		UniCharCount		length;
		bool				leading;
		ItemCount			numberOfSelectedLines;
		SSelectedBlock*		selBlocks;
	
		long lastClickTime;					// time and Point of last click to check
		Point lastClickPoint;				// for double-click
		long lastDblClickTime;				// time and Point of last double-click to check
		Point lastDblClickPoint;			// for triple-click
		
		EventLoopTimerRef	timer;
		bool				caretVisible;
		SRect32				lastCaret;
		UInt32				caretTime;		// Last time caret was blinked

		SSelectionInfo()
		{
			withHilight = false;
			caret = 0;
			length = 0;
			leading = true;
			numberOfSelectedLines = 0;
			selBlocks = NULL;
			
			timer = NULL;
			caretVisible = false;
			lastCaret.left = lastCaret.top = lastCaret.right = lastCaret.bottom = 0;
			caretTime = 0;
		}
		~SSelectionInfo()
		{
			delete[] selBlocks;
		}
	};

	class CATSUITextLayout : public LATSUITextLayout
	{
	public:
		CATSUITextLayout() {}

		ItemCount DetermineLineBreaksFast(ATSUTextMeasurement inLineWidth, UniCharCount inTextLength);
		void MeasureLinesMore(
							UniCharCount			inTextLength,
							ATSUTextMeasurement		inLineWidth,
							ItemCount&				outLineCount,
							std::vector<UniCharArrayOffset>&		outLineBreaks,
							std::vector<unsigned long>&			outLineHeights,
							std::vector<unsigned long>&			outLineDescents,
							std::vector<unsigned long>&			outLineOffsets);
	};

	CGContextRef		mCurrentContext;
	CATSUITextLayout	mTextLayout;
	LATSUIStyle			mStyle;
	RGBColor			mBackground;
	RGBColor			mCurrentBackground;
	
	UniChar*			mTextPtr;
	UniChar*			mHiddenPtr;
	UniCharCount		mTextLength;
	UniCharCount		mTextCapacity;

	SSelectionInfo		mSelection;
	
	ItemCount					mLineCount;
	std::vector<UniCharArrayOffset>	mLineBreaks;
	std::vector<unsigned long>		mLineHeights;
	std::vector<unsigned long>		mLineDescents;
	std::vector<unsigned long>		mLineOffsets;

	Rect				mMargins;

	bool				mPasswordMode;
	bool				mReadOnly;
	bool				mSelectable;
	bool				mWordWrap;
	bool				mAllowStyles;
	bool				mRedrawOn;
	bool				mAdjustPending;
	
	bool				mPendingDrawCaret;

	LATSUIStyle*		mPendingStyle;

	CMultiUndoer*		mUndoer;
	CTWTypingAction*	mTypingAction;
	
	TSMDocumentID		mTSMDocument;
	
	ItemCount OffsetToLine(UniCharArrayOffset offset) const;

	ItemCount PointToLine(Point inLocalPt) const;
	ItemCount PointToLine(SPoint32 inImagePt) const;

	UniCharArrayOffset PointToOffset(Point inLocalPt, bool& leading) const;
	UniCharArrayOffset PointToOffset(SPoint32 inImagePt, bool& leading) const;
	
	Point OffsetToPoint(UniCharArrayOffset offset, bool leading = true) const;
	SPoint32 OffsetToImagePoint(UniCharArrayOffset offset, bool leading = true) const;
	
	SPoint32 MarginToImagePoint(const Point& inLocalPt) const;
	Point ImageToMarginPoint(const SPoint32& pt) const;
	SPoint32 ImageToMarginPoint32(const SPoint32& pt) const;
	SRect32 ImageToMarginRect32(const SRect32& rect) const;
	void CalcMarginFrameRect(Rect& frame) const;


	// Selection related
	bool GotDoubleClick(const EventRecord& theEvent);
	bool GotTripleClick(const EventRecord& theEvent);
	void SelectLine(UniCharArrayOffset thePos, bool leading, UniCharArrayOffset* startSel, UniCharArrayOffset* endSel);
	void SelectWord(UniCharArrayOffset thePos, bool leading, UniCharArrayOffset* startSel, UniCharArrayOffset* endSel);
	void ModifySelection(UniCharArrayOffset newStart, UniCharArrayOffset newEnd);
	void SetSelectedBlocks();
	void HilightCurrentSelection(UniCharArrayOffset startLine, UniCharArrayOffset endLine);
	void UnhilightCurrentSelection(UniCharArrayOffset startLine, UniCharArrayOffset endLine);
	void ShowSelection(bool refresh);

	static pascal void IdleTimer(EventLoopTimerRef inTimer, void* inUserData);
	void Idle(UInt32* maxSleep);
	void BlinkCaret();
	void DrawCaret(bool on);

	void GetHiliteRgn(RgnHandle rgn);

	// Keyboard related
	void DoArrowKey(UniChar key, EventModifiers mods);
	UniCharArrayOffset OffsetArrowKey(UniCharArrayOffset start, UniChar key, EventModifiers mods);
	void DoScrollKey(UniChar key, EventModifiers mods);
	
	void InsertText(const UniChar* text, UniCharCount length, bool undoable = true);
	void InsertTextAndStyles(const UniChar* text, UniCharCount length, Handle styles, bool undoable = true);
	void InsertText(const UniChar* text, UniCharCount length, UniCharArrayOffset offset);
	void InsertTextAndStyles(const UniChar* text, UniCharCount length, Handle styles, UniCharArrayOffset offset);
	void DeleteText(UniCharArrayOffset offset, UniCharCount length);

	void GetBreakInfo(UniCharArrayOffset start, UniCharArrayOffset end,
						ItemCount start_line, ItemCount end_line,
						std::vector<UniCharArrayOffset>& outLineBreaks);

	void GetLineInfo(ItemCount start_line, ItemCount end_line,
							const std::vector<UniCharArrayOffset>& lineBreaks,
							std::vector<unsigned long>&			outLineHeights,
							std::vector<unsigned long>&			outLineDescents,
							std::vector<unsigned long>&			outLineOffsets);

	// Text conversion
	Handle GetTextRangeAs_utxt() const
	{
		return GetTextRangeAs_utxt(mSelection.caret, mSelection.length);
	}
	Handle GetTextRangeAs_utxt(UniCharArrayOffset start, UniCharCount length) const;
	Handle GetTextRangeAs_TEXT() const
	{
		return GetTextRangeAs_TEXT(mSelection.caret, mSelection.length);
	}
	Handle GetTextRangeAs_TEXT(UniCharArrayOffset start, UniCharCount length) const;
	Handle GetTextRangeAs_TEXT(Handle utxt) const;
	Handle GetStyleRangeAs_ustl() const
	{
		return GetStyleRangeAs_ustl(mSelection.caret, mSelection.length);
	}
	Handle GetStyleRangeAs_ustl(UniCharArrayOffset start, UniCharCount length) const;

private:
	void InitTextWidget();

	void NewCapacity(UniCharCount capacity);
	
	void CreatePendingStyle();

	UniChar* GetTextPtr() const
	{
		return mPasswordMode ? mHiddenPtr : mTextPtr;
	}
};


#endif
