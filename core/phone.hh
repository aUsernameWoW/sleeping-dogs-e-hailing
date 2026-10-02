#pragma once

// The phone contact: appended to the phone's contact list (UIHK_PDAPhoneContactsWidget::PopulateList) while a
// taxi can be called, and handled in LaunchSubOption: the call screen the game shows for a mission contact
// (LaunchCallMission's UI part), then taxi::Call. Nothing is added to the game's progression (a
// `PDA.add_contact` trigger could end up in the save and outlive the mod as a broken contact).

namespace phone
{
	void Install();
}
