#pragma once

/*
**	Hardware media keys and the system media slot.
**
**	The game installs the track step handler once during startup. The platform key loop
**	routes the hardware media keys to it, and on macOS the bridge claims the system's now
**	playing slot so those keys come here instead of the last music app. Away from macOS
**	the claim is a no-op. Each direction is +1 for the next track and -1 for the previous.
*/
void Media_Keys_Init(void (*step)(int direction));
bool Media_Keys_Step(int direction);
void Media_Keys_Claim(bool active, char const* title);
bool Media_Keys_Claimed(void);
