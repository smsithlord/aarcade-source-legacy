#include "cbase.h"

#include "c_awesomiumlisteners.h"
#include "c_anarchymanager.h"
//#include "aa_globals.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace Awesomium
{
	const char* WebStringToCharString(WebString web_string)
	{
		int len = web_string.ToUTF8(null, 0);
		char* buf = new char[len + 1];
		web_string.ToUTF8(buf, len);
		buf[len] = 0;	// null terminator

		std::string title = buf;
		delete[] buf;

		return VarArgs("%s", title.c_str());
	}
}

// MASTER
void MasterLoadListener::OnDocumentReady(WebView* caller, const WebURL& url)
{
	DevMsg("MasterLoadListener: OnDocumentReady: %s\n", WebStringToCharString(url.spec()));

	// IT IS NOW SAFE TO CREATE GLOBAL JAVASCRIPT OBJECTS
	C_WebBrowser* pWebBrowser = g_pAnarchyManager->GetWebManager()->GetWebBrowser();
//	int iState = pWebBrowser->GetState();
	//if (iState == 1)
	//{
		pWebBrowser->OnMasterWebViewDocumentReady();
		//caller->ExecuteJavascript(WSLit("window.open('data:text/html,<html><body></body></html>', '', 'width=200,height=100');"), WSLit(""));
		/*
	}
	else if (iState == 2)
	{
		// extract a web tab id
		std::string id;
		std::string spec = WebStringToCharString(url.spec());

		size_t foundPrefix = spec.find("data:text/html,<html><body>");
		if (foundPrefix != std::string::npos)
		{
			size_t foundPostfix = spec.find("</body></html>");
			if (foundPostfix != std::string::npos)
			{
				// we probably have an id
				id = spec.substr(foundPrefix + 27, foundPostfix);
			}
		}

		DevMsg("Detected ID is: %s\n", id.c_str());
		pWebBrowser->OnCreateWebViewDocumentReady(id);
		//caller->ExecuteJavascript(WSLit("window.open('data:text/html,<html><body></body></html>;', '', 'width=200,height=100');"), WSLit(""));
	}
	*/
}

void MasterViewListener::OnAddConsoleMessage(WebView* caller, const WebString &message, int line_number, const WebString &source)
{
	DevMsg("MasterViewListener: OnAddConsoleMessage: %i\t%s\n", line_number, WebStringToCharString(message));
}

void MasterViewListener::OnShowCreatedWebView(WebView *caller, WebView *new_view, const WebURL &opener_url, const WebURL &target_url, const Rect &initial_pos, bool is_popup)
{
	DevMsg("MasterViewListener: OnShowCreatedWebView: %s\n", WebStringToCharString(target_url.spec()));

	C_WebBrowser* pWebBrowser = g_pAnarchyManager->GetWebManager()->GetWebBrowser();

	std::string urlSpec = WebStringToCharString(target_url.spec());
	size_t foundPrefix = urlSpec.find("asset://newwindow/");
	if (foundPrefix == 0)
	{
		// extract a web tab id
		std::string id = urlSpec.substr(18);
		DevMsg("ID here is: %s\n", id.c_str());
		pWebBrowser->PrepareWebView(new_view, id);
	}
	else
	{
		new_view->Destroy();
	}
}

void MasterViewListener::OnChangeTargetURL(WebView* caller, const WebURL &url)
{
	DevMsg("MasterViewListener: OnChangeTargetURL: %s\n", WebStringToCharString(url.spec()));
}

// REGULAR
void LoadListener::OnDocumentReady(WebView* caller, const WebURL& url)
{
	DevMsg("LoadListener: OnDocumentReady: %s\n", WebStringToCharString(url.spec()));

	C_WebBrowser* pWebBrowser = g_pAnarchyManager->GetWebManager()->GetWebBrowser();

	std::string urlSpec = WebStringToCharString(url.spec());
	size_t foundPrefix = urlSpec.find("asset://newwindow/");
	if (foundPrefix == 0)
	{
		// extract a web tab id
		std::string id = urlSpec.substr(18);
		pWebBrowser->OnCreateWebViewDocumentReady(caller, id);
	}
	else
	{
		if (!g_pAnarchyManager->GetWebManager()->GetHudReady())
		{
			//foundPrefix = urlSpec.find("asset://ui/hud.html");
			//if (foundPrefix == 0)
				//pWebBrowser->OnHudWebViewDocumentReady(caller, g_pAnarchyManager->GetWebManager()->GetHudWebTab()->GetId());
			//else
		//	{
				foundPrefix = urlSpec.find("asset://ui/loading.html");
				if (foundPrefix == 0)
					pWebBrowser->OnHudWebViewDocumentReady(caller, g_pAnarchyManager->GetWebManager()->GetHudWebTab()->GetId());
				//pWebBrowser->OnLoadingWebViewDocumentReady(caller, g_pAnarchyManager->GetWebManager()->GetHudWebTab()->GetId());
		//	}
		}
		/*
		else
		{
			foundPrefix = urlSpec.find("asset://ui/loading.html");
			if ( foundPrefix == 0 )
				pWebBrowser->OnLoadingWebViewDocumentReady(caller, g_pAnarchyManager->GetWebManager()->GetHudWebTab()->GetId());
		}
		*/
	}
}

void ViewListener::OnAddConsoleMessage(WebView* caller, const WebString &message, int line_number, const WebString &source)
{
	DevMsg("ViewListener: OnAddConsoleMessage: %i\t%s\n", line_number, WebStringToCharString(message));
}

void ViewListener::OnShowCreatedWebView(WebView *caller, WebView *new_view, const WebURL &opener_url, const WebURL &target_url, const Rect &initial_pos, bool is_popup)
{
	DevMsg("ViewListener: OnShowCreatedWebView: %s\n", WebStringToCharString(target_url.spec()));
	//C_WebBrowser* pWebBrowser = g_pAnarchyManager->GetWebManager()->GetWebBrowser();
	//pWebBrowser->PrepareWebView(new_view);
	new_view->Destroy();
}

void ViewListener::OnChangeTargetURL(WebView* caller, const WebURL &url)
{
	DevMsg("ViewListener: OnChangeTargetURL: %s\n", WebStringToCharString(url.spec()));
}