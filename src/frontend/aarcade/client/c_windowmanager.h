#ifndef C_WINDOW_MANAGER_H
#define C_WINDOW_MANAGER_H

#include "c_windowinstance.h"
#include <map>
#include <vector>

class C_WindowManager
{
public:
	C_WindowManager();
	~C_WindowManager();

	void Init();

	void CloseAllInstances();

	bool IsPresetHiddenWindow(std::string className, std::string title);

	void GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances);
	void PollUpdate();
	C_WindowInstance* CreateWindowInstance();
	void DestroyWindowInstance(C_WindowInstance* pInstance);

	bool FindHiddenWindowTask(C_WindowInstance* pWindowInstance);
	void UnhideTask(C_WindowInstance* pWindowInstance);
	void HideTask(C_WindowInstance* pWindowInstance);
	bool DoPendingSwitch();
	void SwitchToWindowInstance(std::string id);	// should also have a version that accepts an instance directly instead of an ID
	void CloseWindowsWindow(std::string id);	// same as the note above.


	C_WindowInstance* FindWindowInstance(HWND unHandle);
	C_WindowInstance* FindWindowInstance(std::string id);

	void AddInstance(C_WindowInstance* pInstance);

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
	void MarkExistingInstance(C_WindowInstance* pInstance);

	// accessors

	// mutators

private:
	std::vector<C_WindowInstance*> m_existingInstances;
	KeyValues* m_pWindowPresets;
	std::string m_pendingSwitchId;
	std::map<std::string, C_WindowInstance*> m_windowInstances;
};

#endif