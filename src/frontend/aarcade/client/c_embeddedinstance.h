#ifndef C_EMBEDDED_INSTANCE_H
#define C_EMBEDDED_INSTANCE_H

#include "c_inputlistener.h"
//#include "c_canvas.h"
//#include <vector>

class C_EmbeddedInstance
{
public:
	C_EmbeddedInstance();
	~C_EmbeddedInstance();

	virtual void OnProxyBind(C_BaseEntity* pBaseEntity) { DevMsg("ERROR: Base method called!\n"); };
	virtual void Render() { DevMsg("ERROR: Base method called!\n"); };
	virtual void RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect) { DevMsg("ERROR: Base method called!\n"); };

	// accessors
	virtual int GetLastRenderedFrame() { return -1; }
	virtual ITexture* GetTexture() { return null; }
	virtual C_InputListener* GetInputListener() { return null; }
	
	// mutators
	
private:

};

#endif