// ===========================================================================
//	LUnicodeEditText.h					PowerPlant 2.2.2	©1997-2000 Metrowerks Inc.
// ===========================================================================

#ifndef _H_LUnicodeEditText
#define _H_LUnicodeEditText
#pragma once

#include <LControlPane.h>
#include <LCommander.h>

#include <SysCFString.h>

#if PP_Uses_Pragma_Import
	#pragma import on
#endif

PP_Begin_Namespace_PowerPlant

// ---------------------------------------------------------------------------

class LUnicodeEditText : public LControlPane,
				  public LCommander {

public:
	enum {	class_ID		= FOUR_CHAR_CODE('utxt'),
			imp_class_ID	= FOUR_CHAR_CODE('iutx') };

						LUnicodeEditText(
								LStream*		inStream,
								ClassIDT		inImpID = imp_class_ID);

						LUnicodeEditText(
								const SPaneInfo&	inPaneInfo,
								LCommander*			inSuperCommander,
								ConstStringPtr		inInitialText,
								ResIDT				inTextTraitsID,
								MessageT			inMessage,
								SInt16				inMaxChars,
								UInt8				inAttributes,
								TEKeyFilterFunc		inKeyFilter,
								bool				inPasswordField = false,
								ClassIDT			inImpID = imp_class_ID);

	virtual				~LUnicodeEditText();

	virtual void			SetDescriptor( CFStringRef inText )
	{
		SetText(inText);
	}
	virtual PPx::CFString	GetDescriptor() const
	{
		return GetText();
	}

	virtual void			SetText( CFStringRef inText );
	virtual PPx::CFString	GetText() const;

	virtual void		SetTextTraitsID( ResIDT inTextTraitsID );
	virtual ResIDT		GetTextTraitsID() const;

	void				SetSelection( ControlEditTextSelectionRec& inSelection );

	void				SelectAll();

	virtual void		GetSelection( ControlEditTextSelectionRec& outSelection ) const;

	virtual	void		GetSelection( AEDesc& outDesc ) const;
	
	virtual bool		HasSelection() const;

	SInt16				GetMaxChars() const		{ return mMaxChars; }

	void				SetMaxChars( SInt16 inMaxChars )
							{
								mMaxChars = inMaxChars;
							}

	virtual Boolean		HandleKeyPress( const EventRecord& inKeyEvent );

	virtual Boolean		ObeyCommand(
								CommandT			inCommand,
								void*				ioParam);

	virtual void		FindCommandStatus(
								CommandT			inCommand,
								Boolean&			outEnabled,
								Boolean&			outUsesMark,
								UInt16&				outMark,
								Str255				outName);

	virtual void		UserChangedText();

protected:
	SInt16				mMaxChars;
	ResIDT				mTextTraitsID;
	bool				mHasWordWrap;
	bool				mIsPassword;

	virtual void		HideSelf();

	virtual void		ActivateSelf();
	virtual void		DeactivateSelf();

	virtual void		EnableSelf();
	virtual void		DisableSelf();

	virtual void		ClickSelf( const SMouseDownEvent& inMouseDown );

	virtual void		BeTarget();
	virtual void		DontBeTarget();

	virtual Boolean		TooManyCharacters( SInt32 inCharsToAdd );

private:
						LUnicodeEditText();
						LUnicodeEditText( const LUnicodeEditText& );
	LUnicodeEditText&			operator = ( const LUnicodeEditText& );

};

PP_End_Namespace_PowerPlant


#if PP_Uses_Pragma_Import
	#pragma import reset
#endif

#endif
