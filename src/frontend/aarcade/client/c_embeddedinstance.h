#ifndef C_EMBEDDED_INSTANCE_H
#define C_EMBEDDED_INSTANCE_H

#include "c_inputlistener.h"
//#include "c_canvas.h"
#include <string>
#include <map>

class C_EmbeddedInstance
{
public:
	C_EmbeddedInstance();
	~C_EmbeddedInstance();

	virtual void OnProxyBind(C_BaseEntity* pBaseEntity) { DevMsg("ERROR: Base method called!\n"); };
	virtual void Render() { DevMsg("ERROR: Base method called!\n"); };
	virtual void RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect) { DevMsg("ERROR: Base method called!\n"); };
	virtual bool IsSelected() { return false; }
	virtual bool HasFocus() { return false; }	// This should be IsSelected!! (HasFocus can't always be determined, depending on the embedded instance.)
	virtual bool Focus() { return false; }
	virtual bool Blur() { return false; }
	virtual bool Select() { return false; }
	virtual bool Deselect() { return false; }
	virtual C_EmbeddedInstance* GetParentSelectedEmbeddedInstance() { return null; }
	virtual std::string GetId();
	virtual void Update() { DevMsg("ERROR: Base method called!\n"); };
	virtual void Close() {};
	virtual std::string GetURL() { return ""; }

	std::string GetOutput(vgui::KeyCode code, bool bShift = false, bool bCtrl = false, bool bAlt = false, bool bWin = false, bool bAutorepeat = false);

	// accessors
	virtual int GetLastRenderedFrame() { return -1; }
	virtual int GetLastVisibleFrame() { return -1; }
	virtual ITexture* GetTexture() { return null; }
	virtual C_InputListener* GetInputListener() { return null; }
	virtual std::string GetOriginalItemId() { return ""; }
	virtual int GetOriginalEntIndex() { return -1; }
	
	// mutators
	virtual void SetOriginalItemId(std::string itemId) {}
	virtual void SetOriginalEntIndex(int index) {};
	
private:
	static std::map<ButtonCode_t, const char*> s_buttonMap;
	static std::map<ButtonCode_t, const char*> s_shiftedButtonMap;
};

#endif