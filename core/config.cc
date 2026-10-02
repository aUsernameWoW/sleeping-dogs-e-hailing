#include "config.hh"

#include <Windows.h>

#include <cstdio>
#include <cwchar>

Config gConfig;

namespace config
{
	static std::wstring gPath;

	static constexpr char kDefaultIni[] =
		"; SDTaxi 配置 / configuration\n"
		"; 1 = 开启 (on), 0 = 关闭 (off)\n"
		"\n"
		"[Phone]\n"
		"; 手机联系人里加一个叫出租车的联系人。 / A contact in the phone that calls a taxi.\n"
		"Contact = 1\n"
		"\n"
		"; 联系人的名字和说明（只能用英文等游戏字体支持的字符；\"$KEY\" 会查游戏自己的文本）。\n"
		"; The contact's name and info line (characters the game's fonts have; \"$KEY\" looks up the game's text).\n"
		"Name = Taxi\n"
		"Info = Call a cab to where you are\n"
		"\n"
		"; 联系人头像（手机联系人贴图包里的贴图名）。 / The portrait (a texture of the phone contacts' pack).\n"
		"Portrait = Portrait_Smartphone_Unknown\n"
		"\n"
		"[Taxi]\n"
		"; 派来的车和司机（游戏的属性集名）。绿色新界的士：object-physical-vehicle-625MHCTaxiGreen01\n"
		"; The vehicle and driver sent (the game's property sets). Green New Territories taxi:\n"
		"; object-physical-vehicle-625MHCTaxiGreen01\n"
		"Vehicle = object-physical-vehicle-625MHCTaxi01\n"
		"Driver = object-physical-character-TaxiDriverMale\n"
		"\n"
		"[Debug]\n"
		"; 在 .asi 旁边写 SDTaxi.log。 / Write SDTaxi.log.\n"
		"Logging = 1\n"
		"\n"
		"; 把游戏脚本的调试输出（Debug.print/println，本来是空操作）也写进日志。\n"
		"; Also log the scripts' debug output (Debug.print/println, which do nothing in this build).\n"
		"ScriptPrints = 1\n"
		"\n"
		"; 记录乘出租车的过程（选目的地、跳过路程、车辆 AI 状态）。\n"
		"; Log taxi rides (destination, skipping the ride, the vehicle's AI state).\n"
		"RideLog = 1\n"
		"\n"
		"; 开发用脚本控制台：按 ConsoleKey 执行 SDTaxi-console.sk 里的脚本，结果写进日志。\n"
		"; Development console: ConsoleKey runs the scripts in SDTaxi-console.sk; the results go to the log.\n"
		"Console = 0\n"
		"\n"
		"; 虚拟键码，0x7A = F11。 / Virtual-key code, 0x7A = F11.\n"
		"ConsoleKey = 0x7A\n";

	static std::wstring ReadString(const wchar_t* section, const wchar_t* key)
	{
		wchar_t text[256] = {};
		GetPrivateProfileStringW(section, key, L"", text, ARRAYSIZE(text), gPath.c_str());
		return text;
	}

	// A text value as UTF-8 (what Scaleform and the scripts take), or the fallback if the key is missing or empty.
	static std::string ReadText(const wchar_t* section, const wchar_t* key, const std::string& fallback)
	{
		const std::wstring text = ReadString(section, key);
		if (text.empty()) {
			return fallback;
		}
		const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string result(static_cast<size_t>(size > 1 ? size - 1 : 0), '\0');
		if (size > 1) {
			WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, result.data(), size, nullptr, nullptr);
		}
		return result;
	}

	static bool ReadBool(const wchar_t* section, const wchar_t* key, bool fallback)
	{
		return GetPrivateProfileIntW(section, key, fallback ? 1 : 0, gPath.c_str()) != 0;
	}

	static int ReadInt(const wchar_t* section, const wchar_t* key, int fallback)
	{
		// Accepts decimal and 0x-prefixed hex (GetPrivateProfileInt only does decimal).
		const std::wstring text = ReadString(section, key);
		wchar_t* end = nullptr;
		const long value = std::wcstol(text.c_str(), &end, 0);
		return end != text.c_str() ? static_cast<int>(value) : fallback;
	}

	void Load(const std::wstring& dir)
	{
		gPath = dir + L"\\SDTaxi.ini";

		if (GetFileAttributesW(gPath.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			FILE* file = nullptr;
			if (_wfopen_s(&file, gPath.c_str(), L"wb") == 0 && file)
			{
				fwrite(kDefaultIni, 1, sizeof(kDefaultIni) - 1, file);
				fclose(file);
			}
		}

		gConfig.mPhoneContact = ReadBool(L"Phone", L"Contact", gConfig.mPhoneContact);
		gConfig.mContactName = ReadText(L"Phone", L"Name", gConfig.mContactName);
		gConfig.mContactInfo = ReadText(L"Phone", L"Info", gConfig.mContactInfo);
		gConfig.mContactPortrait = ReadText(L"Phone", L"Portrait", gConfig.mContactPortrait);
		gConfig.mTaxiVehicle = ReadText(L"Taxi", L"Vehicle", gConfig.mTaxiVehicle);
		gConfig.mTaxiDriver = ReadText(L"Taxi", L"Driver", gConfig.mTaxiDriver);
		gConfig.mLogging = ReadBool(L"Debug", L"Logging", gConfig.mLogging);
		gConfig.mScriptPrints = ReadBool(L"Debug", L"ScriptPrints", gConfig.mScriptPrints);
		gConfig.mRideLog = ReadBool(L"Debug", L"RideLog", gConfig.mRideLog);
		gConfig.mConsole = ReadBool(L"Debug", L"Console", gConfig.mConsole);
		gConfig.mConsoleKey = ReadInt(L"Debug", L"ConsoleKey", gConfig.mConsoleKey);
	}
}
