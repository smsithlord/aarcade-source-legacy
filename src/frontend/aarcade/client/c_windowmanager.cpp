#include "cbase.h"

#include "Filesystem.h"
//#include "aa_globals.h"
#include "c_windowmanager.h"
#include "c_anarchymanager.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_WindowManager::C_WindowManager()
{
	DevMsg("WindowManager: Constructor\n");

	m_pWindowPresets = null;
}

C_WindowManager::~C_WindowManager()
{
	DevMsg("WindowManager: Destructor\n");
	this->CloseAllInstances();
}

void C_WindowManager::Init()
{
	m_pWindowPresets = new KeyValues("presets");
	if (!m_pWindowPresets->LoadFromFile(g_pFullFileSystem, "window_presets.txt", "DEFAULT_WRITE_PATH"))
	{
		m_pWindowPresets->deleteThis();
		m_pWindowPresets = new KeyValues("presets");
	}

	// Add in the global hidden windows
	KeyValues* pWindowKV = m_pWindowPresets->CreateNewKey();
	pWindowKV->SetString("className", "Valve001");
	pWindowKV->SetString("title", "AArcade: Source");
	pWindowKV->SetBool("hidden", 1);

	pWindowKV = m_pWindowPresets->CreateNewKey();
	pWindowKV->SetString("className", "Progman");
	pWindowKV->SetString("title", "Program Manager");
	pWindowKV->SetBool("hidden", 1);

	pWindowKV = m_pWindowPresets->CreateNewKey();
	pWindowKV->SetString("className", "CEF-OSR-WIDGET");
	pWindowKV->SetString("title", "NVIDIA GeForce Overlay");
	pWindowKV->SetBool("hidden", 1);
}

void C_WindowManager::CloseAllInstances()
{
	// iterate over all web tabs and call their destructors
	for (auto it = m_windowInstances.begin(); it != m_windowInstances.end(); ++it)
	{
		C_WindowInstance* pWindowInstance = it->second;
		pWindowInstance->SelfDestruct();
	}

	m_windowInstances.clear();
}

C_WindowInstance* C_WindowManager::CreateWindowInstance()
{
	C_WindowInstance* pWindowInstance = new C_WindowInstance();
	return pWindowInstance;
}

C_WindowInstance* C_WindowManager::FindWindowInstance(HWND hwnd)
{
	auto foundWindowInstance = m_windowInstances.begin();
	while (foundWindowInstance != m_windowInstances.end())
	{
		if (foundWindowInstance->second->GetHWND() == hwnd)
			return foundWindowInstance->second;
		else
			foundWindowInstance++;
	}

	return null;
}

void C_WindowManager::AddInstance(C_WindowInstance* pInstance)
{
	m_windowInstances[pInstance->GetId()] = pInstance;
}

C_WindowInstance* C_WindowManager::FindWindowInstance(std::string id)
{
	auto foundWindowInstance = m_windowInstances.find(id);
	if (foundWindowInstance != m_windowInstances.end())
		return foundWindowInstance->second;
	else
		return null;
}

void C_WindowManager::DestroyWindowInstance(C_WindowInstance* pInstance)
{
	auto foundWindowInstance = m_windowInstances.find(pInstance->GetId());
	if (foundWindowInstance != m_windowInstances.end())
		m_windowInstances.erase(foundWindowInstance);

	pInstance->SelfDestruct();
}

void C_WindowManager::SwitchToWindowInstance(std::string id)
{
	m_pendingSwitchId = id;	// Delay until the user lets go of the TAB button.
}

void C_WindowManager::UnhideTask(C_WindowInstance* pWindowInstance)
{
	pWindowInstance->SetHidden(false);
}

void C_WindowManager::HideTask(C_WindowInstance* pWindowInstance)
{
	pWindowInstance->SetHidden(true);
}

bool C_WindowManager::DoPendingSwitch()
{
	if (m_pendingSwitchId != "")
	{
		std::string id = m_pendingSwitchId;
		m_pendingSwitchId = "";

		C_WindowInstance* pWindowInstance = this->FindWindowInstance(id);
		if (pWindowInstance)
		{
			SwitchToThisWindow(pWindowInstance->GetHWND(), true);	// 2nd param is to indicate if it was an alt+tab to switch there.  this might be important if it has to do with the handling of releasing the TAB button after.
			//ShowWindow(pWindowInstance->GetHWND(), SW_SHOW);
			return true;
		}
		else
			DevMsg("Unable to find Window instance w/ ID %s\n", id.c_str());
	}

	return false;
}

void C_WindowManager::CloseWindowsWindow(std::string id)
{
	C_WindowInstance* pWindowInstance = this->FindWindowInstance(id);
	if (pWindowInstance)
	{
		CloseWindow(pWindowInstance->GetHWND());	// 2nd param is to indicate if it was an alt+tab to switch there.  this might be important if it has to do with the handling of releasing the TAB button after.
	}
	else
		DevMsg("Unable to find Window instance w/ ID %s\n", id.c_str());

}

void C_WindowManager::MarkExistingInstance(C_WindowInstance* pInstance)
{
	m_existingInstances.push_back(pInstance);
}

BOOL CALLBACK C_WindowManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{

	char class_name[AA_MAX_STRING -1];
	char title[AA_MAX_STRING -1];
	GetClassName(hwnd, class_name, sizeof(class_name));
	GetWindowText(hwnd, title, sizeof(title));

	std::string windowTitle = title;
	std::string windowClass = class_name;

	if (IsWindowVisible(hwnd) && windowTitle != "")
	{
		//DevMsg("Window Type: %s - %s\n", windowClass.c_str(), windowTitle.c_str());


		C_WindowInstance* pInstance = g_pAnarchyManager->GetWindowManager()->FindWindowInstance(hwnd);
		if (!pInstance)
		{
			pInstance = g_pAnarchyManager->GetWindowManager()->CreateWindowInstance();
			pInstance->Init("", hwnd, windowTitle, windowClass);
		}
		else
		{
			pInstance->SetTitle(windowTitle);
		}

		g_pAnarchyManager->GetWindowManager()->MarkExistingInstance(pInstance);
	}

	return TRUE;
}

void C_WindowManager::PollUpdate()
{
	::EnumWindows((WNDENUMPROC)this->EnumWindowsProc, NULL);

	// Now any instance that isn't in m_existingInstances was closed by the user, and we should get rid of it too.
	std::vector<C_WindowInstance*> instances;
	for (auto it = m_windowInstances.begin(); it != m_windowInstances.end(); ++it)
		instances.push_back(it->second);

	bool bShouldRemove;
	C_WindowInstance* pInstance;
	for (unsigned int i = 0; i < instances.size(); i++)
	{
		pInstance = instances[i];

		bShouldRemove = true;

		// if this instance isn't on the existingInstances list, it should die.
		for (unsigned int j = 0; j < m_existingInstances.size(); j++)
		{
			if (pInstance == m_existingInstances[j])
			{
				bShouldRemove = false;
				break;
			}
		}

		if (bShouldRemove)
			this->DestroyWindowInstance(pInstance);
	}

	// clear it out now, because there's no reason to remember
	m_existingInstances.clear();
}

/* NEEDS ATL (ACTIVE TEMPLATE LIBRARY) TO WORK
//#include <windows.h>
//#include <atlbase.h>
void C_WindowManager::SaveIconToFile(HICON hico, LPCTSTR szFileName, BOOL bAutoDelete = FALSE)
{
	::PICTDESC pd = { sizeof(pd), PICTYPE_ICON };
	pd.icon.hicon = hico;

	CComPtr<IPicture> pPict = NULL;
	CComPtr<IStream>  pStrm = NULL;
	LONG cbSize = 0;

	BOOL res = FALSE;

	res = SUCCEEDED(::CreateStreamOnHGlobal(NULL, TRUE, &pStrm));
	res = SUCCEEDED(::OleCreatePictureIndirect(&pd, IID_IPicture, bAutoDelete, (void**)&pPict));
	res = SUCCEEDED(pPict->SaveAsFile(pStrm, TRUE, &cbSize));

	if (res)
	{
		// rewind stream to the beginning
		LARGE_INTEGER li = { 0 };
		pStrm->Seek(li, STREAM_SEEK_SET, NULL);

		// write to file
		HANDLE hFile = ::CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			DWORD dwWritten = 0, dwRead = 0, dwDone = 0;
			BYTE  buf[4096];
			while (dwDone < cbSize)
			{
				if (SUCCEEDED(pStrm->Read(buf, sizeof(buf), &dwRead)))
				{
					::WriteFile(hFile, buf, dwRead, &dwWritten, NULL);
					if (dwWritten != dwRead)
						break;
					dwDone += dwRead;
				}
				else
					break;
			}

			_ASSERTE(dwDone == cbSize);
			::CloseHandle(hFile);
		}
	}
}
*/

bool C_WindowManager::IsPresetHiddenWindow(std::string className, std::string title)
{
	for (KeyValues *sub = m_pWindowPresets->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		if (!Q_strcmp(sub->GetString("className"), className.c_str()) && !Q_strcmp(sub->GetString("title"), title.c_str()))
		{
			if (sub->GetBool("hidden"))
				return true;
		}
	}

	return false;
}

void C_WindowManager::GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances)
{
	this->PollUpdate();

	auto it = m_windowInstances.begin();
	while (it != m_windowInstances.end())
	{
		embeddedInstances.push_back(it->second);
		it++;
	}
}