#include <Windows.h>

#include <MinHook.h>

#include <string>

#include "core/config.hh"
#include "core/console.hh"
#include "core/crash.hh"
#include "core/log.hh"
#include "core/phone.hh"
#include "core/ride.hh"
#include "core/skookum.hh"
#include "core/taxi.hh"

static std::wstring GetModuleDirectory(HMODULE module)
{
	wchar_t path[MAX_PATH] = {};
	const DWORD length = GetModuleFileNameW(module, path, ARRAYSIZE(path));
	if (length == 0 || length >= ARRAYSIZE(path)) {
		return L".";
	}

	std::wstring dir(path, length);
	const size_t slash = dir.find_last_of(L"\\/");
	return slash == std::wstring::npos ? L"." : dir.substr(0, slash);
}

BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);

		// Pin ourselves: the hooks point into this module.
		HMODULE pinned;
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, reinterpret_cast<LPCWSTR>(&DllMain), &pinned);

		const std::wstring dir = GetModuleDirectory(module);
		config::Load(dir);

		if (gConfig.mLogging) {
			logger::Open(dir + L"\\SDTaxi.log");
			crash::Install(dir);
		}

		LOG("SDTaxi loaded (Contact=%d Name=\"%s\" Vehicle=%s Driver=%s ScriptPrints=%d RideLog=%d Console=%d ConsoleKey=0x%02X)", gConfig.mPhoneContact,
			gConfig.mContactName.c_str(), gConfig.mTaxiVehicle.c_str(), gConfig.mTaxiDriver.c_str(), gConfig.mScriptPrints, gConfig.mRideLog,
			gConfig.mConsole, gConfig.mConsoleKey);

		const MH_STATUS status = MH_Initialize();
		if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
			LOG("MinHook failed to initialize (%d), nothing hooked", status);
			return TRUE;
		}
		skookum::Install(gConfig.mScriptPrints);
		taxi::Install(dir);
		phone::Install();
		console::Install(dir);
		ride::Install();
	}

	return TRUE;
}
