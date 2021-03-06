//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// CTooltipSupport written 2005 by Arne Scheffler
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2008, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "ctooltipsupport.h"
#include "cvstguitimer.h"

#if WINDOWS
#define _WIN32_WINNT 0x0501	// Works only on Windows XP and above because of TTM_POPUP
#include <commctrl.h>
extern void* hInstance;
inline HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }
static void* InitTooltip (CFrame* frame);
#endif

#if DEBUG
#define DEBUGLOG 0
#endif

//------------------------------------------------------------------------
/*! @class VSTGUI::CTooltipSupport
A generic way to add tooltip support to VSTGUI.
@section ctooltipsupport_example Example
Adding a tooltip to a view
\code
const char* tooltipText = "This is a tooltip";
view->setAttribute (kCViewTooltipAttribute, strlen (tooltipText)+1, tooltipText);
\endcode
Adding CTooltipSupport
\code
CTooltipSupport* tooltipSupport = new CTooltipSupport (frame);
\endcode
------------------------------------------------------------------------*/
/**
 * @param frame CFrame object
 * @param delay tooltip delay time in milliseconds
 */
CTooltipSupport::CTooltipSupport (CFrame* frame, int delay)
: timer (0)
, frame (frame)
, currentView (0)
, platformObject (0)
{
	timer = new CVSTGUITimer (this, delay);
	frame->setMouseObserver (this);
	frame->remember ();

	#if WINDOWS
	platformObject = InitTooltip (frame);

	#endif
}

//------------------------------------------------------------------------
CTooltipSupport::~CTooltipSupport ()
{
	timer->forget ();
	frame->setMouseObserver (0);
	frame->forget ();

	#if WINDOWS
	if (platformObject)
		DestroyWindow ((HWND)platformObject);

	#endif
}

//------------------------------------------------------------------------
void CTooltipSupport::onMouseEntered (CView* view, CFrame* frame)
{
	#if DEBUGLOG
	DebugPrint ("IMouseObserver::onMouseEntered (%s)\n", view->getClassName ());
	#endif
	currentView = view;
	timer->start ();
}

//------------------------------------------------------------------------
void CTooltipSupport::onMouseExited (CView* view, CFrame* frame)
{
	#if DEBUGLOG
	DebugPrint ("IMouseObserver::onMouseExited (%s)\n", view->getClassName ());
	#endif
	timer->stop ();
	currentView = 0;
	hideTooltip ();
}

//------------------------------------------------------------------------
void CTooltipSupport::hideTooltip ()
{
	#if MAC
	HMHideTag ();

	#elif WINDOWS
	if (platformObject)
		SendMessage ((HWND)platformObject, TTM_POP, 0, 0);

	#endif
}

//------------------------------------------------------------------------
void CTooltipSupport::showTooltip ()
{
	if (currentView)
	{
		CRect r (currentView->getVisibleSize ());
		CPoint p;
		currentView->localToFrame (p);
		r.offset (p.x, p.y);

		// get the tooltip from the view
		char* tooltip = 0;
		long tooltipSize = 0;
		if (currentView->getAttributeSize (kCViewTooltipAttribute, tooltipSize))
		{
			tooltip = (char*)malloc (tooltipSize + 1);
			memset (tooltip, 0, tooltipSize+1);
			if (!currentView->getAttribute (kCViewTooltipAttribute, tooltipSize, tooltip, tooltipSize))
			{
				free (tooltip);
				tooltip = 0;
			}
		}
		if (tooltip)
		{
			#if MAC
			CCoord x, y;
			currentView->getFrame ()->getPosition (x,y);
			r.offset (x, y);

			HMHelpContentRec helpContent = {0};
			helpContent.version = 0;
			helpContent.absHotRect.left = r.left;
			helpContent.absHotRect.right = r.right;
			helpContent.absHotRect.top = r.top;
			helpContent.absHotRect.bottom = r.bottom;
			helpContent.tagSide = kHMDefaultSide;
			helpContent.content[0].contentType = kHMCFStringContent;
			helpContent.content[0].u.tagCFString = CFStringCreateWithCString (0, tooltip, kCFStringEncodingUTF8);
			HMDisplayTag(&helpContent);
			CFRelease (helpContent.content[0].u.tagCFString);

			#elif WINDOWS
			UTF8StringHelper tooltipText (tooltip);
			if (platformObject)
			{
				RECT rc;
				rc.left = (LONG)r.left;
				rc.top = (LONG)r.top;
				rc.right = (LONG)r.right;
				rc.bottom = (LONG)r.bottom;
				TOOLINFO ti = {0};
			    ti.cbSize = sizeof(TOOLINFO);
				ti.hwnd = (HWND)frame->getSystemWindow ();
				ti.uId = 0;
				ti.rect = rc;
				ti.lpszText = (TCHAR*)(const TCHAR*)tooltipText;
				SendMessage ((HWND)platformObject, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);
				SendMessage ((HWND)platformObject, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
 				SendMessage ((HWND)platformObject, TTM_POPUP, 0, 0);
			}

			#endif
			free (tooltip);
		}
	}
}

//------------------------------------------------------------------------
CMessageResult CTooltipSupport::notify (CBaseObject* sender, const char* msg)
{
	if (msg == CVSTGUITimer::kMsgTimer)
	{
		showTooltip ();
		timer->stop ();
		return kMessageNotified;
	}
	return kMessageUnknown;
}

#if WINDOWS
void* InitTooltip (CFrame* frame)
{
    TOOLINFO    ti;
	// Create the ToolTip control.
    HWND hwndTT = CreateWindow (TOOLTIPS_CLASS, TEXT(""),
                          WS_POPUP,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, (HMENU)NULL, GetInstance (),
                          NULL);

    // Prepare TOOLINFO structure for use as tracking ToolTip.
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd   = (HWND)frame->getSystemWindow ();
    ti.uId    = (UINT)0;
    ti.hinst  = GetInstance ();
    ti.lpszText  = TEXT("This is a tooltip");
	ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;

	// Add the tool to the control
    if (!SendMessage (hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti))
	{
		DestroyWindow (hwndTT);
		return 0;
    }

	return hwndTT;
}
#endif
