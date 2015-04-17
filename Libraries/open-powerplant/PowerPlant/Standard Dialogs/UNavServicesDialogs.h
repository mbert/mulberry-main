// Copyright ©2005, 2006 Freescale Semiconductor, Inc.
// Please see the License for the specific language governing rights and
// limitations under the License.
// ===========================================================================
//	UNavServicesDialogs.h		PowerPlant 2.2.2	©1998-2005 Metrowerks Inc.
// ===========================================================================
//
//	Implementation of UStandardDialogs, LFileChooser, and LFileDesignator
//	that uses Navigation Services

#ifndef _H_UNavServicesDialogs
#define _H_UNavServicesDialogs
#pragma once

#include <PP_Prefix.h>
#include <LFileTypeList.h>
#include <UAppleEventsMgr.h>
#include <Navigation.h>
#include <PPxFSObject.h>

#if PP_Uses_Pragma_Import
	#pragma import on
#endif

PP_Begin_Namespace_PowerPlant

// ---------------------------------------------------------------------------

namespace UNavServicesDialogs {

	// -----------------------------------------------------------------------
	//	Wrapper class for a NavReplyRecord

	class	StNavReplyRecord {
		public:
								StNavReplyRecord();
								~StNavReplyRecord();

			void				SetDefaultValues();

			operator NavReplyRecord*()		{ return &mNavReply; }

			NavDialogRef&		GetDialog()	{ return mNavDialog; }

			NavReplyRecord&		Get()		{ return mNavReply; }

			UInt32&				Result()	{ return mNavResult; }

			bool				IsValid() const
											{ return mNavReply.validRecord; }

			bool				IsReplacing() const
											{ return mNavReply.replacing; }

			bool				IsStationery() const
											{ return mNavReply.isStationery; }

			AEDescList			Selection()	const
											{ return mNavReply.selection; }

			ScriptCode			KeyScript()	const
											{ return mNavReply.keyScript; }

			void				GetFileSpec(
										PPx::FSObject&		outFileSpec) const;

		protected:
			NavDialogRef		mNavDialog;
			NavReplyRecord		mNavReply;
			UInt32				mNavResult;

		private:					// Unimplemented to prevent copying
								StNavReplyRecord(
										const StNavReplyRecord&);
			StNavReplyRecord&	operator = (
										const StNavReplyRecord&);
	};

	// -----------------------------------------------------------------------

	#include <UStandardDialogs.i>

	// -----------------------------------------------------------------------

	class	LFileChooser {
		public:
			#include <LFileChooser.i>

		protected:
			StNavReplyRecord			mNavReply;
			NavDialogCreationOptions	mNavCreateOptions;
			NavObjectFilterProcPtr		mNavFilterProc;
			NavPreviewProcPtr			mNavPreviewProc;
			StAEDescriptor				mDefaultLocation;
			bool						mSelectDefault;
	};

	// -----------------------------------------------------------------------

	class	LFileDesignator {
		public:
			#include <LFileDesignator.i>

		protected:
			StNavReplyRecord			mNavReply;
			NavDialogCreationOptions	mNavCreateOptions;
			OSType						mFileType;
			OSType						mFileCreator;
			StAEDescriptor				mDefaultLocation;
			bool						mSelectDefault;
			
			friend pascal void NavEventProc(NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
	
	};

	// -----------------------------------------------------------------------
	//	Event filter callback function

	pascal void	NavEventProc(
						NavEventCallbackMessage		inSelector,
						NavCBRecPtr					ioParams,
						NavCallBackUserData			ioUserData);

	class	StNavEventUPP {
		public:
			StNavEventUPP( NavEventProcPtr inProcPtr );
			~StNavEventUPP();

			operator NavEventUPP()		{ return mNavEventUPP; }

		protected:
			NavEventUPP		mNavEventUPP;
	};

	// -----------------------------------------------------------------------
	//	Object Filter UPP

	class	StNavObjectFilterUPP {
		public:
			StNavObjectFilterUPP( NavObjectFilterProcPtr inProcPtr );
			~StNavObjectFilterUPP();

			operator NavObjectFilterUPP()	{ return mNavObjectFilterUPP; }

		protected:
			NavObjectFilterUPP	mNavObjectFilterUPP;
	};

	// -----------------------------------------------------------------------
	//	Preview UPP

	class	StNavPreviewUPP {
		public:
			StNavPreviewUPP( NavPreviewProcPtr inProcPtr );
			~StNavPreviewUPP();

			operator NavPreviewUPP()		{ return mNavPreviewUPP; }

		protected:
			NavPreviewUPP		mNavPreviewUPP;
	};
}

PP_End_Namespace_PowerPlant

#if PP_Uses_Pragma_Import
	#pragma import reset
#endif

#endif
