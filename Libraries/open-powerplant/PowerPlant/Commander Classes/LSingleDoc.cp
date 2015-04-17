// Copyright ©2005, 2006 Freescale Semiconductor, Inc.
// Please see the License for the specific language governing rights and
// limitations under the License.
// ===========================================================================
//	LSingleDoc.cp				PowerPlant 2.2.2	©1994-2005 Metrowerks Inc.
// ===========================================================================
//
//	Document which associates one file with one window

#ifdef PowerPlant_PCH
	#include PowerPlant_PCH
#endif

#include <LSingleDoc.h>
#include <LWindow.h>
#include <LFile.h>

#include <LString.h>
#include <PP_Messages.h>

#include <AEInteraction.h>
#include <AERegistry.h>
#include <AEObjects.h>
#include <AEPackObject.h>

#include <SysCFString.h>

PP_Begin_Namespace_PowerPlant


// ---------------------------------------------------------------------------
//	¥ LSingleDoc							Default Constructor		  [public]
// ---------------------------------------------------------------------------
//

LSingleDoc::LSingleDoc()
{
	mWindow = nil;
	mFile   = nil;
}


// ---------------------------------------------------------------------------
//	¥ LSingleDoc(LCommander*)				Constructor				  [public]
// ---------------------------------------------------------------------------

LSingleDoc::LSingleDoc(
	LCommander*		inSuper)

	: LDocument(inSuper)
{
	mWindow = nil;
	mFile   = nil;
}


// ---------------------------------------------------------------------------
//	¥ ~LSingleDoc							Destructor				  [public]
// ---------------------------------------------------------------------------

LSingleDoc::~LSingleDoc()
{
	delete mWindow;
	delete mFile;
}


// ---------------------------------------------------------------------------
//	¥ AllowSubRemoval												  [public]
// ---------------------------------------------------------------------------

Boolean
LSingleDoc::AllowSubRemoval(
	LCommander*		inSub)
{
	if (inSub == mWindow) {

			// Check if the current AppleEvent is a "close" event
			// sent to the Window. If so, we handle it as if the
			// "close" event were sent to the Document

		AppleEvent	currentEvent;
		DescType	theType;
		DescType	theAttr = typeNull;
		Size		theSize;
		::AEGetTheCurrentEvent(&currentEvent);
		if (currentEvent.descriptorType != typeNull) {
			::AEGetAttributePtr(&currentEvent, keyEventClassAttr,
				typeType, &theType, &theAttr, sizeof(DescType),
				&theSize);
			if (theAttr == kAECoreSuite) {
				::AEGetAttributePtr(&currentEvent, keyEventIDAttr,
					typeType, &theType, &theAttr, sizeof(DescType),
					&theSize);
				if (theAttr == kAEClose) {
					DoAEClose(currentEvent);
					return false;
				}
			}
		}

		AttemptClose(true);			// A non-AppleEvent close
		return false;

	} else {
		return true;
	}
}


// ---------------------------------------------------------------------------
//	¥ GetDescriptor													  [public]
// ---------------------------------------------------------------------------
//	Pass back the name of a Document

PPx::CFString
LSingleDoc::GetDescriptor() const
{
	PPx::CFString cfstr;
	if ((mFile != nil) && mIsSpecified) {
		PPx::FSObject	fileSpec;			// Document name is same as its File
		mFile->GetSpecifier(fileSpec);
		cfstr = fileSpec.GetName();

	} else if (mWindow != nil) {	// No File, use name of its Window
		cfstr = PPx::CFString(mWindow->CopyCFDescriptor(), false);

	} else {						// No File and No Window
		//   Document name is empty string
	}

	return cfstr;
}


// ---------------------------------------------------------------------------
//	¥ UsesFileSpec													  [public]
// ---------------------------------------------------------------------------
//	Returns whether the Document's File has the given FSRef

Boolean
LSingleDoc::UsesFileSpec(
	const PPx::FSObject&	inFileSpec) const
{
	Boolean usesFS = false;

	if (mFile != nil) {
		usesFS = mFile->UsesSpecifier(inFileSpec);
	}

	return usesFS;
}


// ---------------------------------------------------------------------------
//	¥ MakeCurrent													  [public]
// ---------------------------------------------------------------------------
//	Make this Document the current one by selecting its Window

void
LSingleDoc::MakeCurrent()
{
	if (mWindow != nil) {
		mWindow->Select();
	}
}

PP_End_Namespace_PowerPlant
