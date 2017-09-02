#ifndef C_LIBRETRO_INSTANCE_H
#define C_LIBRETRO_INSTANCE_H

#include "c_embeddedinstance.h"
#include "libretro.h"
#include "portaudio.h"
#include "c_inputlistenerlibretro.h"
#include <string>
#include <vector>
#include <map>

// openGL stuff
#include <stdio.h>
#include <stdlib.h>
// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32 true
#define GLFW_EXPOSE_NATIVE_WGL true
#include <GLFW/glfw3native.h>
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

struct RunningLibretroCores_t
{
	int count;
	std::string last_error;
	std::string last_msg;
};

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

	// hardware acceleration stuff
	retro_hw_context_reset_t context_reset;
	retro_hw_context_reset_t context_destroy;
};

struct libretro_core_option {
	std::string name_internal;
	std::string name_display;
	std::vector<std::string> values;
};

class C_LibretroInstance;

struct memory_map_t {
	//RETRO_MEMORY_RTC
	size_t rtcsize;
	uint8_t* rtcdata;

	//RETRO_MEMORY_SAVE_RAM
	size_t saveramsize;
	uint8_t* saveramdata;

	//RETRO_MEMORY_SYSTEM_RAM
	size_t systemramsize;
	uint8_t* systemramdata;

	//RETRO_MEMORY_VIDEO_RAM
	size_t videoramsize;
	uint8_t* videoramdata;
};

struct LibretroInstanceInfo_t
{
	RunningLibretroCores_t* runninglibretrocores;
	int state;
	bool paused;
	bool reset;
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
	std::string assetspath;
	std::string systempath;
	std::string savepath;
	CSysModule* module;
	uint threadid;
	C_LibretroInstance* libretroinstance;
	std::string core;
	std::string game;
	std::vector<libretro_core_option*> options;
	//unsigned int numOptions;
	//std::map<std::string, std::string> optioncurrentvalues;
	//std::vector<int> optionscurrentvalues;
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
	float volume;
	//int16_t* audiobuffer;
	//unsigned int audiobuffersize;
	//unsigned int audiobufferpos;
	//int16_t* safebuffer;
	//unsigned int safebuffersize;
	//unsigned int safebufferpos;
	bool processingaudio;
	//std::map<std::string, float> inputstate;
	GLFWwindow* window;
	GLuint* framebuffer;


	//used elements:
	//context_type - handled
	//context_reset - TODO
	//get_current_framebuffer - handled externally
	//get_proc_address - handled externally
	//depth - handled
	//stencil - handled
	//bottom_left_origin - TODO
	//version_major - handled
	//version_minor - handled
	//cache_context - ignored (treated as always true)
	//context_destroy - TODO
	//debug_context - handled
	/*
	video* out_chain;
	sh_vercoordloc;
	sh_texcoordloc;
	sh_texcoordbuf;
	sh_vertexbuf_first;
	hdc;
	bool is3d;
	in_texwidth;
	in_texheight;
	in_lastwidth;
	in_lastheight;
	out_width;
	out_height;
	in_texwidth;
	in_texheight;

	unsigned int sh_passes;
	GLuint sh_prog;
	GLuint sh_tex;
	GLuint sh_fbo;
	in2_fmt;
	in2_type;
	in2_bytepp;
	in3;//
	in3_renderbuffer;
	*/


	const retro_controller_info* portdata;
	std::vector<int> currentPortTypes;
	unsigned int numports;
	KeyValues* libretrokeybinds;
	KeyValues* corekeybinds;
	KeyValues* gamekeybinds;
	KeyValues* inputstate;
	KeyValues* coreCoreOptions;
	KeyValues* gameCoreOptions;

	// hardware acceleration stuff
	retro_hw_context_type context_type;
	bool depth;
	bool stencil;
	bool bottom_left_origin;
	unsigned version_major;
	unsigned version_minor;
	bool cache_context;
	bool debug_context;

	// system info stuff
	std::string library_name;      // Descriptive name of library. Should not contain any version numbers, etc.
	std::string library_version;   // Descriptive version of core.
	std::string valid_extensions;  // A string listing probably content extensions the core will be able to load, separated with pipe. I.e. "bin|rom|iso". Typically used for a GUI to filter out extensions.
	bool need_fullpath;	// If true, retro_load_game() is guaranteed to provide a valid pathname in retro_game_info::path. ::data and ::size are both invalid. If false, ::data and ::size are guaranteed to be valid, but ::path might not be valid. This is typically set to true for libretro implementations that must load from file. Implementations should strive for setting this to false, as it allows the frontend to perform patching, etc.
	bool block_extract;	// If true, the frontend is not allowed to extract any archives before loading the real content. Necessary for certain libretro implementations that load games from zipped archives.

	size_t statesize;
	void* statedata;// = malloc(pitch*height);

	memory_map_t* memorymap;

	std::string prettycore;
	std::string prettygame;
	KeyValues* settings;


	//KeyValues* activekeybinds;
};

class C_LibretroInstance : public C_EmbeddedInstance
{
public:
	C_LibretroInstance();
	~C_LibretroInstance();
	void SelfDestruct();

	std::string GetId() { return m_id; }

	void CleanUpTexture();

	KeyValues* GetOverlayKV() { return m_pOverlayKV; }
	void SetOverlay(std::string overlayId);
	void SaveOverlay(std::string type, std::string overlayId, float x, float y, float width, float height);
	void ClearOverlay(std::string type, std::string overlayId);
	void Init(std::string id, std::string title, int iEntIndex);
	bool CreateWorkerThread(std::string core);
	void Update();
	bool LoadCore(std::string coreFile = "");
	static bool LoadGame();
	void SetReset(bool bValue);
	void SetPause(bool bValue);
	void SetVolume(float fVolume);
	bool GetPause();
	void OnGameLoaded();
	void OnCoreLoaded();
	std::string GetLibretroCore();
	std::string GetLibretroFile();
	//float GetLibretroVolume();
	static bool BuildInterface(libretro_raw* raw, void* pLib);
	static void CreateAudioStream();
	static void DestroyAudioStream();
	std::string GetOriginalItemId() { return m_originalItemId; }
	int GetOriginalEntIndex() { return m_iOriginalEntIndex; }
	void SetOriginalItemId(std::string itemId) { m_originalItemId = itemId; }
	void SetOriginalEntIndex(int val) { m_iOriginalEntIndex = val; }

	bool HasInfo() { return (m_info != null); }
	std::vector<libretro_core_option*>& GetAllOptions() { return m_info->options; }	// others should always check if m_info exists first themselves!!
	//int GetOptionCurrentValue(unsigned int index);
	//std::string GetOptionCurrentValue(std::string name_internal);

	bool IsSelected();
	bool HasFocus();
	bool Focus();
	bool Blur();
	bool Select();
	bool Deselect();

	void Close();
	void GetFullscreenInfo(float& fPositionX, float& fPositionY, float& fSizeX, float& fSizeY, std::string& overlayId);

	std::string GetURL() { return ""; }


	// threaded
	//unsigned Worker(void *params);

//	int16_t GetInputState(LibretroInstanceInfo_t* info, unsigned int port, unsigned int device, unsigned int index, unsigned int id);

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
	void ResizeFrameFromRGB888(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth);
	void CopyLastFrame(unsigned char* dest, unsigned int width, unsigned int height, size_t pitch, unsigned int depth);

	void OnProxyBind(C_BaseEntity* pBaseEntity);
	void Render();
	void RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect);
	C_EmbeddedInstance* GetParentSelectedEmbeddedInstance();

	void SaveLibretroKeybind(std::string type, unsigned int retroport, unsigned int retrodevice, unsigned int retroindex, unsigned int retrokey, std::string steamkey);
	void SaveLibretroOption(std::string type, std::string name_internal, std::string value);

	// accessors
	std::string GetOverlayId() { return m_overlayId; }
	libretro_raw* GetRaw() { return m_raw; }
	LibretroInstanceInfo_t* GetInfo() { return m_info; }
	ITexture* GetTexture() { return m_pTexture; }
	int GetLastVisibleFrame() { return m_iLastVisibleFrame; }
	int GetLastRenderedFrame() { return m_iLastRenderedFrame; }
	C_InputListener* GetInputListener();
	//std::mutex m_mutex;
	std::string GetOriginalGame() { return m_originalGame; }
//	std::string GetOriginalItemId() { return m_originalItemId; }
	std::string GetTitle() { return m_title; }

	// mutators
	bool SetGame(std::string file);
	void SetOriginalGame(std::string file) { m_originalGame = file; }
//	void SetOriginalItemId(std::string itemId) { m_originalItemId = itemId; }
	void SetTitle(std::string title) { m_title = title; }

private:
	std::string m_overlayId;
	KeyValues* m_pOverlayKV;
	std::string m_originalGame;
	int m_iLastVisibleFrame;
	ITexture* m_pTexture;
	int m_iLastRenderedFrame;
	std::string m_title;
	std::string m_id;
	std::string m_originalItemId;
	libretro_raw* m_raw;
	//std::string m_userBase;
	//std::string m_corePath;
	//std::string m_assetsPath;
	//std::string m_systemPath;
	//std::string m_savePath;
	LibretroInstanceInfo_t* m_info;
	int m_iOriginalEntIndex;
	//float m_fPositionX;
	//float m_fPositionY;
	//float m_fSizeX;
	//float m_fSizeY;
	//std::string m_file;
	//C_OpenGLManager* m_pOpenGLManager;
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