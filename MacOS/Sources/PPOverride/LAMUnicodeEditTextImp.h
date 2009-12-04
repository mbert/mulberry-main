// ===========================================================================
//	LAMUnicodeEditTextImp.h			PowerPlant 2.2.2	©1997-2000 Metrowerks Inc.
// ===========================================================================

#ifndef _H_LAMUnicodeEditTextImp
#define _H_LAMUnicodeEditTextImp
#pragma once

#include <LAMControlImp.h>

#if PP_Uses_Pragma_Import
	#pragma import on
#endif

PP_Begin_Namespace_PowerPlant

// ---------------------------------------------------------------------------

class	LAMUnicodeEditTextImp : public LAMControlImp {
public:
						LAMUnicodeEditTextImp( LStream* inStream = nil );

	virtual				~LAMUnicodeEditTextImp();

protected:
};

PP_End_Namespace_PowerPlant


#if PP_Uses_Pragma_Import
	#pragma import reset
#endif

#endif
