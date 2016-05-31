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
#include "c_webtab.h"

class CWebSurfaceProxy : public CEntityMaterialProxy
{
public:
	CWebSurfaceProxy();
	~CWebSurfaceProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind(C_BaseEntity *pC_BaseEntity = null);
	virtual void Release();

	// mutators
	//void SetId(std::string id) { m_id = id; }

	// accessors
	IMaterial *GetMaterial() { return m_pMaterial; }
	static CWebSurfaceRegen* GetTextureRegenerator() { return s_pWebSurfaceRegen; };

private:
	int				m_iState;
	std::string		m_id;
	C_WebTab*		m_pWebTab;
	IMaterial*		m_pMaterial;
	IMaterialVar*	m_pMaterialTextureVar;
	ITexture*		m_pOriginalTexture;
	std::string		m_originalId;
	int				m_iOriginalAutoCreate;
	std::string		m_originalUrl;

//	static int				s_textureCount;	// probably needed for cleanup??
	static CWebSurfaceRegen* s_pWebSurfaceRegen;

	ITexture* CreateTexture(C_BaseEntity* pEntity);
};

EXPOSE_INTERFACE(CWebSurfaceProxy, IMaterialProxy, "websurface" IMATERIAL_PROXY_INTERFACE_VERSION);

#endif //WEB_SURFACE_PROXY_H