#include "phone.hh"

#include <Windows.h>

#include <MinHook.h>

#include <cstdint>
#include <cstring>

#include "config.hh"
#include "hash.hh"
#include "log.hh"
#include "mem.hh"
#include "scan.hh"
#include "taxi.hh"

namespace phone
{
	using mem::Read;

	// Layouts (installed build, IDA types; checked against LaunchCallMission's instructions in Install):
	// - UIHK_PDAPhoneContactsWidget: mState +0x8 (6 = STATE_SHOULD_EXIT, what LaunchCallMission sets), mSelectedIndex
	//   +0x10, mSymbolList (qArray<qSymbol>: size +0x0, p +0x8) +0x100.
	// - UIHKScreenHud::mInstance → PDA (UIHK_PDAWidget*) +0x200 → mPhoneContact (qString) +0x2D0, IncomingCall widget
	//   +0x1C0, mOutgoingCall +0x2F8, mVoiceMail +0x2FA.
	// qSymbol arguments are passed by pointer (the MSVC x64 ABI for a class type with a destructor).
	using PopulateListFn = int(__fastcall*)(void* widget, void* screen);
	using AddContactFn = void(__fastcall*)(void* widget, void* screen, const uint32_t* symbol, const char* name, const char* portrait,
		const char* info);
	using LaunchSubOptionFn = void(__fastcall*)(uint8_t* widget, void* screen);
	using StringSetFn = void(__fastcall*)(void* string, const char* text);
	using SetCallerNameFn = void(__fastcall*)(void* incomingCall, const char* name, const char* portrait, bool voiceMail);
	using AnswerPhoneCallFn = bool(__fastcall*)(void* pda);

	static PopulateListFn gPopulateList = nullptr;
	static AddContactFn gAddContact = nullptr;
	static LaunchSubOptionFn gLaunchSubOption = nullptr;
	static StringSetFn gStringSet = nullptr;          // UFG::qString::Set
	static SetCallerNameFn gSetCallerName = nullptr;  // UIHK_PDAIncomingCallWidget::SetCallerName
	static AnswerPhoneCallFn gAnswerPhoneCall = nullptr;
	static uint8_t** gHud = nullptr;                  // UIHKScreenHud::mInstance
	static void* gContactImage = nullptr;             // UIHK_PDAWidget::mContactImage (qString)

	static constexpr uint32_t kContact = hash::String32("SDTaxi");

	static int __fastcall PopulateListHook(void* widget, void* screen)
	{
		int count = gPopulateList(widget, screen);
		if (taxi::Available()) {
			gAddContact(widget, screen, &kContact, gConfig.mContactName.c_str(), gConfig.mContactPortrait.c_str(), gConfig.mContactInfo.c_str());
			++count;
		}
		else {
			LOG("phone: contact list opened while a taxi is on its way: no taxi contact");
		}
		return count;
	}

	// LaunchCallMission after its trigger lookup: the outgoing call, already answered, and the contact list closes.
	// The dispatch script hangs up (PDA.end_phone_call), as the valet's gameslice does.
	static bool ShowCall(uint8_t* widget)
	{
		uint8_t* hud = *gHud;
		uint8_t* pda = hud ? Read<uint8_t*>(hud, 0x200) : nullptr;
		if (!pda) {
			return false;
		}
		const char* name = gConfig.mContactName.c_str();
		const char* portrait = gConfig.mContactPortrait.c_str();
		gStringSet(pda + 0x2D0, name);
		gStringSet(gContactImage, portrait);
		pda[0x2F8] = 1;
		pda[0x2FA] = 0;
		gSetCallerName(pda + 0x1C0, name, portrait, false);
		gAnswerPhoneCall(pda);
		const uint32_t shouldExit = 6;
		std::memcpy(widget + 0x8, &shouldExit, sizeof(shouldExit));
		return true;
	}

	static void __fastcall LaunchSubOptionHook(uint8_t* widget, void* screen)
	{
		const uint32_t index = Read<uint32_t>(widget, 0x10);
		const uint32_t count = Read<uint32_t>(widget, 0x100);
		const uint8_t* symbols = Read<const uint8_t*>(widget, 0x108);
		if (!screen || index >= count || Read<uint32_t>(symbols, index * sizeof(uint32_t)) != kContact) {
			gLaunchSubOption(widget, screen);
			return;
		}
		const bool shown = ShowCall(widget);
		LOG("phone: the player called the taxi contact%s", shown ? "" : " (no HUD, no call screen)");
		taxi::Call(shown);
	}

	void Install()
	{
		if (!gConfig.mPhoneContact) {
			LOG("phone: contact off");
			return;
		}
		// Literal patterns straight into FindUnique, so tools\pdb.ps1 verify checks them.
		uint8_t* populate = scan::FindUnique("UIHK_PDAPhoneContactsWidget::PopulateList",
			"40 53 55 56 57 41 56 48 83 EC 30 48 C7 44 24 20 FE FF FF FF 4C 8B F2 48 8B D9 33 F6");
		gAddContact = reinterpret_cast<AddContactFn>(scan::FindUnique("UIHK_PDAPhoneContactsWidget::AddContact",
			"48 85 D2 0F 84 ? ? ? ? 48 8B C4 55 41 54 41 56 48 8D 68 B8"));
		uint8_t* launch = scan::FindUnique("UIHK_PDAPhoneContactsWidget::LaunchSubOption",
			"48 85 D2 0F 84 ? ? ? ? 48 83 EC 48 48 C7 44 24 30 FE FF FF FF");
		uint8_t* call = scan::FindUnique("UIHK_PDAPhoneContactsWidget::LaunchCallMission",
			"48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 30 49 8B E9");
		// LaunchCallMission's call screen: mov rbx, UIHKScreenHud::mInstance; mov rbx, [rbx+200h] (PDA);
		// lea rcx, [rbx+2D0h]; call qString::Set; lea rcx, mContactImage; lea rcx, [rbx+1C0h];
		// mov byte [rbx+2F8h], 1; mov byte [rbx+2FAh], 0; call SetCallerName; call AnswerPhoneCall;
		// mov dword [r15+8], 6.
		const bool ok = populate && gAddContact && launch && call && taxi::Ready() && scan::Matches(call + 0x93, "48 8B 1D") &&
			scan::Matches(call + 0x9F, "48 8B 9B 00 02 00 00") && scan::Matches(call + 0xAE, "48 8D 8B D0 02 00 00") &&
			scan::Matches(call + 0xB8, "E8") && scan::Matches(call + 0xBD, "48 8D 0D") && scan::Matches(call + 0xCC, "48 8D 8B C0 01 00 00") &&
			scan::Matches(call + 0xDC, "C6 83 F8 02 00 00 01 C6 83 FA 02 00 00 00") && scan::Matches(call + 0xEA, "E8") &&
			scan::Matches(call + 0xF6, "48 8B 89 00 02 00 00 E8") && scan::Matches(call + 0x102, "41 C7 47 08 06 00 00 00");
		if (!ok) {
			LOG("phone: contact list functions MISSING or not as expected (or no taxi), no taxi contact");
			return;
		}
		gHud = static_cast<uint8_t**>(scan::RipTarget(call + 0x96));
		gStringSet = reinterpret_cast<StringSetFn>(scan::RipTarget(call + 0xB9));
		gContactImage = scan::RipTarget(call + 0xC0);
		gSetCallerName = reinterpret_cast<SetCallerNameFn>(scan::RipTarget(call + 0xEB));
		gAnswerPhoneCall = reinterpret_cast<AnswerPhoneCallFn>(scan::RipTarget(call + 0xFE));

		if (MH_CreateHook(populate, reinterpret_cast<void*>(&PopulateListHook), reinterpret_cast<void**>(&gPopulateList)) != MH_OK ||
			MH_EnableHook(populate) != MH_OK ||
			MH_CreateHook(launch, reinterpret_cast<void*>(&LaunchSubOptionHook), reinterpret_cast<void**>(&gLaunchSubOption)) != MH_OK ||
			MH_EnableHook(launch) != MH_OK) {
			LOG("phone: hooking the contact list failed, no taxi contact");
			return;
		}
		LOG("phone: taxi contact \"%s\" in the phone's contacts", gConfig.mContactName.c_str());
	}
}
