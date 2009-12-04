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


// CMFCExtras.cp

#ifdef __MULBERRY
#include "CLog.h"
#endif

extern void AFXAPI _AfxPreInitDialog(
	CWnd* pWnd, LPRECT lpRectOld, DWORD* pdwStyleOld);

extern void AFXAPI _AfxPostInitDialog(
	CWnd* pWnd, const RECT& rectOld, DWORD dwStyleOld);

LRESULT AFXAPI AfxCallWndProc(CWnd* pWnd, HWND hWnd, UINT nMsg,
	WPARAM wParam = 0, LPARAM lParam = 0)
{
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	MSG oldState = pThreadState->m_lastSentMsg;   // save for nesting
	pThreadState->m_lastSentMsg.hwnd = hWnd;
	pThreadState->m_lastSentMsg.message = nMsg;
	pThreadState->m_lastSentMsg.wParam = wParam;
	pThreadState->m_lastSentMsg.lParam = lParam;

#ifdef _DEBUG
	if (afxTraceFlags & traceWinMsg)
		_AfxTraceMsg(_T("WndProc"), &pThreadState->m_lastSentMsg);
#endif

	// Catch exceptions thrown outside the scope of a callback
	// in debug builds and warn the user.
	LRESULT lResult;
	try
	{
#ifndef _AFX_NO_OCC_SUPPORT
		// special case for WM_DESTROY
		//if ((nMsg == WM_DESTROY) && (pWnd->m_pCtrlCont != NULL))
		//	pWnd->m_pCtrlCont->OnUIActivate(NULL);
#endif

		// special case for WM_INITDIALOG
		CRect rectOld;
		DWORD dwStyle;
		if (nMsg == WM_INITDIALOG)
			_AfxPreInitDialog(pWnd, &rectOld, &dwStyle);

		// delegate to object's WindowProc
		lResult = pWnd->WindowProc(nMsg, wParam, lParam);

		// more special case for WM_INITDIALOG
		if (nMsg == WM_INITDIALOG)
			_AfxPostInitDialog(pWnd, rectOld, dwStyle);
	}
	catch (CException* e)
	{
#ifdef __MULBERRY
		CLOG_LOGCATCH(CException*);
#endif

		lResult = AfxGetThread()->ProcessWndProcException(e, &pThreadState->m_lastSentMsg);
		TRACE1("Warning: Uncaught exception in WindowProc (returning %ld).\n", lResult);
		DELETE_EXCEPTION(e);
	}
	catch (exception& e)
	{
#ifdef __MULBERRY
		CLOG_LOGCATCH(exception&);
#endif

		lResult = 1;
		TRACE1("Warning: Uncaught exception in WindowProc (returning %ld).\n", lResult);
	}
	catch (...)
	{
#ifdef __MULBERRY
		CLOG_LOGCATCH(...);
#endif

		lResult = 1;
		TRACE1("Warning: Uncaught exception in WindowProc (returning %ld).\n", lResult);
	}

	pThreadState->m_lastSentMsg = oldState;
	return lResult;
}

#ifdef __MULBERRY
typedef struct CatchInfo {
	void			*location;				//	pointer to complete exception object
	void			*typeinfo;				//	pointer to complete exception object's typeinfo
	void			*dtor;					//	pointer to complete exception object's destructor function (0: no destructor)
	void			*sublocation;			//	pointer to handlers subobject in complete exception object
	long			pointercopy;			//	adjusted pointer copy for pointer matches (sublocation will point to the loaction)
	void			*stacktop;				//	pointer to handlers stack top (reused for exception specification in specification handler)
}	CatchInfo;

typedef struct ThrowContext {
	long		EBX, ESI, EDI;				//  space for EBX, ESI and EDI
	char 		*EBP;						
	char		*returnaddr;				//	return address
	char		*throwtype;					//	throw type argument (0L: rethrow: throw; )
	void		*location;					//	location argument (0L: rethrow: throw; )
	void		*dtor;						//	dtor argument
	CatchInfo	*catchinfo;					//	pointer to rethrow CatchInfo (or 0L)
	unsigned char XMM4[16],XMM5[16],		//	nonvolatile registers (saved only if procflags & _CPU_FEATURE_SSE)
					XMM6[16],XMM7[16];
}	ThrowContext;

extern void ExX86_User_ThrowHandler(ThrowContext *context);
void ExX86_User_ThrowHandler(ThrowContext *context)
{
	char stacktrace[4096];
	stacktrace[0] = 0;
	char buffer[256];
	unsigned long max_stack = 5;	// Prevent infinite recursion if stack loops back on itself

	DWORD* framePtr = (DWORD*)context->EBP;

	while(max_stack-- > 0)
	{
		::snprintf(buffer, 256, "\r\n%08X %08X %08X %08X %08X %08X", framePtr, framePtr[1], framePtr[2], framePtr[3], framePtr[4], framePtr[5]);
		::strncat(stacktrace, buffer, 4096);
		if (framePtr[1] == 0)
			break;
		framePtr = (DWORD*) framePtr[0];
		if (IsBadReadPtr(framePtr, 8))
			break;
	}
	::strncat(stacktrace, "\r\n\r\n", 4096);

	// Do nothing - will be overridden at link time
	CLog::LogThrow(context->throwtype == NULL ? "rethrow" : context->throwtype, (int)context->returnaddr, __FUNCTION__, stacktrace, 0);
}
#endif
