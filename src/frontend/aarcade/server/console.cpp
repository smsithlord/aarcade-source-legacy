#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void AddGlowEffect(const CCommand &args)
{
	CBaseEntity* pEntity = CBaseEntity::Instance(Q_atoi(args[1]));
	if (pEntity)
		pEntity->AddGlowEffect();
}
ConCommand addgloweffect("addgloweffect", AddGlowEffect, "Adds a glow around the entity.", FCVAR_HIDDEN);

void RemoveGlowEffect(const CCommand &args)
{
	CBaseEntity* pEntity = CBaseEntity::Instance(Q_atoi(args[1]));
	if (pEntity)
		pEntity->RemoveGlowEffect();
}
ConCommand removegloweffect("removegloweffect", RemoveGlowEffect, "Removes a glow around the entity.", FCVAR_HIDDEN);