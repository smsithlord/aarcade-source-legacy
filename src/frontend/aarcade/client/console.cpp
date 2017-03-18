#include "cbase.h"

//#include "c_simple_image_entity.h"
//#include "c_webViewInput.h"
//#include "aa_globals.h"
#include "c_anarchymanager.h"

#include "c_openglmanager.h"
#include "filesystem.h"
#include <algorithm>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//ConVar xbmc_enable( "xbmc_enable", "0", FCVAR_ARCHIVE );
ConVar default_width( "default_width", "256", FCVAR_ARCHIVE);
ConVar default_height( "default_height", "256", FCVAR_ARCHIVE);
ConVar broadcast_mode("broadcast_mode", "0", FCVAR_NONE);
ConVar kodi_info("kodi_info", "xbmc:xbmc@192.168.0.100:8080", FCVAR_ARCHIVE, "The username:password@ip:port of the 1st Kodi host.");

bool IsFileEqual(const char* inFileA, std::string inFileB)
{
	std::string fileA = inFileA;
	std::string fileB = inFileB;

	std::transform(fileA.begin(), fileA.end(), fileA.begin(), ::tolower);
	std::transform(fileB.begin(), fileB.end(), fileB.begin(), ::tolower);

	std::replace(fileA.begin(), fileA.end(), '\\', '/');
	std::replace(inFileB.begin(), inFileB.end(), '\\', '/');

	return (fileA == fileB);
}

void DumpItem(const CCommand &args)
{
	KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(std::string(args[1])));
	if (pItemKV)
	{
		DevMsg("Item %s:\n", args[1]);
		for (KeyValues *sub = pItemKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			if (!sub->GetFirstSubKey())
				DevMsg("\t%s: %s\n", sub->GetName(), sub->GetString());
			else
			{
				DevMsg("\t%s:\n", sub->GetName());
				for (KeyValues *sub2 = sub->GetFirstSubKey(); sub2; sub2 = sub2->GetNextKey())
				{
					if (!sub2->GetFirstSubKey())
						DevMsg("\t\t%s: %s\n", sub2->GetName(), sub2->GetString());
					else
					{
						DevMsg("\t\t%s:\n", sub2->GetName());
						for (KeyValues *sub3 = sub2->GetFirstSubKey(); sub3; sub3 = sub3->GetNextKey())
						{
							if (!sub3->GetFirstSubKey())
								DevMsg("\t\t\t%s: %s\n", sub3->GetName(), sub3->GetString());
						}
					}
				}
			}
		}
	}
}
ConCommand dump_item("dump_item", DumpItem, "Usage: dump the item for the given item ID to the console");

void TestFunction( const CCommand &args )
{
	// WORKING SEND/RECIEVE FILE CALLS
	//#include "inetchannel.h"
//	INetChannel* pINetChannel = static_cast<INetChannel*>(engine->GetNetChannelInfo());
//	pINetChannel->RequestFile("downloads/<hash>.vtf", false);
//	pINetChannel->SendFile("stuff/test.jpg", 0, false);

	// WORKING CURSOR POSITIONS
//	#include "vgui/IInput.h"
//	#include <vgui_controls/Controls.h>
//	int x, y;
//	vgui::input()->GetCursorPos(x, y);

	// save out text versions of all the required stuff so that they can be added to the player's library when ever needed (setup or repair)
	// SCRAPERS (just make backup copies of the scraper .js files)

	/*
	unsigned int i;
	unsigned int max;
	KeyValues* entry;
	std::string fileName;
	size_t found;
	std::map<std::string, KeyValues*>::iterator it;
	bool bIsDefault;

	// MAPS
	std::vector<std::string> defaultMapNames;
	defaultMapNames.push_back("dm_lockdown.bsp");
	defaultMapNames.push_back("hub_floors.bsp");
	defaultMapNames.push_back("hub_highrise.bsp");
	defaultMapNames.push_back("hub_walls.bsp");
	defaultMapNames.push_back("learn_basic.bsp");
	defaultMapNames.push_back("meta_hood.bsp");
	defaultMapNames.push_back("oververse.bsp");
	defaultMapNames.push_back("sm_acreage.bsp");
	defaultMapNames.push_back("sm_apartment.bsp");
	defaultMapNames.push_back("sm_apartmentsuite.bsp");
	defaultMapNames.push_back("sm_expo.bsp");
	defaultMapNames.push_back("sm_gallery.bsp");
	defaultMapNames.push_back("sm_garage.bsp");
	defaultMapNames.push_back("sm_orchard.bsp");
	defaultMapNames.push_back("sm_primo.bsp");

	std::vector<std::string>::iterator defaultMapsIt;
	std::map<std::string, KeyValues*>& maps = g_pAnarchyManager->GetMetaverseManager()->GetAllMaps();
	it = maps.begin();
	while (it != maps.end())
	{
		entry = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);
		defaultMapsIt = std::find(defaultMapNames.begin(), defaultMapNames.end(), std::string(entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file")));
		if (defaultMapsIt != defaultMapNames.end())
		{
			//DevMsg("Map file: %s\n", entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"));
			fileName = entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file");
			found = fileName.find_last_of(".");
			fileName = fileName.substr(0, found);

			it->second->SaveToFile(g_pFullFileSystem, VarArgs("defaultLibrary/maps/%s.txt", fileName.c_str()), "DEFAULT_WRITE_PATH");
		}

		it++;
	}

	// CABINETS
	std::vector<std::string> defaultCabinetNames;
	defaultCabinetNames.push_back("models/cabinets/wood_cabinet_steam.mdl");
	defaultCabinetNames.push_back("models/cabinets/wood_cabinet.mdl");
	defaultCabinetNames.push_back("models/icons/wall_pad_w.mdl");
	defaultCabinetNames.push_back("models/icons/wall_pad_t.mdl");
	defaultCabinetNames.push_back("models/cabinets/wall_arcade.mdl");
	defaultCabinetNames.push_back("models/cabinets/two_player_arcade.mdl");
	defaultCabinetNames.push_back("models/cabinets/tripple_standup_racer.mdl");
	defaultCabinetNames.push_back("models/cabinets/trading_card_table.mdl");
	defaultCabinetNames.push_back("models/cabinets/trading_card_big_table.mdl");
	defaultCabinetNames.push_back("models/cabinets/trading_card_big.mdl");
	defaultCabinetNames.push_back("models/cabinets/trading_card.mdl");
	defaultCabinetNames.push_back("models/cabinets/theater_facade.mdl");
	defaultCabinetNames.push_back("models/frames/tall_rotator.mdl");
	defaultCabinetNames.push_back("models/cabinets/tabletop_tv.mdl");
	defaultCabinetNames.push_back("models/cabinets/tabletop_console_steam.mdl");
	defaultCabinetNames.push_back("models/icons/table_pad_w.mdl");
	defaultCabinetNames.push_back("models/icons/table_pad_t.mdl");
	defaultCabinetNames.push_back("models/cabinets/swordfish.mdl");
	defaultCabinetNames.push_back("models/cabinets/standup_car_racer.mdl");
	defaultCabinetNames.push_back("models/banners/spinning_cap.mdl");
	defaultCabinetNames.push_back("models/cabinets/sound_pillar.mdl");
	defaultCabinetNames.push_back("models/cabinets/single_car_racer.mdl");
	defaultCabinetNames.push_back("models/cabinets/racer_multiscreen.mdl");
	defaultCabinetNames.push_back("models/cabinets/posterscreen.mdl");
	defaultCabinetNames.push_back("models/cabinets/poster.mdl");
	defaultCabinetNames.push_back("models/cabinets/pinball_standard.mdl");
	defaultCabinetNames.push_back("models/frames/pic_wide_l.mdl");
	defaultCabinetNames.push_back("models/frames/pic_tall_l.mdl");
	defaultCabinetNames.push_back("models/cabinets/phaser_rifle_coop.mdl");
	defaultCabinetNames.push_back("models/cabinets/phaser_gun_coop.mdl");
	defaultCabinetNames.push_back("models/cabinets/pc_wallmount_small.mdl");
	defaultCabinetNames.push_back("models/cabinets/pc_wallmount.mdl");
	defaultCabinetNames.push_back("models/cabinets/pc_kiosk_standard.mdl");
	defaultCabinetNames.push_back("models/cabinets/normal_laptop.mdl");
	defaultCabinetNames.push_back("models/cabinets/movie_stand_standard.mdl");
	defaultCabinetNames.push_back("models/cabinets/movie_display_wallmount.mdl");
	defaultCabinetNames.push_back("models/cabinets/motoracer.mdl");
	defaultCabinetNames.push_back("models/cabinets/lunar.mdl");
	defaultCabinetNames.push_back("models/cabinets/imax.mdl");
	defaultCabinetNames.push_back("models/cabinets/icade.mdl");
	defaultCabinetNames.push_back("models/cabinets/future_speaker.mdl");
	defaultCabinetNames.push_back("models/cabinets/four_player_arcade.mdl");
	defaultCabinetNames.push_back("models/cabinets/extended_four_player_arcade.mdl");
	defaultCabinetNames.push_back("models/cabinets/enclosed_flight.mdl");
	defaultCabinetNames.push_back("models/cabinets/double_phasergun_coop.mdl");
	defaultCabinetNames.push_back("models/cabinets/double_car_racer.mdl");
	defaultCabinetNames.push_back("models/cabinets/console_steam.mdl");
	defaultCabinetNames.push_back("models/cabinets/console_kiosk_steam.mdl");
	defaultCabinetNames.push_back("models/cabinets/coffee_cabinet.mdl");
	defaultCabinetNames.push_back("models/Frames/ceiling_pic_wide_l.mdl");
	defaultCabinetNames.push_back("models/frames/ceiling_pic_tall_l.mdl");
	defaultCabinetNames.push_back("models/cabinets/cd_wall.mdl");
	defaultCabinetNames.push_back("models/cabinets/cd_table.mdl");
	defaultCabinetNames.push_back("models/cabinets/cd_player_headphones.mdl");
	defaultCabinetNames.push_back("models/cabinets/cd_headphones_wall.mdl");
	defaultCabinetNames.push_back("models/cabinets/camcorder.mdl");
	defaultCabinetNames.push_back("models/cabinets/cabsolo.mdl");
	defaultCabinetNames.push_back("models/cabinets/buttonmasher.mdl");
	defaultCabinetNames.push_back("models/cabinets/brainiac.mdl");
	defaultCabinetNames.push_back("models/cabinets/boxcade.mdl");
	defaultCabinetNames.push_back("models/cabinets/big_movie_wallmount_no_banner.mdl");
	defaultCabinetNames.push_back("models/cabinets/big_movie_wallmount.mdl");
	defaultCabinetNames.push_back("models/banners/big_marquee.mdl");
	max = defaultCabinetNames.size();

	//std::vector<std::string>::iterator defaultCabinetsIt;
	std::map<std::string, KeyValues*>& cabinets = g_pAnarchyManager->GetMetaverseManager()->GetAllModels();
	it = cabinets.begin();
	while (it != cabinets.end())
	{
		entry = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);
		if (entry->GetInt("dynamic") == 1)
		{
			//defaultCabinetsIt = std::find(defaultCabinetNames.begin(), defaultCabinetNames.end(), std::string(entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file")));
			//if (defaultCabinetsIt != defaultCabinetNames.end())
			bIsDefault = false;
			for (i = 0; i < max; i++)
			{
				bIsDefault = IsFileEqual(entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"), defaultCabinetNames[i]);
				if (bIsDefault)
					break;
			}

			if ( bIsDefault )
			{
				//DevMsg("Cabinet file: %s\n", entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"));
				fileName = entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file");
				found = fileName.find_last_of(".");
				fileName = fileName.substr(0, found);
				found = fileName.find_last_of("/\\");
				fileName = fileName.substr(found + 1);

				it->second->SaveToFile(g_pFullFileSystem, VarArgs("defaultLibrary/cabinets/%s.txt", fileName.c_str()), "DEFAULT_WRITE_PATH");
			}
		}

		it++;
	}

	// MODELS
	std::vector<std::string> defaultModelNames;
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_n.mdl");
	defaultModelNames.push_back("models/de_vegas/service_trolly.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_g.mdl");
	defaultModelNames.push_back("models/de_vegas/sith_sphynx.mdl");
	defaultModelNames.push_back("models/props/sithlord/floorprojector.mdl");
	defaultModelNames.push_back("models/props/sithlord/table.mdl");
	defaultModelNames.push_back("models/sithlord/giftbox.mdl");
	defaultModelNames.push_back("models/props/sithlord/longbar.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_u.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_j.mdl");
	defaultModelNames.push_back("models/de_halloween/tombstone.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_c.mdl");
	defaultModelNames.push_back("models/de_vegas/cash_cart.mdl");
	defaultModelNames.push_back("models/props/sithlord/walltubelight_rainbow.mdl");
	defaultModelNames.push_back("models/de_halloween/jacklight.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_x.mdl");
	defaultModelNames.push_back("models/sithlord/xmasbell.mdl");
	defaultModelNames.push_back("models/props/sithlord/colorrectangle.mdl");
	defaultModelNames.push_back("models/props/sithlord/lightstrobe.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_q.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_f.mdl");
	defaultModelNames.push_back("models/de_vegas/card_table.mdl");
	defaultModelNames.push_back("models/de_halloween/spooky_tree.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_o.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_t.mdl");
	defaultModelNames.push_back("models/last_resort/villa_chair.mdl");
	defaultModelNames.push_back("models/props/sithlord/lightsyrin.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_w.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_e.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_i.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_m.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_y.mdl");
	defaultModelNames.push_back("models/props/stormy/floorsign_games.mdl");
	defaultModelNames.push_back("models/last_resort/villa_couch.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_k.mdl");
	defaultModelNames.push_back("models/sithlord/candycane.mdl");
	defaultModelNames.push_back("models/cabinets/room_divider.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_a.mdl");
	defaultModelNames.push_back("models/props/sithlord/walltubelight.mdl");
	defaultModelNames.push_back("models/props/sithlord/wood_shelf.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_s.mdl");
	defaultModelNames.push_back("models/cabinets/newton_toy.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_b.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_p.mdl");
	defaultModelNames.push_back("models/props/sithlord/ceilingprojector.mdl");
	defaultModelNames.push_back("models/props/sithlord/studiolight_floor_alwayson.mdl");
	defaultModelNames.push_back("models/de_vegas/roulette_light.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_l.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_z.mdl");
	defaultModelNames.push_back("models/sithlord/xmastree.mdl");
	defaultModelNames.push_back("models/props/sithlord/colorcube.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_h.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_v.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_d.mdl");
	defaultModelNames.push_back("models/props/stormy/neon_alphabet/flurolight_r.mdl");
	defaultModelNames.push_back("models/props/sithlord/colorsquare.mdl");
	max = defaultModelNames.size();

	//std::vector<std::string>::iterator defaultModelsIt;
	std::map<std::string, KeyValues*>& models = g_pAnarchyManager->GetMetaverseManager()->GetAllModels();
	it = models.begin();
	while (it != models.end())
	{
		entry = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);
		//defaultModelsIt = std::find(defaultModelNames.begin(), defaultModelNames.end(), std::string(entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file")));
		//if (defaultModelsIt != defaultModelNames.end())
		bIsDefault = false;
		for (i = 0; i < max; i++)
		{
			bIsDefault = IsFileEqual(entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"), defaultModelNames[i]);
			if (bIsDefault)
				break;
		}

		if (bIsDefault)
		{
			//DevMsg("Model file: %s\n", entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"));
			fileName = entry->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file");
			found = fileName.find_last_of(".");
			fileName = fileName.substr(0, found);
			found = fileName.find_last_of("/\\");
			fileName = fileName.substr(found + 1);

			it->second->SaveToFile(g_pFullFileSystem, VarArgs("defaultLibrary/models/%s.txt", fileName.c_str()), "DEFAULT_WRITE_PATH");
		}

		it++;
	}

	// TYPES
	std::vector<std::string> defaultTypeNames;

	// basic types
	defaultTypeNames.push_back("websites");
	defaultTypeNames.push_back("youtube");
	defaultTypeNames.push_back("images");
	defaultTypeNames.push_back("twitch");
	defaultTypeNames.push_back("videos");
	defaultTypeNames.push_back("cards");
	defaultTypeNames.push_back("pc");
	defaultTypeNames.push_back("movies");
	defaultTypeNames.push_back("tv");
	defaultTypeNames.push_back("comics");
	defaultTypeNames.push_back("music");
	defaultTypeNames.push_back("books");
	defaultTypeNames.push_back("maps");
	defaultTypeNames.push_back("other");	// probably not a real time.  this is probably empty type.

	// retro types
	defaultTypeNames.push_back("wii");
	defaultTypeNames.push_back("gba");
	defaultTypeNames.push_back("32x");
	defaultTypeNames.push_back("n64");
	defaultTypeNames.push_back("snes");
	defaultTypeNames.push_back("ds");
	defaultTypeNames.push_back("3ds");
	defaultTypeNames.push_back("gameboy");
	defaultTypeNames.push_back("genesis");
	defaultTypeNames.push_back("gamecube");
	defaultTypeNames.push_back("arcade");
	defaultTypeNames.push_back("ps");
	defaultTypeNames.push_back("ps2");
	defaultTypeNames.push_back("ps3");
	defaultTypeNames.push_back("ps4");
	defaultTypeNames.push_back("megadrive");
	defaultTypeNames.push_back("nes");
	defaultTypeNames.push_back("gamegear");
	defaultTypeNames.push_back("wiiu");
	defaultTypeNames.push_back("switch");
	defaultTypeNames.push_back("atari5200");
	defaultTypeNames.push_back("gbc");
	defaultTypeNames.push_back("psp");
	defaultTypeNames.push_back("sms");
	defaultTypeNames.push_back("3do");
	defaultTypeNames.push_back("pinball");
	defaultTypeNames.push_back("neogeo");

	std::vector<std::string>::iterator defaultTypesIt;
	std::map<std::string, KeyValues*>& types = g_pAnarchyManager->GetMetaverseManager()->GetAllTypes();
	it = types.begin();
	while (it != types.end())
	{
		entry = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);
		defaultTypesIt = std::find(defaultTypeNames.begin(), defaultTypeNames.end(), std::string(entry->GetString("title")));
		if (defaultTypesIt != defaultTypeNames.end())
		{
			//DevMsg("Type title: %s\n", entry->GetString("title"));
			fileName = entry->GetString("title");

			it->second->SaveToFile(g_pFullFileSystem, VarArgs("defaultLibrary/types/%s.txt", fileName.c_str()), "DEFAULT_WRITE_PATH");
		}

		it++;
	}

	// APPS
	std::vector<std::string> defaultAppNames;

	// retro apps
	//defaultAppNames.push_back("BAM");
	defaultAppNames.push_back("DeSmuME");
	defaultAppNames.push_back("Dolphin");
	defaultAppNames.push_back("ePSXe");
	defaultAppNames.push_back("FCEUX");
	defaultAppNames.push_back("Fusion");
	defaultAppNames.push_back("JNES");
	defaultAppNames.push_back("Kawaks");
	defaultAppNames.push_back("MAME");
	//defaultAppNames.push_back("MESS");
	defaultAppNames.push_back("PCSX2");
	defaultAppNames.push_back("PPSSPP");
	defaultAppNames.push_back("Project64");
	defaultAppNames.push_back("Snes9x");
	////defaultAppNames.push_back("SSF");	// not supported yet until helper apps are officially supported
	defaultAppNames.push_back("VisualBoyAdvance");
	defaultAppNames.push_back("VPinball");
	//defaultAppNames.push_back("VPinbqall8");
	defaultAppNames.push_back("ZSNES");	

	std::vector<std::string>::iterator defaultAppsIt;
	std::map<std::string, KeyValues*>& apps = g_pAnarchyManager->GetMetaverseManager()->GetAllApps();
	it = apps.begin();
	while (it != apps.end())
	{
		entry = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);
		defaultAppsIt = std::find(defaultAppNames.begin(), defaultAppNames.end(), std::string(entry->GetString("title")));
		if (defaultAppsIt != defaultAppNames.end())
		{
			DevMsg("App title: %s\n", entry->GetString("title"));
			fileName = entry->GetString("title");

			it->second->SaveToFile(g_pFullFileSystem, VarArgs("defaultLibrary/apps/%s.txt", fileName.c_str()), "DEFAULT_WRITE_PATH");
		}

		it++;
	}

	*/

	/*
	// broken & not working.
	CMatRenderContextPtr pRenderContext(materials);

	int x, y, w, h;
	pRenderContext->GetViewport(x, y, w, h);
	//pRenderContext = materials->GetRenderContext();

	//ITexture *pRtFullFrame = NULL;
	//pRtFullFrame = materials->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);

	Rect_t rect;
	rect.x = x;
	rect.y = y;
	rect.width = w;
	rect.height = h;
	//TEXTURE_GROUP_RENDER_TARGET
	ITexture* pTexture = g_pMaterialSystem->CreateProceduralTexture("TesterTexture", TEXTURE_GROUP_VGUI, w, h, IMAGE_FORMAT_BGR888, 1);

	pRenderContext->CopyRenderTargetToTextureEx(pTexture, 0, &rect, &rect);
	unsigned int width = pTexture->GetActualWidth();
	DevMsg("Text dev is: %i\n", width);
	*/

	// Get pointer to FullFrameFB
//	ITexture *pRtFullFrame = NULL;
	//pRtFullFrame = materials->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);

	//if (pRtFullFrame)
	//{
		//pRtFullFrame->Sa
		//DevMsg("Saving FB to file...\n");
		//pRtFullFrame->GetResourceData()
		//pRtFullFrame->SaveToFile("materials/tester/test.vtf");
		//pRtFullFrame->
	//}
		//pRtFullFrame->SaveToFile(VarArgs("materials/tester/test.vtf))

	//ITexture* pTexture = 
	//pRenderContext->CopyRenderTargetToTexture()

	// NEW TEST

	//webviewinput->Create();
	//DevMsg("Planel created.\n");

	//g_pAnarchyManager->TestSQLite();

	/*
	DevMsg("Setting url to overlay test...\n");
	C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	pHudInstance->SetUrl("asset://ui/cabinetSelect.html");
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, null, true);
	*/

	//system("Arcade_Launcher.bat");

	//C_OpenGLManager* pOpenGLManager = new C_OpenGLManager();
	//pOpenGLManager->Init();





	/*
	// Scan user profile.
	// 1. Activate input mode.
	// 2. Navigate to the user's games list on their Steam profile in the in-game Steamworks browser.
	// 3. Notify & instruct the user if their profile is set to private, otherwise have an "IMPORT" button appear.
	// 4. Import all games from their list into a KeyValues file ownedGames.key
	// 5. Load all entries from ownedGames.key as items, but do not automatically save them out until the user modifies them.

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (g_pAnarchyManager->GetSelectedEntity())
		g_pAnarchyManager->DeselectEntity("asset://ui/blank.html");
	else
		pHudBrowserInstance->SetUrl("asset://ui/blank.html");
	
	CSteamID sid = steamapicontext->SteamUser()->GetSteamID();
	std::string profileUrl = "http://www.steamcommunity.com/profiles/" + std::string(VarArgs("%llu", sid.ConvertToUint64())) + "/games/?tab=all";

	C_SteamBrowserInstance* pSteamBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
	pSteamBrowserInstance->SetActiveScraper("importSteamGames", "", "");
	pSteamBrowserInstance->Init("", profileUrl, null);
	pSteamBrowserInstance->Focus();
	pSteamBrowserInstance->Select();
	g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pSteamBrowserInstance);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pSteamBrowserInstance);
	*/

	// GOOD STEAM GAMES IMPOOOOOOORT!!
	//g_pAnarchyManager->BeginImportSteamGames();

	/*
	KeyValues* kv = new KeyValues("tester");
	kv->SetString("val", "Will it work? xxxxxx");

	// NOTE: The Source filesystem will try to auto-lowercase file names!!!
	// So the file must be created case-correct outside of the Source filesystem and already exist before the Source filesystem writes to it.
	kv->SaveToFile(g_pFullFileSystem, "tEsTeR.key", "DEFAULT_WRITE_PATH");
	*/


	/*
	CUtlBuffer buf;
	KeyValues* pObjectKV = new KeyValues("originalTester");//pInstanceObjectsKV->FindKey(VarArgs("%s/local", objectId.c_str()), true);
	pObjectKV->SetString("originalTesterKey", "yup");
	pObjectKV->SetString("originalTesterKey2", "yup2");
	pObjectKV->SetString("originalTesterKey3", "yup3");
	pObjectKV->WriteAsBinary(buf);
	pObjectKV->deleteThis();

	int size = buf.Size();
	DevMsg("Buffer size here is: %i\n", size);
	void* mem = malloc(size);
	Q_memcpy(mem, buf.Base(), size);

	CUtlBuffer buf2(0, size, 0);
	buf2.CopyBuffer(mem, size);
	int size2 = buf2.Size();
	DevMsg("Processed buffer size is: %i\n", size2);
	
	KeyValues* pTesterKV = new KeyValues("reduxTester");
	pTesterKV->ReadAsBinary(buf2);
	DevMsg("Annd here the big result is: %s\n", pTesterKV->GetString("originalTesterKey"));
	pTesterKV->deleteThis();
	*/
}
ConCommand test_function( "testfunc", TestFunction, "Usage: executes an arbitrary hard-coded C++ routine" );

void TestFunctionOff(const CCommand &args)
{
	g_pAnarchyManager->TestSQLite2();
	//g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	//g_pAnarchyManager->GetHUDManager
}
ConCommand test_function_off("testfunc2", TestFunctionOff, "Usage: executes an arbitrary hard-coded C++ routine");

void ImportSteamGames(const CCommand &args)
{
	g_pAnarchyManager->BeginImportSteamGames();
}
ConCommand import_steam_games("import_steam_games", ImportSteamGames, "Usage: imports your steam games from your public profile");

void WheelUp(const CCommand &args)
{
	g_pAnarchyManager->GetInputManager()->OnMouseWheeled(1);
}
ConCommand wheel_up("wheelup", WheelUp, "Usage: mouse wheel up");

void WheelDown(const CCommand &args)
{
	g_pAnarchyManager->GetInputManager()->OnMouseWheeled(-1);
}
ConCommand wheel_down("wheeldown", WheelDown, "Usage: mouse wheel down");

void RunEmbeddedLibretro(const CCommand &args)
{
	C_LibretroManager* pLibretroManager = g_pAnarchyManager->GetLibretroManager();
	if (pLibretroManager)
		pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
		//pLibretroManager->RunEmbeddedLibretro("mednafen_psx_libretro.dll", "X:\\Emulators\\PSP\\roms\\ffn-spac.iso");
		//pLibretroManager->RunEmbeddedLibretro("mednafen_psx_libretro.dll", "X:\\Emulators\\PS\\roms\\Need For Speed 4 - High Stakes [U] [SLUS-00826].cue");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("mednafen_psx_libretro.dll", "X:\\Emulators\\PS\\roms\\Need For Speed 4 - High Stakes [U] [SLUS-00826].cue");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:/Emulators/N64/Roms/GoldenEye 007 (U) [!].zip");
		//pLibretroManager->RunEmbeddedLibretro("mame_libretro.dll", "X:\\Emulators\\Arcade\\roms\\sfrush.zip");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		
		//pLibretroManager->RunEmbeddedLibretro("mame2014_libretro.dll", "X:\\Emulators\\Arcade\\roms\\lethalen.zip");
		//pLibretroManager->RunEmbeddedLibretro("snes9x_libretro.dll", "X:\\Emulators\\SNES\\Roms\\Donkey Kong Country - Competition Cartridge (U).smc");
		//pLibretroManager->RunEmbeddedLibretro("mame_libretro.dll", "X:\\Emulators\\Arcade\\roms\\lethalen.zip");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
		//pLibretroManager->RunEmbeddedLibretro("snes9x_libretro.dll", "X:\\Emulators\\SNES\\Roms\\Donkey Kong Country - Competition Cartridge (U).smc");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/TV/Beavis & Butthead/Beavis and Butthead - Season 7/731 Drinking Butt-ies.mpg");
	
		//pLibretroManager->RunEmbeddedLibretro("mame_libretro.dll", "X:\\Emulators\\Arcade\\roms\\lethalen.zip");
		//pLibretroManager->RunEmbeddedLibretro("snes9x_libretro.dll", "X:\\Emulators\\SNES\\Roms\\Donkey Kong Country - Competition Cartridge (U).smc");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
		//pLibretroManager->RunEmbeddedLibretro("mame2014_libretro.dll", "X:\\Emulators\\Arcade\\roms\\lethalen.zip");
		
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
	
		//pLibretroManager->RunEmbeddedLibretro("V:/Movies/Jay and silent Bob Strike Back (2001).avi");
}
ConCommand run_embedded_libretro("run_embedded_libretro", RunEmbeddedLibretro, "Usage: runs embedded apps");

void RunEmbeddedSteamBrowser(const CCommand &args)
{
	C_SteamBrowserManager* pSteamBrowserManager = g_pAnarchyManager->GetSteamBrowserManager();
	if (pSteamBrowserManager)
		pSteamBrowserManager->RunEmbeddedSteamBrowser();
}
ConCommand run_embedded_steam_browser("run_embedded_steam_browser", RunEmbeddedSteamBrowser, "Usage: runs embedded apps");

void RunEmbeddedAwesomiumBrowser(const CCommand &args)
{
	C_AwesomiumBrowserManager* pAwesomiumBrowserManager = g_pAnarchyManager->GetAwesomiumBrowserManager();
	if (pAwesomiumBrowserManager)
		pAwesomiumBrowserManager->RunEmbeddedAwesomiumBrowser();
}
ConCommand run_embedded_awesomium_browser("run_embedded_awesomium_browser", RunEmbeddedAwesomiumBrowser, "Usage: runs embedded apps");

void Continuous(const CCommand &args)
{
	g_pAnarchyManager->TaskRemember();
}
ConCommand continuous("task_remember", Continuous, "Usage: sets the selected entity as continuous play.");

void CloseAll(const CCommand &args)
{
	if (g_pAnarchyManager->GetSelectedEntity())
		g_pAnarchyManager->DeselectEntity();

	g_pAnarchyManager->GetCanvasManager()->CloseAllInstances();
}
ConCommand closeall("task_clear", CloseAll, "Usage: closes all open instaces (execpt for important game system ones)");

void ShowTaskMenu(const CCommand &args)
{
	g_pAnarchyManager->ShowTaskMenu();
}
ConCommand showtaskmenu("+task_menu", ShowTaskMenu, "Usage: check which in-game tasks are open.");

void HideTaskMenu(const CCommand &args)
{
	g_pAnarchyManager->HideTaskMenu();
}
ConCommand hidetaskmenu("-task_menu", HideTaskMenu, "Usage: hides the task menu.");

/*
void RememberWrapper(const CCommand &args)
{
	engine->ClientCmd("setcontinuous\n");
}
ConCommand rememberwrapper("-remember", RememberWrapper, "Usage: wrapper for the remember button to mean setcontinous now.");
*/

void RunAArcade(const CCommand &args)
{
	g_pAnarchyManager->RunAArcade();
}
ConCommand run_aarcade("run_aarcade", RunAArcade, "Usage: runs AArcade");

/*
void TestFunction2( const CCommand &args )
{
	// WORKING SEND/RECIEVE FILE CALLS
//	#include "inetchannel.h"
//	INetChannel* pINetChannel = static_cast<INetChannel*>(engine->GetNetChannelInfo());
//	pINetChannel->RequestFile("downloads/<hash>.vtf", false);
//	pINetChannel->SendFile("stuff/test.jpg", 0, false);

	// WORKING CURSOR POSITIONS
//	#include "vgui/IInput.h"
//	#include <vgui_controls/Controls.h>
//	int x, y;
//	vgui::input()->GetCursorPos(x, y);

	// NEW TEST


	webviewinput->Create();
	DevMsg("Planel created.\n");
}

ConCommand test_function2( "testfunc2", TestFunction2, "Usage: executes an arbitrary hard-coded C++ routine" );
*/

void AnarchyManager(const CCommand &args)
{
	g_pAnarchyManager->AnarchyStartup();
}

ConCommand anarchymanager("anarchymanager", AnarchyManager, "Starts the Anarchy Manager.", FCVAR_HIDDEN);

void BuildContextUp(const CCommand &args)
{
	// check if a propshortcut is under the player's crosshair
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (Q_strcmp(pPlayer->GetActiveWeapon()->GetName(), "weapon_physcannon"))
	{
		CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType("weapon_physcannon");
		if (pWeapon && pPlayer->GetActiveWeapon()->CanHolster())
			engine->ClientCmd("phys_swap");
		return;
	}

	//bool SwitchToNextBestWeapon(C_BaseCombatWeapon *pCurrent);

	//virtual C_BaseCombatWeapon	*GetActiveWeapon(void) const;
	//int					WeaponCount() const;
	//C_BaseCombatWeapon	*GetWeapon(int i) const;

	//if (!pPlayer)
	//return;

	//if (pPlayer->GetHealth() <= 0)
	//return;

	bool bAutoChooseLibrary = true;

	// fire a trace line
	trace_t tr;
	Vector forward;
	pPlayer->EyeVectors(&forward);
	UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_COORD_RANGE, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

	C_BaseEntity *pEntity = (tr.DidHitNonWorldEntity()) ? tr.m_pEnt : null;

	// only allow prop shortcuts
	C_PropShortcutEntity* pShortcut = (pEntity) ? dynamic_cast<C_PropShortcutEntity*>(pEntity) : null;
	if (pShortcut && tr.fraction != 1.0)
		bAutoChooseLibrary = false;	// TODO: If you want to highlight the object that the context menu applies to, now's the time.

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (bAutoChooseLibrary)
	{
		//DevMsg("DISPLAY MAIN MENU\n");
		if (g_pAnarchyManager->GetSelectedEntity())
			g_pAnarchyManager->DeselectEntity("asset://ui/libraryBrowser.html");
		else
			pHudBrowserInstance->SetUrl("asset://ui/libraryBrowser.html");

		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true);
	}
	else
	{
		//DevMsg("DISPLAY BUILD MODE CONTEXT MENU\n");
		if (g_pAnarchyManager->GetInputManager()->GetInputMode())
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

		if (g_pAnarchyManager->GetSelectedEntity())
			g_pAnarchyManager->DeselectEntity("asset://ui/buildModeContext.html");
		else
			pHudBrowserInstance->SetUrl("asset://ui/buildModeContext.html");

		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pHudBrowserInstance);
	}

	return;
}
ConCommand buildcontextup("-remote_control", BuildContextUp, "Open up the library, or shows the edit object menu.", FCVAR_NONE);

void BuildContextDown(const CCommand &args)
{
	// do nothing
}
ConCommand buildcontextdown("+remote_control", BuildContextDown, "Open up the library, or shows the edit object menu.", FCVAR_NONE);

void ActivateInputMode(const CCommand &args)
{
	// FIXME: Need to reject commands that are sent before the AArcade system is ready.
	//bool fullscreen = (args.ArgC() > 1);
	
	// if not spawning an object, do regular stuff
	if (!g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity())
	{
		C_EmbeddedInstance* pSelectedEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (pSelectedEmbeddedInstance)
			g_pAnarchyManager->GetInputManager()->ActivateInputMode(false, false, pSelectedEmbeddedInstance);// fullscreen);
	}
	else
	{
		C_EmbeddedInstance* pSelectedEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (pSelectedEmbeddedInstance && pSelectedEmbeddedInstance->GetId() == "hud")
		{
			g_pAnarchyManager->GetInputManager()->SetInputCapture(true);

			C_AwesomiumBrowserInstance* pHudInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(pSelectedEmbeddedInstance);
			std::vector<std::string> params;
			params.push_back("transform");
			pHudInstance->DispatchJavaScriptMethod("cmdListener", "switchMode", params);
		}
			//pSelectedEmbeddedInstance->
			//g_pAnarchyManager->GetInputManager()->ActivateInputMode(false, false, pSelectedEmbeddedInstance);// fullscreen);
		/*
		//g_pAnarchyManager->DeactivateObjectPlacementMode(false);

		// undo changes AND cancel
		C_PropShortcutEntity* pShortcut = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
		g_pAnarchyManager->DeactivateObjectPlacementMode(false);

		//std::string id = pShortcut->GetObjectId();
		//g_pAnarchyManager->GetInstanceManager()->ResetObjectChanges(pShortcut);

		// "save" cha
		//m_pInstanceManager->ApplyChanges(id, pShortcut);
		DevMsg("CHANGES REVERTED\n");
		*/
	}
}
ConCommand activateinputmode("+input_mode", ActivateInputMode, "Turns ON input mode.", FCVAR_NONE);

void DeactivateInputMode(const CCommand &args)
{
	if( !g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity() )
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode();
	else
	{
		C_EmbeddedInstance* pSelectedEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (pSelectedEmbeddedInstance && pSelectedEmbeddedInstance->GetId() == "hud")
		{
			g_pAnarchyManager->GetInputManager()->SetInputCapture(false);

			C_AwesomiumBrowserInstance* pHudInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(pSelectedEmbeddedInstance);
			std::vector<std::string> params;
			params.push_back("browse");
			pHudInstance->DispatchJavaScriptMethod("cmdListener", "switchMode", params);

			if (g_pAnarchyManager->GetInputManager()->GetMainMenuMode() && engine->IsInGame())
				engine->ClientCmd("gamemenucommand ResumeGame");
		}
	}
}
ConCommand deactivateinputmode("-input_mode", DeactivateInputMode, "Turns OFF input mode.", FCVAR_NONE);

void AttemptSelectObject(const CCommand &args)
{
	g_pAnarchyManager->AttemptSelectEntity();

	if (broadcast_mode.GetBool())
		g_pAnarchyManager->xCastSetLiveURL();
}
ConCommand attemptselectobject("select", AttemptSelectObject, "Attempts to select the object under your crosshair.", FCVAR_NONE);

void Launch( const CCommand &args )
{
	g_pAnarchyManager->GetLibretroManager()->CreateLibretroInstance();

	//g_pFullFileSystem->AddSearchPath(installFolder, "GAME", PATH_ADD_TO_TAIL);

	//std::string fullPath = VarArgs("%s\\", installFolder);

	//unsigned int uNumModels = 0;
	//unsigned int uNumItems = 0;
	//std::string id = VarArgs("%llu", details->m_nPublishedFileId);




	//g_pAnarchyManager->GetInstanceManager()->LoadLegacyInstance();




	/*
	g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Importing Old AArcade Data", "importfolder", "0", "1", "0");
	std::string path = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
	g_pAnarchyManager->GetMetaverseManager()->LoadFirstLocalItemLegacy(true, path, "", "");
	g_pFullFileSystem->AddSearchPath(path.c_str(), "MOD", PATH_ADD_TO_TAIL);
	g_pFullFileSystem->AddSearchPath(path.c_str(), "GAME", PATH_ADD_TO_TAIL);
	//DevMsg("Loaded %u items from %s\n", uNumItems, path.c_str());
	*/












//	uNumItems = g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalItemsLegacy(uNumModels, path, "", "");
	//			g_pFullFileSystem->AddSearchPath(installFolder, "MOD", PATH_ADD_TO_TAIL);
	//DevMsg("Loaded %u items from %s\n", uNumItems, path.c_str());

	//if( args.ArgC() < 2 )
//		return;
	/*
	C_PropSimpleImageEntity* pProp = NULL;
	pProp = dynamic_cast<C_PropSimpleImageEntity*>( C_BaseEntity::Instance( Q_atoi(args[1]) ) );

	if( !pProp )
	{
		DevMsg("Invalid entindex specified for activate command!\n");
		return;
	}

	pProp->OnUse();
	*/
}

ConCommand launch( "aa_activated", Launch, "Usage: aa_activated entindex" );

void DetectAllMaps(const CCommand &args)
{
	g_pAnarchyManager->GetMetaverseManager()->DetectAllMaps();
	//DevMsg("Detect all maps!\n");
}
ConCommand detectallmaps("detectallmaps", DetectAllMaps, "Usage: aa_activated entindex");

void SpawnObjects(const CCommand &args)
{
	g_pAnarchyManager->GetInstanceManager()->SpawnActionPressed();

	/*
	std::string instanceId = g_pAnarchyManager->GetInstanceId();
	if (instanceId != "")
	{
		std::string uri = "asset://ui/spawnItems.html?max=" + std::string(args[1]);

		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(pHudBrowserInstance);
		pHudBrowserInstance->SetUrl(uri);
		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, pHudBrowserInstance);
	}
	*/
}
ConCommand spawnobjects("spawnobjects", SpawnObjects, "Usage: ...");