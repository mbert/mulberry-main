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

//-----------------------------------------------------------
//	Netscape Postal API DLL 
//	
//	The functions exported in this DLL are the ones Navigator
//	looks for on startup and invokes appropriately.
//
//	Author : Chak Nanga, Netscape Communications Corp.
//------------------------------------------------------------

#include <stdlib.h>
#include "MulberryMAPI.h"
#include "url_encoding.h"

#include <stdio.h>
#include <strstream.h>
#include <iostream.h>

//#define DEBUG_BOX

HINSTANCE hInst;

BOOL __stdcall DllMain(HINSTANCE inst, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
#ifdef DEBUG_BOX
	MessageBox(NULL, "DLL_XXX_ATTACH", NULL, MB_OK);
#endif

		  break;

		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
#ifdef DEBUG_BOX
	MessageBox(NULL, "DLL_XXX_DETACH", NULL, MB_OK);
#endif

	      break;
   }

   return TRUE;
}

FAR ULONG PASCAL MAPIAddress(
    LHANDLE lhSession,
    ULONG ulUIParam,
    LPSTR lpszCaption,
    ULONG nEditFields,
    LPSTR lpszLabels,
    ULONG nRecips,
    lpMapiRecipDesc lpRecips,
    FLAGS flFlags,
    ULONG ulReserved,
    LPULONG lpnNewRecips,
    lpMapiRecipDesc FAR *lppNewRecips
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPIAddress", NULL, MB_OK);
#endif
	MessageBeep(-1L);
	return MAPI_E_NOT_SUPPORTED;
}

FAR ULONG PASCAL MAPIDeleteMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    LPSTR lpszMessageID,
    FLAGS flFlags,
    ULONG ulReserved
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPIDeleteMail", NULL, MB_OK);
#endif
	MessageBeep(-1L);
	return MAPI_E_NOT_SUPPORTED;
}

FAR ULONG PASCAL MAPIDetails(
    LHANDLE lhSession,
    ULONG ulUIParam,
    lpMapiRecipDesc lpRecip,
    FLAGS flFlags,
    ULONG ulReserved
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPIDetails", NULL, MB_OK);
#endif
	MessageBeep(-1L);
	return MAPI_E_NOT_SUPPORTED;
}

FAR ULONG PASCAL MAPIFindNext(
    LHANDLE lhSession,
    ULONG ulUIParam,
    LPSTR lpszMessageType,
    LPSTR lpszSeedMessageID,
    FLAGS flFlags,
    ULONG ulReserved,
    LPSTR lpszMessageID
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPIFindNext", NULL, MB_OK);
#endif
	MessageBeep(-1L);
	return MAPI_E_NOT_SUPPORTED;
}

FAR ULONG PASCAL MAPIFreeBuffer(
    LPVOID pv
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPIFreeBuffer", NULL, MB_OK);
#endif
	MessageBeep(-1L);
	return MAPI_E_NOT_SUPPORTED;
}

FAR ULONG PASCAL MAPILogoff(
    LHANDLE lhSession,
    ULONG ulUIParam,
    FLAGS flFlags,
    ULONG ulReserved
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPILogoff", NULL, MB_OK);
#endif
	return SUCCESS_SUCCESS;
}

FAR ULONG PASCAL MAPILogon(
    ULONG ulUIParam,
    LPSTR lpszProfileName,
    LPSTR lpszPassword,
    FLAGS flFlags,
    ULONG ulReserved,
    LPLHANDLE lplhSession
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPILogon", NULL, MB_OK);
#endif
	*lplhSession = 1L;
	return SUCCESS_SUCCESS;
}

FAR ULONG PASCAL MAPIReadMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    LPSTR lpszMessageID,
    FLAGS flFlags,
    ULONG ulReserved,
    lpMapiMessage FAR *lppMessage
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPIReadMail", NULL, MB_OK);
#endif
	MessageBeep(-1L);
	return MAPI_E_NOT_SUPPORTED;
}

FAR ULONG PASCAL MAPIResolveName(
    LHANDLE lhSession,
    ULONG ulUIParam,
    LPSTR lpszName,
    FLAGS flFlags,
    ULONG ulReserved,
    lpMapiRecipDesc FAR *lppRecip
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPIResolveName", NULL, MB_OK);
#endif
	MessageBeep(-1L);
	return MAPI_E_NOT_SUPPORTED;
}

FAR ULONG PASCAL MAPISaveMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    lpMapiMessage lpMessage,
    FLAGS flFlags,
    ULONG ulReserved,
    LPSTR lpszMessageID
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPISaveMail", NULL, MB_OK);
#endif
	MessageBeep(-1L);
	return MAPI_E_NOT_SUPPORTED;
}

FAR ULONG PASCAL MAPISendDocuments(
    ULONG ulUIParam,
    LPSTR lpszDelimChar,
    LPSTR lpszFilePaths,
    LPSTR lpszFileNames,
    ULONG ulReserved
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPISendDocuments", NULL, MB_OK);
#endif
	MessageBeep(-1L);
	return MAPI_E_NOT_SUPPORTED;
}

FAR ULONG PASCAL MAPISendMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    lpMapiMessage lpMessage,
    FLAGS flFlags,
    ULONG ulReserved
)
{
#ifdef DEBUG_BOX
	MessageBox(NULL, "MAPISendMail", NULL, MB_OK);
#endif

	ostrstream out;
	bool q_mark = true;

	// Do basic mailto
	out << "mailto:";
	
	// Look for recipients
	for(int i = 0; i < lpMessage->nRecipCount; i++)
	{
		// Check on recip type and copy in header detail
		switch(lpMessage->lpRecips[i].ulRecipClass)
		{
		case MAPI_ORIG:
			continue;
		case MAPI_TO:
			out << (q_mark ? "?" : "&") << "to=";
			break;
		case MAPI_CC:
			out << (q_mark ? "?" : "&") << "cc=";
			break;
		case MAPI_BCC:
			out << (q_mark ? "?" : "&") << "bcc=";
			break;
		}
		q_mark = false;
		
		// copy in address detail
		const char* p = lpMessage->lpRecips[i].lpszAddress;
		if (!::strncmp(p, "SMTP:", 5))
			p += 5;
		char* encoded = EncodeURL(p);
		out << (encoded ? encoded : p);
		if (encoded)
			free(encoded);
	}

	// Look for subject
	if (lpMessage->lpszSubject && *lpMessage->lpszSubject)
	{
		char* encoded = EncodeURL(lpMessage->lpszSubject);
		out << (q_mark ? "?" : "&") << "subject=" << (encoded ? encoded : lpMessage->lpszSubject);
		q_mark = false;
		if (encoded)
			free(encoded);
	}

	// Look for body
	if (lpMessage->lpszNoteText && *lpMessage->lpszNoteText)
	{
		char* encoded = EncodeURL(lpMessage->lpszNoteText);
		out << (q_mark ? "?" : "&") << "body=" << (encoded ? encoded : lpMessage->lpszNoteText);
		q_mark = false;
		if (encoded)
			free(encoded);
	}

	// Look for files
	for(int i = 0; i < lpMessage->nFileCount; i++)
	{
		// Get name of attachment
		const char* fpath = lpMessage->lpFiles[i].lpszPathName;
		const char* fname = lpMessage->lpFiles[i].lpszFileName;
		
		if (!fname || !*fname || strrchr(fname, '\\'))
		{
			fname = strrchr(fpath, '\\');
			if (fname) fname++;
		}
		
		// Must duplicate this attachment into Temp
		char temp_path[MAX_PATH];
		char temp_path_[MAX_PATH];
		char temp_dir[MAX_PATH];
		if (GetTempPath(MAX_PATH, temp_dir))
		{
 			strcpy(temp_path, temp_dir);
 			strcpy(temp_path_, temp_dir);
 			strcat(temp_path_, "_");
 			strcat(temp_path, fname);
 			strcat(temp_path_, fname);
 		}
 		else
 		{
#ifdef DEBUG_BOX
			DWORD err = GetLastError();
			char temp_err[1024];
			snprintf(temp_err, 1024, "No temp directory, errno = %ld", err);
			MessageBox(NULL, temp_err, NULL, MB_OK);
#endif
 			// Fail if no temp directory
			return MAPI_E_FAILURE;
 		}

 		// If source file is in TEMP then we must copy it
 		if (!strncmp(temp_dir, fpath, strlen(temp_dir)))
 		{
	 		// Do copy to standard name
	 		if (!CopyFile(fpath, temp_path_, true))
	 		{
	 			DWORD last_err = GetLastError();

#ifdef DEBUG_BOX
 				char temp_err[1024];
 				snprintf(temp_err, 1024, "Orig. file: \"%s\"  Temp. file: \"%s\"  errno = %ld", fpath, temp_path_, last_err);
				MessageBox(NULL, temp_err, NULL, MB_OK);
#endif

 				if (last_err == ERROR_FILE_EXISTS)
 				{
					DeleteFile(temp_path_);
		 			if (!CopyFile(fpath, temp_path_, true))
		 			{
#ifdef DEBUG_BOX
		 				DWORD err = GetLastError();
		 				char temp_err[1024];
		 				snprintf(temp_err, 1024, "Unable to copy to temp directory after removal, errno = %ld", err);
						MessageBox(NULL, temp_err, NULL, MB_OK);
#endif
	 	 				return MAPI_E_FAILURE;
	 	 			}
	 			}
	 			else
	 			{
 	 				return MAPI_E_FAILURE;
 	 			}
	 		}
			strcpy(temp_path, temp_path_);
		}
		else
			strcpy(temp_path, fpath);
 
		// Encode attachment name and add to URL
		char* encoded = EncodeURL(temp_path);
		out << (q_mark ? "?" : "&") << "x-mulberry-file=" << (encoded ? encoded : temp_path);
		q_mark = false;
		if (encoded)
			free(encoded);
	}

	// NULL terminate the string
	out << ends;

#ifdef DEBUG_BOX
	MessageBox(NULL, out.str(), NULL, MB_OK);
	out.freeze(false);
#endif

	// Now do Shell open of mailto URL
	char dir[MAX_PATH];
	if (GetCurrentDirectory(MAX_PATH, dir))
	{
		ShellExecute((HWND) ulUIParam, "open", out.str(), 0L, dir, 0);
		out.freeze(false);
	}

	return SUCCESS_SUCCESS;
}
