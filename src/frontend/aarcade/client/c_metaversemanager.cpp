#include "cbase.h"

#include "c_anarchymanager.h"
#include "aa_globals.h"
#include "c_metaversemanager.h"
#include "filesystem.h"
#include <algorithm>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_MetaverseManager::C_MetaverseManager()
{
	DevMsg("MetaverseManager: Constructor\n");
	m_pWebTab = null;
	m_pPreviousSearchInfo = null;
}

C_MetaverseManager::~C_MetaverseManager()
{
	DevMsg("MetaverseManager: Destructor\n");

	if (m_pPreviousSearchInfo)
		m_pPreviousSearchInfo->deleteThis();

	// m_apps
	while (!m_apps.empty())
	{
		m_apps.begin()->second->deleteThis();
		m_apps.erase(m_apps.begin());
	}

	// m_models
	while (!m_models.empty())
	{
		m_models.begin()->second->deleteThis();
		m_models.erase(m_models.begin());
	}

	// m_items
	while (!m_items.empty())
	{
		m_items.begin()->second->deleteThis();
		m_items.erase(m_items.begin());
	}

	// m_types
	while (!m_types.empty())
	{
		m_types.begin()->second->deleteThis();
		m_types.erase(m_types.begin());
	}
}

void C_MetaverseManager::OnWebTabCreated(C_WebTab* pWebTab)
{
	m_pWebTab = pWebTab;
}

KeyValues* C_MetaverseManager::LoadLocalItemLegacy(bool& bIsModel, std::string file, std::string filePath, std::string workshopIds, std::string mountIds)
{
	//KeyValues* pItem2 = new KeyValues("item");
	//bIsModel = false;
//	return pItem2;

	KeyValues* pItem = new KeyValues("item");
	bool bLoaded;
	
	std::string fullFile = filePath + file;
	if (filePath != "")
	{
		//std::string fullFile = filePath + file;
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
		KeyValues* pGeneration = pItem->FindKey("generation");
		if (!pGeneration)
		{
			// update us to 3rd generation
			pItem->SetInt("generation", 3);

			// add standard info (except for id)
			pItem->SetInt("local/info/created", 0);
			pItem->SetString("local/info/owner", "local");
			pItem->SetInt("local/info/removed", 0);
			pItem->SetString("local/info/remover", "");
			pItem->SetString("local/info/alias", "");

			// determine if this is a model or not
			std::string modelFile = pItem->GetString("filelocation");
			size_t foundExt = modelFile.find(".mdl");
			if (foundExt == modelFile.length() - 4)
			{
				pItem->SetString("local/info/id", g_pAnarchyManager->GenerateUniqueId().c_str());
				//DevMsg("Unique ID: %s\n", pItem->GetString("local/info/id"));

				// build a generation 3 model
				std::string goodTitle = pItem->GetString("title");
				if (Q_strcmp(pItem->GetString("group"), ""))
					goodTitle = VarArgs("%s - %s", pItem->GetString("group"), goodTitle.c_str());

				pItem->SetString("local/title", goodTitle.c_str());
				pItem->SetString("local/keywords", "");
				pItem->SetInt("local/dynamic", 0);
				pItem->SetString(VarArgs("local/platforms/%s/id", AA_PLATFORM_ID), AA_PLATFORM_ID);
				pItem->SetString(VarArgs("local/platforms/%s/file", AA_PLATFORM_ID), modelFile.c_str());
				pItem->SetString(VarArgs("local/platforms/%s/download", AA_PLATFORM_ID), "");

				pItem->SetString(VarArgs("local/platforms/%s/workshopId", AA_PLATFORM_ID), workshopIds.c_str());
				pItem->SetString(VarArgs("local/platforms/%s/mountId", AA_PLATFORM_ID), mountIds.c_str());

				// remove everything not in local or current or generation
				std::vector<KeyValues*> victims;
				for (KeyValues *sub = pItem->GetFirstSubKey(); sub; sub = sub->GetNextKey())
				{
					if (Q_strcmp(sub->GetName(), "local") && Q_strcmp(sub->GetName(), "local") && Q_strcmp(sub->GetName(), "generation"))
						victims.push_back(sub);
						//sub->SetString(null, "");
				}

				for (unsigned int i = 0; i < victims.size(); i++)
				{
					pItem->RemoveSubKey(victims[i]);
				}
				/*
				std::string max = VarArgs("%u", 50000);
				std::string min = "0";
				std::string current = VarArgs("%u", 50000);
				g_pAnarchyManager->GetLoadingManager()->AddMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", min, current, max);
				*/

				// TODO: Generate an ID and add this to the library!!
				bIsModel = true;
				return null;
			}
			else
			{
				pItem->SetString("local/info/id", g_pAnarchyManager->ExtractLegacyId(file, pItem).c_str());

				//pItem->SetString("local/title", pItem->GetString("title"));

				std::string goodTitle = pItem->GetString("title");
				if (Q_strcmp(pItem->GetString("group"), ""))
					goodTitle = VarArgs("%s - %s", pItem->GetString("group"), goodTitle.c_str());

				pItem->SetString("local/title", goodTitle.c_str());
				pItem->SetString("local/description", pItem->GetString("description"));
				pItem->SetString("local/file", pItem->GetString("filelocation"));

				// NEEDS RESOLVING!!
				std::string legacyType = pItem->GetString("type");
				std::string resolvedType = this->ResolveLegacyType(legacyType);
				pItem->SetString("local/type", resolvedType.c_str());

				// NEEDS RESOLVING!!
				std::string legacyApp = pItem->GetString("app");
				std::string resolvedApp = this->ResolveLegacyApp(legacyApp);
				pItem->SetString("local/app", resolvedApp.c_str());
				pItem->SetString("local/reference", pItem->GetString("detailsurl"));
				pItem->SetString("local/preview", pItem->GetString("trailerurl"));
				pItem->SetString("local/download", pItem->GetString("downloadurl"));
				pItem->SetString("local/stream", "");

				// NEEDS RESOLVING!!
				std::string resolvedScreen = pItem->GetString("screens/low");
				if (resolvedScreen == "")
				{
					resolvedScreen = pItem->GetString("screenslocation");
					if (resolvedScreen.find(":") != 1 || !g_pFullFileSystem->FileExists(resolvedScreen.c_str(), ""))
						resolvedScreen = "";
				}
				pItem->SetString("local/screen", resolvedScreen.c_str());

				// NEEDS RESOLVING!!
				std::string resolvedMarquee = pItem->GetString("marquees/low");
				if (resolvedMarquee == "")
				{
					resolvedMarquee = pItem->GetString("marqueeslocation");
					if (resolvedMarquee.find(":") != 1 || !g_pFullFileSystem->FileExists(resolvedMarquee.c_str(), ""))
						resolvedMarquee = "";
				}
				pItem->SetString("local/marquee", resolvedMarquee.c_str());
				
				// TODO: Generate an ID and add this to the library!!
			}
		}
	}

	bIsModel = false;
	return pItem;
}

unsigned int C_MetaverseManager::LoadAllLocalItemsLegacy(unsigned int& uNumModels, std::string filePath, std::string workshopIds, std::string mountIds)
{
	uNumModels = 0;

	FileFindHandle_t testFolderHandle;
	std::string fullPath = filePath + "library\\*";
	
	const char *pFoldername = g_pFullFileSystem->FindFirst(fullPath.c_str(), &testFolderHandle);

	std::vector<KeyValues*> items;

	KeyValues* responseKv;
	bool bIsModel;
	unsigned int modelCount = 0;
	unsigned int count = 0;
	while (pFoldername)
	{
		if (!Q_strcmp(pFoldername, ".") || !Q_strcmp(pFoldername, ".."))
		{
			pFoldername = g_pFullFileSystem->FindNext(testFolderHandle);
			continue;
		}

		std::string FolderPath = VarArgs("library\\%s", pFoldername);
		if (!g_pFullFileSystem->FindIsDirectory(testFolderHandle))
		{
			DevMsg("All items files must be within a subfolder!\n");
			pFoldername = g_pFullFileSystem->FindNext(testFolderHandle);
			continue;
		}

		FileFindHandle_t testFileHandle;
		const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%s\\*.itm", FolderPath.c_str()), &testFileHandle);

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

			bIsModel = false;
			responseKv = this->LoadLocalItemLegacy(bIsModel, foundName, filePath, workshopIds, mountIds);
			if ( responseKv )
			{
				// don't actually add us to the m_items yet!!
				items.push_back(responseKv);
				//count++;
			}

			if (bIsModel)
				modelCount++;
		}

		g_pFullFileSystem->FindClose(testFileHandle);

		pFoldername = g_pFullFileSystem->FindNext(testFolderHandle);
	}

	g_pFullFileSystem->FindClose(testFolderHandle);

	unsigned int numResponses = items.size();
	unsigned int i;
	KeyValues* pItem;
	for (i = 0; i < numResponses; i++)
	{
		pItem = items[i];

		// now actually resolve the models and add the items!
		// NEEDS RESOLVING!!
		std::string resolvedModel = this->ResolveLegacyModel(pItem->GetString("lastmodel"));
		pItem->SetString("local/model", resolvedModel.c_str());
		//pItem->SetString("current/model", resolvedModel.c_str());

		// remove everything not in local or current or generation
		for (KeyValues *sub = pItem->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			if (Q_strcmp(sub->GetName(), "local") && Q_strcmp(sub->GetName(), "local") && Q_strcmp(sub->GetName(), "generation"))
				sub->SetString(null, "");
		}

		std::string id = VarArgs("%s", pItem->GetString("local/info/id"));
		m_items[id] = pItem;
		count++;
	}

	while (!items.empty())
		items.pop_back();

	uNumModels = modelCount;
	return count;
}

KeyValues* C_MetaverseManager::LoadLocalType(std::string file, std::string filePath)
{

	KeyValues* pType = new KeyValues("type");
	bool bLoaded;

	if (filePath != "")
	{
		std::string fullFile = filePath + file;
		bLoaded = pType->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), "");
	}
	else
		bLoaded = pType->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

	if (!bLoaded)
	{
		pType->deleteThis();
		pType = null;
	}
	else
	{
		// TODO: Look up any alias here first!!
		KeyValues* pActive = pType->FindKey("current");
		if (!pActive)
			pActive = pType->FindKey("local");

		if (pActive)
		{
			std::string id = pActive->GetString("info/id");
			m_types[id] = pType;
			DevMsg("adding type: %s\n", id.c_str());
		}
		else
		{
			pType->deleteThis();
			pType = null;
		}
	}

	return pType;
}

unsigned int C_MetaverseManager::LoadAllLocalTypes(std::string filePath)
{
	unsigned int count = 0;
	FileFindHandle_t testFileHandle;
	const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%slibrary\\types\\*.key", filePath.c_str()), &testFileHandle);

	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		foundName = VarArgs("library\\types\\%s", pFilename);
		pFilename = g_pFullFileSystem->FindNext(testFileHandle);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);

		for (int i = 0; path_buffer[i] != '\0'; i++)
			path_buffer[i] = tolower(path_buffer[i]);
		// FINISHED MAKING THE FILE PATH NICE

		foundName = path_buffer;
		if (this->LoadLocalType(foundName, filePath))
			count++;
	}

	g_pFullFileSystem->FindClose(testFileHandle);
	return count;
}

std::string C_MetaverseManager::ResolveLegacyType(std::string legacyType)
{
	if (legacyType == "")
		return "-KKa1MHJTls2KqNphWFM";

	// iterate through the types
	KeyValues* active;
	for (std::map<std::string, KeyValues*>::iterator it = m_types.begin(); it != m_types.end(); ++it)
	{
		active = it->second->FindKey("current");
		if (!active)
			active = it->second->FindKey("local");

		if (!Q_stricmp(active->GetString("title"), legacyType.c_str()))
		{
			return it->first;
		}
	}

	return "";
}

KeyValues* C_MetaverseManager::LoadLocalApp(std::string file, std::string filePath)
{

	KeyValues* pApp = new KeyValues("app");
	bool bLoaded;

	if (filePath != "")
	{
		std::string fullFile = filePath + file;
		bLoaded = pApp->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), "");
	}
	else
		bLoaded = pApp->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

	if (!bLoaded)
	{
		pApp->deleteThis();
		pApp = null;
	}
	else
	{
		// TODO: Look up any alias here first!!
		KeyValues* pActive = pApp->FindKey("current");
		if (!pActive)
			pActive = pApp->FindKey("local");

		std::string id = pActive->GetString("info/id");
		m_apps[id] = pApp;
	}
}

unsigned int C_MetaverseManager::LoadAllLocalApps(std::string filePath)
{
	unsigned int count = 0;
	FileFindHandle_t testFileHandle;
	const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%slibrary\\apps\\*.key", filePath.c_str()), &testFileHandle);

	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		foundName = VarArgs("library\\apps\\%s", pFilename);
		pFilename = g_pFullFileSystem->FindNext(testFileHandle);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);

		for (int i = 0; path_buffer[i] != '\0'; i++)
			path_buffer[i] = tolower(path_buffer[i]);
		// FINISHED MAKING THE FILE PATH NICE

		foundName = path_buffer;
		if (this->LoadLocalApp(foundName, filePath))
			count++;
	}

	g_pFullFileSystem->FindClose(testFileHandle);
	return count;
}

std::string C_MetaverseManager::ResolveLegacyApp(std::string legacyApp)
{
	if (legacyApp == "")
		return "";

	// iterate through the types
	KeyValues* active;
	for (std::map<std::string, KeyValues*>::iterator it = m_apps.begin(); it != m_apps.end(); ++it)
	{
		active = it->second->FindKey("current");
		if (!active)
			active = it->second->FindKey("local");

		if (!Q_stricmp(active->GetString("title"), legacyApp.c_str()))
			return it->first;
	}

	return "";
}

KeyValues* C_MetaverseManager::GetFirstLibraryType()
{
	m_previousGetTypeIterator = m_types.begin();
	if (m_previousGetTypeIterator != m_types.end())
		return m_previousGetTypeIterator->second;

	return null;
}

KeyValues* C_MetaverseManager::GetNextLibraryType()
{
	if (m_previousGetTypeIterator != m_types.end())
	{
		m_previousGetTypeIterator++;
		
		if (m_previousGetTypeIterator != m_types.end())
			return m_previousGetTypeIterator->second;
	}

	return null;
}

KeyValues* C_MetaverseManager::GetLibraryType(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_types.find(id);
	if (it != m_types.end())
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::GetFirstLibraryItem()
{
	m_previousGetItemIterator = m_items.begin();
	if (m_previousGetItemIterator != m_items.end())
		return m_previousGetItemIterator->second;

	return null;
}

KeyValues* C_MetaverseManager::GetNextLibraryItem()
{
	if (m_previousGetItemIterator != m_items.end())
	{
		m_previousGetItemIterator++;

		if (m_previousGetItemIterator != m_types.end())
			return m_previousGetItemIterator->second;
	}

	return null;
}

KeyValues* C_MetaverseManager::GetLibraryItem(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_items.find(id);
	if (it != m_items.end())
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryItem(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it)
{
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	//unsigned int i, numTokens;
	bool bFoundMatch;
	while (it != m_items.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = potential->FindKey("current");
		if (!active)
			active = potential->FindKey("local");
		if (active)
		{
			// active has the potential item data
			// pSearchInfo has the search criteria
			bool bGood = false;
			for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
			{
				fieldName = searchField->GetName();
				if (fieldName == "title")
				{
					if (!Q_strcmp(searchField->GetString(), ""))
						bGood = true;
					else
					{
						potentialBuf = active->GetString("title");
						std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

						searchBuf = searchField->GetString();
						std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

						/*
						searchTokens.clear();
						g_pAnarchyManager->Tokenize(searchBuf, searchTokens, " ");
						numTokens = searchTokens.size();

						for (i = 0; i < numTokens; i++)
						{
						if (searchTokens[i].length() < 2)
						continue;

						if (potentialBuf.find(searchTokens[i]) != std::string::npos)
						{
						bFoundMatch = true;
						break;
						}
						}
						*/

						if (potentialBuf.find(searchBuf) != std::string::npos)
							bGood = true;
						else
							bGood = false;
					}
				}
				else if (fieldName == "type")
				{
					if (!Q_strcmp(searchField->GetString(), "") || !Q_strcmp(active->GetString("type"), searchField->GetString()))
						bGood = true;
					else
						bGood = false;
				}

				if (!bGood)
					break;
			}

			if (bGood)
			{
				bFoundMatch = true;
				break;
			}
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryItem(KeyValues* pSearchInfo)
{
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	unsigned int i, numTokens;
	bool bFoundMatch;
	std::map<std::string, KeyValues*>::iterator it = m_items.begin();
	while (it != m_items.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = potential->FindKey("current");
		if (!active)
			active = potential->FindKey("local");
		if (active)
		{
			// active has the potential item data
			// pSearchInfo has the search criteria
			for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
			{
				fieldName = searchField->GetName();
				if (fieldName == "title")
				{
					potentialBuf = searchField->GetString();
					std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

					searchBuf = pSearchInfo->GetString(fieldName.c_str());
					searchTokens.clear();
					g_pAnarchyManager->Tokenize(searchBuf, searchTokens, ", ");
					numTokens = searchTokens.size();

					for (i = 0; i < numTokens; i++)
					{
						if (searchTokens[i].length() < 2)
							continue;

						if (potentialBuf.find(searchTokens[i]) != std::string::npos)
						{
							bFoundMatch = true;
							break;
						}
					}

					if (bFoundMatch)
						break;

				}
			}
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	// Do this here because we MUST handle the deletion for findNext and findFirst, so this usage should match!!
	pSearchInfo->deleteThis();

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindFirstLibraryItem(KeyValues* pSearchInfo)
{
	// remember this search query
	if (!m_pPreviousSearchInfo)
		m_pPreviousSearchInfo = pSearchInfo;// new KeyValues("search");
	else if (m_pPreviousSearchInfo != pSearchInfo)	// this should never be called!!!
	{
		m_pPreviousSearchInfo->deleteThis();
		m_pPreviousSearchInfo = pSearchInfo;
	}

	m_previousFindItemIterator = m_items.begin();

	// start the search
	KeyValues* response = this->FindLibraryItem(m_pPreviousSearchInfo, m_previousFindItemIterator);
	return response;
}

KeyValues* C_MetaverseManager::FindNextLibraryItem()
{
	// continue the search
	KeyValues* response = null;
	m_previousFindItemIterator++;
	if (m_previousFindItemIterator != m_items.end())
		response = this->FindLibraryItem(m_pPreviousSearchInfo, m_previousFindItemIterator);
	return response;
}

KeyValues* C_MetaverseManager::GetLibraryApp(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_apps.find(id);
	if (it != m_apps.end())
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::LoadLocalModel(std::string file, std::string filePath)
{

	KeyValues* pModel = new KeyValues("model");
	bool bLoaded;

	if (filePath != "")
	{
		std::string fullFile = filePath + file;
		bLoaded = pModel->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), "");
	}
	else
		bLoaded = pModel->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

	if (!bLoaded)
	{
		pModel->deleteThis();
		pModel = null;
	}
	else
	{
		// TODO: Look up any alias here first!!
		KeyValues* pActive = pModel->FindKey("current");
		if (!pActive)
			pActive = pModel->FindKey("local");

		std::string id = pActive->GetString("info/id");
		m_models[id] = pModel;
	}
}

unsigned int C_MetaverseManager::LoadAllLocalModels(std::string filePath)
{
	unsigned int count = 0;
	FileFindHandle_t testFileHandle;
	const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%slibrary\\models\\*.key", filePath.c_str()), &testFileHandle);

	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		foundName = VarArgs("library\\models\\%s", pFilename);
		pFilename = g_pFullFileSystem->FindNext(testFileHandle);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);

		for (int i = 0; path_buffer[i] != '\0'; i++)
			path_buffer[i] = tolower(path_buffer[i]);
		// FINISHED MAKING THE FILE PATH NICE

		foundName = path_buffer;
		if (this->LoadLocalModel(foundName, filePath))
			count++;
	}

	g_pFullFileSystem->FindClose(testFileHandle);
	return count;
}

std::string C_MetaverseManager::ResolveLegacyModel(std::string legacyModel)
{
	// iterate through the models
	KeyValues* active;
	for (std::map<std::string, KeyValues*>::iterator it = m_models.begin(); it != m_models.end(); ++it)
	{
		active = it->second->FindKey("current");
		if (!active)
			active = it->second->FindKey("local");

		// MAKE THE FILE PATH NICE
		char niceModel[AA_MAX_STRING];
		Q_strcpy(niceModel, legacyModel.c_str());
		V_FixSlashes(niceModel);

		//for (int i = 0; niceModel[i] != '\0'; i++)
		//	niceModel[i] = tolower(niceModel[i]);
		// FINISHED MAKING THE FILE PATH NICE

		if (!Q_stricmp(active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID)), niceModel))
		{
			return it->first;
		}
	}

	return "";
}