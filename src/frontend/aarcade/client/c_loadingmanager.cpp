#include "cbase.h"
#include "c_loadingmanager.h"

//#include "aa_globals.h"
#include "c_anarchymanager.h"
//#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_LoadingManager::C_LoadingManager()
{
	DevMsg("LoadingManager: Constructor\n");
	m_pWebTab = null;
}

C_LoadingManager::~C_LoadingManager()
{
	DevMsg("LoadingManager: Destructor\n");
}


//void C_LoadingManager::Init()
//{
//	m_pWebTab = g_pAnarchyManager->GetWebManager()->
//}

void C_LoadingManager::Reset()
{
	if (m_pWebTab)
	{
		if (m_pWebTab != g_pAnarchyManager->GetWebManager()->GetHudWebTab())
			g_pAnarchyManager->GetWebManager()->RemoveWebTab(m_pWebTab);

		m_pWebTab = null;
	}
}

void C_LoadingManager::SetHeader(std::string text)
{
	LoadingMessage_t* pLoadingMessage = new LoadingMessage_t;
	pLoadingMessage->name = "setTitle";
	pLoadingMessage->arguments.push_back(text);

	m_loadingMessages.push_back(pLoadingMessage);

	if (m_pWebTab)
	{
		if (m_pWebTab->GetState() >= 2)
			this->DispatchMethods();
	}
	else
		this->CreateWebTab();
}

void C_LoadingManager::AddMessage(std::string type, std::string text, std::string title, std::string id, std::string min, std::string max, std::string current)
{
	LoadingMessage_t* pLoadingMessage = new LoadingMessage_t;
	pLoadingMessage->name = "addMessage";
	pLoadingMessage->arguments.push_back(type);
	pLoadingMessage->arguments.push_back(text);
	pLoadingMessage->arguments.push_back(title);
	pLoadingMessage->arguments.push_back(id);
	pLoadingMessage->arguments.push_back(min);
	pLoadingMessage->arguments.push_back(max);
	pLoadingMessage->arguments.push_back(current);

	m_loadingMessages.push_back(pLoadingMessage);

	if (m_pWebTab)
	{
		if (m_pWebTab->GetState() >= 2)
			this->DispatchMethods();
	}
	else
		this->CreateWebTab();
}

void C_LoadingManager::CreateWebTab()
{
	/* just use the hud
	m_pWebTab = g_pAnarchyManager->GetWebManager()->CreateWebTab("asset://ui/loading.html", "loading", false);
	g_pAnarchyManager->GetWebManager()->SelectWebTab(m_pWebTab);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true);
	*/

	m_pWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
	g_pAnarchyManager->GetWebManager()->SelectWebTab(m_pWebTab);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true);
	this->OnWebTabReady();
}

void C_LoadingManager::DispatchMethods()
{
	if (m_pWebTab->GetState() < 2)
		return;

	std::vector<LoadingMessage_t*> loadingMessages;

	/*
	std::vector<std::string> methodArguments;
	for (auto loadingMessage : m_loadingMessages)
		methodArguments.push_back(loadingMessage->text);
	m_pWebTab->DispatchJavaScriptMethod("window.loadingManager", "addMessage", methodArguments);
	*/

	for (auto loadingMessage : m_loadingMessages)
	{
		//std::vector<std::string> methodArguments;
//		methodArguments.push_back(loadingMessage->text);
		m_pWebTab->DispatchJavaScriptMethod("window.loadingManager", loadingMessage->name, loadingMessage->arguments);
	}

	this->ClearMethods();
}

void C_LoadingManager::ClearMethods()
{
	LoadingMessage_t* pLoadingMessage;
	while (!m_loadingMessages.empty())
	{
		pLoadingMessage = m_loadingMessages[m_loadingMessages.size() - 1];
		delete pLoadingMessage;
		m_loadingMessages.pop_back();
	}
}

void C_LoadingManager::OnWebTabReady()
{
	this->DispatchMethods();
}