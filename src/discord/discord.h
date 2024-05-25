// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2018-2020 by Sally "TehRealSalt" Cochenour.
// Copyright (C) 2018-2024 by Kart Krew.
// Copyright (C) 2020-2024 by Star "Guy Who Names Scripts After Him" ManiaKG.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  discord.h
/// \brief Globalizes Discord Rich Presence data

#ifndef __DISCORD__
#define __DISCORD__

#ifdef HAVE_DISCORDSUPPORT

#include "discord_rpc.h"
#include "../doomdef.h"

// ------------------------ //
//        Variables
// ------------------------ //

// Please feel free to provide your own Discord app if you're making a new custom build :)
#define DISCORD_APPID "1013126566236135516"

// length of IP strings
#define IP_SIZE 21

// Allows for SRB2 to use the discriminators of Discord usernames! (even though they removed them :p)
#define DISCORD_DISCRIMINATORS

// ------------------------ //
//         Structs
// ------------------------ //

// Discord connection status
typedef enum {
	DRPC_NOTCONNECTED = 0,
	DRPC_INITIALIZED,
	DRPC_DISCONNECTED,
	DRPC_CONNECTED
} DRPC_Status_t;

//#define DISCORD_STRUCT

#ifdef DISCORD_STRUCT
typedef struct discordInfo_s {
	DRPC_Status_t ConnectionStatus;

	UINT8 maxPlayers;
	boolean joinsAllowed;
	UINT8 whoCanInvite;

	INT16 serverRoom;
} discordInfo_t;
#else
extern struct discordInfo_s {
	DRPC_Status_t ConnectionStatus;

	UINT8 maxPlayers;
	boolean joinsAllowed;
	UINT8 whoCanInvite;

	INT16 serverRoom;
} discordInfo;
#endif

typedef struct discordRequest_s {
	char *username; // Discord user name.
	char *discriminator; // Discord discriminator (The little hashtag thing after the username). Separated for a "hide discriminators" cvar.
	char *userID; // The ID of the Discord user, gets used with Discord_Respond()
	
	// HAHAHA, no.
	// *Maybe* if it was only PNG I would boot up curl just to get AND convert this to Doom GFX,
	// but it can *also* be a JEPG, WebP, or GIF :)
	// Hey, wanna add ImageMagick as a dependency? :dying:
	//patch_t *avatar;

	struct discordRequest_s *next; // Next request in the list.
	struct discordRequest_s *prev; // Previous request in the list. Not used normally, but just in case something funky happens, this should repair the list.
} discordRequest_t;

extern discordRequest_t *discordRequestList;

// ------------------------ //
//        Functions
// ------------------------ //

// ========
// STATUSES
// ========

void DRPC_ScoreStatus(char *string);
void DRPC_EmblemStatus(char *string);
void DRPC_EmeraldStatus(char *string);
void DRPC_PlaytimeStatus(char *string);

// ====
// MAIN
// ====

/*--------------------------------------------------
	const char *DRPC_ReturnUsername(const DiscordUser *user);

		Returns the Discord username of the user.
--------------------------------------------------*/

const char *DRPC_ReturnUsername(const DiscordUser *user);


/*--------------------------------------------------
	void DRPC_RemoveRequest(void);

		Removes an invite from the list.
--------------------------------------------------*/

void DRPC_RemoveRequest(discordRequest_t *removeRequest);


/*--------------------------------------------------
	void DRPC_Init(void);

		Initalizes Discord Rich Presence by linking the Application ID
		and setting the callback functions.
--------------------------------------------------*/

void DRPC_Init(void);


/*--------------------------------------------------
	void DRPC_UpdatePresence(void);

		Updates what is displayed by Rich Presence on the user's profile.
		Should be called whenever something that is displayed is
		changed in-game.
--------------------------------------------------*/

void DRPC_UpdatePresence(void);


/*--------------------------------------------------
	void DRPC_Shutdown(void)

		Clears everything related to Discord Rich Presence.
		Only runs when the game closes or crashes.
--------------------------------------------------*/

void DRPC_Shutdown(void);


#endif // HAVE_DISCORDSUPPORT
#endif // __DISCORD__
