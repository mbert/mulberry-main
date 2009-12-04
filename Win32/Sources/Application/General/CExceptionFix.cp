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

#include <stdio.h>	// 960711: Wouldn't compile without it.
#include <stdarg.h>
#include <exception>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "CLetterDoc.h"

// Any of these bits means handler is called in unwinding mode

#define UNWINDING_MASK			0x66

// Exception code for a C++ exception

#define CPP_EXCEPTION_CODE		0xE06D7363

char sExceptionMulberryVersion[32] = {0};

char sCrashLogDir[1024] = {0};
char sCrashLogPath[1024] = {0};

// Default exception handler for the Metrowerks C/C++ runtime library 

void AFXAPI DumpStack(FILE* fout, LPCONTEXT context);

extern "C" int _MWCHandler(
    PEXCEPTION_RECORD exc, void */*exc_stack*/, LPCONTEXT context, void *mystery);       /* hh 971207 Added prototype */

extern "C" int _MWCHandler(
    PEXCEPTION_RECORD exc, void */*exc_stack*/, LPCONTEXT context, void */*mystery*/)
{
	// Protect against exceptions inside of the exception handler itself
	//
	// Policy:
	// Re-entrant level 1 : do safety-save, crash dump, alert, terminate
	// Re-entrant level 2 : crash dump, alert, terminate
	// Re-entrant level 3 : alert, terminate
	// Re-entrant level 4 : terminate
	static unsigned long reentrant_ctr = 0;

	// Bump up the re-entrant counter before doing anything else
	reentrant_ctr++;

    TCHAR buffer[1024];
	asm
	{
	fclex /* clear fp status word so fp instructions in runtime don't 
	         rethrow the same exception as a side effect.
	      */
	
	}

	// Don't do anything if we are unwinding
	
	if (exc->ExceptionFlags & UNWINDING_MASK)
	{
	    return 1;
	}

	// Do safety save of any open drafts at re-entrant level one only
	unsigned long safety_total = 0;
	unsigned long safety_recovered = 0;
	if (reentrant_ctr <= 1)
	{
		try
		{
			safety_recovered = CLetterDoc::SaveAllTemporary();
		}
		catch(...)
		{
		}
	}

	// Check for a C++ exception
	
	if (exc->ExceptionCode == CPP_EXCEPTION_CODE)
	{
		// Call the terminate function, which the user
		// might replace
		
		_STD::terminate();
		
		// should not return, but it it does, we will
		// put out the message below.
	}

#if 1
	// Dump crash info to file - re-enrant level one and two only
	bool did_dump = false;
	if (reentrant_ctr <= 2)
	{
		::strcpy(sCrashLogPath, sCrashLogDir);
		::strcat(sCrashLogPath, "CrashDump.txt");
		FILE* fout = ::fopen(sCrashLogPath, "a");
		if (fout)
		{
			char buf[1024];

			::snprintf(buf, 1024, "------------------------------------------------------\n");
			::fputs(buf, fout);

			time_t systime = ::time(NULL);
			::snprintf(buf, 1024, "Mulberry %s Crash Dump: %s\n\nRegisters:\n", sExceptionMulberryVersion, ::ctime(&systime));
			::fputs(buf, fout);

			::snprintf(buf, 1024, "eax=%08X", context->Eax);
			::fputs(buf, fout);

			::snprintf(buf, 1024, " ebx=%08X", context->Ebx);
			::fputs(buf, fout);

			::snprintf(buf, 1024, " ecx=%08X\n", context->Ecx);
			::fputs(buf, fout);

			::snprintf(buf, 1024, "edx=%08X", context->Edx);
			::fputs(buf, fout);

			::snprintf(buf, 1024, " esi=%08X", context->Esi);
			::fputs(buf, fout);

			::snprintf(buf, 1024, " edi=%08X\n", context->Edi);
			::fputs(buf, fout);

			::snprintf(buf, 1024, "eip=%08X", context->Eip);
			::fputs(buf, fout);

			::snprintf(buf, 1024, " esp=%08X", context->Esp);
			::fputs(buf, fout);

			::snprintf(buf, 1024, " ebp=%08X\n\n", context->Ebp);
			::fputs(buf, fout);

#if 1
			::snprintf(buf, 1024, "Stack trace:\nFramePtr ReturnAd Param#1  Param#2  Param#3  Param#4  ");
			::fputs(buf, fout);

			unsigned long max_stack = 50;	// Prevent infinite recursion if stack loops back on itself
			DWORD* framePtr = (DWORD*)context->Ebp;
			while(max_stack-- > 0)
			{
				::snprintf(buf, 1024, "\n%08X %08X %08X %08X %08X %08X", framePtr, framePtr[1], framePtr[2], framePtr[3], framePtr[4], framePtr[5]);
				::fputs(buf, fout);
				if (framePtr[1] == 0)
					break;
				framePtr = (DWORD*) framePtr[0];
				if (IsBadReadPtr(framePtr, 8))
					break;
			}

			::snprintf(buf, 1024, "\n\n");
			::fputs(buf, fout);
#else
			DumpStack(fout, context);
#endif

			::fclose(fout);
			did_dump = true;
		}
	}
#endif
	
    // Make a message - re-entrant levels one, two and three only
    if (reentrant_ctr <= 3)
    {
#ifdef UNICODE   

		if (safety_recovered)
			swprintf(buffer, sizeof(buffer)+1,L"Unhandled exception: %08x\n" 
			                                  L"At address:          %08x\n\n"
			                                  L"Saved %d Drafts for recovery\n\n"
			                                  L"%s\n%s\n",
						    			    exc->ExceptionCode ,
			    						    context->Eip,
			    						    safety_recovered,
			    						    did_dump ? "CrashDump.txt file written to:" : "CrashDump.txt file could not be written",
			    						    did_dump ? sCrashLogPath : "");
		else
			swprintf(buffer, sizeof(buffer)+1,L"Unhandled exception: %08x\n" 
			                                  L"At address:          %08x\n\n"
			                                  L"%s\n%s\n",
						    			    exc->ExceptionCode ,
			    						    context->Eip,
			    						    did_dump ? "CrashDump.txt file written to:" : "CrashDump.txt file could not be written",
			    						    did_dump ? sCrashLogPath : "");
	    
	    			    
#else
#if(__dest_os == __wince_os)
 #error "the macro UNICODE must be defined when compiling the runtime for Windows CE"
#endif
		if (safety_recovered)
			snprintf(buffer, 1024, "Unhandled exception: %08x\n"
			   			    "At address:          %08x\n\n"
			   			    "Saved %d Drafts for recovery\n\n"
			                "%s\n%s\n",
			   			    exc->ExceptionCode,
			   			    context->Eip,
			   			    safety_recovered,
			   			    did_dump ? "CrashDump.txt file written to:" : "CrashDump.txt file could not be written",
			   			    did_dump ? sCrashLogPath : "");
		else
			snprintf(buffer, 1024, "Unhandled exception: %08x\n"
			   			    "At address:          %08x\n\n"
			                "%s\n%s\n",
			   			    exc->ExceptionCode,
			   			    context->Eip,
			   			    did_dump ? "CrashDump.txt file written to:" : "CrashDump.txt file could not be written",
			   			    did_dump ? sCrashLogPath : "");
#endif /* UNICODE */

    // And display it in a message box
#if(__dest_os == __wince_os)
		MessageBox(NULL, buffer, TEXT("Unhandled Exception"), MB_OK );
#else    			    
		MessageBox(NULL, buffer, TEXT("Unhandled Exception"), MB_OK | MB_TASKMODAL);
#endif
	}

    // return success
    ExitProcess(1);
   
    return 1;
}


/*
	Change Record
 * hh 971207 Added _MWCHandler prototype
 * mf 980827 fix for CE
 * mf 980903 another fix for CE, missing comma
 * es mf 000707 fix to failure of displaying message box
                in handling unmasked exceptions when msl was
                fully optimized.  
                
*/
