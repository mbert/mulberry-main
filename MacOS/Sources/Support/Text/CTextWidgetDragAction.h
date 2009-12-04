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


// Header for CTextWidgetDragAction class

#ifndef __CTextWidgetDragAction__MULBERRY__
#define __CTextWidgetDragAction__MULBERRY__

// Resources

const ResIDT	STRx_RedoDragTextActions = 300;
const ResIDT	STRx_UndoDragTextActions = 301;
const SInt16		str_DragInsert = 1;

// Classes

class CTextWidget;
class LCommander;

class CTextWidgetDragAction : public LAction {

private:
	CTextWidget*		mTextWidget;
	LCommander*			mTextCommander;
	CommandT			mActionCommand;

	Handle				mDeletedTextH;
	UniCharCount		mDeletedTextLen;
	UniCharArrayOffset	mSelStart;
	UniCharArrayOffset	mSelEnd;
	Handle				mDeletedStyleH;

	Handle				mDroppedTextH;
	UniCharCount		mDroppedTextLen;
	UniCharCount		mActualTextLen;
	Handle				mDroppedStyleH;

	UniCharArrayOffset	mInsertPos;

	bool				mDeleteOriginal;
	bool				mListItem;

public:

					CTextWidgetDragAction(CTextWidget*		inTextWidget,
											LCommander*		inTextCommander,
											Handle			inDroppedTextH,
											UniCharCount	inDroppedTextLen,
											Handle			inDroppedStyleH,
											UniCharArrayOffset	inInsertPos,
											bool			delete_original,
											bool			list_item = false);
	virtual 		~CTextWidgetDragAction();

	virtual void		Redo();
	virtual void		Undo();

protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();
	
};

#endif
