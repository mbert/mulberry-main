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


// Header for INET client class

#ifndef __CINETCLIENTRESPONSES__MULBERRY__
#define __CINETCLIENTRESPONSES__MULBERRY__

enum EINETResponseCode
{
	cTagOK,
	cTagNO,
	cTagBAD,
	cStarOK,
	cStarNO,
	cStarBAD,
	cStarPREAUTH,
	cStarBYE,
	cStarCAPABILITY,
	cPlusLabel,
	cNoResponse,
	cResponseError,
	cStarFLAGS,
	cStarLIST,
	cStarLSUB,
	cStarMAILBOX,
	cStarSEARCH,
	cStarSTATUS,
	cStarBBOARD,
	cStarSORT,
	cStarTHREAD,
	cStarACL,
	cStarLISTRIGHTS,
	cStarMYRIGHTS,
	cStarQUOTA,
	cStarQUOTAROOT,
	cStarNAMESPACE,
	cMsgEXISTS,
	cMsgRECENT,
	cMsgEXPUNGE,
	cMsgSTORE,
	cMsgFETCH,
	cMsgCOPY,
	cStarOPTION,
	cStarADDRESSBOOK,
	cStarSEARCHADDRESS,
	cStarFETCHADDRESS,
	cStarACLADDRESSBOOK,
	cStarMYRIGHTSADDRESSBOOK,
	cStarACAP,
	cStarALERT,
	cStarENTRY,
	cStarMODTIME
};

#endif
