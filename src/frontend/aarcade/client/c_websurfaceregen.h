#ifndef WEB_SURFACE_REGEN_H
#define WEB_SURFACE_REGEN_H

//#include "aa_globals.h"
#include <string>
#include "materialsystem/ITexture.h"
#include "c_embeddedinstance.h"

class CWebSurfaceRegen : public ITextureRegenerator
{
public:
	CWebSurfaceRegen();
	void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect );
	void Release();

	// mutators
	void SetEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance) { m_pEmbeddedInstance = pEmbeddedInstance; }

private:
	C_EmbeddedInstance* m_pEmbeddedInstance;
};

#endif //WEB_SURFACE_REGEN_H