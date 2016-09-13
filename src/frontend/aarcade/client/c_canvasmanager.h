#ifndef C_CANVAS_MANAGER_H
#define C_CANVAS_MANAGER_H

//#include "c_canvas.h"
//#include <vector>
#include "c_canvasregen.h"
#include "c_embeddedinstance.h"

class C_CanvasManager
{
public:
	C_CanvasManager();
	~C_CanvasManager();

	CCanvasRegen* GetOrCreateRegen();

	//bool IsPriorityEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance);
//	unsigned int GetNumPriorityEmbeddedInstances() { return 1; }	// just the selected embedded instance for now.  TODO: Add the hud web tab embedded instance to the priority list.
	bool ShouldRender(C_EmbeddedInstance* pEmbeddedInstance);

	// accessors

	// mutators
	void SetLastRenderedFrame(int frame) { m_iLastRenderedFrame = frame; }
//	void SetLastPriorityRenderedFrame(int frame) { m_iLastPriorityRenderedFrame = frame; }
	
private:
//	CCanvasRegen* m_pRegen;
	int m_iLastRenderedFrame;
	int m_iLastPriorityRenderedFrame;
	int m_iVisibleCanvasesLastFrame = -1;
	int m_iVisiblePriorityCanvasesLastFrame = -1;
	int m_iVisibleCanvasesCurrentFrame = 0;
	int m_iVisiblePriorityCanvasesCurrentFrame = 0;
//	int m_iLastRenderedFrame = -1;
//	int m_iLastPriorityRenderedFrame = -1;
	//m_pWebBrowser = null;
	CCanvasRegen* m_pCanvasRegen;
	C_EmbeddedInstance* m_pSelectedEmbeddedInstance;
};

#endif