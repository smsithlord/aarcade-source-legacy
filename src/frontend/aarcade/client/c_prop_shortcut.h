#ifndef C_PROP_SHORTCUT_ENTITY_H
#define C_PROP_SHORTCUT_ENTITY_H

#include "aa_globals.h"
#include "c_props.h"
#include "c_embeddedinstance.h"
//#include "DynamicArtworkRegen.h"
#include <vector>
#include <string>

class C_PropShortcutEntity : public C_BaseAnimating
//class C_PropShortcutEntity : public C_DynamicProp
{
public:
	DECLARE_CLIENTCLASS();
	//DECLARE_CLASS(C_PropShortcutEntity, C_DynamicProp);
	DECLARE_CLASS(C_PropShortcutEntity, C_BaseAnimating);
	C_PropShortcutEntity();
	~C_PropShortcutEntity();

	virtual void	OnPreDataChanged(DataUpdateType_t updateType);
	void OnDataChanged(DataUpdateType_t updateType);
	void Spawn();
	void Release();
	void Initialize();

	void Precache();

	std::string GetItemId();
	bool GetSlave();
	void GetEmbeddedInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances);
	//void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);

	// accessors

private:
	bool m_bInitialized;
	KeyValues* pItem;
	bool m_bSlave;
	char m_itemId[AA_MAX_STRING];
};

#endif //C_PROP_SHORTCUT_ENTITY_H