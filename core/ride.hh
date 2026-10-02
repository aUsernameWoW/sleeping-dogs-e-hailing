#pragma once

// Taxi ride diagnostics (log only): the transit functions the world map calls when the player picks a destination
// in a taxi, and the AI driver state of the player's vehicle whenever it changes. Built to find out why a
// script-spawned taxi drives the whole way where an ambient one skips it (TransitUtility::OnExitMap →
// AiDriverComponent::WarpToDestination).

namespace ride
{
	void Install();

	// Also log this vehicle's AI driver state when it changes (a script Vehicle, i.e. a UFG::TSVehicle; null stops).
	// The dispatch script hands over its taxi with `Debug.println("[SDTaxi:watch]", taxi)`; taxi.cc stops watching
	// when the dispatch is over (the instance may go away then).
	void Watch(const void* vehicle);
}
