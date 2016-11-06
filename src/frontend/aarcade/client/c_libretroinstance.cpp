// ;..\..\portaudio\lib\portaudio_x86.lib

#include "cbase.h"
#include "aa_globals.h"

//#include "aa_globals.h"
#include "c_libretroinstance.h"
#include "c_anarchymanager.h"
#include "../../../public/vgui_controls/Controls.h"
#include "vgui/IInput.h"
#include "c_canvasregen.h"
#include "c_embeddedinstance.h"
#include <mutex>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_LibretroInstance::C_LibretroInstance()
{
	DevMsg("LibretroInstance: Constructor\n");
	m_pTexture = null;
	m_iLastRenderedFrame = -1;
	m_iLastVisibleFrame = -1;
}

C_LibretroInstance::~C_LibretroInstance()
{
	DevMsg("LibretroInstance: Destructor\n");

	if (m_pTexture)
	{
		m_pTexture->SetTextureRegenerator(null);

		g_pAnarchyManager->GetCanvasManager()->UnreferenceTexture(m_pTexture);
		m_pTexture->DecrementReferenceCount();
		m_pTexture->DeleteIfUnreferenced();
		m_pTexture = null;
	}
}

void C_LibretroInstance::SelfDestruct()
{
	DevMsg("LibretroInstance: SelfDestruct\n");
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->NotifyInstanceAboutToDie(this);
	m_info->libretroinstance = null;
	m_info->close = true;
	/*
	if (m_pLastFrameData)
		free(m_pLastFrameData);

	if (m_pPostData)
		free(m_pPostData);
	*/

	delete this;
}

void C_LibretroInstance::Init(std::string id)
{
	m_id = id;
	if (m_id == "")
		m_id = g_pAnarchyManager->GenerateUniqueId();

	// create the texture (each instance has its own texture)
	std::string textureName = "canvas_";
	textureName += m_id;

	int iWidth = 1280;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth();
	int iHeight = 720;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight();
	//int iWidth = 1920;
	//int iHeight = 1080;

	//m_pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);

	//if (!m_pTexture)

	if (!g_pMaterialSystem->IsTextureLoaded(textureName.c_str()))
		m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);
	else
		m_pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);

	// get the regen and assign it
	CCanvasRegen* pRegen = g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen();
	//pRegen->SetEmbeddedInstance(this);
	m_pTexture->SetTextureRegenerator(pRegen);
	
	m_corePath = engine->GetGameDirectory();
	m_corePath += "/cores";

	m_raw = new libretro_raw();
}

void C_LibretroInstance::Update()
{
	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (m_info->state == 2)
		OnCoreLoaded();
	else if (m_info->state == 5 && m_info->audiostream)	// added m_info to try and detect failed video loads!! (FIXME: Should be removed after proper failed video load is added elsewhere.
	{
		// update input state
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_SELECT] = vgui::input()->IsKeyDown(KEY_XBUTTON_BACK);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_START] = vgui::input()->IsKeyDown(KEY_XBUTTON_START) || vgui::input()->IsKeyDown(KEY_ENTER);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_UP] = vgui::input()->IsKeyDown(KEY_XBUTTON_UP);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_DOWN] = vgui::input()->IsKeyDown(KEY_XBUTTON_DOWN);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_LEFT] = vgui::input()->IsKeyDown(KEY_XBUTTON_LEFT);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_RIGHT] = vgui::input()->IsKeyDown(KEY_XBUTTON_RIGHT);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_A] = vgui::input()->IsKeyDown(KEY_XBUTTON_B);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_B] = vgui::input()->IsKeyDown(KEY_XBUTTON_A);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_X] = vgui::input()->IsKeyDown(KEY_XBUTTON_Y);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_Y] = vgui::input()->IsKeyDown(KEY_XBUTTON_X);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_L] = vgui::input()->IsKeyDown(KEY_XBUTTON_LEFT_SHOULDER);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_R] = vgui::input()->IsKeyDown(KEY_XBUTTON_RIGHT_SHOULDER);

		this->OnProxyBind(null);

		/*
		if (m_raw)
		{
			DevMsg("LibretroInstance: Update\n");
			//m_raw->run();
			DevMsg("after\n");
		}
		*/
	}
}

bool C_LibretroInstance::LoadCore()
{
	//C_ArcadeResources* pClientArcadeResources = C_ArcadeResources::GetSelf();

	//long startTime = pClientArcadeResources->GetSystemTime();

	//char fullFilename[MAX_PATH];
	//g_pFullFileSystem->RelativePathToFullPath(pFilename, USE_GAME_PATH, fullFilename, sizeof(fullFilename));

	/*
	std::string core = "D:\\Projects\\AArcade-Source\\game\\frontend\\cores\\ffmpeg_libretro.dll";
	m_pModule = Sys_LoadModule(core.c_str());

	if (!m_pModule)
	{
		Msg("Failed to load %s\n", core.c_str());
		// FIXME FIX ME Probably need to clean up!
		return false;
	}

	HMODULE	hModule = reinterpret_cast<HMODULE>(m_pModule);

	if (!this->BuildInterface(&hModule))
	{
		DevMsg("libretro: Failed to build interface!\n");
		// FIXME FIX ME Probably need to clean up!
		return false;
	}
	*/

//	if (ThreadInMainThread())
	//	DevMsg("22 MAIN THREAD!\n");
	//else
//		DevMsg("22 CHILD THREAD!\n");

	std::string core = "D:\\Projects\\AArcade-Source\\game\\frontend\\cores\\ffmpeg_libretro.dll";
	CreateWorkerThread(core);

	return true;
//	LibretroInstanceInfo_t* info = new LibretroInstanceInfo_t;
//	info->id = "taco";

//	CreateSimpleThread(&C_LibretroInstance::Worker, info);

	// Bind the callbacks that cores will call in us
	//m_raw->set_environment(this->cbEnvironment);
//	m_raw->set_environment(&this->cbEnvironment);
	/*
	m_raw->set_video_refresh(&this->cbVideoRefresh);
	m_raw->set_audio_sample(&this->cbAudioSample);
	m_raw->set_audio_sample_batch(&this->cbAudioSampleBatch);
	m_raw->set_input_poll(&this->cbInputPoll);
	m_raw->set_input_state(&this->cbInputState);

	if (m_raw->api_version() != RETRO_API_VERSION)
	{
		DevMsg("libretro: Failed version check!\n");
		// FIXME FIX ME Probably need to clean up!
		return false;
	}
	*/

	DevMsg("FIN!\n\n");
	return true;
	/*

	//	struct retro_system_info info;
	//	s_raw->get_system_info(&info);

	Msg("Successfully loaded libretro module at %s in %i milliseconds.\n", pFilename, pClientArcadeResources->GetSystemTime() - startTime);
	s_bCoreIsLoaded = true;
	*/
}

void C_LibretroInstance::OnGameLoaded()
{
	DevMsg("Game finished loading.\n");
	m_info->state = 5;
}

bool C_LibretroInstance::LoadGame()
{
	//std::string filename = "V:\\Movies\\Teenage Mutant Ninja Turtles(1990).avi";

	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return false;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	/*
	if (file == "")
	{
		info->game = "V:\\Movies\\Teenage Mutant Ninja Turtles(1990).avi";
		//file = info->game;
	}
	else
		info->game = file;
	*/


	std::string filename = info->game;
	//DevMsg("filename: %s\n", filename.c_str());

//	DevMsg("Load the damn game, son!\n");
//	return false;
	///*

	/*
	bool bForceDataLoad = true;
	bool bForceDataLoad = false;
	*/

	//char* filenameFixed = new char[MAX_STRING_LENGTH];
	//Q_strcpy(filenameFixed, pFilename);
	//V_FixSlashes(filenameFixed, '/');

	bool bDidItWork = false;
		bool bFilenameOnly = true;

	info->videoformat = RETRO_PIXEL_FORMAT_0RGB1555;
	//V:\MoviesTeenage Mutant Ninja Turtles(1990).avi

	DevMsg("libretro: LOAD GAME %s\n", filename.c_str());

	info->raw->init();	// could take a while
	if (info->close)
		return false;

	/*
	if (!g_pFullFileSystem->FileExists(pFilename))
	{
		DevMsg("libretro: Error - File does not exist!\n");
		return false;
	}

	if (s_bSupportsNoGame)
	{
		DevMsg("libretro: Error - This core does not support games.\n");
		return false;
	}

	std::string buf = pFilename;
	bool bZipDetected = (buf.find(".zip") == buf.length() - 4 || buf.find(".ZIP") == buf.length() - 4);

	if (bFilenameOnly && !bZipDetected && !bForceDataLoad)
	{
	*/
		char name[AA_MAX_STRING];
		Q_strcpy(name, filename.c_str());

		struct retro_game_info game;
		game.path = name;
		game.data = NULL;
		game.size = 0;
		game.meta = NULL;

		DevMsg("loading game\n");
		if (!info->raw->load_game(&game))	// this could take a while
		{
			DevMsg("ERROR LOADING LIBRETRO GAME!!\n");
		}
		else
		{
			DevMsg("done loading game\n");
			if (!info->close)
				info->state = 5;
			else
				DevMsg("However, the instance already wants to close. :(\n");
		}

		//pLibretroInstance->OnGameLoaded();
		//bDidItWork = s_raw->load_game(&game);
		/*
	}
	else
	{
		if (bZipDetected)
		{
			DevMsg("libretro: ZIP file detected. Attempting to extract the 1st file..\n");
			HZIP hz = OpenZip((void*)pFilename, 0, ZIP_FILENAME);

			int zipIndex = 0;
			ZIPENTRY zipEntry;
			ZRESULT result = GetZipItem(hz, zipIndex, &zipEntry);

			while (result == ZR_OK)
			{
				if (zipEntry.attr & FILE_ATTRIBUTE_DIRECTORY)
				{
					zipIndex++;
					result = GetZipItem(hz, zipIndex, &zipEntry);
					continue;
				}

				break;
			}

			if (result != ZR_OK)
			{
				DevMsg("libretro: Error - failed to locate a valid file in ZIP.");
				CloseZip(hz);
				return false;
			}

			//unsigned int fileSize = g_pFullFileSystem->Size(pFilename);
			long fileSize = zipEntry.unc_size;

			//unsigned char* fileData = new unsigned char[fileSize*20];

			void* fileData = malloc(fileSize);
			//Q_memcpy(dest, data, fileSize);

			result = UnzipItem(hz, zipIndex, fileData, fileSize, ZIP_MEMORY);

			if (result != ZR_OK && result != 1536)
			{
				DevMsg("libretro: Error - failed to unzip the file. ERROR CODE %i\n", result);
				CloseZip(hz);

				free(fileData);
				return false;
			}

			//fileData[fileSize] = 0; // null terminator

			CloseZip(hz);

			// The unzipped file is now in fileData.  Give it to the core.
			struct retro_game_info game;
			game.path = pFilename;
			game.data = fileData;
			game.size = fileSize;
			game.meta = NULL;

			bDidItWork = s_raw->load_game(&game);

			free(fileData);
		}
		else
		{
			struct retro_game_info game;
			game.path = pFilename;
			game.data = NULL;
			game.size = 0;
			game.meta = NULL;

			FileHandle_t fileHandle = filesystem->Open(pFilename, "rb");
			if (fileHandle)
			{
				int bufferSize = filesystem->Size(fileHandle);
				unsigned char* fileContents = new unsigned char[bufferSize + 1];

				filesystem->Read((void*)fileContents, bufferSize, fileHandle);
				fileContents[bufferSize] = 0; // null terminator

				filesystem->Close(fileHandle);

				game.data = fileContents;
				game.size = bufferSize;

				bDidItWork = s_raw->load_game(&game);

				delete[] fileContents;
			}
		}
	}

	if (bDidItWork)
	{
		s_bGameIsLoaded = true;
		s_bReadyToRun = true;
	}

	return bDidItWork;
	*/

	return true;
}

void C_LibretroInstance::OnCoreLoaded()
{
	DevMsg("Core finished loading!\n");
	m_info->coreloaded = true;
	m_info->state = 3;

	// automatically load a game right away...
//	m_info->game = "V:/Movies/Flash Gordon (1980).avi";//file
	//m_info->game = "V:/Movies/Jay and silent Bob Strike Back (2001).avi";
	//"V:\\Movies\\Judge Dredd (1995).mp4";
}

//bool C_LibretroManager::BuildInterface(void* pLib, struct libretro_raw * myInterface)
bool C_LibretroInstance::BuildInterface(libretro_raw* raw, void* pLib)
{
	HMODULE hModule = *static_cast<HMODULE*>(pLib);

	//void(*get_system_info)(struct retro_system_info * info);
	raw->set_environment = (void(*)(retro_environment_t))GetProcAddress(hModule, "retro_set_environment");
	raw->set_video_refresh = (void(*)(retro_video_refresh_t))GetProcAddress(hModule, "retro_set_video_refresh");
	raw->set_audio_sample = (void(*)(retro_audio_sample_t))GetProcAddress(hModule, "retro_set_audio_sample");
	raw->set_audio_sample_batch = (void(*)(retro_audio_sample_batch_t))GetProcAddress(hModule, "retro_set_audio_sample_batch");
	raw->set_input_poll = (void(*)(retro_input_poll_t))GetProcAddress(hModule, "retro_set_input_poll");
	raw->set_input_state = (void(*)(retro_input_state_t))GetProcAddress(hModule, "retro_set_input_state");
	raw->init = (void(*)(void))GetProcAddress(hModule, "retro_init");
	raw->deinit = (void(*)(void))GetProcAddress(hModule, "retro_deinit");
	raw->api_version = (unsigned(*)(void))GetProcAddress(hModule, "retro_api_version");
	raw->get_system_info = (void(*)(struct retro_system_info*))GetProcAddress(hModule, "retro_get_system_info");
	raw->get_system_av_info = (void(*)(struct retro_system_av_info*))GetProcAddress(hModule, "retro_get_system_av_info");
	raw->set_controller_port_device = (void(*)(unsigned, unsigned))GetProcAddress(hModule, "retro_set_controller_port_device");
	raw->reset = (void(*)(void))GetProcAddress(hModule, "retro_reset");
	raw->run = (void(*)(void))GetProcAddress(hModule, "retro_run");
	raw->serialize_size = (size_t(*)(void))GetProcAddress(hModule, "retro_serialize_size");
	raw->serialize = (bool(*)(void* data, size_t size))GetProcAddress(hModule, "retro_serialize");
	raw->unserialize = (bool(*)(const void* data, size_t size))GetProcAddress(hModule, "retro_unserialize");
	raw->cheat_reset = (void(*)(void))GetProcAddress(hModule, "retro_cheat_reset");
	raw->cheat_set = (void(*)(unsigned, bool, const char*))GetProcAddress(hModule, "retro_cheat_set");
	raw->load_game = (bool(*)(const struct retro_game_info *))GetProcAddress(hModule, "retro_load_game");
	raw->load_game_special = (bool(*)(unsigned, const struct retro_game_info*, size_t))GetProcAddress(hModule, "retro_load_game_special");
	raw->unload_game = (void(*)(void))GetProcAddress(hModule, "retro_unload_game");
	raw->get_region = (unsigned(*)(void))GetProcAddress(hModule, "retro_get_region");
	raw->get_memory_data = (void*(*)(unsigned))GetProcAddress(hModule, "retro_get_memory_data");
	raw->get_memory_size = (size_t(*)(unsigned))GetProcAddress(hModule, "retro_get_memory_size");

	return true;
}

//float lastAudioFrame = 0;
typedef float SAMPLE;
static int audiocallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	uint uId = ThreadGetCurrentId();
	//DevMsg("Audio Thread ID: %u\n", uId);

	LibretroInstanceInfo_t* info = (LibretroInstanceInfo_t*)userData;
	//	DevMsg("Pos: %i\n", info->audiobufferpos);
	if (info->audiobufferpos < info->audiobuffersize)
	{
		info->processingaudio = false;
		return paContinue;
	}
	//if (info->processingaudio)
	//		return paContinue;

	//	if (!inputBuffer)
	//	return paContinue;

	//	if (lastAudioFrame != gpGlobals->framecount)
	//{
	//		lastAudioFrame = gpGlobals->framecount;
	//		lastFrameNumber = gpGlobals->framecount;

	//Q_memcpy(outputBuffer, info->audiobuffer, info->audiobufferframes * 2 * sizeof(int16_t));

	//int16_t* copyBuffer = new int16_t[info->audiobufferpos * 2];
	//	Q_memcpy(copyBuffer, info->audiobuffer, info->audiobufferpos * 2 * sizeof(int16_t));

	Q_memcpy(outputBuffer, info->audiobuffer, info->audiobufferpos * 2 * sizeof(int16_t));
	info->audiobufferpos = 0;
	info->processingaudio = false;
	//	}

	return paContinue;
	//Q_memcpy(outputBuffer, inputBuffer, framesPerBuffer * 2 * sizeof(int16_t));

	/*
	unsigned int i;
	SAMPLE *out = (SAMPLE*)outputBuffer;
	if (!inputBuffer)
	{
	for (i = 0; i < framesPerBuffer; i++)
	{
	*out++ = 0;
	*out++ = 0;
	}
	}
	else
	{
	const SAMPLE *in = (const SAMPLE*)inputBuffer;
	for (int i = 0; i < framesPerBuffer; i++)
	{
	*out++ = *in++;
	*out++ = *in++;
	}
	}

	return paContinue;
	*/
}

void C_LibretroInstance::CreateAudioStream()
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	DevMsg("Sample rate is: %i\n", info->samplerate);

	PaStreamParameters outputParameters;
	outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paInt16;
	//outputParameters.suggestedLatency = 0.032;// 0.064;// Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
	//outputParameters.suggestedLatency = 1 / (info->framerate * 1.0);
	outputParameters.hostApiSpecificStreamInfo = NULL;

	PaStream *stream;
	PaError err = Pa_OpenStream(
		&stream,
		NULL,
		&outputParameters,
		info->samplerate,
		paFramesPerBufferUnspecified,
		paNoFlag,//paClipOff,      // we won't output out of range samples so don't bother clipping them
		null, //audiocallback no callback, use blocking API
		info); // no callback, so no callback userData

	info->audiostream = stream;

	info->audiobuffersize = 1024;
	info->audiobuffer = new int16_t[info->audiobuffersize];
	info->audiobufferpos = 0;

	//	info->safebuffersize = info->audiobuffersize;
	//	info->safebuffer = new int16_t[info->safebuffersize];
	//	info->safebufferpos = 0;


	if (err != paNoError)
		DevMsg("Failed to open stream.\n");
	else
		DevMsg("Opened PA stream!\n");

	err = Pa_StartStream(stream);

	if (err != paNoError)
		DevMsg("Failed to start stream.\n");
	else
		DevMsg("Started stream!\n");
}

void C_LibretroInstance::DestroyAudioStream()
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	PaStream *stream = static_cast<PaStream*>(info->audiostream);

	PaError err = Pa_StopStream(stream);

	if (err != paNoError)
		DevMsg("Failed to stop stream.\n");
	else
		DevMsg("Stopped PA stream!\n");

	err = Pa_CloseStream(stream);
	if (err != paNoError)
		DevMsg("Failed to close stream.\n");
	else
		DevMsg("Closed PA stream!\n");

	info->samplerate = 0;
	//	info->audiobufferpos = 0;
}

unsigned MyThread(void *params)
{
	LibretroInstanceInfo_t* info = (LibretroInstanceInfo_t*)params; // always use a struct!

	if (info->libretroinstance && !info->close)
	{
		//DevMsg("Cannot print to console from this threaded function\n");

		CSysModule* pModule = Sys_LoadModule(info->core.c_str());
		if (!pModule)
		{
			//	Msg("Failed to load %s\n", info->core.c_str());
			// FIXME FIX ME Probably need to clean up!
			return 0;
		}

		HMODULE	hModule = reinterpret_cast<HMODULE>(pModule);
		if (!C_LibretroInstance::BuildInterface(info->raw, &hModule))
		{
			//DevMsg("libretro: Failed to build interface!\n");
			return 0;
		}
		else
			info->module = pModule;

		info->threadid = ThreadGetCurrentId();

		//	C_LibretroInstance* pLibretroInstance;
		libretro_raw* raw = info->raw;
		int state = info->state;
		unsigned int lastFrameNumber = 0;
		bool bDidNotify = false;

		//while (state != 666)
		while (!info->close)
		{
		//	if ( info->close )
			state = info->state;
			//pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
			//raw = pLibretroInstance->GetRaw();
			//if (!raw)
			//			continue;

			if (state == 0)
			{
				CSysModule* myModule = Sys_LoadModule(VarArgs("%s\\bin\\portaudio_x86.dll", engine->GetGameDirectory()));
				if (myModule)
				{
					DevMsg("portaudio_x86.dll loaded successfully.\n");

					PaError err = Pa_Initialize();
					if (err != paNoError)
						DevMsg("Failed to initialize PA.\n");
					else
						DevMsg("Initialized PA successfuly!\n");
				}
				else
					DevMsg("Failed to load portaudio_x86.dll.\n");

				// this should be done in the manager class.
				raw->set_environment(C_LibretroInstance::cbEnvironment);
				raw->set_video_refresh(C_LibretroInstance::cbVideoRefresh);
				raw->set_audio_sample(C_LibretroInstance::cbAudioSample);
				raw->set_audio_sample_batch(C_LibretroInstance::cbAudioSampleBatch);
				raw->set_input_poll(C_LibretroInstance::cbInputPoll);
				raw->set_input_state(C_LibretroInstance::cbInputState);

				/*
				if (s_raw->api_version() != RETRO_API_VERSION)
				{
				DevMsg("libretro: Failed version check!\n");
				// FIXME FIX ME Probably need to clean up!
				return false;
				}
				*/

				// skip state 2
				info->state = 1;
				g_pAnarchyManager->GetLibretroManager()->OnLibretroInstanceCreated(info);	// FIXME: If instance is closed by the time this line is reached, might cause the crash!
			}
			else if (state == 3)
			{
				// load a game if we have one
				if (info->game != "")
				{
					//	DevMsg("Load the game next!!\n");
					info->state = 4;
					C_LibretroInstance::LoadGame();
					info->raw->run();
					//	DevMsg("cuatro\n");
				}
			}
			else if (state == 5)
			{
				/*
				if (!info->processingaudio)
				{
				info->processingaudio = true;
				info->raw->run();
				}
				*/

				info->raw->run();

				/*
				bool bShouldRender = false;
				if (lastFrameNumber != gpGlobals->framecount)
				{
				lastFrameNumber = gpGlobals->framecount;

				if (info->framerate == 0)
				bShouldRender = true;
				else
				{
				float dif = 1 / (info->framerate * 1.0);
				if (gpGlobals->curtime - info->lastrendered >= dif * 1.5 || true)	// sense the blocking audio API is being used, we should render every chance we get to be synced to audio.
				bShouldRender = true;
				}

				if (bShouldRender)
				{
				//				info->lastrendered = gpGlobals->curtime;
				//				if (info->readyfornextframe)
				info->raw->run();
				}
				}
				*/
			}
		}
	}

	if (info)
	{
		// clean up the memory
		if (info->module)
			Sys_UnloadModule(info->module);

		if (info->lastframedata)
			free(info->lastframedata);

		delete info;
	}

	return 0;
}

int C_LibretroInstance::GetOptionCurrentValue(unsigned int index)
{
	if (m_info && index < m_info->optionscurrentvalues.size())
		return m_info->optionscurrentvalues[index];
	else
		return 0;
}

bool C_LibretroInstance::IsSelected()
{
	return (this == g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance());
}

bool C_LibretroInstance::HasFocus()
{
	return (this == g_pAnarchyManager->GetLibretroManager()->GetFocusedLibretroInstance());
}

bool C_LibretroInstance::Focus()
{
	return g_pAnarchyManager->GetLibretroManager()->FocusLibretroInstance(this);
}

bool C_LibretroInstance::Select()
{
	return g_pAnarchyManager->GetLibretroManager()->SelectLibretroInstance(this);
}

bool C_LibretroInstance::Blur()
{
	if (g_pAnarchyManager->GetLibretroManager()->GetFocusedLibretroInstance())
		g_pAnarchyManager->GetLibretroManager()->FocusLibretroInstance(null);

	return true;
}

bool C_LibretroInstance::Deselect()
{
	return g_pAnarchyManager->GetLibretroManager()->SelectLibretroInstance(null);
}

void C_LibretroInstance::Close()
{
	g_pAnarchyManager->GetLibretroManager()->DestroyLibretroInstance(this);
}

bool C_LibretroInstance::CreateWorkerThread(std::string core)
{
	/*
	CSysModule* pModule = Sys_LoadModule(core.c_str());

	if (!pModule)
	{
		Msg("Failed to load %s\n", core.c_str());
		// FIXME FIX ME Probably need to clean up!
		return false;
	}

	HMODULE	hModule = reinterpret_cast<HMODULE>(pModule);
	if (!C_LibretroInstance::BuildInterface(m_raw, &hModule))
	{
		DevMsg("libretro: Failed to build interface!\n");
		// FIXME FIX ME Probably need to clean up!
		return false;
	}
	*/

	m_info = new LibretroInstanceInfo_t;
	m_info->state = 0;
	m_info->close = false;
	m_info->id = "";
	m_info->ready = false;
	m_info->readyfornextframe = true;
	m_info->copyingframe = false;
	m_info->readytocopyframe = false;
	m_info->coreloaded = false;
	m_info->gameloaded = false;
	m_info->raw = m_raw;
	m_info->corepath = m_corePath;
	m_info->module = null;// pModule;
	m_info->threadid = 0;
	m_info->libretroinstance = this;
	m_info->core = core;
	m_info->game = m_originalGame;
	m_info->lastframedata = null;
	m_info->lastframewidth = 0;
	m_info->lastframeheight = 0;
	m_info->lastframepitch = 0;
	m_info->videoformat = RETRO_PIXEL_FORMAT_UNKNOWN;
	m_info->optionshavechanged = false;
//	m_info->audiobuffer = null;
//	m_info->audiobuffersize = 1024;
//	m_info->audiobufferpos = 0;
	m_info->audiostream = null;
	m_info->samplerate = 0;
	m_info->framerate = 30;
	m_info->lastrendered = 0;
	m_info->audiobuffer = null;
	m_info->audiobuffersize = 0;
	m_info->audiobufferpos = 0;
	//m_info->safebuffer = null;
	//m_info->safebuffersize = 0;
	//m_info->safebufferpos = 0;
	m_info->processingaudio = false;
	
	CreateSimpleThread(MyThread, m_info);

	//ThreadId_t pThreadId = 420L;
	//CreateSimpleThread(MyThread, m_info, &pThreadId, 0U);

	/*
	if (!g_CMyAsyncThread.IsAlive())
		g_CMyAsyncThread.Start();

	if (!g_CMyAsyncThread.IsAlive())
		DevMsg("CreateAThreadAndCallTheFunction() failed to start the thread!\n");

	// Thread safety: make some local copies! The real ones could be deleted/changed while we execute.
	char* NewParameter1 = "init";// FUNCTION_THAT_CREATES_A_COPY_OF_THE_MEMORY(Parameter1);
	char* NewParameter2 = "Another test";// FUNCTION_THAT_CREATES_A_COPY_OF_THE_MEMORY(Parameter1);

	g_CMyAsyncThread.CallThreadFunction(NewParameter1, NewParameter2);
	g_CMyAsyncThread.CallWorker(CMyAsyncThread::EXIT);
	*/
	return true;
}

/*
bool CMyAsyncThread::FunctionToBeRunFromInsideTheThread(char* Parameter1, char* Parameter2)
{
	if (!Q_strcmp(Parameter1, "init"))
	{
		//DevMsg("Thread yo: %s %s\n", Parameter1, Parameter2);
		C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
		if (pLibretroInstance)
		{
			libretro_raw* raw = pLibretroInstance->GetRaw();
			raw->set_environment(&this->cbEnvironment);
		}
	}
	return true;
}

void CMyAsyncThread::Update()
{
	DevMsg("once?\n");
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
	if (pLibretroInstance)
	{
		DevMsg("Yarbles\n");
		libretro_raw* raw = pLibretroInstance->GetRaw();
	//	raw->run();
	}
}
*/
/*
unsigned C_LibretroInstance::Worker(void *params)
{
	DevMsg("whaaaaat\n");
}
*/
void C_LibretroInstance::cbMessage(enum retro_log_level level, const char * fmt, ...)
{
	va_list args;

	char* msg = new char[AA_MAX_STRING];

	va_start(args, fmt);
	int neededlen = Q_vsnprintf(msg, 64, fmt, args);
	va_end(args);

	DevMsg("libretro: %s\n", msg);

	delete[] msg;
}

///*
HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule);

	return hModule;
}
//*/

// http://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

const char* GetFormatName(int format)
{
	if (format == 0)
		return "0RGB1555";
	else if (format == 1)
		return "XRGB8888";
	else if (format == 2)
		return "RGB565";
	else
		return "UNKOWN";
}

bool C_LibretroInstance::cbEnvironment(unsigned cmd, void* data)
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return false;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	//	DevMsg("libretro: Environment called %u.\n", (unsigned int)cmd);

	//1 SET_ROTATION, no known supported core uses that. Cores are expected to deal with failures, anyways.
	//2 GET_OVERSCAN, I have no opinion. Use the default.
	if (cmd == RETRO_ENVIRONMENT_GET_CAN_DUPE) //3
	{
		DevMsg("libretro: Asking backend if CAN_DUPE.\n");
		//*(bool*)data = true;
		return false;
	}

	//4 was removed and can safely be ignored.
	//5 was removed and can safely be ignored.
	//6 SET_MESSAGE, ignored because I don't know what to do with that.
	//7 SHUTDOWN, ignored because no supported core has any reason to have Off buttons.
	//8 SET_PERFORMANCE_LEVEL, ignored because I don't support a wide range of powers.
	if (cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY || cmd == RETRO_ENVIRONMENT_GET_LIBRETRO_PATH || cmd == RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY) // note that libretro path might be wanting a full file location including extension, but ignore that for now and treat it like the others.
	{
		char* buf = "fail";// new char[AA_MAX_STRING];

		// Libretro path is actually NOT what they are asking for when they ask for SYSTEM path.
	/////////	Q_strcpy(buf, m_corePath.c_str());

		//Q_strcpy(buf, "X:\\Emulators\\Genesis\\roms");

		//Q_strcpy(buf, "X:\\Emulators\\Genesis\\roms");
		//V_FixSlashes(buf, '/');

		DevMsg("libretro: Fetching ");
		if (cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY)
			DevMsg("system");
		else if (cmd == RETRO_ENVIRONMENT_GET_LIBRETRO_PATH)
			DevMsg("libretro");
		else if (cmd == RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY)
			DevMsg("content");

		DevMsg(" directory %s\n", buf);

		(*(const char**)data) = buf;

		delete[] buf;
		return true;
	}

	/*
	if (cmd == RETRO_ENVIRONMENT_SET_VARIABLES)//16
	{
		//HMODULE hModule = GetCurrentModule();	// client.dll
		//HINSTANCE hInstance = GetModuleHandle(0);	// AArcade.exe
		WCHAR  cwBuffer[2048] = { 0 };
		LPWSTR pszBuffer = cwBuffer;
		DWORD  dwMaxChars = _countof(cwBuffer);
		DWORD  dwLength = 0;

		GetModuleFileNameW(hModule, pszBuffer, dwMaxChars);

		std::wstring wString = cwBuffer;
		std::string result = utf8_encode(wString);
		DevMsg("Result: %s\n", result.c_str());

		//long threadId = GetCurrentThreadId();
		//DevMsg("ID is: %l\n", threadId);
		//ThreadSetDebugName()

		//DevMsg("Value is: %s\n", m_info->id.c_str());
//		GetThreadHandle();

		if (ThreadInMainThread())
			DevMsg("MAIN THREAD!\n");
		else
			DevMsg("CHILD THREAD!\n");

		uint uId = ThreadGetCurrentId();
		C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
		DevMsg("Size is: %i\n", pLibretroInstance->GetInfo()->options.size());
	}
	*/

	if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) //10
	{
		enum retro_pixel_format newfmt = *(enum retro_pixel_format *)data;
		if (newfmt == RETRO_PIXEL_FORMAT_0RGB1555 || newfmt == RETRO_PIXEL_FORMAT_XRGB8888 ||
			newfmt == RETRO_PIXEL_FORMAT_RGB565)
		{
			DevMsg("libretro: Setting video format to %s\n", GetFormatName(newfmt));
			info->videoformat = newfmt;
			return true;
		}
		else
			return false;
	}

	if (cmd == RETRO_ENVIRONMENT_SET_HW_RENDER) //14
	{
		DevMsg("core requested to set environment HW render\n");
		return false;
	}

	if (cmd == RETRO_ENVIRONMENT_GET_VARIABLE) //15
	{
		struct retro_variable * variable = (struct retro_variable*)data;

		variable->value = NULL;
		unsigned int numOptions = info->options.size();

		DevMsg("Requesting variable: %s and numOptions are %i\n", variable->key, numOptions);
		/*
		for (unsigned int i = 0; i < s_options.size(); i++)
		{
		for (unsigned int j = 0; j < s_options[i]->values.size(); i++)
		{
		DevMsg("Value: %s\n", s_options[i]->values[j].c_str());
		}
		}
		*/


		for (unsigned int i = 0; i < numOptions; i++)
		{
			if (!Q_strcmp(variable->key, info->options[i]->name_internal))
			{
				//char* buf = new char[MAX_STRING_LENGTH];

				//Q_strcpy(buf, s_options[i]->values[s_optionsCurrentValues[i]]);


				//				DevMsg("libretro: Fetching variable %s = %s.\n", s_options[i]->name_internal, s_options[i]->values[s_optionsCurrentValues[i]].c_str());

				variable->value = VarArgs("%s", info->options[i]->values[info->optionscurrentvalues[i]].c_str());
				//variable->value = buf;

				//delete[] buf;

				break;
			}
		}

		info->optionshavechanged = false;

		/*
		for (unsigned int i = 0; i<this->core_opt_num; i++)
		{
		if (!strcmp(variable->key, this->core_opts[i].name_internal))
		{
		variable->value = this->core_opts[i].values[this->core_opt_current_values[i]];
		}
		}
		this->core_opt_changed = false;
		*/
		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_SET_VARIABLES)//16
	{
		DevMsg("libretro: RETRO_ENVIRONMENT_SET_VARIABLES\n");
		const struct retro_variable * variables = (const struct retro_variable*)data;

		while (!info->options.empty())
			info->options.pop_back();

		const struct retro_variable * variables_count = variables;

		while (variables_count->key) variables_count++;
		unsigned int numvars = variables_count - variables;

		bool bOptionListHasChanged = true;
		bool bOptionsHaveChanged = true;
		
		for (unsigned int i = 0; i<numvars; i++)
		{
			// Initialize to 0 index for this variable's value
			info->optionscurrentvalues.push_back(0);

			DevMsg("libretro: Setting up environment variable %s with definition %s of %u\n", variables[i].key, variables[i].value, i);

			libretro_core_option* pOption = new libretro_core_option();
			pOption->name_internal = variables[i].key;

			const char * values = Q_strstr(variables[i].value, "; ");

			//if the value does not contain "; ", the core is broken, and broken cores can break shit in whatever way they want, anyways.
			//let's segfault.
			// In other words, values would be null and a crash would occur when we tried to use it.
			pOption->name_display = VarArgs("%s", variables[i].value);

			unsigned int numvalues = 1;
			const char * valuescount = values;
			while (*valuescount)
			{
				if (*valuescount == '|') numvalues++;
				valuescount++;
			}

			std::string buf;
			const char * nextvalue = values;
			for (unsigned int j = 0; j<numvalues; j++)
			{
				nextvalue = values;
				while (*nextvalue && *nextvalue != '|') nextvalue++;
				unsigned int valuelen = nextvalue - values;

				buf = values;
				if (j == 0)
					buf = buf.substr(2, valuelen - 2);
				else
					buf = buf.substr(0, valuelen);

				pOption->values.push_back(buf);
				values = nextvalue + 1;
			}

			info->options.push_back(pOption);
		}

		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE) //17
	{
		//DevMsg("libretro: Fetching if options have changed (%s).\n", (s_bOptionsHaveChanged)?"true":"false");
		*(bool*)data = info->optionshavechanged;
		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_GET_LOG_INTERFACE) //27
	{
		struct retro_log_callback * logcb = (struct retro_log_callback*)data;
		logcb->log = &C_LibretroInstance::cbMessage;
		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_SHUTDOWN)
	{
		info->close = true;
		// should probably show some kind of related items screen when a video ends.
		return true;
	}

	const char * const names[] = {
		"(invalid)",
		"SET_ROTATION",
		"GET_OVERSCAN",
		"GET_CAN_DUPE",
		"(removed)",
		"(removed)",
		"SET_MESSAGE",
		"SHUTDOWN",
		"SET_PERFORMANCE_LEVEL",
		"GET_SYSTEM_DIRECTORY",
		"SET_PIXEL_FORMAT",
		"SET_INPUT_DESCRIPTORS",
		"SET_KEYBOARD_CALLBACK",
		"SET_DISK_CONTROL_INTERFACE",
		"SET_HW_RENDER",
		"GET_VARIABLE",
		"SET_VARIABLES",
		"GET_VARIABLE_UPDATE",
		"SET_SUPPORT_NO_GAME",
		"GET_LIBRETRO_PATH",
		"(removed)",
		"SET_FRAME_TIME_CALLBACK",
		"SET_AUDIO_CALLBACK",
		"GET_RUMBLE_INTERFACE",
		"GET_INPUT_DEVICE_CAPABILITIES",
		"GET_SENSOR_INTERFACE",
		"GET_CAMERA_INTERFACE",
		"GET_LOG_INTERFACE",
		"GET_PERF_INTERFACE",
		"GET_LOCATION_INTERFACE",
		"GET_CONTENT_DIRECTORY",
		"GET_SAVE_DIRECTORY",
		"SET_SYSTEM_AV_INFO",
		"SET_PROC_ADDRESS_CALLBACK",
		"SET_SUBSYSTEM_INFO",
		"SET_CONTROLLER_INFO",
		"SET_MEMORY_MAPS",
		"SET_GEOMETRY",
		"GET_USERNAME",
		"GET_LANGUAGE",
	};

	if ((cmd&~RETRO_ENVIRONMENT_EXPERIMENTAL) < sizeof(names) / sizeof(*names))
		C_LibretroInstance::cbMessage(RETRO_LOG_WARN, "Ignored unsupported environment command #%u %s.", cmd, names[cmd&~RETRO_ENVIRONMENT_EXPERIMENTAL]);
	else
		C_LibretroInstance::cbMessage(RETRO_LOG_WARN, "Ignored unsupported environment command #%u.", cmd);

	return false;
}

void C_LibretroInstance::cbVideoRefresh(const void * data, unsigned width, unsigned height, size_t pitch)
{
	if (!data)
		return;

	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	if (info->close)
		return;

	info->lastrendered = gpGlobals->curtime;

	if (!info->readyfornextframe || info->copyingframe )
		return;

	info->readyfornextframe = false;
	info->readytocopyframe = false;

	if (info->samplerate == 0)
	{
		struct retro_system_av_info avinfo;
		info->raw->get_system_av_info(&avinfo);

		if (avinfo.timing.sample_rate > 0)
		{
			info->samplerate = int(avinfo.timing.sample_rate);
			info->framerate = int(avinfo.timing.fps);
			C_LibretroInstance::CreateAudioStream();
		}
	}

	WORD red_mask = 0xF800;
	WORD green_mask = 0x7E0;
	WORD blue_mask = 0x1F;
	
	//DevMsg("Doin it\n");

	//DevMsg("video refresh\n");

	void* dest = malloc(pitch*height);
	Q_memcpy(dest, data, pitch*height);

	if (info->lastframedata)
		free(info->lastframedata);

	info->lastframedata = dest;
	info->lastframewidth = width;
	info->lastframeheight = height;
	info->lastframepitch = pitch;

	info->readytocopyframe = true;

	//pLibretroInstance->m_mutex.unlock();
	//DevMsg("Child Unlock\n");
}

void C_LibretroInstance::cbAudioSample(int16_t left, int16_t right)
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	/*
	if (info->audiobufferpos < 512)
	{
	double leftRegular = (int)left * 1.0;// pLibretroVolumeScaleConVar->GetFloat();
	if (left > 32767)
	leftRegular = 32767.0;
	if (left < -32768.0)
	leftRegular = -32768.0;

	double rightRegular = (int)right * 1.0;// s_pLibretroVolumeScaleConVar->GetFloat();
	if (right > 32767)
	rightRegular = 32767.0;
	if (right < -32768.0)
	rightRegular = -32768.0;

	info->audiobuffer[info->audiobufferpos++] = (int16_t)leftRegular;
	info->audiobuffer[info->audiobufferpos++] = (int16_t)rightRegular;
	}
	*/
}

size_t C_LibretroInstance::cbAudioSampleBatch(const int16_t * data, size_t frames)
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return 0;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	/*
	if ( info->processingaudio)
		return 0;
	else
		info->processingaudio = true;
		*/

	if (info->samplerate == 0)
	{
		struct retro_system_av_info avinfo;
		info->raw->get_system_av_info(&avinfo);

		if (avinfo.timing.sample_rate > 0)
		{
			info->samplerate = int(avinfo.timing.sample_rate);
			info->framerate = int(avinfo.timing.fps);
			C_LibretroInstance::CreateAudioStream();
		}
	}

	if (info->samplerate <= 0 || frames <= 0 || !info->audiostream ) // || s_threadAudioParams.frames
		return 0;

	PaError err = Pa_WriteStream(info->audiostream, data, frames);// paFramesPerBufferUnspecified);
	return frames;

	int16_t* buffer = info->audiobuffer;
	unsigned int size = info->audiobuffersize;
	unsigned int pos = info->audiobufferpos;

	unsigned int processedCount = size - pos;
	if (processedCount > frames)
		processedCount = frames;

	if (processedCount > 0)
	{
		Q_memcpy(buffer + (pos * 2), data, sizeof(int16_t) * processedCount * 2);
		info->audiobufferpos += processedCount;
	}
	return processedCount;
}

void C_LibretroInstance::cbInputPoll(void)
{
		//DevMsg("libretro: Input Poll called.\n");
}

int16_t C_LibretroInstance::cbInputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return false;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	/*
	#define RETRO_DEVICE_ID_JOYPAD_B        0
	#define RETRO_DEVICE_ID_JOYPAD_Y        1
	#define RETRO_DEVICE_ID_JOYPAD_SELECT   2
	#define RETRO_DEVICE_ID_JOYPAD_START    3
	#define RETRO_DEVICE_ID_JOYPAD_UP       4
	#define RETRO_DEVICE_ID_JOYPAD_DOWN     5
	#define RETRO_DEVICE_ID_JOYPAD_LEFT     6
	#define RETRO_DEVICE_ID_JOYPAD_RIGHT    7
	#define RETRO_DEVICE_ID_JOYPAD_A        8
	#define RETRO_DEVICE_ID_JOYPAD_X        9
	#define RETRO_DEVICE_ID_JOYPAD_L       10
	#define RETRO_DEVICE_ID_JOYPAD_R       11
	#define RETRO_DEVICE_ID_JOYPAD_L2      12
	#define RETRO_DEVICE_ID_JOYPAD_R2      13
	#define RETRO_DEVICE_ID_JOYPAD_L3      14
	#define RETRO_DEVICE_ID_JOYPAD_R3      15
	*/
	//DevMsg("libretro: Input State called.\n");
	//DevMsg("libretro: Input State called with port %u, device %u, index %u, and ID %u.\n", port, device, index, id);

	//if (port != 0 || device != 1 || index != 0 || true)
	//	return (int16_t)0;

	if (info->inputstate.find(id) != info->inputstate.end() )
		return (int16_t)info->inputstate[id];
	else
		return (int16_t)0;
}

void C_LibretroInstance::ResizeFrameFromRGB565(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
//	uint uId = ThreadGetCurrentId();
//	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
//	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	LibretroInstanceInfo_t* info = m_info;
	if (!info->lastframedata)
		return;
	
	info->readyfornextframe = false;

	//	DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);

	WORD red_mask = 0xF800;
	WORD green_mask = 0x7E0;
	WORD blue_mask = 0x1F;

	uint16* pRealSrc = (uint16*)pSrc;

	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{

		unsigned int srcY = dstY * sourceHeight / destHeight;
		uint16* pSrcRow = pRealSrc + (srcY * ((int)sourcePitch / 2));

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;

			int red = (pSrcRow[srcX] & red_mask) >> 11;
			int green = (pSrcRow[srcX] & green_mask) >> 5;
			int blue = (pSrcRow[srcX] & blue_mask);

			pDstCur[0] = blue * (255 / 31);
			pDstCur[1] = green * (255 / 63);
			pDstCur[2] = red * (255 / 31);

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	info->readyfornextframe = true;
}

void C_LibretroInstance::ResizeFrameFromRGB1555(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
//	uint uId = ThreadGetCurrentId();
//	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
//	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	LibretroInstanceInfo_t* info = m_info;
	if (!info->lastframedata)
		return;

	info->readyfornextframe = false;

	//	DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);

	WORD red_mask = 0x7C00;
	WORD green_mask = 0x03E0;
	WORD blue_mask = 0x001F;

	uint16* pRealSrc = (uint16*)pSrc;

	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{

		unsigned int srcY = dstY * sourceHeight / destHeight;
		uint16* pSrcRow = pRealSrc + (srcY * ((int)sourcePitch / 2));

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;

			int red = (pSrcRow[srcX] & red_mask) >> 10;
			int green = (pSrcRow[srcX] & green_mask) >> 5;
			int blue = (pSrcRow[srcX] & blue_mask);

			pDstCur[0] = blue * (255 / 31);
			pDstCur[1] = green * (255 / 31);
			pDstCur[2] = red * (255 / 31);

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	info->readyfornextframe = true;
}

void C_LibretroInstance::ResizeFrameFromXRGB8888(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
	//DevMsg("Thread ID: %u\n", ThreadGetCurrentId);
//	uint uId = ThreadGetCurrentId();
//	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
//	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	//LibretroInstanceInfo_t* info = m_info;

	//if (!m_info->lastframedata)
//	DevMsg("Main Lock\n");
	if (!m_info->lastframedata)
		return;

//	m_mutex.lock();
//	if (!m_info->lastframedata || !m_info->readyfornextframe)
	//	return;


	//m_info->readyfornextframe = false;

	//DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);
//	DevMsg("Test: %s\n", pDest);

	unsigned int sourceWidthCopy = sourceWidth;
	unsigned int sourceHeightCopy = sourceHeight;
	size_t sourcePitchCopy = sourcePitch;
	unsigned int sourceDepthCopy = sourceDepth;

	//void* pSrcCopy = malloc(sourcePitchCopy * sourceHeightCopy);
	//Q_memcpy(pSrcCopy, pSrc, sourcePitchCopy * sourceHeightCopy);


	const unsigned char* pRealSrc = (const unsigned char*)pSrc;
	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{
		unsigned int srcY = dstY * sourceHeight / destHeight;
		const unsigned char* pSrcRow = pRealSrc + srcY*(sourcePitch);

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;
			pDstCur[0] = pSrcRow[srcX*sourceDepth + 0];
			pDstCur[1] = pSrcRow[srcX*sourceDepth + 1];
			pDstCur[2] = pSrcRow[srcX*sourceDepth + 2];

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	/*
	const unsigned char* pRealSrc = (const unsigned char*)pSrc;
	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{
		unsigned int srcY = dstY * sourceHeight / destHeight;
		const unsigned char* pSrcRow = pRealSrc + srcY*(sourcePitch);

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;
			pDstCur[0] = pSrcRow[srcX*sourceDepth + 0];
			pDstCur[1] = pSrcRow[srcX*sourceDepth + 1];
			pDstCur[2] = pSrcRow[srcX*sourceDepth + 2];

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}
	*/

//	free(pSrcCopy);

	m_info->readyfornextframe = true;

//	m_mutex.unlock();
//	DevMsg("Main Unlock\n");
}

void C_LibretroInstance::CopyLastFrame(unsigned char* dest, unsigned int width, unsigned int height, size_t pitch, unsigned int depth)
{
	if (m_info->copyingframe || !m_info->readytocopyframe || g_pAnarchyManager->GetSuspendEmbedded())
		return;

	m_info->copyingframe = true;
	m_info->readytocopyframe = false;
	//DevMsg("Render: Do it!\n");
	//RETRO_PIXEL_FORMAT_0RGB1555

//	uint uId = ThreadGetCurrentId();
//	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
	//LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	//LibretroInstanceInfo_t* info = m_info;
	if (m_info->videoformat == RETRO_PIXEL_FORMAT_RGB565)
		this->ResizeFrameFromRGB565(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 3, width, height, pitch, depth);
	else if (m_info->videoformat == RETRO_PIXEL_FORMAT_XRGB8888)
		this->ResizeFrameFromXRGB8888(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 4, width, height, pitch, depth);
	else
		this->ResizeFrameFromRGB1555(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 3, width, height, pitch, depth);

	m_info->copyingframe = false;
}

void C_LibretroInstance::OnProxyBind(C_BaseEntity* pBaseEntity)
{
	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

//	if (m_id == "images")
//		return;

	/*
	if ( pBaseEntity )
	DevMsg("WebTab: OnProxyBind: %i\n", pBaseEntity->entindex());
	else
	DevMsg("WebTab: OnProxyBind\n");
	*/

	// visiblity test
	if (m_iLastVisibleFrame < gpGlobals->framecount)
	{
		if (!g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
			g_pAnarchyManager->GetCanvasManager()->IncrementVisibleCanvasesCurrentFrame();
		else
			g_pAnarchyManager->GetCanvasManager()->IncrementVisiblePriorityCanvasesCurrentFrame();
	}
	m_iLastVisibleFrame = gpGlobals->framecount;

	if (m_iLastRenderedFrame < gpGlobals->framecount)
	{
		/*
		if (!g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
			g_pAnarchyManager->GetCanvasManager()->IncrementVisibleCanvasesCurrentFrame();
		else
			g_pAnarchyManager->GetCanvasManager()->IncrementVisiblePriorityCanvasesCurrentFrame();
		*/

		if (g_pAnarchyManager->GetCanvasManager()->ShouldRender(this) && m_info->readytocopyframe)
			Render();
	}
}

void C_LibretroInstance::Render()
{
//	if (m_id == "images")
	//	return;
	//DevMsg("Rendering texture: %s\n", m_pTexture->GetName());
	//	DevMsg("Render Web Tab: %s\n", this->GetTexture()->Ge>GetId().c_str());
	//DevMsg("WebTab: Render: %s on %i\n", m_id.c_str(), gpGlobals->framecount);
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->SetEmbeddedInstance(this);
	m_pTexture->Download();

	m_iLastRenderedFrame = gpGlobals->framecount;

	if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
		g_pAnarchyManager->GetCanvasManager()->SetLastPriorityRenderedFrame(gpGlobals->framecount);
	else
		g_pAnarchyManager->GetCanvasManager()->SetLastRenderedFrame(gpGlobals->framecount);
}

void C_LibretroInstance::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (m_info->state == 5)
		this->CopyLastFrame(pVTFTexture->ImageData(0, 0, 0), pSubRect->width, pSubRect->height, pSubRect->width * 4, 4);
}

C_InputListener* C_LibretroInstance::GetInputListener()
{
	return g_pAnarchyManager->GetLibretroManager()->GetInputListener();
}

bool C_LibretroInstance::SetGame(std::string file)
{
	if (!m_info || m_info->gameloaded || m_info->close)
		return false;

	m_info->game = file;
	return true;
}

C_EmbeddedInstance* C_LibretroInstance::GetParentSelectedEmbeddedInstance()
{
	return g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
}