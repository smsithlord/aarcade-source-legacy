#ifndef WEB_SURFACE_REGEN_H
#define WEB_SURFACE_REGEN_H

//#include "aa_globals.h"
#include <string>
#include "materialsystem/ITexture.h"
#include "c_webtab.h"

class CWebSurfaceRegen : public ITextureRegenerator
{
public:
	CWebSurfaceRegen();
	void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect );
	void Release();

	// mutators
	void SetWebTab(C_WebTab* pWebTab) { m_pWebTab = pWebTab; }

private:
	C_WebTab* m_pWebTab;
};

#endif //WEB_SURFACE_REGEN_H