#include "cbase.h"

#include "c_anarchymanager.h"
#include "aa_globals.h"
#include "c_mount.h"
#include <algorithm>
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_Mount::C_Mount()
{
	DevMsg("Mount: Constructor\n");
	m_bActive = false;
}

C_Mount::~C_Mount()
{
	DevMsg("Mount: Destructor\n");
}

void C_Mount::Init(std::string id, std::string title, std::string base, std::vector<std::string> paths)
{
	m_id = id;
	m_title = title;
	m_base = base;

	unsigned int size = paths.size();
	unsigned int i;
	for (i = 0; i < size; i++)
		m_paths.push_back(paths[i]);
}

bool C_Mount::Activate()
{
	// Loop through the SteamLibrary folders until you find what we're looking for.
	std::vector<std::string> libraryPaths = *g_pAnarchyManager->GetMountManager()->GetLibraryPaths();

	std::string path;
	unsigned int numPaths = m_paths.size();
	unsigned int i;
	bool bHasDotVpk;

	std::string libraryPath;
	unsigned int numLibraryPaths = libraryPaths.size();
	unsigned int j;
	char fixedLibraryPath[AA_MAX_STRING];

	for (i = 0; i < numPaths; i++)
	{
		path = m_paths[i];
		if (path.find(".vpk") != std::string::npos || path.find(".VPK") != std::string::npos)
			bHasDotVpk = true;
		else
			bHasDotVpk = false;

		for (j = 0; j < numLibraryPaths; j++)
		{
			Q_strcpy(fixedLibraryPath, libraryPaths[j].c_str());
			V_FixSlashes(fixedLibraryPath, '/');

			//DevMsg("Test: %s%s/%s\n", fixedLibraryPath, m_base.c_str(), path.c_str());
			if ((bHasDotVpk && g_pFullFileSystem->FileExists(VarArgs("%s%s/%s", fixedLibraryPath, m_base.c_str(), path.c_str()), "")) || g_pFullFileSystem->IsDirectory(VarArgs("%s%s/%s", fixedLibraryPath, m_base.c_str(), path.c_str()), ""))
			{
				//DevMsg("Pushing onto mounted paths: %s - %s - %s\n", fixedLibraryPath, m_base.c_str(), path.c_str());
				m_mountedPaths.push_back(VarArgs("%s%s/%s", fixedLibraryPath, m_base.c_str(), path.c_str()));
				break;
			}
		}
	}

	numPaths = m_mountedPaths.size();
	for (i = 0; i < numPaths; i++)
	{
		path = m_mountedPaths[i];
		g_pFullFileSystem->AddSearchPath(path.c_str(), "GAME", PATH_ADD_TO_TAIL);
		//engine->ClientCmd(VarArgs("snd_buildcache \"%s\";", path.c_str()));
		DevMsg("Mounted %s for %s\n", path.c_str(), m_id.c_str());
	}

	m_bActive = true;

	return true;
}

bool C_Mount::DoesOwn(std::string file)
{
	// Check in BASE + PATH for EVERY path.
	// Note that the path may be "." to indicate the BASE with nothing after it.

	/*
	
		std::string m_id;
		std::string m_base;
		std::vector<std::string> m_paths;
		std::vector<std::string> m_mountedPaths;

	*/

//	DevMsg("File is: %s\n", file.c_str());

	std::string lowerFile = file;
	std::transform(lowerFile.begin(), lowerFile.end(), lowerFile.begin(), ::tolower);

	char lowerFileBuf[AA_MAX_STRING];
	Q_strcpy(lowerFileBuf, lowerFile.c_str());
	V_FixSlashes(lowerFileBuf);
	lowerFile = lowerFileBuf;

	//DevMsg("Lower file is: %s\n", lowerFile.c_str());

	size_t found;
	std::string lowerPath;
	char lowerPathBuf[AA_MAX_STRING];
	unsigned int max = m_mountedPaths.size();
	for (unsigned int i = 0; i < max; i++)
	{
		lowerPath = m_mountedPaths[i];
		std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

		Q_strcpy(lowerPathBuf, lowerPath.c_str());
		V_FixSlashes(lowerPathBuf);
		lowerPath = lowerPathBuf;

		// if there is _dir.vpk in the lowerPath, remove the _dir part, because the actual mounted paths that Source engine generates don't use that.
		found = lowerPath.find("_dir.vpk");
		if (found != std::string::npos)
			lowerPath = lowerPath.substr(0, found) + lowerPath.substr(found + 4);

	//	DevMsg("Mounted Path: %s\n", lowerPath.c_str());

		if (lowerFile.find(lowerPath) == 0)
		{
		//	DevMsg("Found match!!\n");
			return true;
		}
	}

	return false;
}