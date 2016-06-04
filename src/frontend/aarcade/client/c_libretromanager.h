#ifndef C_LIBRETRO_MANAGER_H
#define C_LIBRETRO_MANAGER_H

#include "c_libretroinstance.h"
//#include "c_libretrosurfaceregen.h"
#include <map>

class C_LibretroManager
{
public:
	C_LibretroManager();
	~C_LibretroManager();

	void Update();

	C_LibretroInstance* CreateLibretroInstance();
	bool SelectLibretroInstance(C_LibretroInstance* pLibretroInstance);
	void OnLibretroInstanceCreated(C_LibretroInstance* pLibretroInstance);
	C_LibretroInstance* FindLibretroInstance(CSysModule* pModule);
	C_LibretroInstance* FindLibretroInstance(uint uId);

	// accessors
	C_LibretroInstance* GetSelectedLibretroInstance() { return m_pSelectedLibretroInstance; }

	// mutators	

private:
	C_LibretroInstance* m_pSelectedLibretroInstance;
	std::map<CSysModule*, C_LibretroInstance*> m_libretroInstances;
	std::map<uint, CSysModule*> m_libretroInstancesModules;

};

#endif