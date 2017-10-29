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
	DEFINE_KEYFIELD(m_objectId, FIELD_STRING, "objectId"),
	DEFINE_KEYFIELD(m_itemId, FIELD_STRING, "itemId"),
	DEFINE_KEYFIELD(m_modelId, FIELD_STRING, "modelId"),
	DEFINE_USEFUNC( UseFunc ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CPropShortcutEntity, DT_PropShortcutEntity)
	SendPropBool(SENDINFO(m_bSlave)),
	SendPropStringT(SENDINFO(m_objectId)),
	SendPropStringT(SENDINFO(m_itemId)),
	SendPropStringT(SENDINFO(m_modelId)),
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

	//SetSolid( SOLID_NONE );
	//SetSolid(SOLID_VPHYSICS);
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
	//Q_strcpymemberBuf = value;
	///*
	if (memberBuf)
		delete[] memberBuf;

	std::string buf = value;

	//memberBuf = new char[buf.size() + 1];
	memberBuf = new char[buf.length() + 1];
	std::copy(buf.begin(), buf.end(), memberBuf);
	memberBuf[buf.length()] = '\0';
	//*/
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

void CPropShortcutEntity::SetItemId(std::string id)
{
	this->PrepForTransit(m_itemIdBuf, id.c_str());
	m_itemId.Set(MAKE_STRING(m_itemIdBuf));
	//NetworkStateChanged();
	//m_itemId = MAKE_STRING(id.c_str());
	//Q_strcpy(m_itemId, id.c_str());
}

void CPropShortcutEntity::SetModelId(std::string id)
{
	DevMsg("Setting model ID to: %s\n", id.c_str());
	//string_t buf = id.c_str();
	//m_modelId.Set(MAKE_STRING(id.c_str()));

	//if (m_modelIdBuf)
		//delete[] m_modelIdBuf;

	//m_modelIdBuf = new char(id.length)

	//DevMsg("Setting model ID to: %s\n", id.c_str());
	this->PrepForTransit(m_modelIdBuf, id.c_str());
	m_modelId.Set(MAKE_STRING(m_modelIdBuf));
	//NetworkStateChanged();

	DevMsg("And val here on server is: %s\n", m_modelId.m_Value.ToCStr());
	//m_modelId = MAKE_STRING(id.c_str());
	//Q_strcpy(m_modelIdBuf, id.c_str());
	//m_modelId = MAKE_STRING(m_modelIdBuf);
	//Q_strcpy(m_itemId, id.c_str());
}

LINK_ENTITY_TO_CLASS(node_info, CNodeInfoEntity);

// Start of our data description for the class
BEGIN_DATADESC(CNodeInfoEntity)

// Save/restore our active state
//	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
//	DEFINE_FIELD( m_flNextChangeTime, FIELD_TIME ),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

// Declare our think function
//DEFINE_THINKFUNC( MoveThink ),

END_DATADESC()

// Name of our entity's model
#define	ENTITY_MODEL	"models/icons/wall_pad_w.mdl"

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void CNodeInfoEntity::Precache(void)
{
	PrecacheModel(ENTITY_MODEL);

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CNodeInfoEntity::Spawn(void)
{
	Precache();

	//SetModel(ENTITY_MODEL);
	SetSolid(SOLID_NONE);
	UTIL_SetSize(this, -Vector(1, 1, 1), Vector(1, 1, 1));
	//UTIL_SetSize(this, -Vector(20, 20, 20), Vector(20, 20, 20));
}

/*
//-----------------------------------------------------------------------------
// Purpose: Think function to randomly move the entity
//-----------------------------------------------------------------------------
void CNodeInfoEntity::MoveThink(void)
{
// See if we should change direction again
if ( m_flNextChangeTime < gpGlobals->curtime )
{
// Randomly take a new direction and speed
Vector vecNewVelocity = RandomVector( -64.0f, 64.0f );
SetAbsVelocity( vecNewVelocity );

// Randomly change it again within one to three seconds
m_flNextChangeTime = gpGlobals->curtime + random->RandomFloat( 1.0f, 3.0f );
}

// Snap our facing to where we're heading
Vector velFacing = GetAbsVelocity();
QAngle angFacing;
VectorAngles( velFacing, angFacing );
SetAbsAngles( angFacing );

// Think every 20Hz
SetNextThink( gpGlobals->curtime + 0.05f );
}
*/

//-----------------------------------------------------------------------------
// Purpose: Toggle the movement of the entity
//-----------------------------------------------------------------------------
void CNodeInfoEntity::InputToggle(inputdata_t &inputData)
{
	/*
	// Toggle our active state
	if ( !m_bActive )
	{
	// Start thinking
	SetThink( &CMyModelEntity::MoveThink );

	SetNextThink( gpGlobals->curtime + 0.05f );

	// Start moving
	SetMoveType( MOVETYPE_FLY );

	// Force MoveThink() to choose a new speed and direction immediately
	m_flNextChangeTime = gpGlobals->curtime;

	// Update m_bActive to reflect our new state
	m_bActive = true;
	}
	else
	{
	// Stop thinking
	SetThink( NULL );

	// Stop moving
	SetAbsVelocity( vec3_origin );
	SetMoveType( MOVETYPE_NONE );

	m_bActive = false;
	}
	*/
}