#ifndef PROP_SHORTCUT_ENTITY_H
#define PROP_SHORTCUT_ENTITY_H

#include "props.h"
/*
#include "materialsystem/ITexture.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/IMaterialProxy.h"
*/
//#include "../game/server/triggers.h"
//#include <vector>

#include <string>

//class CPropShortcutEntity : public CDynamicProp
class CPropShortcutEntity : public CBaseAnimating
{
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	//DECLARE_CLASS(CPropShortcutEntity, CDynamicProp);
	DECLARE_CLASS(CPropShortcutEntity, CBaseAnimating);
	CPropShortcutEntity();
	void Spawn();
	void Precache( void );
	bool CreateVPhysics();
	void UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int ObjectCaps()	{ return (BaseClass::ObjectCaps() | FCAP_IMPULSE_USE); }
	void SetItemId(std::string id);
	void SetModelId(std::string id);

	void PrepForTransit(char* &memberBuf, const char* value);

private:
	CNetworkVar(bool, m_bSlave);
	CNetworkVar(string_t, m_objectId);
	char* m_objectIdBuf;
	CNetworkVar(string_t, m_itemId);
	char* m_itemIdBuf;
	CNetworkVar(string_t, m_modelId);
	char* m_modelIdBuf;
};

class CNodeInfoEntity : public CBaseAnimating
{
public:
	DECLARE_CLASS(CNodeInfoEntity, CBaseAnimating);
	DECLARE_DATADESC();

	CNodeInfoEntity()
	{
		//m_bActive = false;
	}

	void Spawn(void);
	void Precache(void);

	//void MoveThink( void );

	// Input function
	void InputToggle(inputdata_t &inputData);

private:

	//bool	m_bActive;
	//float	m_flNextChangeTime;
};

#endif //PROP_SHORTCUT_ENTITY_H