#ifndef C_WEB_TAB_H
#define C_WEB_TAB_H

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include "vgui/MouseCode.h"
//#include "vgui_controls/Controls.h"

class C_WebTab
{
public:
	C_WebTab(std::string url, std::string id);
	~C_WebTab();

	// accessors
	std::string GetId() { return m_id; }
	std::string GetInitialUrl() { return m_initialUrl; }
	int GetLastRenderedFrame() { return m_iLastRenderFrame; }
	ITexture* GetTexture() { return m_pTexture; }

	// mutators
	void SetState(int state) { m_iState = state; }

	void OnProxyBind(C_BaseEntity* pBaseEntity = null);
	void Render();
	void RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect);
	void GetMousePos(float &fMouseX, float &fMouseY);
	void MouseMove(float fMouseX, float fMouseY);
	void MousePress(vgui::MouseCode code);
	void MouseRelease(vgui::MouseCode code);

private:
	float m_fMouseX;
	float m_fMouseY;
	int m_iState;
	std::string m_id;
	std::string m_initialUrl;
	ITexture* m_pTexture;
	int m_iLastRenderFrame;
};

#endif