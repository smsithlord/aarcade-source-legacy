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

	void PrepForTransit(char* &memberBuf, const char* value);

private:
	CNetworkVar(bool, m_bSlave);
	CNetworkVar(string_t, m_objectId);
	char* m_objectIdBuf;
	CNetworkVar(string_t, m_itemId);
	char* m_itemIdBuf;
};

#endif //PROP_SHORTCUT_ENTITY_H