#include "cbase.h"
#include "prop_shortcut_entity.h"
//#include "assert.h"

//#include "hl2mp_player.h"

#include <KeyValues.h>
#include "Filesystem.h"

//#include "inetchannelinfo.h"
//#include "arcaderesources.h"

#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( prop_shortcut, CPropShortcutEntity );

BEGIN_DATADESC(CPropShortcutEntity)
	DEFINE_KEYFIELD(m_bSlave, FIELD_BOOLEAN, "slave"),
	DEFINE_KEYFIELD(m_itemId, FIELD_STRING, "itemId"),
	DEFINE_USEFUNC( UseFunc ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CPropShortcutEntity, DT_PropShortcutEntity)
	SendPropBool(SENDINFO(m_bSlave)),
	SendPropStringT(SENDINFO(m_itemId)),
END_SEND_TABLE()

#define	ENTITY_MODEL	"models/cabinets/two_player_arcade.mdl"

CPropShortcutEntity::CPropShortcutEntity()
{
}

void CPropShortcutEntity::Precache(void)
{
	PrecacheModel( UTIL_VarArgs("%s", this->GetModelName()) );
	BaseClass::Precache();
}

void CPropShortcutEntity::Spawn()
{
//	DevMsg("Yaaaarrrr!!!!\n\n");
	AddEffects(EF_NOSHADOW);
	BaseClass::Spawn();

	Precache();
	SetModel( UTIL_VarArgs("%s", this->GetModelName()) );	// We should figure out what our model is going to be before we get to this line.

	SetSolid( SOLID_NONE );
	SetMoveType(MOVETYPE_NONE);
	//SetSolid( SOLID_VPHYSICS );
	
	//UTIL_SetSize( this, -Vector(100,100,100), Vector(100,100,100) );
	//SetMoveType( MOVETYPE_VPHYSICS );
	//SetUse( &CPropHotlinkEntity::UseFunc );

	/*
		if( CreateVPhysics() )
		{
			IPhysicsObject *pPhysics = this->VPhysicsGetObject();
			if( pPhysics )
			{
				pPhysics->EnableMotion(!nophysics);
			}
		}

		//	SetMoveType(MOVETYPE_NONE);
		IPhysicsObject* pPhysics = this->VPhysicsGetObject();
		if (!pPhysics && this->CreateVPhysics())
			pPhysics = this->VPhysicsGetObject();

		if (pPhysics)
		{
			pPhysics->EnableMotion(false);
		}
	*/
}

void CPropShortcutEntity::UseFunc(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	
}

void CPropShortcutEntity::PrepForTransit(char* &memberBuf, const char* value)
{
	if (memberBuf)
		delete[] memberBuf;

	std::string buf = value;

	memberBuf = new char[buf.size() + 1];
	std::copy(buf.begin(), buf.end(), memberBuf);
	memberBuf[buf.size()] = '\0';
}

/*
void CPropHotlinkEntity::SetLiveURL(const char* value)
{
	this->PrepForTransit(m_liveURLBuf, value);

	m_liveURL.Set(MAKE_STRING(m_liveURLBuf));
}
*/

bool CPropShortcutEntity::CreateVPhysics()
{
	BaseClass::CreateVPhysics();
	VPhysicsInitNormal( SOLID_VPHYSICS, 0, false );
	return true;
}