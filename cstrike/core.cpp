#define _CRTDBG_MAP_ALLOC
// used: [win] shgetknownfolderpath
#include <shlobj_core.h>

#include "core.h"

// used: features setup
#include "features.h"
// used: string copy
#include "utilities/crt.h"
// used: mem
#include "utilities/memory.h"
// used: l_print
#include "utilities/log.h"
// used: inputsystem setup/restore
#include "utilities/inputsystem.h"
// used: draw destroy
#include "utilities/draw.h"
#include "utilities/notify.h"

// used: interfaces setup/destroy
#include "core/interfaces.h"
// used: sdk setup
#include "core/sdk.h"
// used: config setup & variables
#include "core/variables.h"
// used: hooks setup/destroy
#include "core/hooks.h"
// used: schema setup/dump
#include "core/schema.h"
// used: convar setup
#include "core/convars.h"
// used: menu
#include "core/menu.h"
#include <format>
// used: product version
#include "sdk/interfaces/iengineclient.h"
#include "InvecntoryChanger.h"
#include "sdk/EntityList/Events.h"

bool CORE::GetWorkingPath(wchar_t* wszDestination)
{
	bool bSuccess = false;
	PWSTR wszPathToDocuments = nullptr;

	// get path to user documents
	if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, nullptr, &wszPathToDocuments)))
	{
		CRT::StringCat(CRT::StringCopy(wszDestination, wszPathToDocuments), CS_XOR(L"\\UNDERAGER_logs\\"));
		bSuccess = true;
		// create directory if it doesn't exist
		if (!CreateDirectoryW(wszDestination, nullptr))
		{
			if (::GetLastError() != ERROR_ALREADY_EXISTS)
			{
				L_PRINT(LOG_ERROR) << CS_XOR("failed to create default working directory, because one or more intermediate directories don't exist");
				bSuccess = false;
			}
		}
	}
	::CoTaskMemFree(wszPathToDocuments);

	NOTIFY::Push({ N_TYPE_SUCCESS, ("Successfully injection!") });

	return bSuccess;
}

static bool Setup(HMODULE hModule)
{
	TIMER_START("SETUP");
#ifdef CS_LOG_CONSOLE
	if (!L::AttachConsole(CS_XOR(L"UNDERAGER (DEBUG)")))
	{
		CS_ASSERT(false); // failed to attach console
		return false;
	}
#endif
#ifdef CS_LOG_FILE
	if (!L::OpenFile(CS_XOR(L"UNDERAGER.log")))
	{
		CS_ASSERT(false); // failed to open file
		return false;
	}
#endif
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("logging system initialization completed");

	// setup game's exported functions
	if (!MEM::Setup())
	{
		CS_ASSERT(false); // failed to setup memory system
		return false;
	}
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("memory system initialization completed");

	if (!MATH::Setup())
	{
		CS_ASSERT(false); // failed to setup math system
		return false;
	}
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("math system initialization completed");

	// grab game's interfaces
	if (!I::Setup())
	{
		CS_ASSERT(false); // failed to setup interfaces
		return false;
	}
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("interfaces initialization completed");

	if (!SDK::Setup())
	{
		CS_ASSERT(false); // failed to setup sdk
		return false;
	}
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("sdk initialization completed");

	// setup input system and replace game's window messages processor with our
	if (!IPT::Setup())
	{
		CS_ASSERT(false); // failed to setup input system
		return false;
	}
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("input system initialization completed");

	// @note: sleep to wait finishing font building
	D::Setup(IPT::hWindow, I::Device, I::DeviceContext);
	/*MENU::UpdateStyle(nullptr);*/
	while (D::bInitialized == false)
		::Sleep(200U);
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("renderer backend initialization completed");

	// initialize feature-related stuff
	if (!F::Setup())
	{
		CS_ASSERT(false); // failed to setup features
		return false;
	}
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("features initialization completed");

	// iterate all valid modules for schema
	std::vector<std::string> vecNeededModules = { CS_XOR("client.dll"), CS_XOR("engine2.dll"), CS_XOR("schemasystem.dll") };
	for (auto& szModule : vecNeededModules)
	{
		if (!SCHEMA::Setup(CS_XOR(L"schema.txt"), szModule.c_str()))
		{
			CS_ASSERT(false); // failed to setup schema system
			return false;
		}
	}
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("schema system initialization completed");

	if (!CONVAR::Dump(CS_XOR(L"convars.txt")))
	{
		CS_ASSERT(false); // failed to setup convars system
		return false;
	}
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("convars dumped completed, output: \"convars.txt\"");

	if (!CONVAR::Setup())
	{
		CS_ASSERT(false); // failed to setup convars system
		return false;
	}
	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("convars system initialization completed");

	// setup hooks
	if (!H::Setup())
	{
		CS_ASSERT(false); // failed to setup hooks
		return false;
	}
	L_PRINT(LOG_NONE) << CS_XOR("hooks initialization completed");
	skin_changer::dumpitems = true;
	L_PRINT(LOG_NONE) << CS_XOR("InventoryChanger initialization completed");

	if (Events->Intilization())
		L_PRINT(LOG_NONE) << CS_XOR("Events initialization completed");

	// setup values to save/load cheat variables into/from files and load default configuration
	if (!C::Setup(CS_XOR(CS_CONFIGURATION_DEFAULT_FILE_NAME)))
		// this error is not critical, only show that
		L_PRINT(LOG_WARNING) << CS_XOR("failed to setup and/or load default configuration");
	else
		L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("configuration system initialization completed");

	L_PRINT(LOG_SUCCESS) << L::SetColor(LOG_COLOR_DEFAULT | LOG_COLOR_DEFAULT) << CS_XOR("UNDERAGER successfully loaded!");
	const double Time = TIMER_END("SETUP");
	L_PRINT(LOG_INFO) << std::format("setup took {:.3f}ms", Time).c_str();
	return true;
}

// @todo: some of those may crash while closing process, because we dont have any dependencies from the game modules, it means them can be unloaded and destruct interfaces etc before our module | modify ldrlist?
static void Destroy()
{
	// destroy chams dependent stuff
	//F::Destroy();

	// restore window messages processor to original
	IPT::Destroy();

	// restore hooks
	H::Destroy();

	I::GameEventManager->RemoveListener(Events);

	// destroy renderer backend
	D::Destroy();

	skin_changer::Shutdown();

#ifdef CS_LOG_CONSOLE
	L::DetachConsole();
#endif
#ifdef CS_LOG_FILE
	L::CloseFile();
#endif
}

bool isCS2Ready() {		
	HWND cs2Win = FindWindowA("SDL_app", "Counter-Strike 2");
	if (!cs2Win)
		return false;
	if (!MEM::GetModuleBaseHandle(NAVSYSTEM_DLL))
		return false;
	if (!MEM::GetModuleBaseHandle(MATERIAL_SYSTEM2_DLL))
		return false;
	return true;
}

DWORD WINAPI PanicThread(LPVOID lpParameter)
{
	// don't let proceed unload until user press specified key
	while (!IPT::IsKeyReleased(C_GET(unsigned int, Vars.nPanicKey)))
		::Sleep(500UL);
	Destroy();
	// call detach code and exit this thread
	::FreeLibraryAndExitThread(static_cast<HMODULE>(lpParameter), EXIT_SUCCESS);	
}
		
extern "C" BOOL WINAPI _CRT_INIT(HMODULE hModule, DWORD dwReason, LPVOID lpReserved);

BOOL APIENTRY CoreEntryPoint(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	// Disables the DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications for the specified dynamic-link library (DLL). This can reduce the size of the working set for some applications
	DisableThreadLibraryCalls(hModule);

	// process destroy of the cheat before crt calls atexit table
	if (dwReason == DLL_PROCESS_DETACH)
		Destroy();

	// dispatch reason for c-runtime, initialize/destroy static variables, TLS etc
	if (!_CRT_INIT(hModule, dwReason, lpReserved))
		return FALSE;

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		CORE::hProcess = MEM::GetModuleBaseHandle(nullptr);

		// basic process check
		if (CORE::hProcess == nullptr)
			return FALSE;

		/*
		 * check did all game modules have been loaded
		 * @note: navsystem.dll is the last loaded module
		 */
		while (!isCS2Ready()) {
			L_PRINT(LOG_INFO) << "Waiting for CS2 to be ready...";
			Sleep(100);
		}

		// save our module handle
		CORE::hDll = hModule;

		// check did we perform main initialization successfully
		if (!Setup(hModule))
		{
			// undo the things we've done
			Destroy();
			return FALSE;
		}	
		// create panic thread, it isn't critical error if it fails
		if (const HANDLE hThread = ::CreateThread(nullptr, 0U, &PanicThread, hModule, 0UL, nullptr); hThread != nullptr)
			::CloseHandle(hThread);
	}

	return TRUE;
}
