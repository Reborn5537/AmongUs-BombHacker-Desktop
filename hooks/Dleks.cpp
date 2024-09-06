#include "pch-il2cpp.h"
#include "_hooks.h"
#include "state.hpp"

bool dConstants_1_ShouldFlipSkeld(MethodInfo* method) {
	bool orig_return = Constants_1_ShouldFlipSkeld(method); // Code for dleks ehT
	if (State.FlipSkeld) {
		return true;
	}
	else if (orig_return)
	{
		State.FlipSkeld = true;
	} //fix later
	return orig_return;
}

int32_t dConstants_1_GetBroadcastVersion(MethodInfo* method) {
	int32_t orig_return = Constants_1_GetBroadcastVersion(method);
	if (State.DisableHostAnticheat) orig_return += 25; //+25 protocol - disables anticheat | No others were found :devious:
	return orig_return;
}

bool dConstants_1_IsVersionModded(MethodInfo* method) {
	if (State.DisableHostAnticheat) return true; //this helps to bypass anticheat in our hosted lobbies
	return false; //If true + true returns = paradox of server selected category
}