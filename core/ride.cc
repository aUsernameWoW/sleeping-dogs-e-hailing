#include "ride.hh"

#include <Windows.h>

#include <MinHook.h>

#include <cstdint>

#include "config.hh"
#include "log.hh"
#include "mem.hh"
#include "scan.hh"
#include "skookum.hh"

namespace ride
{
	using mem::Read;

	// Layouts (installed build, IDA types):
	// - AiDriverComponent: m_pSimObject +0x28 (SimComponent), m_pRoadSpace (RebindingComponentHandle) +0x140 with
	//   m_pSimComponent +0x18 → +0x158, m_DriveToCoroutine (AIdPtr) +0x338, m_DrivingMode +0x348, m_DrivingRole +0x34C,
	//   m_bIsParked +0x5B9, m_IsAmbient +0x5BA, m_AmbientDriverType +0x5BC, m_AiScriptControlled +0x5B2.
	// - RoadSpaceComponent: mDestinationPosition +0x670 (what OnSelectDestination sets and WarpToDestination needs
	//   nonzero).
	// - SimObject: m_Name (qSymbol) +0x48, m_pTransformNodeComponent +0x58; TransformNodeComponent world position
	//   +0xB0 (as of its last update).
	using GetAiFn = void*(__fastcall*)();
	using SelectDestinationFn = void(__fastcall*)(const float* destination);
	using VoidFn = void(__fastcall*)();
	using WarpFn = bool(__fastcall*)(void* ai);
	using FromVehicleFn = bool(__fastcall*)(void* vehicle);

	static GetAiFn gGetPlayerAi = nullptr; // TransitUtility::GetPlayerVehicleAiDriver
	static SelectDestinationFn gSelectDestination = nullptr;
	static VoidFn gExitMap = nullptr;
	static WarpFn gWarp = nullptr;
	static VoidFn gPlaceTransitVehicle = nullptr;
	static FromVehicleFn gFromVehicle = nullptr;

	struct State
	{
		void* mAi = nullptr;
		uint32_t mName = 0;
		uint32_t mMode = 0;
		uint32_t mRole = 0;
		bool mScript = false;
		bool mAmbient = false;
		bool mParked = false;
		uint32_t mAmbientType = 0;
		bool mDriveTo = false;
		float mDestination[3] = {};
		float mSpeed = 0.0f;
		float mPosition[3] = {};

		bool SameAs(const State& o) const
		{
			return mAi == o.mAi && mMode == o.mMode && mRole == o.mRole && mScript == o.mScript && mAmbient == o.mAmbient &&
				mParked == o.mParked && mDriveTo == o.mDriveTo && mDestination[0] == o.mDestination[0] &&
				mDestination[1] == o.mDestination[1];
		}
	};

	static const void* gWatched = nullptr; // UFG::TSVehicle*: mAIDriverComponent handle +0xE0, its m_pSimComponent +0xF8

	static State Capture(void* ai)
	{
		State s;
		s.mAi = ai;
		if (!ai) {
			return s;
		}
		s.mName = Read<uint32_t>(Read<void*>(ai, 0x28), 0x48);
		s.mMode = Read<uint32_t>(ai, 0x348);
		s.mRole = Read<uint32_t>(ai, 0x34C);
		s.mScript = Read<uint8_t>(ai, 0x5B2) != 0;
		s.mAmbient = Read<uint8_t>(ai, 0x5BA) != 0;
		s.mParked = Read<uint8_t>(ai, 0x5B9) != 0;
		s.mAmbientType = Read<uint32_t>(ai, 0x5BC);
		s.mDriveTo = Read<void*>(ai, 0x338) != nullptr;
		const void* roadSpace = Read<void*>(ai, 0x158);
		for (int i = 0; i < 3; ++i) {
			s.mDestination[i] = Read<float>(roadSpace, 0x670 + 4 * i);
		}
		s.mSpeed = Read<float>(ai, 0x420); // m_fCurrentForwardSpeed
		const void* transform = Read<void*>(Read<void*>(ai, 0x28), 0x58);
		for (int i = 0; i < 3; ++i) {
			s.mPosition[i] = Read<float>(transform, 0xB0 + 4 * i);
		}
		return s;
	}

	static void LogState(const char* when, const State& s)
	{
		if (!s.mAi) {
			LOG("ride: %s: no AI driver (or no vehicle)", when);
			return;
		}
		LOG("ride: %s: vehicle '%s' at (%.1f, %.1f), speed %.1f, mode %u, role %u, script-controlled %d, ambient %d (type %u), parked %d, "
			"drive-to coroutine %d, destination (%.1f, %.1f, %.1f)",
			when, skookum::SymbolName(s.mName).c_str(), s.mPosition[0], s.mPosition[1], s.mSpeed, s.mMode, s.mRole, s.mScript, s.mAmbient,
			s.mAmbientType, s.mParked, s.mDriveTo, s.mDestination[0], s.mDestination[1], s.mDestination[2]);
	}

	void Watch(const void* vehicle)
	{
		if (vehicle != gWatched) {
			LOG("ride: %s", vehicle ? "watching the dispatched taxi's AI driver" : "stopped watching the taxi");
		}
		gWatched = vehicle;
	}

	static void LogNow(const char* when)
	{
		LogState(when, Capture(gGetPlayerAi()));
	}

	static void __fastcall SelectDestinationHook(const float* destination)
	{
		LOG("ride: OnSelectDestination(%.1f, %.1f)", destination ? destination[0] : 0.0f, destination ? destination[1] : 0.0f);
		gSelectDestination(destination);
		LogNow("after OnSelectDestination");
	}

	static void __fastcall ExitMapHook()
	{
		LogNow("before OnExitMap");
		gExitMap();
		LogNow("after OnExitMap");
	}

	static bool __fastcall WarpHook(void* ai)
	{
		const State before = Capture(ai);
		const bool warped = gWarp(ai);
		LOG("ride: WarpToDestination from destination (%.1f, %.1f, %.1f): %s", before.mDestination[0], before.mDestination[1],
			before.mDestination[2], warped ? "warped" : "FAILED (no destination)");
		return warped;
	}

	static void __fastcall PlaceTransitVehicleHook()
	{
		LOG("ride: PlaceTransitVehicle");
		gPlaceTransitVehicle();
	}

	static bool __fastcall FromVehicleHook(void* vehicle)
	{
		const bool transit = gFromVehicle(vehicle);
		LOG("ride: SetWorldMapFromVehicle('%s'): %s", skookum::SymbolName(Read<uint32_t>(vehicle, 0x48)).c_str(),
			transit ? "transit view" : "normal map");
		return transit;
	}

	// Twice a second: log the player's vehicle's and the watched taxi's AI state when it changed.
	static void Tick(float delta)
	{
		static float sinceCheck = 0.0f;
		static State lastPlayer;
		static State lastTaxi;
		sinceCheck += delta;
		if (sinceCheck < 0.5f) {
			return;
		}
		sinceCheck = 0.0f;
		const State player = Capture(gGetPlayerAi());
		if (!player.SameAs(lastPlayer)) {
			LogState(player.mAi && !lastPlayer.mAi ? "player got in" : "player's vehicle", player);
		}
		lastPlayer = player;

		const State taxi = Capture(gWatched ? Read<void*>(gWatched, 0xF8) : nullptr);
		if (gWatched && !taxi.SameAs(lastTaxi)) {
			LogState("taxi", taxi);
		}
		lastTaxi = taxi;
	}

	void Install()
	{
		if (!gConfig.mRideLog) {
			return;
		}
		// Literal patterns straight into FindUnique, so tools\pdb.ps1 verify checks them.
		uint8_t* select = scan::FindUnique("TransitUtility::OnSelectDestination", "40 55 53 41 55 48 8D 6C 24 C0 48 81 EC 40 01 00 00");
		uint8_t* exitMap = scan::FindUnique("TransitUtility::OnExitMap",
			"40 53 48 83 EC 20 E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? 48 8B 48 28");
		uint8_t* warp = scan::FindUnique("AiDriverComponent::WarpToDestination", "40 55 53 41 56 48 8D 6C 24 B9 48 81 EC 90 00 00 00 33 DB");
		uint8_t* place = scan::FindUnique("TransitUtility::PlaceTransitVehicle",
			"48 8B C4 57 48 81 EC 90 00 00 00 48 C7 40 A8 FE FF FF FF 48 89 58 10");
		uint8_t* fromVehicle = scan::FindUnique("UIHKScreenWorldMap::SetWorldMapFromVehicle",
			"40 57 48 83 EC 20 48 8D 15 ? ? ? ? 48 8B F9 E8 ? ? ? ? 84 C0 74 19");
		if (!select || !exitMap || !warp || !place || !fromVehicle || !skookum::Ready()) {
			LOG("ride: transit functions MISSING (or no script tick), no ride log");
			return;
		}
		// OnExitMap starts with `call TransitUtility::GetPlayerVehicleAiDriver`.
		gGetPlayerAi = reinterpret_cast<GetAiFn>(scan::RipTarget(exitMap + 7));

		if (MH_CreateHook(select, reinterpret_cast<void*>(&SelectDestinationHook), reinterpret_cast<void**>(&gSelectDestination)) != MH_OK ||
			MH_EnableHook(select) != MH_OK ||
			MH_CreateHook(exitMap, reinterpret_cast<void*>(&ExitMapHook), reinterpret_cast<void**>(&gExitMap)) != MH_OK ||
			MH_EnableHook(exitMap) != MH_OK ||
			MH_CreateHook(warp, reinterpret_cast<void*>(&WarpHook), reinterpret_cast<void**>(&gWarp)) != MH_OK ||
			MH_EnableHook(warp) != MH_OK ||
			MH_CreateHook(place, reinterpret_cast<void*>(&PlaceTransitVehicleHook), reinterpret_cast<void**>(&gPlaceTransitVehicle)) != MH_OK ||
			MH_EnableHook(place) != MH_OK ||
			MH_CreateHook(fromVehicle, reinterpret_cast<void*>(&FromVehicleHook), reinterpret_cast<void**>(&gFromVehicle)) != MH_OK ||
			MH_EnableHook(fromVehicle) != MH_OK) {
			LOG("ride: hooking the transit functions failed, no ride log");
			return;
		}
		skookum::OnTick(&Tick);
		skookum::OnPrintTag("[SDTaxi:watch]", &Watch);
		LOG("ride: logging taxi rides (transit calls, the player's vehicle AI state)");
	}
}
