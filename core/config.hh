#pragma once

#include <string>

struct Config
{
	// The taxi contact in the phone (core/phone.cc). Name and info are shown as written (the game's fonts have
	// no Chinese); a "$KEY" is looked up in the game's text. The portrait is a texture of the contact list's pack.
	bool mPhoneContact = true;
	std::string mContactName = "Taxi";
	std::string mContactInfo = "Call a cab to where you are";
	std::string mContactPortrait = "Portrait_Smartphone_Unknown";

	// What is sent (core/taxi.cc): property sets of the vehicle and its driver.
	std::string mTaxiVehicle = "object-physical-vehicle-625MHCTaxi01";
	std::string mTaxiDriver = "object-physical-character-TaxiDriverMale";

	bool mLogging = true;

	// Log the scripts' Debug.print/println (ours and the game's; empty in this build).
	bool mScriptPrints = true;

	// Log taxi rides: the world map's transit calls and the player's vehicle AI state (core/ride.cc).
	bool mRideLog = true;

	// Development console (core/console.cc): ConsoleKey runs SDTaxi-console.sk.
	bool mConsole = false;
	int mConsoleKey = 0x7A; // VK_F11
};

extern Config gConfig;

namespace config
{
	// Reads SDTaxi.ini from `dir`, writing a commented default first if there is none.
	void Load(const std::wstring& dir);
}
