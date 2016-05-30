#ifndef WEB_SURFACE_PROXY_H
#define WEB_SURFACE_PROXY_H

//#include <vector>
//#include <map>
//#include "c_baseentity.h"
#include <KeyValues.h>
#include "proxyentity.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/IMaterialProxy.h"
//#include "imaterialproxydict.h"
#include "c_websurfaceregen.h"

class CWebSurfaceProxy : public CEntityMaterialProxy
{
public:
	CWebSurfaceProxy();
	~CWebSurfaceProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind(C_BaseEntity *pC_BaseEntity = null);
	virtual void Release();

	// accessors
	IMaterial *GetMaterial();
	static CWebSurfaceRegen* GetTextureRegenerator() { return s_pWebSurfaceRegen; };

private:
	IMaterial*				m_pMaterial;
	ITexture*				m_pTexture;
	ITexture*				m_pLoadingTexture;
	IMaterialVar*			m_pMaterialTextureVar;
	//DynamicImage				m_image;
	//int						m_channel;
	//std::map<int, DynamicImage*> m_pImageInstances;	// Each prop has its own texture and simple image. The entities themselves will clean up the textures that we create for them.

	static int				s_textureCount;	// probably needed for cleanup??S
	static CWebSurfaceRegen* s_pWebSurfaceRegen;

	ITexture* CreateTexture(C_BaseEntity* pEntity);
};
EXPOSE_INTERFACE(CWebSurfaceProxy, IMaterialProxy, "websurface" IMATERIAL_PROXY_INTERFACE_VERSION);
//EXPOSE_MATERIAL_PROXY( CSimpleImageProxy, SimpleImage );

#endif //WEB_SURFACE_PROXY_H