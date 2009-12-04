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

#ifndef _MULBERRYMAPI_H
#define _MULBERRYMAPI_H

#ifdef __cplusplus
	#define EXPORT32 extern "C" __declspec(dllexport)
#else
	#define EXPORT32 extern __declspec(dllexport)
#endif

#include <MAPI.h>

//---------------------------------------------------------------------
// Interface definition.
//---------------------------------------------------------------------

EXPORT32 FAR ULONG PASCAL MAPIAddress(
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
);

EXPORT32 FAR ULONG PASCAL MAPIDeleteMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    LPSTR lpszMessageID,
    FLAGS flFlags,
    ULONG ulReserved
);

EXPORT32 FAR ULONG PASCAL MAPIDetails(
    LHANDLE lhSession,
    ULONG ulUIParam,
    lpMapiRecipDesc lpRecip,
    FLAGS flFlags,
    ULONG ulReserved
);

EXPORT32 FAR ULONG PASCAL MAPIFindNext(
    LHANDLE lhSession,
    ULONG ulUIParam,
    LPSTR lpszMessageType,
    LPSTR lpszSeedMessageID,
    FLAGS flFlags,
    ULONG ulReserved,
    LPSTR lpszMessageID
);

EXPORT32 FAR ULONG PASCAL MAPIFreeBuffer(
    LPVOID pv
);

EXPORT32 FAR ULONG PASCAL MAPILogoff(
    LHANDLE lhSession,
    ULONG ulUIParam,
    FLAGS flFlags,
    ULONG ulReserved
);

EXPORT32 FAR ULONG PASCAL MAPILogon(
    ULONG ulUIParam,
    LPSTR lpszProfileName,
    LPSTR lpszPassword,
    FLAGS flFlags,
    ULONG ulReserved,
    LPLHANDLE lplhSession
);

EXPORT32 FAR ULONG PASCAL MAPIReadMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    LPSTR lpszMessageID,
    FLAGS flFlags,
    ULONG ulReserved,
    lpMapiMessage FAR *lppMessage
);

EXPORT32 FAR ULONG PASCAL MAPIResolveName(
    LHANDLE lhSession,
    ULONG ulUIParam,
    LPSTR lpszName,
    FLAGS flFlags,
    ULONG ulReserved,
    lpMapiRecipDesc FAR *lppRecip
);

EXPORT32 FAR ULONG PASCAL MAPISaveMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    lpMapiMessage lpMessage,
    FLAGS flFlags,
    ULONG ulReserved,
    LPSTR lpszMessageID
);

EXPORT32 FAR ULONG PASCAL MAPISendDocuments(
    ULONG ulUIParam,
    LPSTR lpszDelimChar,
    LPSTR lpszFilePaths,
    LPSTR lpszFileNames,
    ULONG ulReserved
);

EXPORT32 FAR ULONG PASCAL MAPISendMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    lpMapiMessage lpMessage,
    FLAGS flFlags,
    ULONG ulReserved
);

#endif
