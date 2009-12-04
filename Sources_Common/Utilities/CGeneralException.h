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


// CGeneralException

#ifndef __CGENERALEXCEPTION__MULBERRY__
#define __CGENERALEXCEPTION__MULBERRY__

#if __framework == __powerplant

// Mac uses Powerplant LException which is derived from std::exception
typedef PP_PowerPlant::LException CGeneralException;

#else

#include <exception>

#include "cdstring.h"

class CGeneralException : public std::exception
{
public:
	CGeneralException(long err_code) :
		_err(err_code) {}
	CGeneralException(long err_code, const char* txt) :
		_err(err_code), _txt(txt) {}
	virtual ~CGeneralException() throw() {}

	long GetErrorCode() const
		{ return _err; }
	const cdstring& GetErrorText() const
		{ return _txt; }

protected:
	long _err;
	cdstring _txt;
};

#endif

#endif
