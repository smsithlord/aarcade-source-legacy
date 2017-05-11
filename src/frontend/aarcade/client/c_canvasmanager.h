#ifndef C_CANVAS_MANAGER_H
#define C_CANVAS_MANAGER_H

//#include "c_canvas.h"
#include <vector>
#include "c_canvasregen.h"
#include "c_embeddedinstance.h"
#include "c_websurfaceproxy.h"

struct DeferredCleanupTexture_t
{
	uint64 startTime;
	ITexture* pTexture;
};

class C_CanvasManager
{
public:
	C_CanvasManager();
	~C_CanvasManager();
	
	void Update();

	CCanvasRegen* GetOrCreateRegen();

	DeferredCleanupTexture_t* FindDeferredCleanupTexture(ITexture* pTexture);
	DeferredCleanupTexture_t* FindDeferredCleanupTexture(std::string textureName);

	void DoOrDeferTextureCleanup(ITexture* pTexture);
	void TextureNotDeferred(ITexture* pTexture);
	void CleanupTextures();
	void CleanupTexture(ITexture* pTexture);

	bool IsPriorityEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance);
	unsigned int GetNumPriorityEmbeddedInstances() { return 2; }	// the selected instance, and the hud instance
	bool ShouldRender(C_EmbeddedInstance* pEmbeddedInstance, bool bPreTest = false);	// ALL EMBEDDED INSTANCES DECENDANTS SHOULD ACTUALLY RENDER IF THIS FUNCTION RETURNS TRUE, FOR BOOKKEEPING REASONS!!! (unless pretest = true)
	// FIXME: That means that all isDirty tests need to take place FROM HERE, or PRIOR to calling this method.
	// The above 2 comments are probably bullshit and uneeded.  Look int oit if you want, or take my fuckin word for it and remove it.

	void RegisterProxy(CWebSurfaceProxy* pProxy);

	void SetMaterialEmbeddedInstanceId(IMaterial* pMaterial, std::string id);

	C_EmbeddedInstance* FindEmbeddedInstance(IMaterial* pMaterial);
	C_EmbeddedInstance* FindEmbeddedInstance(std::string id);

	void IncrementVisibleCanvasesCurrentFrame() { m_iVisibleCanvasesCurrentFrame++; }
	void IncrementVisiblePriorityCanvasesCurrentFrame() { m_iVisiblePriorityCanvasesCurrentFrame++; }

	void LevelShutdownPreEntity();
	void LevelShutdownPostEntity();
	void CloseAllInstances();
	void CloseInstance(std::string id);
	void SwitchToInstance(std::string id);
	void GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances);

	void CaptureInstanceThumbnail(C_EmbeddedInstance* pEmbeddedInstance);

	void RefreshItemTextures(std::string itemId, std::string channel);

	void UnreferenceTexture(ITexture* pTexture);
	void UnreferenceEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance);

	C_EmbeddedInstance* GetFirstInstanceToDisplay();

	// accessors
	C_EmbeddedInstance* GetDisplayInstance() { return m_pDisplayInstance; }

	// mutators
	void SetDisplayInstance(C_EmbeddedInstance* pEmbeddedInstance) { m_pDisplayInstance = pEmbeddedInstance;  }
	void SetDifferentDisplayInstance(C_EmbeddedInstance* pEmbeddedInstance);
	void SetLastRenderedFrame(int frame) { m_iLastRenderedFrame = frame; }
	void SetLastPriorityRenderedFrame(int frame) { m_iLastPriorityRenderedFrame = frame; }
	
private:
	bool m_bUseDeferredTextureCleanUp;
	std::vector<std::string> m_deadTextures;
	//std::vector<std::map<std::string, DeferredCleanupTexture_t*>::iterator> m_deadTextures;
	//std::vector<ITexture*> m_pendingTextureCleanup;
	//std::map<std::string, DeferredCleanupTexture_t*> m_pendingTextureCleanup;
	std::vector<DeferredCleanupTexture_t*> m_pendingTextureCleanup;
	C_EmbeddedInstance* m_pDisplayInstance;

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
	//C_EmbeddedInstance* m_pSelectedEmbeddedInstance;
	std::vector<CWebSurfaceProxy*> m_webSurfaceProxies;
	std::map<IMaterial*, std::string> m_materialEmbeddedInstanceIds;
};

#endif