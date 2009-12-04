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


// Source for CSMTPWindow class

#include "CSMTPWindow.h"

#include "CMailboxTable.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CSMTPAccountManager.h"
#if __dest_os == __win32_os
#include "CSMTPFrame.h"
#endif
#include "CSMTPSender.h"
#include "CSMTPTable.h"

#if __dest_os == __linux_os
#include <JXImageWidget.h>
#endif

void CSMTPWindow::ResetConnection()
{
	static_cast<CSMTPTable*>(GetMailboxView()->GetTable())->UpdateState();
}
