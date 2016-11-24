#ifndef C_AWESOMIUM_LISTENERS_H
#define C_AWESOMIUM_LISTENERS_H

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
//#include "vgui_controls/Controls.h"

using namespace Awesomium;

// MASTER
class MasterLoadListener : public WebViewListener::Load
{
	void OnBeginLoadingFrame(WebView* caller, int64 frame_id, bool is_main_frame, const WebURL& url, bool is_error_page) {};
	void OnFailLoadingFrame(WebView* caller, int64 frame_id, bool is_main_frame, const WebURL& url, int error_code, const WebString& error_desc) {};
	void OnFinishLoadingFrame(WebView* caller, int64 frame_id, bool is_main_frame, const WebURL& url) {};
	void OnDocumentReady(WebView* caller, const WebURL& url);
};

class MasterViewListener : public WebViewListener::View
{
	void OnChangeTitle(WebView* caller, const WebString &title) {};
	void OnChangeAddressBar(WebView* caller, const WebURL &url) {};
	void OnChangeTooltip(WebView* caller, const WebString &tooltip) {};
	void OnChangeTargetURL(WebView* caller, const WebURL &url);
	void OnChangeCursor(WebView* caller, Cursor cursor) {};
	void OnChangeFocus(WebView* caller, FocusedElementType focused_type) {};
	void OnAddConsoleMessage(WebView* caller, const WebString &message, int line_number, const WebString &source);
	void OnShowCreatedWebView(WebView* caller, WebView* new_view, const WebURL &opener_url, const WebURL &target_url, const Rect &initial_pos, bool is_popup);
};

// REGULAR
class LoadListener : public WebViewListener::Load
{
	void OnBeginLoadingFrame(WebView* caller, int64 frame_id, bool is_main_frame, const WebURL& url, bool is_error_page) {};
	void OnFailLoadingFrame(WebView* caller, int64 frame_id, bool is_main_frame, const WebURL& url, int error_code, const WebString& error_desc) {};
	void OnFinishLoadingFrame(WebView* caller, int64 frame_id, bool is_main_frame, const WebURL& url);
	void OnDocumentReady(WebView* caller, const WebURL& url);
};

class ViewListener : public WebViewListener::View
{
	void OnChangeTitle(WebView* caller, const WebString &title) {};
	void OnChangeAddressBar(WebView* caller, const WebURL &url) {};
	void OnChangeTooltip(WebView* caller, const WebString &tooltip) {};
	void OnChangeTargetURL(WebView* caller, const WebURL &url);
	void OnChangeCursor(WebView* caller, Cursor cursor) {};
	void OnChangeFocus(WebView* caller, FocusedElementType focused_type) {};
	void OnAddConsoleMessage(WebView* caller, const WebString &message, int line_number, const WebString &source);
	void OnShowCreatedWebView(WebView* caller, WebView* new_view, const WebURL &opener_url, const WebURL &target_url, const Rect &initial_pos, bool is_popup);
};

class MenuListener : public WebViewListener::Menu
{
	void OnShowPopupMenu(WebView *caller, const WebPopupMenuInfo &menu_info);
	void OnShowContextMenu(WebView *caller, const WebContextMenuInfo &menu_info) {};
};

class ProcessListener : public WebViewListener::Process
{
	void OnUnresponsive(WebView* caller);
	void OnResponsive(WebView* caller);
	void OnCrashed(WebView* caller, TerminationStatus status);
};

#endif