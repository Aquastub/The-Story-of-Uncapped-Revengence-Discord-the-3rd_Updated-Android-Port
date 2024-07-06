// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2023 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  d_event.h
/// \brief Event handling

#ifndef __D_EVENT__
#define __D_EVENT__

#include "doomtype.h"
#include "g_state.h"

// Input event types.
typedef enum
{
	ev_keydown,
	ev_keyup,
	ev_console,
	ev_mouse,
	ev_joystick,
	ev_mouse2,
	ev_joystick2,
	// STAR STUFF: DRRR unique events //
	ev_gamepad_device_added,
	ev_gamepad_device_removed,
	ev_gamepad_axis
	// ADDED! //
} evtype_t;

// Event structure.
typedef struct
{
	evtype_t type;
	INT32 key; // keys/mouse/joystick buttons
	INT32 x; // mouse/joystick x move
	INT32 y; // mouse/joystick y move
	boolean repeated; // key repeat
	INT32 device; // STAR STUFF: DRRR: which device ID it belongs to (controller ID) //
} event_t;

//
// GLOBAL VARIABLES
//
#define MAXEVENTS 128

extern event_t events[MAXEVENTS];
extern INT32 eventhead, eventtail;

#endif
