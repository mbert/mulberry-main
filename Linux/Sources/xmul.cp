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

#include "CMulberryApp.h"
#include "CMulberryMDIServer.h"
#include "CLog.h"
#include "CUtils.h"

#include <exception>
#include <iostream>

#include <JXDisplay.h>
#include <JXMenu.h>
#include <JXTEBase.h>
#include <JXTEBase16.h>
#include <JXWindow.h>
#include <jXGlobals.h>

#ifdef _J_USE_XFT
#include <X11/Xft/Xft.h>
#endif

JXApplication* globalApp;

void ParseTextOptions(const int argc, char* argv[]);
void PrintHelp();
void PrintVersion();

bool xdebug = false;

int main(int argc, char* argv[])
{
	// Always turn off JX asserts here - command line option may turn it back on
	JAssertBase::SetAction(JAssertBase::kIgnoreFailure);

	// Deal withh basic options
	ParseTextOptions(argc, argv);

	// Deal with MDI behaviour
	if (!CMulberryMDIServer::WillBeMDIServer(CMulberryApp::GetAppSignature(), argc, argv))
		return 0;

	try
	{
#ifdef _J_USE_XFT
		if (!XftInit(NULL) || !XftInitFtLibrary())
		{
			cerr << "Could not initialise Xft library" << std::endl;
			throw -1;
		}
#endif

		// Turn on multi-threaded X
		::XInitThreads();

		CMulberryApp* app = new CMulberryApp(&argc, argv);
		
		// Turn on XError sync capability if requested
		if (xdebug)
			::XSynchronize(*app->GetCurrentDisplay(), true);

		JXCreateDefaultDocumentManager(kFalse);
		globalApp = app;

		// Changing these "global" values has to come _AFTER_ the app is created
		// because JX reads it's prefs files when the app object is created

		// Set WM_COMMAND string for all windows
		JXWindow::SetDefaultWMCommand("mulberry");

		//Set menu style to Windows
		JXMenu::SetDefaultStyle(JXMenu::kWindowsStyle);

		// Copy the X windows way - CTextBase now takes care of
		// of using different clipboards for copy-select vs menu copy/cut/paste
		JTextEditor::ShouldCopyWhenSelect(kTrue);
		JTextEditor16::ShouldCopyWhenSelect(kTrue);

		// Replace illegal chars instead of remove them
		// We need to do this as Mulberry does its own style processing before
		// text is inserted, and then applies those styles after insert
		JTextEditor::ShouldReplaceIllegalChars(kTrue);
		JTextEditor16::ShouldReplaceIllegalChars(kTrue);

		// Use Windows style home/end key
		JXTEBase::ShouldUseWindowsHomeEnd(kTrue);
		JXTEBase16::ShouldUseWindowsHomeEnd(kTrue);

		// Don't move caret on scroll - we'll do it ourselves only for key presses
		JXTEBase::CaretShouldFollowScroll(kFalse);
		JXTEBase16::CaretShouldFollowScroll(kFalse);

		// Mulberry does throw exceptions all the way up - at least for now
		// we should just loop here until Run() completes without exception
		bool run_done = false;
		while(!run_done)
		{
			try
			{
				app->Run();
				run_done = true;
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

			}
		}
	}
	catch (const std::exception& ex1)
	{
		CLOG_LOGCATCH(std::exception&);

		cout << "Standard exception caught: " << ex1.what() << std::endl;
	}
	catch (const JError& ex)
	{
		CLOG_LOGCATCH(JError&);

		cout << "JError exception caught: " << ex.GetMessage() << std::endl;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		cout << "Nonstandard exception caught. Hrm. " << std::endl;
	}

	return 0;
}

void ParseTextOptions(const int argc, char* argv[])
{
	long index = 1;
	while (index < argc)
	{
		if (!::strcmp(argv[index], "-h") ||
			!::strcmp(argv[index], "--help"))
		{
			PrintHelp();
			exit(0);
		}
		else if (!::strcmp(argv[index], "-v") ||
				 !::strcmp(argv[index], "--version"))
		{
			PrintVersion();
			exit(0);
		}
		else if (!::strcmp(argv[index], "-assert-print"))
		{
			JAssertBase::SetAction(JAssertBase::kPrint);
		}
		else if (!::strcmp(argv[index], "-assert-ask"))
		{
			JAssertBase::SetAction(JAssertBase::kAskUser);
		}
		else if (!::strcmp(argv[index], "-assert-abort"))
		{
			JAssertBase::SetAction(JAssertBase::kAbort);
		}
		else if (!::strcmp(argv[index], "-xassert"))
		{
			xdebug = true;
		}
		else if (!::strcmp(argv[index], "-gdb"))
		{
			JAssertBase::SetAction(JAssertBase::kPrint);
			xdebug = true;
		}
		index++;
	}
}

void PrintHelp()
{
	cout << 
			"\n"
			"                   Mulberry\n"
			"       Internet Email from the Ground Up\n"
			"\n"
			"Mulberry is a GUI-based email and calendar client\n"
			"supporting the IMAP and POP3 internet protocols for\n"
			"remote storage and manipulation of email, or local\n"
			"mailboxes. It also supports SMTP for sending email,\n"
			"as well as IMSP, ACAP and LDAP internet protocols\n"
			"for remote preferences, remote address books and\n"
			"directory services. HTTP, WebDAV and CalDAV calendar\n"
			"access is also provided.\n"
			"\n"
			"Mulberry is currently available for Macintosh, Win32 and\n"
			"Linux.\n"
			"\n"
			"Usage:  <options>\n"
			"\n"
			"-h, --help\n"
			"        prints this help\n"
			"\n"
			"-v, --version\n"
			"        prints the version number\n"
			"\n"
			"-D <directory path> - default: user home directory\n"
			"        location for .mulberry directory for default\n"
			"        preferences and local storage of mailboxes etc\n"
			"\n"
			"-p <file path>      - default: ~/.mulberry/preferences.mbp\n"
			"        preference file to use on startup\n"
			"\n"
			"-plugins <directory path>\n"
			"        override location for Mulberry plugins\n"
			"\n"
			"-extras <directory path>\n"
			"        alternate location for mailcap, mime.types and\n"
			"        url.helpers files\n"
			"\n"
			"-u <url>\n"
			"        mailto url to generate a draft message\n"
			"        on startup\n"
			"\n"
			"-d, --disconnect\n"
			"        prompt to start in disconnected mode if\n"
			"        not already set to do so\n"
			"\n"
			"\n"
			"Environment Variables:\n"
			"\n"
			"PRINTCMD - the default print command that appears in\n"
			"           the print dialog\n"
			"\n"
			"PAPERSIZE - the default papersize\n"
			"            allowed values:\n"
			"              USLetter\n"
			"              USLegal\n"
			"              USExecutive\n"
			"              A4Letter\n"
			"              B5Letter\n"
			"\n"
			"Contact:\n"
			"        <mailto:mulberry-support@mulberrymail.com>\n"
			"        <http://www.mulberrymail.com>\n"
			"\n";
}


void PrintVersion()
{
	cout << std::endl;
	cout << GetVersionText(CMulberryApp::GetVersionNumber()).c_str() << std::endl;
	cout << std::endl;
}
