#ifndef C_STEAM_BROWSER_LISTENER_H
#define C_STEAM_BROWSER_LISTENER_H

#include "../../public/steam/steam_api.h"
#include "c_steambrowsermanager.h"
#include <map>
#include <string>

class C_SteamBrowserListener
{
public:
	C_SteamBrowserListener();
	~C_SteamBrowserListener();

	void CreateBrowser(std::string instanceId);
	bool HasPendingInstanceId(std::string instanceId);
	void RemovePendingInstance(std::string instanceId);

	CCallResult<C_SteamBrowserListener, HTML_BrowserReady_t> m_BrowserReadyInitial;
	void BrowserInstanceBrowserReadyInitial(HTML_BrowserReady_t *pResult, bool bIOFailure);
	bool OnCallback(unsigned int unHandle);

private:
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceBrowserReady, HTML_BrowserReady_t, m_BrowserReady);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceNeedsPaint, HTML_NeedsPaint_t, m_NeedsPaint);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceStartRequest, HTML_StartRequest_t, m_StartRequest);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceCloseBrowser, HTML_CloseBrowser_t, m_CloseBrowser);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceURLChanged, HTML_URLChanged_t, m_URLChanged);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceFinishedRequest, HTML_FinishedRequest_t, m_FinishedRequest);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceOpenLinkInTab, HTML_OpenLinkInNewTab_t, m_OpenLinkInTab);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceChangedTitle, HTML_ChangedTitle_t, m_ChangedTitle);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceSearchResults, HTML_SearchResults_t, m_SearchResults);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceCanGoBackAndForward, HTML_CanGoBackAndForward_t, m_CanGoBackAndForward);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceHorizontalScroll, HTML_HorizontalScroll_t, m_HorizontalScroll);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceVerticalScroll, HTML_VerticalScroll_t, m_VerticalScroll);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceLinkAtPosition, HTML_LinkAtPosition_t, m_LinkAtPosition);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceJSAlert, HTML_JSAlert_t, m_JSAlert);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceJSConfirm, HTML_JSConfirm_t, m_JSConfirm);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceFileOpenDialog, HTML_FileOpenDialog_t, m_FileOpenDialog);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceNewWindow, HTML_NewWindow_t, m_NewWindow);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceSetCursor, HTML_SetCursor_t, m_SetCursor);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceStatusText, HTML_StatusText_t, m_StatusText);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceShowToolTip, HTML_ShowToolTip_t, m_ShowToolTip);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceUpdateToolTip, HTML_UpdateToolTip_t, m_UpdateToolTip);
	STEAM_CALLBACK(C_SteamBrowserListener, BrowserInstanceHideToolTip, HTML_HideToolTip_t, m_HideToolTip);

	std::map<std::string, bool> m_pendingInstanceIds;
};

#endif