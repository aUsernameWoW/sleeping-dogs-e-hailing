#pragma once

// The taxi dispatch: a SkookumScript coroutine (embedded here, or SDTaxi-dispatch.sk next to the .asi while
// developing) that hangs up the call, finds a spawn spot out of sight 40-150 m away (like the Car Valet), spawns a
// taxi with a taxi driver, drives it to where the player called from and stops. From there it is the game's own
// taxi: "HIRE TAXI", the fare, the map, the skip. The blip goes when the player gets in, and after the ride the
// driver may be suspended and despawned like any other. One taxi at a time.

#include <string>

namespace taxi
{
	void Install(const std::wstring& dir);

	// Whether Install found the scripts (the phone contact needs a taxi to send).
	bool Ready();

	// Whether a taxi can be called now (none on its way).
	bool Available();

	// The player called the taxi contact; `callShown`: the call screen is up and the script should hang up. The
	// dispatch starts on the next script tick.
	void Call(bool callShown);
}
