#ifndef WEB_SURFACE_REGEN_H
#define WEB_SURFACE_REGEN_H

//#include "aa_globals.h"
#include <string>
#include "materialsystem/ITexture.h"

class CWebSurfaceRegen : public ITextureRegenerator
{
public:
	CWebSurfaceRegen();
	void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect );	// calling download is the exact same (??) as calling this function directly.  it's even synchronous.
	void Release();

//	void SetDynamicImage(DynamicImage* pImage) { m_pImage = pImage; };
//	DynamicImage* GetDynamicImage() { return m_pImage; };

private:
//	DynamicImage* m_pImage;
};

#endif //WEB_SURFACE_REGEN_H