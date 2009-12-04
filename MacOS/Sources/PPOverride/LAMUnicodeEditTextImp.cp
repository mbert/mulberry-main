// ===========================================================================
//	LAMUnicodeEditTextImp.cp			PowerPlant 2.2.2	©1997-2001 Metrowerks Inc.
// ===========================================================================
//
//	Implementation for Appearance Manager EditText item.
//
//	The class does not create a ControlHandle and it inherits from
//	LControlImp rather than LAMControlImp.
//
//	The LEditText ControlPane handles everything related to the text.
//	The only thing this class does is draw the frame and focus ring
//	around the text area.

#ifdef PowerPlant_PCH
	#include PowerPlant_PCH
#endif

#include <LAMUnicodeEditTextImp.h>

PP_Begin_Namespace_PowerPlant


// ---------------------------------------------------------------------------
//	¥ LAMUnicodeEditTextImp						Stream Constructor		  [public]
// ---------------------------------------------------------------------------
//	Do not access inStream. It may be nil. Read Stream data in Init()

LAMUnicodeEditTextImp::LAMUnicodeEditTextImp(
	LStream*	inStream)

	: LAMControlImp(inStream)
{
}


// ---------------------------------------------------------------------------
//	¥ ~LAMUnicodeEditTextImp						Destructor				  [public]
// ---------------------------------------------------------------------------

LAMUnicodeEditTextImp::~LAMUnicodeEditTextImp()
{
}



PP_End_Namespace_PowerPlant
