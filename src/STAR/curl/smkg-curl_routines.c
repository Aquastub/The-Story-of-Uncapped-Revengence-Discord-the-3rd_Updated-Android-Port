// SONIC ROBO BLAST 2; TSOURDT3RD
//-----------------------------------------------------------------------------
// Copyright (C) 2023-2024 by Star "Guy Who Names Scripts After Him" ManiaKG.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  smkg-curl_routines.c
/// \brief Easy curl routines for TSoURDt3rd

#ifdef HAVE_CURL
#include <curl/curl.h>

#include "smkg-curl.h"
#include "../smkg-cvars.h"
#include "../star_vars.h"
#include "../smkg-misc.h"
#include "../menus/smkg-m_sys.h" // menumessage //

#include "../../d_main.h"
#include "../../g_game.h"
#include "../../i_system.h"
#include "../../i_video.h"
#include "../../v_video.h"
#include "../../z_zone.h"

// ------------------------ //
//        Functions
// ------------------------ //

//
// void TSoURDt3rd_Curl_FindUpdateRoutine(void)
// Finds the current version of TSoURDt3rd from the Github Repository.
//
void TSoURDt3rd_Curl_FindUpdateRoutine(void)
{
	char *return_info = NULL;
	char *return_version = NULL;
	INT32 return_code = 0;

	char version_url[256];
	UINT32 version_number = 0;

	const char *header_string = NULL;
	char message_string[256];
	INT32 message_type = 0;

	if (!menuactive && !cv_tsourdt3rd_main_checkforupdatesonstartup.value)
	{
		tsourdt3rd_local.checked_version = true;
		return;
	}
	else if (tsourdt3rd_local.checked_version)
		return;

	if (!dedicated)
	{
		M_DrawTextBox(52, BASEVIDHEIGHT/2-10, 25, 2);
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, V_MENUCOLORMAP, "Checking for updates...");
		I_FinishUpdate(); // page flip or blit buffer
	}
	else
		STAR_CONS_Printf(STAR_CONS_TSOURDT3RD, "Checking for updates...\n");

	sprintf(version_url,
		"https://raw.githubusercontent.com/StarManiaKG/The-Story-of-Uncapped-Revengence-Discord-the-3rd/%s/src/STAR/star_webinfo.h",
		compbranch
	);
	TSoURDt3rd_Curl_FindStringWithinURL(
		TSoURDt3rd_FIL_AccessFile("TSoURDt3rd", "tsourdt3rd_data.txt", "w+"),
		"#define TSOURDT3RDVERSION",
		version_url,
		&return_info,
		&return_code
	);

	switch (return_code)
	{
		case TSOURDT3RD_CURL_DATAFOUND:
		{
			return_info = return_info + 26;
			TSoURDt3rd_M_RemoveStringChars(return_info, "\"");

			return_version = Z_StrDup(return_info);
			return_info = TSoURDt3rd_M_RemoveStringChars(return_info, ".");

			version_number = (UINT32)atoi(return_info);
			if (version_number < 100)
				version_number *= 10; // add another decimal for subversions

			if (TSoURDt3rd_CurrentVersion() < version_number)
			{
				header_string = "Update TSoURDt3rd, please";
				sprintf(message_string,
					"You're using an outdated version of TSoURDt3rd.\n\n"
					"The most recent version is: \x82%s\x80\n"
					"You're using version: \x82%s\x80\n\n"
					"Check the SRB2 Message Board for the latest version!",
				return_version, TSOURDT3RDVERSION);
				message_type = STAR_CONS_TSOURDT3RD_ALERT;
			}
			else if (TSoURDt3rd_CurrentVersion() > version_number)
			{
				header_string = "Hello TSoURDt3rd beta user!";
				sprintf(message_string,
					"You're using a version of TSoURDt3rd\n"
					"that hasn't even released yet.\n\n"
					"You're probably a tester or coder,"
					"\nand in that case, hello!\n\n"
					"Enjoy messing around with the build!"
				);
				message_type = STAR_CONS_TSOURDT3RD_ALERT;
			}
			else
			{
				header_string = "No updates found!";
				sprintf(message_string, "Your TSoURDt3rd Executable is up-to-date! Have fun!");
				message_type = STAR_CONS_TSOURDT3RD_NOTICE;
			}

			break;
		}

		default:
		{
			header_string = "Failed to check for updates!";
			sprintf(message_string,
				"Couldn't properly check for updates.\n"
				"Maybe try again?\n"
				"\n"
				"If you don't know how to try again,\n"
				"just head to the 'Main Options' submenu,\n"
				"under the 'TSoURDt3rd Options' menu.\n"
				"You'll find the option to retry there."
			);
			message_type = STAR_CONS_TSOURDT3RD_ALERT;
			break;
		}
	}

	STAR_CONS_Printf(message_type, "%s\n", message_string);
	if (!dedicated)
		TSoURDt3rd_M_StartMessage(header_string, message_string, NULL, MM_NOTHING, NULL, NULL);

	Z_Free(return_version);

	TSoURDt3rd_FIL_RemoveFile("TSoURDt3rd", "tsourdt3rd_data.txt");
	tsourdt3rd_local.checked_version = true;
}

#endif // HAVE_CURL
