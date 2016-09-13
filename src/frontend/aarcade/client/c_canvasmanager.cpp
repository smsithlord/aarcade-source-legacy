#include "cbase.h"

#include "aa_globals.h"
#include "c_canvasmanager.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_CanvasManager::C_CanvasManager()
{
	DevMsg("CanvasManager: Constructor\n");
	//m_pRegen = null;

	//m_iCanvasWidth = 1280;
	//m_iCanvasHeight = 720;
	//m_iCanvasWidth = 1920;
	//m_iCanvasHeight = 1080;
	m_iVisibleCanvasesLastFrame = -1;
	m_iVisiblePriorityCanvasesLastFrame = -1;
	m_iVisibleCanvasesCurrentFrame = 0;
	m_iVisiblePriorityCanvasesCurrentFrame = 0;
	m_iLastRenderedFrame = -1;
	m_iLastPriorityRenderedFrame = -1;
	//m_pWebBrowser = null;
	m_pCanvasRegen = null;
	m_pSelectedEmbeddedInstance = null;

	/*
	m_iWebSurfaceWidth = 1280;
	m_iWebSurfaceHeight = 720;
	//m_iWebSurfaceWidth = 1920;
	//m_iWebSurfaceHeight = 1080;
	m_bHudPriority = true;// false;
	m_bSelectedPriority = true;// false;
	m_iVisibleWebTabsLastFrame = -1;
	m_iVisiblePriorityWebTabsLastFrame = -1;
	m_iVisibleWebTabsCurrentFrame = 0;
	m_iVisiblePriorityWebTabsCurrentFrame = 0;
	m_iLastRenderedFrame = -1;
	m_iLastPriorityRenderedFrame = -1;
	m_pWebBrowser = null;
	m_pWebSurfaceRegen = null;
	m_pSelectedWebTab = null;
	m_pFocusedWebTab = null;
	m_pHudWebTab = null;
	m_bHudReady = false;
	*/

	//g_pAnarchyManager->SetState(AASTATE_LIBRETROMANAGER);
}

C_CanvasManager::~C_CanvasManager()
{
	DevMsg("CanvasManager: Destructor\n");
}

CCanvasRegen* C_CanvasManager::GetOrCreateRegen()
{
	if (!m_pCanvasRegen)
		m_pCanvasRegen = new CCanvasRegen;

	return m_pCanvasRegen;
}

bool C_CanvasManager::ShouldRender(C_EmbeddedInstance* pEmbeddedInstance)
{
	// don't render more than 1 web tab per frame
	if (m_iLastRenderedFrame == gpGlobals->framecount || m_iLastPriorityRenderedFrame == gpGlobals->framecount)
		return false;

	/*
	bool bIsPriorityCanvas = this->IsPriorityCanvas(pEmbeddedInstance->);
	if (bIsPriorityWebTab)
	{
		int iLastRenderedFrame = pWebTab->GetLastRenderedFrame();

		// always render the 1st time
		if (iLastRenderedFrame <= 0)
			return true;

		// render if we're the only visible (priority) web tab
		//	if (m_iVisiblePriorityWebTabsLastFrame <= 1)
		//		return true;

		// render if we've waited long enough for all other (priority) web views to render

		// Need to wait an extra tick if there are any non-priority web tabs waiting.
		int iExtraOne = 0;
		if (m_iVisibleWebTabsLastFrame > 0)
			iExtraOne = 1;

		if (gpGlobals->framecount - iLastRenderedFrame >= m_iVisiblePriorityWebTabsLastFrame + iExtraOne)
			return true;
	}
	else
	{
	*/
		//DevMsg("num priority web views: %i\n", m_iVisiblePriorityWebTabsLastFrame);
		int iLastRenderedFrame = pEmbeddedInstance->GetLastRenderedFrame();

		//	int frameCount = m_iLastPriorityRenderedFrame;// gpGlobals->framecount - m_iLastPriorityRenderedFrame;
		// we are a regular web tab

		// don't render if there are still priority web tabs waiting to render
		//	if (m_iVisiblePriorityWebTabsLastFrame > 0 && frameCount - m_iLastPriorityRenderedFrame >= m_iVisiblePriorityWebTabsLastFrame)
		//	return false;

		// always render the 1st time
		if (iLastRenderedFrame <= 0)
			return true;

		// render if we're the only visible (regular) web tab
		//	if (m_iVisibleWebTabsLastFrame <= 1)
		//		return true;

		// render if we've waited long enough for all other (regular) web views to render
		//if (frameCount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)
		if (m_iVisiblePriorityCanvasesLastFrame > 0 && m_iLastPriorityRenderedFrame - iLastRenderedFrame >= m_iVisiblePriorityCanvasesLastFrame + m_iVisibleCanvasesLastFrame - 1)
			return true;
		else if (m_iVisiblePriorityCanvasesLastFrame <= 0 && gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleCanvasesLastFrame)
			return true;
	//}

	//	if (m_iLastRenderedFrame != gpGlobals->framecount && (iLastRenderedFrame <= 0 || m_iVisibleWebTabsLastFrame <= 1 || ((pWebTab == m_pHudWebTab || pWebTab == m_pSelectedWebTab) && m_iVisiblePriorityWebTabsLastFrame <= 1) || (((pWebTab == m_pHudWebTab || pWebTab == m_pSelectedWebTab) && m_iVisiblePriorityWebTabsLastFrame <= 1) || gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)))
	//		return true;

	return false;
}