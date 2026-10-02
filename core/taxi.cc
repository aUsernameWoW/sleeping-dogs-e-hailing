#include "taxi.hh"

#include <Windows.h>

#include <atomic>
#include <cstdio>
#include <string>

#include "config.hh"
#include "log.hh"
#include "ride.hh"
#include "skookum.hh"

namespace taxi
{
	static std::wstring gOverridePath;
	static std::atomic<bool> gCallPending{ false };
	static std::atomic<bool> gHangUp{ false };
	static skookum::Run* gRun = nullptr; // the dispatch in progress (game thread only)
	static bool gReady = false;

	// Placeholders: {HANGUP} (hang up the call screen or not), {VEHICLE} and {DRIVER} (property sets).
	// Notes on the script (see CLAUDE.md for the findings behind them):
	// - The spot search is the Car Valet's: _find_vehicle_spawn_xform off screen, 40-70 m widening to 150 m.
	// - A String name is made unique by the game (a Symbol would be used as is, and the taxi of an earlier call may
	//   still be driving around under it).
	// - object-physical-character-drivers-taxi is only a parent set; TaxiDriverMale is the spawnable taxi driver.
	//   Without a driver the taxi is a locked parked car.
	// - The taxi drives to the road position nearest the player (_path_to_xform with use_road_position), re-aimed
	//   every 5 s at where the player is now, until it is within 12 m of them (or 2 minutes pass). A road position
	//   can be 25 m from a player standing off the road: the taxi waits there until they come closer.
	// - Then it's held with stop() every 0.5 s until the player sits in it: one stop() only sets the AI driver's
	//   mode to 0 (idle), and the car AI took it on to the old destination and drove off as the player walked up.
	// - "[SDTaxi:watch]" hands the taxi to core/ride.cc, which logs its AI driver's state changes.
	// - The speed is printed every 0.5 s on the way: ~5 s passed before the first taxis moved, none in a later test.
	// - While the player waits and rides it's an ordinary hired taxi; NoSuspend only keeps the driver alive until
	//   the ride is over.
	static constexpr char kDispatch[] = R"sk(
!player !spawn_xform !found !min_dist !max_dist
player: World.c_player
Debug.println("[SDTaxi] dispatch: the player is at ", player.get_pos())
if {HANGUP} [
	_wait(2.0)
	PDA.end_phone_call()
]
spawn_xform: Transform!()
found: false
min_dist: 40.0
max_dist: 70.0
loop [
	!timeout
	timeout: 0.0001
	found := false
	c_world._find_vehicle_spawn_xform(player.get_pos(), min_dist, max_dist, true, timeout, found, spawn_xform, false)
	if found [
		exit
	]
	max_dist := max_dist + 10.0
	if max_dist >= 150.0 [
		exit
	]
	_wait()
]
if found.not() [
	Debug.println("[SDTaxi] no spawn spot 40-150 m from the player")
	HintText.show_info_popup("No taxi can get here.")
]
else [
	!taxi
	Debug.println("[SDTaxi] spawning the taxi at ", spawn_xform.get_pos(), " (", max_dist, " m search radius)")
	taxi: c_world.spawn_object_at_xform(spawn_xform, '{VEHICLE}', "SDTaxi_Taxi")<>Vehicle
	if taxi.is_nil() [
		Debug.println("[SDTaxi] the taxi didn't spawn")
		HintText.show_info_popup("No taxi can get here.")
	]
	else [
		!driver !arrived !boarded !i
		driver: Character.create_at_xform(spawn_xform, '{DRIVER}', "SDTaxi_Driver")
		if driver.is_nil() [
			driver: Character.create_at_xform(spawn_xform, 'object-physical-character-AmbientMale1', "SDTaxi_Driver")
			Debug.println("[SDTaxi] no taxi driver, an ambient man instead: ", driver)
		]
		driver.enable_script_control(false)
		driver.set_suspend_option('PedSuspendOption_NoSuspend')
		driver.force_enter_vehicle(taxi, true, true)
		taxi.minimap_add_blip("friendly")
		taxi.set_scripted_driving_role("Taxi")
		Debug.println("[SDTaxi:watch]", taxi)
		Debug.println("[SDTaxi] taxi ", taxi, ", driver ", taxi.get_driver(), ": on the way")
		arrived: false
		race [
			[
				player._wait_near_actor(taxi, 12.0)
				arrived := true
				Debug.println("[SDTaxi] within 12 m of the player")
			]
			[
				loop [
					race [
						[
							taxi._path_to_xform(player.get_xform(), true)
							_wait(3.0)
						]
						[
							_wait(5.0)
						]
					]
				]
			]
			[
				_wait(120.0)
				Debug.println("[SDTaxi] not near the player after 2 minutes")
			]
			[
				loop [
					Debug.println("[SDTaxi] on the way: speed ", taxi.get_speed(), ", at ", taxi.get_pos(), ", player at ", player.get_pos())
					_wait(0.5)
				]
			]
		]
		boarded: false
		if arrived [
			taxi.stop()
			Debug.println("[SDTaxi] stopped at ", taxi.get_pos(), ", player at ", player.get_pos())
			i: 0
			loop [
				if taxi.is_valid_simobject().not() [
					Debug.println("[SDTaxi] the taxi is gone")
					exit
				]
				if player.is_the_passenger(taxi) [
					boarded := true
					exit
				]
				i := i + 1
				if i >= 240 [
					exit
				]
				taxi.stop()
				_wait(0.5)
			]
		]
		taxi.minimap_remove_blip()
		if boarded [
			Debug.println("[SDTaxi] the player got in")
			loop [
				_wait(1.0)
				if taxi.is_valid_simobject().not() [
					exit
				]
				if player.is_the_passenger(taxi).not() [
					exit
				]
			]
			Debug.println("[SDTaxi] the player got out at ", player.get_pos())
		]
		else [
			Debug.println("[SDTaxi] nobody got in; it drives off")
			taxi.wander(true, true)
		]
		driver.set_suspend_option('PedSuspendOption_SuspendAllowed')
		Debug.println("[SDTaxi] released the taxi")
	]
]
)sk";

	static void Replace(std::string& text, const std::string& what, const std::string& with)
	{
		for (size_t at = 0; (at = text.find(what, at)) != std::string::npos; at += with.size()) {
			text.replace(at, what.size(), with);
		}
	}

	// SDTaxi-dispatch.sk next to the .asi, read at every call, replaces the embedded script (development).
	static bool ReadOverride(std::string& text)
	{
		FILE* file = nullptr;
		if (_wfopen_s(&file, gOverridePath.c_str(), L"rb") != 0 || !file) {
			return false;
		}
		char buffer[4096];
		size_t n;
		while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0) {
			text.append(buffer, n);
		}
		fclose(file);
		if (text.size() >= 3 && text.compare(0, 3, "\xEF\xBB\xBF") == 0) {
			text.erase(0, 3);
		}
		return true;
	}

	static void Tick(float)
	{
		if (gRun && gRun->mFinished) {
			LOG("taxi: dispatch over, the taxi contact is back");
			gRun = nullptr;
			ride::Watch(nullptr);
		}
		if (!gCallPending.exchange(false)) {
			return;
		}
		if (gRun) {
			LOG("taxi: a taxi is already on its way, call ignored");
			return;
		}
		std::string source;
		const bool overridden = ReadOverride(source);
		if (!overridden) {
			source = kDispatch;
		}
		Replace(source, "{HANGUP}", gHangUp ? "true" : "false");
		Replace(source, "{VEHICLE}", gConfig.mTaxiVehicle);
		Replace(source, "{DRIVER}", gConfig.mTaxiDriver);
		LOG("taxi: dispatching (%s script, vehicle %s, driver %s)", overridden ? "SDTaxi-dispatch.sk" : "built-in", gConfig.mTaxiVehicle.c_str(),
			gConfig.mTaxiDriver.c_str());
		skookum::Run* run = skookum::Start("dispatch", source);
		if (!run->mFinished) {
			gRun = run;
		}
	}

	void Install(const std::wstring& dir)
	{
		if (!skookum::Ready()) {
			LOG("taxi: no scripts, no taxis");
			return;
		}
		gOverridePath = dir + L"\\SDTaxi-dispatch.sk";
		skookum::OnTick(&Tick);
		gReady = true;
	}

	bool Ready()
	{
		return gReady;
	}

	bool Available()
	{
		return gReady && !gRun && !gCallPending;
	}

	void Call(bool callShown)
	{
		gHangUp = callShown;
		gCallPending = true;
	}
}
