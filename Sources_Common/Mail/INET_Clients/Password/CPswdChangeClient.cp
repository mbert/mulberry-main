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


// Code for password changing client class

#include "CPswdChangeClient.h"

#include "CINETCommon.h"
#include "CPswdChangePlugin.h"
#include "CStatusWindow.h"

#if __framework != __powerplant
#include "StValueChanger.h"
#endif

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CPswdChangeClient::CPswdChangeClient()
{
	// Create the TCP stream
	mStream = new CTCPStream;

	// Init instance variables
	InitPOPPASSDClient();

}

CPswdChangeClient::~CPswdChangeClient()
{
}

void CPswdChangeClient::InitPOPPASSDClient()
{
	// Init instance variables
	mLogType = CLog::eLogPlugin;
	mServerPort = 0;

}

// Get default port
tcp_port CPswdChangeClient::GetDefaultPort()
{
	// TCP/IP-based sub-classes must deal with this
	return mServerPort;
}

// Do Password change
void CPswdChangeClient::ChangePassword(CPswdChangePlugin* plugin)
{

	// Set default port only on non-copy construction
	bool connection_up = false;

	try
	{
		// Set the server address
		mServerAddr = plugin->GetServerIP();
		mServerPort = plugin->GetServerPort();

		// Make connection to server
		Open();

		// Start login action
		StINETClientAction status(this, "Status::INET::LoggingOn", "Error::INET::OSErrLogon", "Error::INET::NoBadLogon");

		// Init TCP
		mStream->TCPOpen();

		// Make connection with server
		mStream->TCPStartConnection();

		// Flag connection
		connection_up = true;

		// Do not allow logging of auth details
		StValueChanger<bool> value(mAllowLog, CLog::AllowAuthenticationLog());

		// Now get plugin to do the change
		plugin->ProcessPswdChange(*mStream, mLog, mLineData, cINETBufferLen);

		// Break connection with server
		mStream->TCPCloseConnection();
	}
	catch (CNetworkException& ex)
	{
		CLOG_LOGCATCH(CNetworkException&);

		// Handle error
		mLastResponse.tag_msg = mLineData;
		INETDisplayError(ex, "Error::INET::OSErrLogon", "Error::INET::NoBadLogon");
		ex.sethandled();

		// Disconnect if connected
		if (connection_up)
			mStream->TCPCloseConnection();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);


		// Handle error
		INETDisplayError(CINETException::err_INETUnknown, "Error::INET::OSErrLogon", "Error::INET::NoBadLogon");

		// Disconnect if connected
		if (connection_up)
			mStream->TCPCloseConnection();
	}
}
