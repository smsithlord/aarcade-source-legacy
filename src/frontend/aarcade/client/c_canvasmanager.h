#ifndef C_CANVAS_MANAGER_H
#define C_CANVAS_MANAGER_H

//#include "c_canvas.h"
#include <vector>
#include "c_canvasregen.h"
#include "c_embeddedinstance.h"
#include "c_websurfaceproxy.h"

class C_CanvasManager
{
public:
	C_CanvasManager();
	~C_CanvasManager();
	
	void Update();

	CCanvasRegen* GetOrCreateRegen();

	bool IsPriorityEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance);
	unsigned int GetNumPriorityEmbeddedInstances() { return 2; }	// the selected instance, and the hud instance
	bool ShouldRender(C_EmbeddedInstance* pEmbeddedInstance);	// ALL EMBEDDED INSTANCES DECENDANTS SHOULD ACTUALLY RENDER IF THIS FUNCTION RETURNS TRUE, FOR BOOKKEEPING REASONS!!!
	// FIXME: That means that all isDirty tests need to take place FROM HERE, or PRIOR to calling this method.
	// The above 2 comments are probably bullshit and uneeded.  Look int oit if you want, or take my fuckin word for it and remove it.

	void RegisterProxy(CWebSurfaceProxy* pProxy);

	void SetMaterialEmbeddedInstanceId(IMaterial* pMaterial, std::string id);

	C_EmbeddedInstance* FindEmbeddedInstance(IMaterial* pMaterial);
	C_EmbeddedInstance* FindEmbeddedInstance(std::string id);

	void IncrementVisibleCanvasesCurrentFrame() { m_iVisibleCanvasesCurrentFrame++; }
	void IncrementVisiblePriorityCanvasesCurrentFrame() { m_iVisiblePriorityCanvasesCurrentFrame++; }

	void LevelShutdownPreEntity();

	void RefreshItemTextures(std::string itemId, std::string channel);

	void UnreferenceTexture(ITexture* pTexture);

	// accessors

	// mutators
	void SetLastRenderedFrame(int frame) { m_iLastRenderedFrame = frame; }
	void SetLastPriorityRenderedFrame(int frame) { m_iLastPriorityRenderedFrame = frame; }
	
private:
//	CCanvasRegen* m_pRegen;
	int m_iLastAllowedRenderedFrame;
	int m_iLastAllowedPriorityRenderedFrame;
	int m_iLastRenderedFrame;
	int m_iLastPriorityRenderedFrame;	// this is more like "last allowed priorityrendereed frame", doesn't mean it actually rendered. (this can be fixed.) TODO: Fix this.
	int m_iVisibleCanvasesLastFrame = -1;
	int m_iVisiblePriorityCanvasesLastFrame = -1;
	int m_iVisibleCanvasesCurrentFrame = 0;
	int m_iVisiblePriorityCanvasesCurrentFrame = 0;
//	int m_iLastRenderedFrame = -1;
//	int m_iLastPriorityRenderedFrame = -1;
	//m_pWebBrowser = null;
	CCanvasRegen* m_pCanvasRegen;
	C_EmbeddedInstance* m_pSelectedEmbeddedInstance;
	std::vector<CWebSurfaceProxy*> m_webSurfaceProxies;
	std::map<IMaterial*, std::string> m_materialEmbeddedInstanceIds;
};

#endif