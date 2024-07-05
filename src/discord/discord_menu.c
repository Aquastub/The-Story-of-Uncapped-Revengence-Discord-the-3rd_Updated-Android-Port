// SONIC ROBO BLAST 2; TSOURDT3RD
//-----------------------------------------------------------------------------
// Copyright (C) 2018-2020 by Sally "TehRealSalt" Cochenour.
// Copyright (C) 2018-2024 by Kart Krew.
// Copyright (C) 2020-2024 by Star "Guy Who Names Scripts After Him" ManiaKG.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  discord_menu.c
/// \brief Discord Rich Presence menu options

#include "discord_menu.h"
#include "discord_cmds.h"
#include "discord.h"
#include "../r_draw.h"
#include "../z_zone.h"
#include "../w_wad.h" // W_CachePatchName //

#include "../STAR/m_menu.h"
#include "../STAR/drrr/kk_hud.h"
#include "../STAR/drrr/km_menu.h"

#ifdef HAVE_DISCORDSUPPORT

// ------------------------ //
//        Variables
// ------------------------ //

struct discordrequestmenu_s discordrequestmenu;

// =====
// MENUS
// =====

static void M_DrawDiscordMenu(void);

static void M_DiscordRequestHandler(INT32 choice);
static void M_DrawDiscordRequests(void);

static void DISC_M_Header(INT32 choice);
static void DISC_M_State(INT32 choice);
static void DISC_M_LImageText(INT32 choice);
static void DISC_M_SImageText(INT32 choice);

static menuitem_t OP_DiscordOptionsMenu[] =
{
	{IT_HEADER,								NULL, 	"Discord Rich Presence...",	NULL,					 	  	        0},
	{IT_STRING | IT_CVAR,					NULL, 	"Rich Presence",			&cv_discordrp,			 	  		    6},
	{IT_STRING | IT_CVAR,					NULL, 	"Streamer Mode",			&cv_discordstreamer,	 	 		   11},

	{IT_HEADER,								NULL,	"Rich Presence Settings",	NULL,					 	 		   20},
	{IT_STRING | IT_CVAR,					NULL, 	"Allow Ask to Join",		&cv_discordasks,		 	 		   26},
	{IT_STRING | IT_CVAR,					NULL,   "Allow Invites",			&cv_discordinvites,		 	 		   31},

	{IT_STRING | IT_CVAR,					NULL, 	"Show on Status",			&cv_discordshowonstatus, 	 	       41},
	{IT_STRING | IT_CVAR,					NULL, 	"Memes on Status",			&cv_discordstatusmemes,	 	 		   46},

	{IT_STRING | IT_CVAR,					NULL, 	"Skin Image Type",			&cv_discordcharacterimagetype,		   56},

	{IT_HEADER,								NULL,   "Custom Status Settings",	NULL,					 	 		   65},
	{IT_STRING | IT_KEYHANDLER,				NULL, 	"Edit Details...",			DISC_M_Header, 	 		   			   71},
	{IT_STRING | IT_KEYHANDLER,				NULL, 	"Edit State...",			DISC_M_State, 	 		   			   76},

	{IT_STRING | IT_CVAR,		        	NULL, 	"Large Image Type",			&cv_discordcustom_imagetype_large,     86},
	{IT_STRING | IT_CVAR,		        	NULL, 	"Small Image Type",			&cv_discordcustom_imagetype_small,     91},

	{IT_STRING | IT_CVAR,		        	NULL, 	"Large Image",				NULL, 								  101}, // Handled by discord_option_onchange //
	{IT_STRING | IT_CVAR,		        	NULL, 	"Small Image",				NULL, 								  106}, // Handled by discord_option_onchange //

	{IT_STRING | IT_KEYHANDLER,				NULL, 	"Edit Large Image Text...", DISC_M_LImageText,      			  116},
	{IT_STRING | IT_KEYHANDLER,				NULL, 	"Edit Small Image Text...",	DISC_M_SImageText,      			  121},
};

static menuitem_t MISC_DiscordRequests[] =
{
	{IT_KEYHANDLER|IT_NOTHING, NULL, "", M_DiscordRequestHandler, 0},
};

menu_t OP_DiscordOptionsDef =
{
	MTREE2(MN_OP_MAIN, MN_OP_DISCORD_OPT),
	"M_DISC",
	sizeof (OP_DiscordOptionsMenu)/sizeof (menuitem_t),
	&OP_MainDef,
	OP_DiscordOptionsMenu,
	M_DrawDiscordMenu,
	30, 30,
	0,
	NULL
};

menu_t MISC_DiscordRequestsDef = {
	MN_OP_DISCORD_RQ,
	NULL,
	sizeof (MISC_DiscordRequests)/sizeof (menuitem_t),
	&MPauseDef,
	MISC_DiscordRequests,
	M_DrawDiscordRequests,
	0, 0,
	0,
	NULL
};

enum
{
	op_richpresence_settings = 3,
	op_discordasks,
	op_discordinvites,

	op_discordshowonstatus,
	op_discordstatusmemes,

	op_discordcharacterimagetype,

	op_customstatus_settings,
	op_discordcustom_details,
	op_discordcustom_state,

	op_discordcustom_imagetype_large,
	op_discordcustom_imagetype_small,

	op_discordcustom_image_large,
	op_discordcustom_image_small,

	op_discordcustom_imagetext_large,
	op_discordcustom_imagetext_small,
};

// ------------------------ //
//        Functions
// ------------------------ //

// ====
// MAIN
// ====

void M_DiscordOptionsTicker(void)
{
	// Register options and menu data //
	INT32 i, j;
	INT32 menuflags;

	INT32 custom_cvartype_index[] = { cv_discordcustom_imagetype_large.value, cv_discordcustom_imagetype_small.value };
	consvar_t *custom_cvar_index[2][4] = {
		[0] = {
			&cv_discordcustom_characterimage_large,
			&cv_discordcustom_supercharacterimage_large,
			&cv_discordcustom_mapimage_large,
			&cv_discordcustom_miscimage_large,
		},
		[1] = {
			&cv_discordcustom_characterimage_small,
			&cv_discordcustom_supercharacterimage_small,
			&cv_discordcustom_mapimage_small,
			&cv_discordcustom_miscimage_small,
		},
	};

	INT32 discord_menuitems[6][5] = {
		[0] = { op_richpresence_settings },
		[1] = {
			op_discordasks,
			op_discordinvites,
			op_discordshowonstatus,
			op_discordstatusmemes,
			op_discordcharacterimagetype,
		},

		[2] = { op_customstatus_settings },
		[3] = {
			op_discordcustom_imagetype_large,
			op_discordcustom_imagetype_small,
		},
		[4] = {
			op_discordcustom_image_large,
			op_discordcustom_image_small,
		},
		[5] = {
			op_discordcustom_details,
			op_discordcustom_state,
			op_discordcustom_imagetext_large,
			op_discordcustom_imagetext_small,
		},
	};
	consvar_t *discord_itemactions = NULL;

	// Set option availability and actions //
	for (i = 0, j = 0; i < 6; j++)
	{
		switch (i)
		{
			case 1:
				menuflags = (cv_discordrp.value ? IT_CVAR|IT_STRING : IT_DISABLED);
				break;

#ifdef DISCORD_SECRETIVE
			case 2:
			case 3:
			case 4:
			case 5:
				menuflags = IT_DISABLED;
				discord_itemactions = NULL;
				break;
#else
			case 2:
				menuflags = ((cv_discordrp.value && cv_discordshowonstatus.value == 9) ? IT_HEADER : IT_DISABLED);
				break;

			case 3:
				menuflags = ((cv_discordrp.value && cv_discordshowonstatus.value == 9) ? IT_CVAR|IT_STRING : IT_DISABLED);
				break;

			case 4:
			{
				menuflags = ((cv_discordrp.value && cv_discordshowonstatus.value == 9) ? IT_CVAR|IT_STRING : IT_DISABLED);

				switch (custom_cvartype_index[j])
				{
					case 0:
					case 1:
					case 2:
						discord_itemactions = custom_cvar_index[j][0];
						break;

					case 3:
					case 4:
					case 5:
						discord_itemactions = custom_cvar_index[j][1];
						break;

					case 6:
						discord_itemactions = custom_cvar_index[j][2];
						break;

					case 7:
						discord_itemactions = custom_cvar_index[j][3];
						break;

					default:
						menuflags = IT_DISABLED;
						discord_itemactions = NULL;
						break;
				}
				break;
			}

			case 5:
				menuflags = ((cv_discordrp.value && cv_discordshowonstatus.value == 9) ? IT_KEYHANDLER|IT_STRING : IT_DISABLED);
				break;
#endif

			default:
				menuflags = (cv_discordrp.value ? IT_HEADER : IT_DISABLED);
				break;
		}

		OP_DiscordOptionsMenu[discord_menuitems[i][j]].status = menuflags;
		if (discord_itemactions)
		{
			OP_DiscordOptionsMenu[discord_menuitems[i][j]].itemaction = discord_itemactions;
			discord_itemactions = NULL;
		}

		if (!discord_menuitems[i][j+1])
		{
			j = -1;
			i++;
		}
	}
}

void M_DiscordOptions(INT32 choice)
{
	(void)choice;

	M_DiscordOptionsTicker();
	M_SetupNextMenu(&OP_DiscordOptionsDef);
}

static void M_DrawDiscordMenu(void)
{
	M_DrawGenericScrollMenu();

	if (discordInfo.ConnectionStatus == DRPC_CONNECTED)
	{
		V_DrawCenteredThinString(BASEVIDWIDTH/2, BASEVIDHEIGHT-8, V_SNAPTOBOTTOM|V_ALLOWLOWERCASE|V_MENUCOLORMAP, va("Connected to: %s", DRPC_ReturnUsername()));
		return;
	}

	// Dang! Discord isn't open!
	if (discordInfo.ConnectionStatus == DRPC_DISCONNECTED)
		V_DrawCenteredThinString(BASEVIDWIDTH/2, BASEVIDHEIGHT-16, V_SNAPTOBOTTOM|V_REDMAP, "Disconnected");
	else
		V_DrawCenteredThinString(BASEVIDWIDTH/2, BASEVIDHEIGHT-16, V_SNAPTOBOTTOM|V_REDMAP, "Not Connected");
	V_DrawCenteredThinString(BASEVIDWIDTH/2, BASEVIDHEIGHT-8, V_SNAPTOBOTTOM|V_ALLOWLOWERCASE|V_REDMAP, "Make sure Discord is open!");
}

static boolean DISC_TypingCloseRoutine(void)
{
	if (strlen(menutyping.cache) < 2)
		return true;
	return false;
}

static void DISC_TypingAbortRoutine(void)
{
	if (strlen(menutyping.cache) >= 2)
		return;

	STAR_M_StartMessage(
		"String too short",
		0,
		"Sorry, Discord requires status strings to\nbe longer than two characters.\n\nPlease type a longer string.\n\n(Press a key)\n",
		NULL,
		MM_NOTHING
	);
	S_StartSound(NULL, sfx_skid);
}

static void DISC_M_Header(INT32 choice)
{
	TSoURDt3rd_M_HandleTyping(
		choice,
		128,
		&cv_discordcustom_details,
		DISC_TypingCloseRoutine,
		DISC_TypingAbortRoutine
	);
}

static void DISC_M_State(INT32 choice)
{
	TSoURDt3rd_M_HandleTyping(
		choice,
		128,
		&cv_discordcustom_state,
		DISC_TypingCloseRoutine,
		DISC_TypingAbortRoutine
	);
}

static void DISC_M_LImageText(INT32 choice)
{
	TSoURDt3rd_M_HandleTyping(
		choice,
		128,
		&cv_discordcustom_imagetext_large,
		DISC_TypingCloseRoutine,
		DISC_TypingAbortRoutine
	);
}

static void DISC_M_SImageText(INT32 choice)
{
	TSoURDt3rd_M_HandleTyping(
		choice,
		128,
		&cv_discordcustom_imagetext_small,
		DISC_TypingCloseRoutine,
		DISC_TypingAbortRoutine
	);
}

// ========
// REQUESTS
// ========

static const char *M_GetDiscordName(discordRequest_t *r)
{
	if (r == NULL)
		return "";
	return DRPC_ReturnUsername();
}

void M_DiscordRequests(INT32 choice)
{
	(void)choice;
	static const tic_t confirmLength = 3*TICRATE/4;

	discordrequestmenu.confirmLength = confirmLength;
	MISC_DiscordRequestsDef.prevMenu = currentMenu;
	M_SetupNextMenu(&MISC_DiscordRequestsDef);
}

void M_DiscordRequestTick(void)
{
	discordrequestmenu.ticker++;

	if (discordrequestmenu.confirmDelay > 0)
	{
		discordrequestmenu.confirmDelay--;

		if (discordrequestmenu.confirmDelay == 0)
		{
			discordrequestmenu.removeRequest = true;
		}
	}

	if (discordrequestmenu.removeRequest == true)
	{
		DRPC_RemoveRequest(discordRequestList);

		if (discordRequestList == NULL)
		{
			// No other requests
			MPauseMenu[4].status = IT_DISABLED; // mpause_discordrequests

			if (currentMenu->prevMenu)
			{
				M_SetupNextMenu(currentMenu->prevMenu);
				itemOn = 5; // mpause_continue
			}
			else
				M_ClearMenus(true);
		}

		discordrequestmenu.removeRequest = false;
	}
}

static void M_DiscordRequestHandler(INT32 choice)
{
	if (discordrequestmenu.confirmDelay > 0)
		return;

	switch (choice)
	{
		case KEY_ENTER:
			Discord_Respond(discordRequestList->userID, DISCORD_REPLY_YES);
			discordrequestmenu.confirmAccept = true;
			discordrequestmenu.confirmDelay = discordrequestmenu.confirmLength;
			S_StartSound(NULL, sfx_s3k63);
			break;

		case KEY_ESCAPE:
			Discord_Respond(discordRequestList->userID, DISCORD_REPLY_NO);
			discordrequestmenu.confirmAccept = false;
			discordrequestmenu.confirmDelay = discordrequestmenu.confirmLength;
			S_StartSound(NULL, sfx_s3kb2);
			break;
	}
}

static void M_DrawDiscordRequests(void)
{
	discordRequest_t *curRequest = discordRequestList;
	UINT8 *colormap;
	patch_t *hand = NULL;

	const char *wantText = "...would like to join!";
	const char *acceptText = "Accept" ;
	const char *declineText = "Decline";

	INT32 x = 100;
	INT32 y = 133;

	INT32 slide = 0;
	INT32 maxYSlide = 18;

	if (discordrequestmenu.confirmDelay > 0)
	{
		if (discordrequestmenu.confirmAccept == true)
		{
			colormap = R_GetTranslationColormap(TC_DEFAULT, SKINCOLOR_GREEN, GTC_CACHE);
			hand = W_CachePatchName("K_LAPH02", PU_CACHE);
		}
		else
		{
			colormap = R_GetTranslationColormap(TC_DEFAULT, SKINCOLOR_RED, GTC_CACHE);
			hand = W_CachePatchName("K_LAPH03", PU_CACHE);
		}

		slide = discordrequestmenu.confirmLength - discordrequestmenu.confirmDelay;
	}
	else
	{
		colormap = R_GetTranslationColormap(TC_DEFAULT, SKINCOLOR_GREY, GTC_CACHE);
	}

	V_DrawFixedPatch(56*FRACUNIT, 150*FRACUNIT, FRACUNIT, 0, W_CachePatchName("K_LAPE01", PU_CACHE), colormap);

	if (hand != NULL)
	{
		fixed_t handoffset = (4 - abs((signed)(skullAnimCounter - 4))) * FRACUNIT;
		V_DrawFixedPatch(56*FRACUNIT, 150*FRACUNIT + handoffset, FRACUNIT, 0, hand, NULL);
	}

	K_DrawSticker(x + (slide * 32), y - 2, V_ThinStringWidth(M_GetDiscordName(curRequest), 0), 0, false);
	V_DrawThinString(x + (slide * 32), y - 1, V_YELLOWMAP, M_GetDiscordName(curRequest));

	K_DrawSticker(x, y + 12, V_ThinStringWidth(wantText, 0), 0, true);
	V_DrawThinString(x, y + 10, 0, wantText);

	INT32 confirmButtonWidth = SHORT(kp_button_a[1][0]->width);
	INT32 declineButtonWidth = SHORT(kp_button_b[1][0]->width);
	INT32 altDeclineButtonWidth = SHORT(kp_button_x[1][0]->width);
	INT32 acceptTextWidth =  V_ThinStringWidth(acceptText, 0);
	INT32 declineTextWidth = V_ThinStringWidth(declineText, 0);
	INT32 stickerWidth = (confirmButtonWidth + declineButtonWidth + altDeclineButtonWidth + acceptTextWidth + declineTextWidth);

	K_DrawSticker(x, y + 26, stickerWidth, 0, true);
	K_drawButtonAnim(x, y + 22, V_SNAPTORIGHT, kp_button_a[1], discordrequestmenu.ticker);

	INT32 xoffs = confirmButtonWidth;

	V_DrawThinString((x + xoffs), y + 24, 0, acceptText);
	xoffs += acceptTextWidth;

	K_drawButtonAnim((x + xoffs), y + 22, V_SNAPTORIGHT, kp_button_b[1], discordrequestmenu.ticker);
	xoffs += declineButtonWidth;

	K_drawButtonAnim((x + xoffs), y + 22, V_SNAPTORIGHT, kp_button_x[1], discordrequestmenu.ticker);
	xoffs += altDeclineButtonWidth;

	V_DrawThinString((x + xoffs), y + 24, 0, declineText);

	y -= 18;

	while (curRequest->next != NULL)
	{
		INT32 ySlide = min(slide * 4, maxYSlide);

		curRequest = curRequest->next;

		const char *discordname = M_GetDiscordName(curRequest);

		K_DrawSticker(x, y - 1 + ySlide, V_ThinStringWidth(discordname, 0), 0, false);
		V_DrawThinString(x, y + ySlide, 0, discordname);

		y -= 12;
		maxYSlide = 12;
	}
}

#endif // HAVE_DISCORDSUPPORT
