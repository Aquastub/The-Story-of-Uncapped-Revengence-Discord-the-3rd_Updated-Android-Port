// SONIC ROBO BLAST 2; TSOURDT3RD
//-----------------------------------------------------------------------------
// Original Copyright (C) 2018-2020 by Sally "TehRealSalt" Cochenour.
// Original Copyright (C) 2018-2024 by Kart Krew.
// Copyright (C) 2020-2024 by Star "Guy Who Names Scripts After Him" ManiaKG.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  menus/menudefs/smkg-options-tsourdt3rd-players.c
/// \brief TSoURDt3rd's player menu options

#include "../../m_menu.h"

// ------------------------ //
//        Variables
// ------------------------ //

static menuitem_t TSoURDt3rd_OP_ServerMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Show Connecting Players",
		&cv_noticedownload, 0},
	{IT_STRING | IT_CVAR, NULL, "Max File Transfer",
		&cv_maxsend, 0},
	{IT_STRING | IT_CVAR, NULL, "File Transfer Packet Rate",
		&cv_downloadspeed, 0},

	{IT_SPACE | IT_DYBIGSPACE, NULL, NULL,
		NULL, 0},

	{IT_HEADER, NULL, "Advanced", NULL, 0},
		{IT_STRING | IT_CVAR | IT_CV_STRING, NULL,
			"Holepunch Server", &cv_rendezvousserver, 0},
};

static tsourdt3rd_menuitems_t TSoURDt3rd_TM_OP_ServerMenu[] =
{
	{NULL, "Notifies users when nodes are currently connecting to a server.", 0, 0},
	{NULL, "The current max file transfer size (in KB).", 0, 0},
	{NULL, "The max rate packets are allowed to be transfered at.", 0, 0},

	{NULL, NULL, 0, 0},

	{NULL, NULL, 0, 0},
	{NULL, "Relay server for holepunching users and hosts.", 0, 0},
};

menu_t TSoURDt3rd_OP_ServerDef =
{
	MTREE3(MN_OP_MAIN, MN_OP_TSOURDT3RD, MN_OP_TSOURDT3RD),
	NULL,
	sizeof (TSoURDt3rd_OP_ServerMenu)/sizeof (menuitem_t),
	&TSoURDt3rd_OP_MainMenuDef,
	TSoURDt3rd_OP_ServerMenu,
	TSoURDt3rd_M_DrawGenericOptions,
	0, 0,
	0,
	NULL
};

tsourdt3rd_menu_t TSoURDt3rd_TM_OP_ServerDef = {
	&TSoURDt3rd_OP_ServerDef,
	TSoURDt3rd_TM_OP_ServerMenu,
	SKINCOLOR_SLATE, 0,
	0,
	NULL,
	2, 5,
	NULL,
	TSoURDt3rd_M_OptionsTick,
	NULL,
	NULL,
	NULL,
	&TSoURDt3rd_TM_OP_MainMenuDef,
	NULL
};
