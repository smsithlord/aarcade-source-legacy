#include "cbase.h"
#include "c_backpackmanager.h"

#include "aa_globals.h"
#include "c_anarchymanager.h"
#include "filesystem.h"
#include <algorithm>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_BackpackManager::C_BackpackManager()
{
	DevMsg("C_BackpackManager: Constructor\n");
	m_bInitialized = false;
}

C_BackpackManager::~C_BackpackManager()
{
	DevMsg("C_BackpackManager: Destructor\n");

	auto it = m_backpacks.begin();
	while (it != m_backpacks.end())
	{
		delete it->second;
		it++;
	}
	m_backpacks.clear();

	/*
	unsigned int max, i;
	auto it = m_details.begin();
	while (it != m_details.end())
	{
		max = it->second->keyValueTags.size();
		for (i = 0; i < max; i++)
			delete it->second->keyValueTags[i];

		it->second->keyValueTags.clear();

		delete it->second;
		it++;
	}

	m_details.clear();
	*/
}

void C_BackpackManager::Init()
{
	DevMsg("C_BackpackManager: Init\n");

	// Get non-dynamic path to the aarcade_user/custom directory
	m_customFolder = engine->GetGameDirectory();
	m_customFolder = m_customFolder.substr(0, m_customFolder.find_last_of("\\"));
	m_customFolder += "\\aarcade_user\\custom\\";

	m_bInitialized = true;

	//this->DetectAllBackpacks(); // disabled
}

C_Backpack* C_BackpackManager::GetBackpack(std::string backpackId)
{
	auto it = m_backpacks.find(backpackId);
	if (it != m_backpacks.end())
		return it->second;
	
	return null;
}

void C_BackpackManager::GetAllBackpacks(std::vector<C_Backpack*>& response)
{
	auto it = m_backpacks.begin();
	while (it != m_backpacks.end())
	{
		response.push_back(it->second);
		it++;
	}
}

void C_BackpackManager::Update()
{
	//DevMsg("C_BackpackManager: Update\n");
}

void C_BackpackManager::DetectAllBackpacks()
{
	// Find all folders in the aarcade_user/custom directory
	C_Backpack* pBackpack;
	std::string backpackFolder;
	FileFindHandle_t findHandle;
	const char* pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%s*", m_customFolder.c_str()), "", &findHandle);
	while (pFilename != NULL)
	{
		if (Q_strcmp(pFilename, ".") && Q_strcmp(pFilename, "..") && g_pFullFileSystem->FindIsDirectory(findHandle))
		{
			backpackFolder = m_customFolder + std::string(pFilename) + std::string("\\");

			// Check if this backpack was already detected
			pBackpack = this->FindBackpackByFolder(backpackFolder);
			if (pBackpack)
				DevMsg("Backpack %s already exists. Skipping.\n", pFilename);
			else
			{
				// If the backpack is not detected yet, create it.
				pBackpack = this->CreateBackpack(backpackFolder);
			}
		}

		pFilename = g_pFullFileSystem->FindNext(findHandle);
	}
}

C_Backpack* C_BackpackManager::FindBackpackWithInstanceId(std::string id)
{
	C_Backpack* pBackpack = null;
	auto it = m_backpacks.begin();
	while (it != m_backpacks.end())
	{
		pBackpack = it->second;
		if (pBackpack->HasInstance(id))
			return pBackpack;

		it++;
	}

	return null;
}

void C_BackpackManager::ActivateAllBackpacks()
{
	auto it = m_backpacks.begin();
	while (it != m_backpacks.end())
	{
		// TODO: Consider workshop's interaction with GEN1 backpacks!!
		it->second->Activate();
		it++;
	}
}

std::string C_BackpackManager::DetectRequiredBackpackForModelFile(std::string modelFile)
{
	std::string backpackId = "";
	// TODO: Consider workshop's interaction with GEN1 backpacks!!

	// BEST CAST: this model already has an entry in the library
	// TODO: work

	// else, WORST CASE: figure out where the model is from based on its file location
	char* fullPath = new char[AA_MAX_STRING];
	PathTypeQuery_t pathTypeQuery;

	std::string modelBuf = modelFile;

	//DevMsg("Here model is: %s\n", modelBuf.c_str());
	g_pFullFileSystem->RelativePathToFullPath(modelBuf.c_str(), "GAME", fullPath, AA_MAX_STRING, FILTER_NONE, &pathTypeQuery);
	modelBuf = fullPath;
	//DevMsg("And now it is: %s\n", modelBuf.c_str());

	// modelBuf is now a full file path to the BSP
	bool bIsBackpack = false;
	//bool bIsLegacyWorkshop = false;

	std::string baseDir = engine->GetGameDirectory();
	std::string importedLegacyDir = g_pAnarchyManager->GetLegacyFolder();
	std::string backpackDir = g_pAnarchyManager->GetWorkshopFolder();

	std::string customFolder = m_customFolder;
	// Source gives the model path in all-lowercase (WHY GABE N?!) so the paths we test against also need to be lowercase
	std::transform(baseDir.begin(), baseDir.end(), baseDir.begin(), ::tolower);
	std::transform(importedLegacyDir.begin(), importedLegacyDir.end(), importedLegacyDir.begin(), ::tolower);
	std::transform(customFolder.begin(), customFolder.end(), customFolder.begin(), ::tolower);

	// check for content from the workshop
	if (modelBuf.find(customFolder) == 0)
	{
		bIsBackpack = true;

		//std::string backpackFolder = modelBuf.substr(customFolder.length());
		//backpackFolder = backpackFolder.substr(0, backpackFolder.find_first_of("/\\"));
		//DevMsg("Magic backpack folder is: %s\n", modelBuf.c_str());

		C_Backpack* pBackpack = this->FindBackpackByFolder(modelBuf);
		if (pBackpack)
			backpackId = pBackpack->GetId();
	}

	return backpackId;
}

C_Backpack* C_BackpackManager::CreateBackpack(std::string backpackFolder)
{
	C_Backpack* pBackpack = new C_Backpack();
	pBackpack->Init("", "", backpackFolder);

	m_backpacks[pBackpack->GetId()] = pBackpack;
	return pBackpack;
}

std::string C_BackpackManager::ExtractBackpackFoldeNameFromPath(std::string backpackFolder)
{
	/*
	std::string backpackFolderName = backpackFolder;
	backpackFolderName = backpackFolderName.substr(0, backpackFolderName.find_last_of("\\"));
	backpackFolderName = backpackFolderName.substr(backpackFolderName.find_last_of("\\") + 1);
	*/

	std::string folderName = backpackFolder.substr(m_customFolder.length());
	folderName = folderName.substr(0, folderName.find_first_of("/\\"));
	//folderName = m_customFolder + folderName + "\\";
	//DevMsg("Magic backpack folder is: %s\n", folderName.c_str());

	return folderName;
}

C_Backpack* C_BackpackManager::FindBackpackByFolder(std::string backpackFolder)
{
	std::string backpackFolderName = m_customFolder + this->ExtractBackpackFoldeNameFromPath(backpackFolder) + "\\";
	//DevMsg("Folder name hereeeee is: %s\n", backpackFolderName.c_str());

	auto it = m_backpacks.begin();// m_backpacks.find(backpackFolderName);
	while (it != m_backpacks.end())
	{
		if (it->second->GetBackpackFolder() == backpackFolderName)
			return it->second;

		it++;
	}

	return null;
}

void C_BackpackManager::LoadBackpack(std::string backpackFolder)
{
	DevMsg("C_BackpackManager::LoadBackpack\n");
	DevMsg("Folder is: %s\n", backpackFolder.c_str());
}