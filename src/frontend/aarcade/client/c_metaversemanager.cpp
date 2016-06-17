#include "cbase.h"

#include "c_anarchymanager.h"
#include "aa_globals.h"
#include "c_metaversemanager.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_MetaverseManager::C_MetaverseManager()
{
	DevMsg("MetaverseManager: Constructor\n");
	m_pWebTab = null;
}

C_MetaverseManager::~C_MetaverseManager()
{
	DevMsg("MetaverseManager: Destructor\n");
}

void C_MetaverseManager::OnWebTabCreated(C_WebTab* pWebTab)
{
	m_pWebTab = pWebTab;
}

KeyValues* C_MetaverseManager::LoadLocalItem(std::string file, std::string filePath)
{
	KeyValues* pItem = new KeyValues("item");
	bool bLoaded;
	
	if (filePath != "")
	{
		std::string fullFile = filePath + file;
		bLoaded = pItem->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), "");
	}
	else
		bLoaded = pItem->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

	if ( !bLoaded )
	{
		pItem->deleteThis();
		pItem = null;
	}
	else
	{
		// determine the generation of this item
		int generation = -1;

		KeyValues* pGeneration = pItem->FindKey("info/generation");
		if (!pGeneration)
		{
//			DevMsg("Loading %s\\%s\n", filePath.c_str(), file.c_str());
			generation = 2;

			// translate this generation 2 item into a generation 3 object
			/*
			std::string keys = "";
			int counter = 0;
			for (KeyValues *sub = pItem->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				if (counter != 0)
					keys += ", ";

				keys += sub->GetName();
				counter++;
			}

			DevMsg("Debug Infoooo: %s\n", keys.c_str());
			*/
		}
		else
			generation = pGeneration->GetInt();
	}

	return pItem;
}

unsigned int C_MetaverseManager::LoadAllLocalItems(std::string filePath)
{
	FileFindHandle_t testFolderHandle;
	std::string fullPath = filePath + "library\\*";
	
	const char *pFoldername = g_pFullFileSystem->FindFirst(fullPath.c_str(), &testFolderHandle);

	unsigned int count = 0;
	while (pFoldername)
	{
		if (!Q_strcmp(pFoldername, ".") || !Q_strcmp(pFoldername, ".."))
		{
			pFoldername = g_pFullFileSystem->FindNext(testFolderHandle);
			continue;
		}

		std::string FolderPath = VarArgs("library/%s", pFoldername);
		if (!g_pFullFileSystem->FindIsDirectory(testFolderHandle))
		{
			DevMsg("All items files must be within a subfolder!\n");
			pFoldername = g_pFullFileSystem->FindNext(testFolderHandle);
			continue;
		}

		FileFindHandle_t testFileHandle;
		const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%s/*.itm", FolderPath.c_str()), &testFileHandle);

		while (pFilename != NULL)
		{
			if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
			{
				pFilename = g_pFullFileSystem->FindNext(testFileHandle);
				continue;
			}

			// Check real quick if this item is of the format [itemName].[workshop_id].itm, ie. check if it has 2 dots or only 1.
			// PROBABLY STILL REQUIRED FOR LEGACY SUPPORT
			std::string foundName = pFilename;
			foundName = foundName.substr(foundName.find_first_of(".") + 1);

			if (foundName.find(".") != std::string::npos)
				continue;

			foundName = VarArgs("%s/%s", FolderPath.c_str(), pFilename);
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);

			// MAKE THE FILE PATH NICE
			char path_buffer[AA_MAX_STRING];
			Q_strcpy(path_buffer, foundName.c_str());
			V_FixSlashes(path_buffer);

			for (int i = 0; path_buffer[i] != '\0'; i++)
				path_buffer[i] = tolower(path_buffer[i]);
			// FINISHED MAKING THE FILE PATH NICE

			foundName = path_buffer;

			//this->AddItemFile(foundName.c_str());

			if (this->LoadLocalItem(foundName, filePath))
				count++;
		}

		g_pFullFileSystem->FindClose(testFileHandle);

		pFoldername = g_pFullFileSystem->FindNext(testFolderHandle);
	}

	g_pFullFileSystem->FindClose(testFolderHandle);

	return count;
}