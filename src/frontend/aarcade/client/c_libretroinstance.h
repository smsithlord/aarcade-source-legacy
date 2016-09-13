#ifndef C_LIBRETRO_INSTANCE_H
#define C_LIBRETRO_INSTANCE_H

#include "c_embeddedinstance.h"
#include "libretro.h"
#include "portaudio.h"
#include "c_inputlistenerlibretro.h"
#include <string>
#include <vector>
//#include <mutex>
//#include "c_libretrosurfaceregen.h"
//#include <map>

/*
typedef struct
{
	float left_phase;
	float right_phase;
}
paTestData;
*/

struct libretro_raw {
	void(*set_environment)(retro_environment_t);
	void(*set_video_refresh)(retro_video_refresh_t);
	void(*set_audio_sample)(retro_audio_sample_t);
	void(*set_audio_sample_batch)(retro_audio_sample_batch_t);
	void(*set_input_poll)(retro_input_poll_t);
	void(*set_input_state)(retro_input_state_t);
	void(*init)(void);
	void(*deinit)(void);
	unsigned(*api_version)(void);
	void(*get_system_info)(struct retro_system_info * info);
	void(*get_system_av_info)(struct retro_system_av_info * info);
	void(*set_controller_port_device)(unsigned port, unsigned device);
	void(*reset)(void);
	void(*run)(void);
	size_t(*serialize_size)(void);
	bool(*serialize)(void* data, size_t size);
	bool(*unserialize)(const void* data, size_t size);
	void(*cheat_reset)(void);
	void(*cheat_set)(unsigned index, bool enabled, const char * code);
	bool(*load_game)(const struct retro_game_info * game);
	bool(*load_game_special)(unsigned game_type, const struct retro_game_info * info, size_t num_info);
	void(*unload_game)(void);
	unsigned(*get_region)(void);
	void* (*get_memory_data)(unsigned id);
	size_t(*get_memory_size)(unsigned id);
};

struct libretro_core_option {
	const char * name_internal;
	const char * name_display;
	std::vector<std::string> values;
};

class C_LibretroInstance;

struct LibretroInstanceInfo_t
{
	int state;
	bool close;
	std::string id;
	bool ready;
	bool readyfornextframe;
	bool copyingframe;
	bool readytocopyframe;
	bool coreloaded;
	bool gameloaded;
	libretro_raw* raw;
	std::string corepath;
	CSysModule* module;
	uint threadid;
	C_LibretroInstance* libretroinstance;
	std::string core;
	std::string game;
	std::vector<libretro_core_option*> options;
	std::vector<int> optionscurrentvalues;
	void* lastframedata;
	unsigned int lastframewidth;
	unsigned int lastframeheight;
	size_t lastframepitch;
	retro_pixel_format videoformat;
	bool optionshavechanged;
	//int16_t* audiobuffer;
	//int audiobuffersize;
	//int audiobufferpos;
	//void* audiostream;
	PaStream* audiostream;
	int samplerate;
	int framerate;
	float lastrendered;
	int16_t* audiobuffer;
	unsigned int audiobuffersize;
	unsigned int audiobufferpos;
	//int16_t* safebuffer;
	//unsigned int safebuffersize;
	//unsigned int safebufferpos;
	bool processingaudio;
};

class C_LibretroInstance : public C_EmbeddedInstance
{
public:
	C_LibretroInstance();
	~C_LibretroInstance();
	void SelfDestruct();

	void Init(std::string id = "");
	bool CreateWorkerThread(std::string core);
	void Update();
	bool LoadCore();
	static bool LoadGame();
	void OnGameLoaded();
	void OnCoreLoaded();
	static bool BuildInterface(libretro_raw* raw, void* pLib);
	static void CreateAudioStream();
	static void DestroyAudioStream();

	// threaded
	//unsigned Worker(void *params);

	// callbacks
	static void cbMessage(enum retro_log_level level, const char * fmt, ...);
	static bool cbEnvironment(unsigned cmd, void* data);
	static void cbVideoRefresh(const void * data, unsigned width, unsigned height, size_t pitch);
	static void cbAudioSample(int16_t left, int16_t right);
	static size_t cbAudioSampleBatch(const int16_t * data, size_t frames);
	static void cbInputPoll(void);
	static int16_t cbInputState(unsigned port, unsigned device, unsigned index, unsigned id);

	void ResizeFrameFromRGB565(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth);
	void ResizeFrameFromRGB1555(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth);
	void ResizeFrameFromXRGB8888(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth);
	void CopyLastFrame(unsigned char* dest, unsigned int width, unsigned int height, size_t pitch, unsigned int depth);

	void OnProxyBind(C_BaseEntity* pBaseEntity);
	void Render();
	void RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect);

	// accessors
	libretro_raw* GetRaw() { return m_raw; }
	LibretroInstanceInfo_t* GetInfo() { return m_info; }
	ITexture* GetTexture() { return m_pTexture; }
	int GetLastRenderedFrame() { return m_iLastRenderedFrame; }
	C_InputListener* GetInputListener();
	//std::mutex m_mutex;

	// mutators	

private:
	ITexture* m_pTexture;
	int m_iLastRenderedFrame;
	std::string m_id;
	libretro_raw* m_raw;
	std::string m_corePath;
	LibretroInstanceInfo_t* m_info;
	//CSysModule* m_pModule;
};

/*
//-----------------------------------------------------------------------------
// A thread that can execute a function asynchronously.
//-----------------------------------------------------------------------------
class CMyAsyncThread : public CWorkerThread
{
public:
	CMyAsyncThread() :
		m_Parameter1(NULL),
		m_Parameter2(NULL)
	{
		SetName("MyAsyncThread");
	}

	~CMyAsyncThread()
	{
	}

	enum
	{
		CALL_FUNC,
		EXIT,
	};

	bool	CallThreadFunction(char* Parameter1, char* Parameter2)
	{
		Assert(!Parameter1);
		Assert(!Parameter2);
		m_Parameter1 = Parameter1;
		m_Parameter2 = Parameter2;
		CallWorker(CALL_FUNC);

		return true;
	}

	int Run()
	{
		unsigned nCall;
		while (WaitForCall(&nCall))
		{
			if (nCall == EXIT)
			{
				Reply(1);
				break;
			}

			// Reset some variables
			char* Parameter1 = m_Parameter1;
			char* Parameter2 = m_Parameter2;
			m_Parameter1 = 0;
			m_Parameter2 = 0;

			Reply(1);

			FunctionToBeRunFromInsideTheThread(Parameter1, Parameter2);
			Update();
		}
		return 0;
	}

private:
	char* m_Parameter1;
	char* m_Parameter2;

public:
	bool FunctionToBeRunFromInsideTheThread(char* Parameter1, char* Parameter2);
	void Update();
	static bool cbEnvironment(unsigned cmd, void* data);
};

static CMyAsyncThread g_CMyAsyncThread;
*/
#endif