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


// Header for password change client class

#ifndef __CPSWDCHANGECLIENT__MULBERRY__
#define __CPSWDCHANGECLIENT__MULBERRY__

#include "CINETClient.h"

// consts

class CPswdChangePlugin;

class CPswdChangeClient: public CINETClient
{

	// I N S T A N C E  V A R I A B L E S

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
			CPswdChangeClient();
	virtual	~CPswdChangeClient();

private:
			void	InitPOPPASSDClient();

public:
	virtual void	ChangePassword(CPswdChangePlugin* plugin);

protected:
	tcp_port mServerPort;

	virtual tcp_port GetDefaultPort();						// Get default port;
};

#endif
