#ifndef CANVAS_REGEN_H
#define CANVAS_REGEN_H

//#include "aa_globals.h"
#include <string>
#include "materialsystem/ITexture.h"
#include "c_embeddedinstance.h"

class CCanvasRegen : public ITextureRegenerator
{
public:
	CCanvasRegen();
	void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect );
	void Release();

	void NotifyInstanceAboutToDie(C_EmbeddedInstance* pInstance);

	// mutators
	void SetEmbeddedInstance(C_EmbeddedInstance* pInstance);

private:
	C_EmbeddedInstance* m_pEmbeddedInstance;
};

#endif