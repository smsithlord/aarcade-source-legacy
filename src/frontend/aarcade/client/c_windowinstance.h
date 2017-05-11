#ifndef C_WINDOW_INSTANCE_H
#define C_WINDOW_INSTANCE_H

#include "cbase.h"
#include "c_embeddedinstance.h"
#include <string>
#include <Windows.h>
//#include "vgui_controls/Controls.h"

class C_WindowInstance : public C_EmbeddedInstance
{
public:
	C_WindowInstance();
	~C_WindowInstance();

	void SelfDestruct();

	// PURE VIRTUALS
	void OnProxyBind(C_BaseEntity* pBaseEntity) {};
	void Render() {};
	void RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect) {};
	bool IsSelected() { return false; }
	bool HasFocus() { return false; }
	bool Focus() { return false; }
	bool Blur() { return false; }
	bool Select() { return false; }
	bool Deselect() { return false; }
	C_EmbeddedInstance* GetParentSelectedEmbeddedInstance() { return null; }
	void Update() {};
	void Close();
	std::string GetURL() { return ""; }

	std::string GetOutput(vgui::KeyCode code, bool bShift = false, bool bCtrl = false, bool bAlt = false, bool bWin = false, bool bAutorepeat = false);

	// accessors
	std::string GetId() { return m_id; }
	std::string GetTitle() { return m_title; }
	int GetLastRenderedFrame() { return -1; }
	int GetLastVisibleFrame() { return -1; }
	ITexture* GetTexture() { return null; }
	C_InputListener* GetInputListener() { return null; }
	std::string GetOriginalItemId() { return ""; }
	int GetOriginalEntIndex() { return -1; }

	// mutators
	void SetOriginalItemId(std::string itemId) {};
	void SetOriginalEntIndex(int index) {};
	void SetTitle(std::string title) { m_title = title; }

	// NON-PURE-VIRTUALS, however, some of them probably should be.
	void Init(std::string id = "", HWND hwnd = null, std::string title = "", std::string className = "");

	// accessors
	HWND GetHWND() { return m_hwnd; }
	bool IsHidden() { return m_bHidden; }
	bool IsPresetHidden() { return m_bPresetHidden; }
	std::string GetWindowClassName() { return m_className; }

	// mutators	
	void SetHWND(HWND hwnd) { m_hwnd = hwnd; }
	void SetHidden(bool bHidden) { m_bHidden = bHidden; }
	void SetPresetHidden(bool bHidden) { m_bPresetHidden = bHidden; }

private:
	bool m_bHidden;
	bool m_bPresetHidden;
	std::string m_title;
	std::string m_className;
	std::string m_id;
	HWND m_hwnd;
};

#endif