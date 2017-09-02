#include "cbase.h"
#include "aa_globals.h"
#include "c_anarchymanager.h"
#include "c_steambrowsermanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_SteamBrowserListener::C_SteamBrowserListener() :
m_BrowserReady(this, &C_SteamBrowserListener::BrowserInstanceBrowserReady),
m_NeedsPaint(this, &C_SteamBrowserListener::BrowserInstanceNeedsPaint),
m_StartRequest(this, &C_SteamBrowserListener::BrowserInstanceStartRequest),
m_CloseBrowser(this, &C_SteamBrowserListener::BrowserInstanceCloseBrowser),
m_URLChanged(this, &C_SteamBrowserListener::BrowserInstanceURLChanged),
m_FinishedRequest(this, &C_SteamBrowserListener::BrowserInstanceFinishedRequest),
m_OpenLinkInTab(this, &C_SteamBrowserListener::BrowserInstanceOpenLinkInTab),
m_ChangedTitle(this, &C_SteamBrowserListener::BrowserInstanceChangedTitle),
m_SearchResults(this, &C_SteamBrowserListener::BrowserInstanceSearchResults),
m_CanGoBackAndForward(this, &C_SteamBrowserListener::BrowserInstanceCanGoBackAndForward),
m_HorizontalScroll(this, &C_SteamBrowserListener::BrowserInstanceHorizontalScroll),
m_VerticalScroll(this, &C_SteamBrowserListener::BrowserInstanceVerticalScroll),
m_LinkAtPosition(this, &C_SteamBrowserListener::BrowserInstanceLinkAtPosition),
m_JSAlert(this, &C_SteamBrowserListener::BrowserInstanceJSAlert),
m_JSConfirm(this, &C_SteamBrowserListener::BrowserInstanceJSConfirm),
m_FileOpenDialog(this, &C_SteamBrowserListener::BrowserInstanceFileOpenDialog),
m_NewWindow(this, &C_SteamBrowserListener::BrowserInstanceNewWindow),
m_SetCursor(this, &C_SteamBrowserListener::BrowserInstanceSetCursor),
m_StatusText(this, &C_SteamBrowserListener::BrowserInstanceStatusText),
m_ShowToolTip(this, &C_SteamBrowserListener::BrowserInstanceShowToolTip),
m_UpdateToolTip(this, &C_SteamBrowserListener::BrowserInstanceUpdateToolTip),
m_HideToolTip(this, &C_SteamBrowserListener::BrowserInstanceHideToolTip)
{
	DevMsg("C_SteamBrowserListener: Constructor\n");
	SetDefLessFunc(m_pendingInstanceIds);
}

C_SteamBrowserListener::~C_SteamBrowserListener()
{
	DevMsg("C_SteamBrowserListener: Destructor\n");
}

void C_SteamBrowserListener::CreateBrowser(std::string instanceId)
{
	DevMsg("C_SteamBrowserListener::CreateBrowser w/ instance ID %s\n", instanceId.c_str());

	// make sure this instanceId is *already* waiting for a callback
	if (this->HasPendingInstanceId(instanceId))
		return;

	// send the API call and wait for a created callback.
	SteamAPICall_t hAPICall = steamapicontext->SteamHTMLSurface()->CreateBrowser("", "");

	// TODO: Confirm that the callback for this initial call must be manually set.
	DevMsg("Map length A: %u\n", m_pendingInstanceIds.Count());
	m_BrowserReadyInitial.Set(hAPICall, this, &C_SteamBrowserListener::BrowserInstanceBrowserReadyInitial);
	m_pendingInstanceIds.InsertOrReplace(instanceId, 1);
	DevMsg("Map length B: %u\n", m_pendingInstanceIds.Count());
	//m_pendingInstanceIds[instanceId] = true;
}

bool C_SteamBrowserListener::HasPendingInstanceId(std::string instanceId)
{
	bool bResponse = false;

	int iInstanceMapIndex = m_pendingInstanceIds.Find(instanceId);
	if (iInstanceMapIndex != m_pendingInstanceIds.InvalidIndex())
		bResponse = true;

	return bResponse;

	/*
	auto it = m_pendingInstanceIds.find(instanceId);
	if (it != m_pendingInstanceIds.end())
		bResponse = true;
		//return true;
	
	//DevMsg("Response is: %i which is %i\n", (int)bResponse, (int)it->second);
	return bResponse;
	*/



	//return false;
	/*
	std::map<std::string, bool>::iterator it = m_pendingInstanceIds.begin();
	while (it != m_pendingInstanceIds.end())
	{
		if (it->first == instanceId)
			return true;

		it++;
	}

	return false;
	*/
}

void C_SteamBrowserListener::BrowserInstanceBrowserReadyInitial(HTML_BrowserReady_t *pResult, bool bIOFailure)
{
	DevMsg("C_SteamBrowserListener::BrowserInstanceBrowserReadyInitial: %u\n", pResult->unBrowserHandle);

	if (bIOFailure || !pResult->unBrowserHandle)
	{
		DevMsg("ERROR: IOFailure or invalid browser handle\n");

		// try again
		// FIXME: Add a limit to how many times we'll try again before accepting our failure
		SteamAPICall_t hAPICall = steamapicontext->SteamHTMLSurface()->CreateBrowser("", "");
		m_BrowserReadyInitial.Set(hAPICall, this, &C_SteamBrowserListener::BrowserInstanceBrowserReadyInitial);
		return;
	}

	// find an instance waiting for a steamworks browser (and get rid of any bad instances we come across)
	std::vector<std::string> badPendingInstances;

	C_SteamBrowserInstance* pBrowserInstance = null;
	for (int iIndex = m_pendingInstanceIds.FirstInorder(); iIndex != m_pendingInstanceIds.InvalidIndex(); iIndex = m_pendingInstanceIds.NextInorder(iIndex))
	{
		pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(m_pendingInstanceIds.Key(iIndex));

		// always remove the tested ID
		badPendingInstances.push_back(m_pendingInstanceIds.Key(iIndex));

		if (pBrowserInstance)
			break;
	}

	/*
	std::map<std::string, bool>::iterator it = m_pendingInstanceIds.begin();
	while (it != m_pendingInstanceIds.end())
	{
		pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(it->first);

		// always remove the tested ID
		badPendingInstances.push_back(it->first);

		if (pBrowserInstance)
		{
			break;
		}

		it++;
	}
	*/

	// remove the bad ids (these are ID's who's instances closed before their steamworks browser was ready
	unsigned int max = badPendingInstances.size();
	for (unsigned int i = 0; i < max; i++)
		this->RemovePendingInstance(badPendingInstances[i]);

	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Steamworks browser is ready, but there's no isntance waiting for it any more!\n");

		// close it
		DevMsg("Closing browser w/ handle %u\n", pResult->unBrowserHandle);
		steamapicontext->SteamHTMLSurface()->RemoveBrowser(pResult->unBrowserHandle);
		return;
	}

	DevMsg("Calling browser ready from initial ready...\n");
	pBrowserInstance->OnBrowserInstanceReady(pResult->unBrowserHandle);
}

void C_SteamBrowserListener::RemovePendingInstance(std::string instanceId)
{
	int iInstanceMapIndex = m_pendingInstanceIds.Find(instanceId);
	if (iInstanceMapIndex != m_pendingInstanceIds.InvalidIndex())
	{
		//DevMsg("Removing pending instance with ID: %s\n", instanceId.c_str());
		m_pendingInstanceIds.RemoveAt(iInstanceMapIndex);// .erase(it);
	}
	/*
	std::map<std::string, bool>::iterator it = m_pendingInstanceIds.find(instanceId);
	if (it != m_pendingInstanceIds.end())
	{
		DevMsg("Removing pending instance with ID: %s\n", instanceId.c_str());
		m_pendingInstanceIds.erase(it);
	}
	*/
}

void C_SteamBrowserListener::BrowserInstanceBrowserReady(HTML_BrowserReady_t *pResult)
{
	DevMsg("C_SteamBrowserListener::BrowserInstanceBrowserReady\n");
	DevMsg("WARNING: This should rarely happen.\n");
}

void C_SteamBrowserListener::BrowserInstanceCloseBrowser(HTML_CloseBrowser_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceCloseBrowser\n");
	
	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceWantsToClose();
}

void C_SteamBrowserListener::BrowserInstanceStartRequest(HTML_StartRequest_t *pResult)
{
	//if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
	//	return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceStartRequest w/ handle %u\n", pResult->unBrowserHandle);
	//SteamAPI_RunCallbacks();
	//steamapicontext->SteamHTMLSurface()->AllowStartRequest(pResult->unBrowserHandle, true);
	////SteamAPI_RunCallbacks();
	//return;

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance to load %s\n", pResult->pchURL);
		return;
	}

	pBrowserInstance->OnBrowserInstanceStartRequest(pResult->pchURL, pResult->pchTarget, pResult->pchPostData, pResult->bIsRedirect);
}

void C_SteamBrowserListener::BrowserInstanceFinishedRequest(HTML_FinishedRequest_t *pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceFinishedRequest\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceFinishedRequest(pResult->pchURL, pResult->pchPageTitle);
}

void C_SteamBrowserListener::BrowserInstanceOpenLinkInTab(HTML_OpenLinkInNewTab_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceOpenLinkInTab\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceOpenLinkInTab(pResult->pchURL);
}

void C_SteamBrowserListener::BrowserInstanceNeedsPaint(HTML_NeedsPaint_t *pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance for NeedsPaint callback.  Removing phantom web tab.\n");
		DevMsg("Closing browser w/ handle %u\n", pResult->unBrowserHandle);
		steamapicontext->SteamHTMLSurface()->RemoveBrowser(pResult->unBrowserHandle);
		//SteamAPI_RunCallbacks();
		return;
	}

	//DevMsg("Browser needs paint w/ handle %u\n", pResult->unBrowserHandle);
	//DevMsg("Page scale: %f\n", pResult->flPageScale);
	// pResult->pBRGA is a pointer to the B8G8R8A8 data for this surface, valid until SteamAPI_RunCallbacks is next called
	pBrowserInstance->OnBrowserInstanceNeedsPaint(pResult->pBGRA, pResult->unWide, pResult->unTall, pResult->unUpdateX, pResult->unUpdateY, pResult->unUpdateWide, pResult->unUpdateTall, pResult->unScrollX, pResult->unScrollY, pResult->flPageScale, pResult->unPageSerial);
}

void C_SteamBrowserListener::BrowserInstanceURLChanged(HTML_URLChanged_t *pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceURLChanged: %u %s\n", pResult->unBrowserHandle, pResult->pchURL);

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceURLChanged(pResult->pchURL, pResult->pchPostData, pResult->bIsRedirect, pResult->pchPageTitle, pResult->bNewNavigation);
}

void C_SteamBrowserListener::BrowserInstanceChangedTitle(HTML_ChangedTitle_t *pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceChangedTitle: %u %s\n", pResult->unBrowserHandle, pResult->pchTitle);

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceChangedTitle(pResult->pchTitle);
}

void C_SteamBrowserListener::BrowserInstanceSearchResults(HTML_SearchResults_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceSearchResults\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceSearchResults(pResult->unResults, pResult->unCurrentMatch);
}

void C_SteamBrowserListener::BrowserInstanceCanGoBackAndForward(HTML_CanGoBackAndForward_t* pResult)
{
	//if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
	//	return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceCanGoBackAndForward: %u\n", pResult->unBrowserHandle);

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceCanGoBackAndForward(pResult->bCanGoBack, pResult->bCanGoForward);
}

void C_SteamBrowserListener::BrowserInstanceHorizontalScroll(HTML_HorizontalScroll_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceHorizontalScroll\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceHorizontalScroll(pResult->unScrollMax, pResult->unScrollCurrent, pResult->flPageScale, pResult->bVisible, pResult->unPageSize);
}

void C_SteamBrowserListener::BrowserInstanceVerticalScroll(HTML_VerticalScroll_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceVerticalScroll\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceVerticalScroll(pResult->unScrollMax, pResult->unScrollCurrent, pResult->flPageScale, pResult->bVisible, pResult->unPageSize);
}

void C_SteamBrowserListener::BrowserInstanceLinkAtPosition(HTML_LinkAtPosition_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceLinkAtPosition\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceLinkAtPosition(pResult->x, pResult->y, pResult->pchURL, pResult->bInput, pResult->bLiveLink);
}

void C_SteamBrowserListener::BrowserInstanceJSAlert(HTML_JSAlert_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceJSAlert\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceJSAlert(pResult->pchMessage);
}

void C_SteamBrowserListener::BrowserInstanceJSConfirm(HTML_JSConfirm_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceJSConfirm\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceJSConfirm(pResult->pchMessage);
}

void C_SteamBrowserListener::BrowserInstanceFileOpenDialog(HTML_FileOpenDialog_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceFileOpenDialog\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceFileOpenDialog(pResult->pchTitle, pResult->pchInitialFile);
}

void C_SteamBrowserListener::BrowserInstanceNewWindow(HTML_NewWindow_t *pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceNewWindow\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceNewWindow(pResult->pchURL, pResult->unX, pResult->unY, pResult->unWide, pResult->unTall, pResult->unNewWindow_BrowserHandle);
}

void C_SteamBrowserListener::BrowserInstanceSetCursor(HTML_SetCursor_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceSetCursor\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceSetCursor(pResult->eMouseCursor);
}

void C_SteamBrowserListener::BrowserInstanceStatusText(HTML_StatusText_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceStatusText\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceStatusText(pResult->pchMsg);
}

void C_SteamBrowserListener::BrowserInstanceShowToolTip(HTML_ShowToolTip_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceShowToolTip\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceShowToolTip(pResult->pchMsg);
}

void C_SteamBrowserListener::BrowserInstanceUpdateToolTip(HTML_UpdateToolTip_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceUpdateToolTip\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceUpdateToolTip(pResult->pchMsg);
}

void C_SteamBrowserListener::BrowserInstanceHideToolTip(HTML_HideToolTip_t* pResult)
{
	if (!g_pAnarchyManager->OnSteamBrowserCallback(pResult->unBrowserHandle))
		return;

	DevMsg("C_SteamBrowserListener::BrowserInstanceHideToolTip\n");

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	if (!pBrowserInstance)
	{
		DevMsg("ERROR: Cannot find associated instance.\n");
		return;
	}

	pBrowserInstance->OnBrowserInstanceHideTollTip();
}