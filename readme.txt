
.ABSTRACT


This is source code of GUI of 0.2.168 version of "Molot" VST compressor 
plugin coded with VSTGUI. Windows (32 and 64 bits) and MacOS X (32 bits)
versions are supported.

Molot sound processing source code replaced with stub.


.FUNCTIONALITY RESTRICTIONS


This code doesn't work with FXpansion AU and RTAS wrappers on Mac!


.USAGE RESTRICTIONS


Source code of Molot GUI can be used without any restrictions in both free 
and commercial projects.

Visual elements (png-files) of Molot GUI must not be used in public 
available free and commercial projects.

Libraries libpng, zlib, VSTSDK and VSTGUI included in this package have 
their own usage restrictions based on their licenses.


.PACKAGE CONTENTS


libpng     - http://www.libpng.org/pub/png/libpng.html
molot01    - Molot VST compressor GUI source code and visual elements
vstsdk2.4  - Steinberg VST SDK with modifications
vstsdk2.4\vstgui_350   - VSTGUI library 3.50 (used in Mac version)
vstsdk2.4\vstgui.sf    - VSTGUI library 3.0 with modifications (used in Windows version)
zlib       - http://zlib.net/


.HOW TO BUILD WINDOWS VERSION


Visual Studio 2005 was used to build Windows version:

1. Open solution molot01\molotgui.sln
2. Build solution in Win32, x64, Debug or Release configurations.


.HOW TO BUILD MAC OS X VERSION


Xcode was not used to build Mac OS X version:

1. Open Terminal
2. Change directory to molot01/vst-osx
3. Enter "make" command to build
4. Start "makedmg.sh" script to prepare .dmg file


.MODIFICATIONS MADE IN VST SDK


diff -r 612a21095dca -r 42f2abc6165e vstsdk2.4/public.sdk/source/vst2.x/vstplugmain.cpp
--- a/vstsdk2.4/public.sdk/source/vst2.x/vstplugmain.cpp	Wed Sep 08 19:10:34 2010 +0000
+++ b/vstsdk2.4/public.sdk/source/vst2.x/vstplugmain.cpp	Thu Jun 23 22:42:05 2011 +0400
@@ -42,7 +42,7 @@
 }
 
 // support for old hosts not looking for VSTPluginMain
-#if (TARGET_API_MAC_CARBON && __ppc__)
+#if (TARGET_API_MAC_CARBON /*&& __ppc__*/)
 VST_EXPORT AEffect* main_macho (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
 #elif WIN32
 VST_EXPORT AEffect* MAIN (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
diff -r 612a21095dca -r 42f2abc6165e vstsdk2.4/vstgui.sf/vstgui/vstgui.cpp
--- a/vstsdk2.4/vstgui.sf/vstgui/vstgui.cpp	Wed Sep 08 19:10:34 2010 +0000
+++ b/vstsdk2.4/vstgui.sf/vstgui/vstgui.cpp	Thu Jun 23 22:42:05 2011 +0400
@@ -2220,7 +2220,7 @@
 	logfont.lfClipPrecision = CLIP_STROKE_PRECIS;
 	logfont.lfOutPrecision  = OUT_STRING_PRECIS;
 	logfont.lfQuality 	    = DEFAULT_QUALITY;
-	logfont.lfCharSet       = ANSI_CHARSET;
+	logfont.lfCharSet       = DEFAULT_CHARSET;  // was ANSI_CHARSET
 
 	HANDLE newFont = CreateFontIndirect (&logfont);
 	SelectObject ((HDC)pSystemContext, newFont);
@@ -4064,6 +4064,8 @@
 	backBuffer = 0;
 	OleInitialize (0);
 
+    InitializeCriticalSection(&m_guard);
+
 	#if DYNAMICALPHABLEND
 	pfnAlphaBlend = 0;
 	pfnTransparentBlt = 0;
@@ -4146,6 +4148,8 @@
 	backBuffer = 0;
 	OleInitialize (0);
 
+    InitializeCriticalSection(&m_guard);
+
 	#if DYNAMICALPHABLEND
 	pfnAlphaBlend = 0;
 	pfnTransparentBlt = 0;
@@ -4215,6 +4219,8 @@
 #if WINDOWS
 	OleUninitialize ();
 
+    DeleteCriticalSection(&m_guard);
+
 	if (backBuffer)
 		backBuffer->forget ();
 
@@ -4268,6 +4274,23 @@
 #endif
 }
 
+CFrame::CGuard::CGuard(CFrame *frame)
+{
+    m_frame = frame;
+#if WINDOWS
+    if (frame != NULL)
+        EnterCriticalSection(&frame->m_guard);
+#endif
+}
+
+CFrame::CGuard::~CGuard()
+{
+#if WINDOWS
+    if (m_frame != NULL)
+        LeaveCriticalSection(&m_frame->m_guard);
+#endif
+}
+
 //-----------------------------------------------------------------------------
 bool CFrame::open (CPoint *point)
 {
@@ -6962,7 +6985,7 @@
 						if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
 							png_set_gray_to_rgb (png_ptr);
 						if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
-							png_set_gray_1_2_4_to_8 (png_ptr);
+							png_set_expand_gray_1_2_4_to_8 (png_ptr);
 						if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS))
 							png_set_tRNS_to_alpha (png_ptr);
 						else
@@ -6972,7 +6995,9 @@
 							png_set_swap (png_ptr);
 							png_set_strip_16 (png_ptr);
 						}
+#if 0
 						if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
+#endif
 							png_set_bgr (png_ptr);
 						png_read_update_info (png_ptr, info_ptr);
 
@@ -8367,9 +8392,28 @@
 {
 	USING_NAMESPACE_VSTGUI
 	CFrame* pFrame = (CFrame*)GetWindowLongPtr (hwnd, GWLP_USERDATA);
+    
+    CFrame::CGuard  guard(pFrame);  // avoid race conditions with idle()
 
 	switch (message)
 	{
+    case WM_NCHITTEST:
+    {
+        HWND wnd_focus, wnd_parent;
+
+        wnd_focus = GetFocus();
+        if (wnd_focus != NULL && wnd_focus != hwnd) {
+            for (wnd_parent = GetParent(hwnd); wnd_parent != NULL; wnd_parent = GetParent(wnd_parent)) {
+                if (wnd_focus == wnd_parent) {
+                    // a hack to get focus from parent & to get mousewheel messages
+                    SetFocus(hwnd);
+                    break;
+                }
+            }
+        }
+
+        break;
+    }
 	case WM_MOUSEWHEEL:
 	{
 		if (pFrame)
@@ -8377,6 +8421,10 @@
 			VSTGUI_CDrawContext context (pFrame, 0, hwnd);
 			VSTGUI_CPoint where (LOWORD (lParam), HIWORD (lParam));
 			short zDelta = (short) HIWORD(wParam);
+
+            // "where" is screen-relative
+        	where.offset(-context.offsetScreen.h, -context.offsetScreen.v);
+
 			pFrame->onWheel (&context, where, (float)zDelta / WHEEL_DELTA);
 		}
 		break;
diff -r 612a21095dca -r 42f2abc6165e vstsdk2.4/vstgui.sf/vstgui/vstgui.h
--- a/vstsdk2.4/vstgui.sf/vstgui/vstgui.h	Wed Sep 08 19:10:34 2010 +0000
+++ b/vstsdk2.4/vstgui.sf/vstgui/vstgui.h	Thu Jun 23 22:42:05 2011 +0400
@@ -1111,6 +1111,14 @@
 	virtual void dumpHierarchy ();
 	#endif
 
+    class CGuard {
+    public:
+        CGuard(CFrame *frame);
+        ~CGuard();
+    private:
+        CFrame  *m_frame;
+    };
+
 	CLASS_METHODS(CFrame, CViewContainer)
 
 	//-------------------------------------------
@@ -1134,6 +1142,8 @@
 	void*     dropTarget;
 	COffscreenContext* backBuffer;
 
+    CRITICAL_SECTION    m_guard;
+
 #elif MOTIF
 	Colormap  colormap;
 	Display  *pDisplay;
