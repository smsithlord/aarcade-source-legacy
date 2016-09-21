#ifndef C_EMBEDDED_INSTANCE_H
#define C_EMBEDDED_INSTANCE_H

#include "c_inputlistener.h"
//#include "c_canvas.h"
#include <string>

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

	std::string GetCharTyped(vgui::KeyCode code, bool bShift = false, bool bCtrl = false, bool bAlt = false);

	// accessors
	virtual int GetLastRenderedFrame() { return -1; }
	virtual ITexture* GetTexture() { return null; }
	virtual C_InputListener* GetInputListener() { return null; }
	
	// mutators
	
private:

};

#endif