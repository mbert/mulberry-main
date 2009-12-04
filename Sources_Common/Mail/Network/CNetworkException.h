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


// CNetworkException

#ifndef __CNETWORKEXCEPTION__MULBERRY__
#define __CNETWORKEXCEPTION__MULBERRY__

#include <exception>

class CNetworkException : public std::exception
{
public:
	enum { class_ID = 'ntwk' };

	CNetworkException(int err_code) :
		_err(err_code), _class(class_ID), _handled(false), _disconnect(false), _reconnect(false) {}
	virtual ~CNetworkException() throw() {}

	int error(void)
		{ return _err; }
	int type(void) const
		{ return _class; }

	bool handled(void) const
		{ return _handled; }
	void sethandled(void)
		{ _handled = true; }

	bool disconnected(void)
		{ return _disconnect; }
	void setdisconnect(void)
		{ _disconnect = true; }

	bool reconnected(void)
		{ return _reconnect; }
	void setreconnect(void)
		{ _reconnect = true; }

protected:
	int _err;
	int _class;
	bool _handled;
	bool _disconnect;
	bool _reconnect;
};

#endif
