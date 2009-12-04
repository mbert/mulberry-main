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


// Header for CINETUtils class

#ifndef __CINETUTILS__MULBERRY__
#define __CINETUTILS__MULBERRY__

class CINETUtils
{
public:

	static bool HasInet();
	static bool HasModem();
	static bool WillDial();
	static bool AutoDial();
	static bool AutoHangup();

	static bool GetInterfaceIPs(unsigned long*& ips);

private:
	enum EInterfaceTest
	{
		eTestINETUp,
		eTestModemUp
	};

	CINETUtils() {}
	~CINETUtils() {}

	static bool InterfaceTest(EInterfaceTest test);
};

#endif