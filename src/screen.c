// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2023 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  screen.c
/// \brief Handles multiple resolutions, 8bpp/16bpp(highcolor) modes

#include "doomdef.h"
#include "doomstat.h"
#include "screen.h"
#include "console.h"
#include "am_map.h"
#include "i_time.h"
#include "i_system.h"
#include "i_video.h"
#include "r_local.h"
#include "r_sky.h"
#include "m_argv.h"
#include "m_misc.h"
#include "v_video.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "z_zone.h"
#include "d_main.h"
#include "d_clisrv.h"
#include "f_finale.h"
#include "y_inter.h" // usebuffer
#include "i_sound.h" // closed captions
#include "s_sound.h" // ditto
#include "g_game.h" // ditto
#include "p_local.h" // P_AutoPause()

#ifdef HWRENDER
#include "hardware/hw_main.h"
#include "hardware/hw_light.h"
#include "hardware/hw_model.h"
#endif

// SRB2Kart
#include "r_fps.h" // R_GetFramerateCap

// STAR STUFF //
#include "STAR/star_vars.h" // loading screen stuff
#include "m_menu.h" // fps coloring command
// END THAT PLEASE //

// --------------------------------------------
// assembly or c drawer routines for 8bpp/16bpp
// --------------------------------------------
void (*colfunc)(void);
void (*colfuncs[COLDRAWFUNC_MAX])(void);

void (*spanfunc)(void);
void (*spanfuncs[SPANDRAWFUNC_MAX])(void);
void (*spanfuncs_npo2[SPANDRAWFUNC_MAX])(void);

// ------------------
// global video state
// ------------------
viddef_t vid;
INT32 setmodeneeded; //video mode change needed if > 0 (the mode number to set + 1)
UINT8 setrenderneeded = 0;

static CV_PossibleValue_t scr_depth_cons_t[] = {{8, "8 bits"}, {16, "16 bits"}, {24, "24 bits"}, {32, "32 bits"}, {0, NULL}};

//added : 03-02-98: default screen mode, as loaded/saved in config
consvar_t cv_scr_width = CVAR_INIT ("scr_width", "1280", CV_SAVE, CV_Unsigned, NULL);
consvar_t cv_scr_height = CVAR_INIT ("scr_height", "800", CV_SAVE, CV_Unsigned, NULL);
consvar_t cv_scr_width_w = CVAR_INIT ("scr_width_w", "640", CV_SAVE, CV_Unsigned, NULL);
consvar_t cv_scr_height_w = CVAR_INIT ("scr_height_w", "400", CV_SAVE, CV_Unsigned, NULL);
consvar_t cv_scr_depth = CVAR_INIT ("scr_depth", "16 bits", CV_SAVE, scr_depth_cons_t, NULL);

consvar_t cv_renderview = CVAR_INIT ("renderview", "On", 0, CV_OnOff, NULL);

CV_PossibleValue_t cv_renderer_t[] = {
	{1, "Software"},
#ifdef HWRENDER
	{2, "OpenGL"},
#endif
	{0, NULL}
};

consvar_t cv_renderer = CVAR_INIT ("renderer", "Software", CV_SAVE|CV_CALL, cv_renderer_t, SCR_ChangeRenderer);

static void SCR_ChangeFullscreen(void);

consvar_t cv_fullscreen = CVAR_INIT ("fullscreen", "Yes", CV_SAVE|CV_CALL, CV_YesNo, SCR_ChangeFullscreen);

// =========================================================================
//                           SCREEN VARIABLES
// =========================================================================

INT32 scr_bpp; // current video mode bytes per pixel
UINT8 *scr_borderpatch; // flat used to fill the reduced view borders set at ST_Init()

// =========================================================================

//  Short and Tall sky drawer, for the current color mode
void (*walldrawerfunc)(void);

boolean R_486 = false;
boolean R_586 = false;
boolean R_MMX = false;
boolean R_SSE = false;
boolean R_3DNow = false;
boolean R_MMXExt = false;
boolean R_SSE2 = false;

void SCR_SetDrawFuncs(void)
{
	//
	//  setup the right draw routines for either 8bpp or 16bpp
	//
	if (true)//vid.bpp == 1) //Always run in 8bpp. todo: remove all 16bpp code?
	{
		colfuncs[BASEDRAWFUNC] = R_DrawColumn_8;
		spanfuncs[BASEDRAWFUNC] = R_DrawSpan_8;

		colfunc = colfuncs[BASEDRAWFUNC];
		spanfunc = spanfuncs[BASEDRAWFUNC];

		colfuncs[COLDRAWFUNC_FUZZY] = R_DrawTranslucentColumn_8;
		colfuncs[COLDRAWFUNC_TRANS] = R_DrawTranslatedColumn_8;
		colfuncs[COLDRAWFUNC_SHADE] = R_DrawShadeColumn_8;
		colfuncs[COLDRAWFUNC_SHADOWED] = R_DrawColumnShadowed_8;
		colfuncs[COLDRAWFUNC_TRANSTRANS] = R_DrawTranslatedTranslucentColumn_8;
		colfuncs[COLDRAWFUNC_TWOSMULTIPATCH] = R_Draw2sMultiPatchColumn_8;
		colfuncs[COLDRAWFUNC_TWOSMULTIPATCHTRANS] = R_Draw2sMultiPatchTranslucentColumn_8;
		colfuncs[COLDRAWFUNC_FOG] = R_DrawFogColumn_8;

		spanfuncs[SPANDRAWFUNC_TRANS] = R_DrawTranslucentSpan_8;
		spanfuncs[SPANDRAWFUNC_TILTED] = R_DrawTiltedSpan_8;
		spanfuncs[SPANDRAWFUNC_TILTEDTRANS] = R_DrawTiltedTranslucentSpan_8;
		spanfuncs[SPANDRAWFUNC_SPLAT] = R_DrawSplat_8;
		spanfuncs[SPANDRAWFUNC_TRANSSPLAT] = R_DrawTranslucentSplat_8;
		spanfuncs[SPANDRAWFUNC_TILTEDSPLAT] = R_DrawTiltedSplat_8;
		spanfuncs[SPANDRAWFUNC_SPRITE] = R_DrawFloorSprite_8;
		spanfuncs[SPANDRAWFUNC_TRANSSPRITE] = R_DrawTranslucentFloorSprite_8;
		spanfuncs[SPANDRAWFUNC_TILTEDSPRITE] = R_DrawTiltedFloorSprite_8;
		spanfuncs[SPANDRAWFUNC_TILTEDTRANSSPRITE] = R_DrawTiltedTranslucentFloorSprite_8;
		spanfuncs[SPANDRAWFUNC_WATER] = R_DrawWaterSpan_8;
		spanfuncs[SPANDRAWFUNC_TILTEDWATER] = R_DrawTiltedWaterSpan_8;
		spanfuncs[SPANDRAWFUNC_SOLID] = R_DrawSolidColorSpan_8;
		spanfuncs[SPANDRAWFUNC_TRANSSOLID] = R_DrawTransSolidColorSpan_8;
		spanfuncs[SPANDRAWFUNC_TILTEDSOLID] = R_DrawTiltedSolidColorSpan_8;
		spanfuncs[SPANDRAWFUNC_TILTEDTRANSSOLID] = R_DrawTiltedTransSolidColorSpan_8;
		spanfuncs[SPANDRAWFUNC_WATERSOLID] = R_DrawWaterSolidColorSpan_8;
		spanfuncs[SPANDRAWFUNC_TILTEDWATERSOLID] = R_DrawTiltedWaterSolidColorSpan_8;
		spanfuncs[SPANDRAWFUNC_FOG] = R_DrawFogSpan_8;
		spanfuncs[SPANDRAWFUNC_TILTEDFOG] = R_DrawTiltedFogSpan_8;

		// Lactozilla: Non-powers-of-two
		spanfuncs_npo2[BASEDRAWFUNC] = R_DrawSpan_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_TRANS] = R_DrawTranslucentSpan_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_TILTED] = R_DrawTiltedSpan_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_TILTEDTRANS] = R_DrawTiltedTranslucentSpan_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_SPLAT] = R_DrawSplat_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_TRANSSPLAT] = R_DrawTranslucentSplat_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_TILTEDSPLAT] = R_DrawTiltedSplat_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_SPRITE] = R_DrawFloorSprite_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_TRANSSPRITE] = R_DrawTranslucentFloorSprite_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_TILTEDSPRITE] = R_DrawTiltedFloorSprite_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_TILTEDTRANSSPRITE] = R_DrawTiltedTranslucentFloorSprite_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_WATER] = R_DrawWaterSpan_NPO2_8;
		spanfuncs_npo2[SPANDRAWFUNC_TILTEDWATER] = R_DrawTiltedWaterSpan_NPO2_8;

	}
/*	else if (vid.bpp > 1)
	{
		I_OutputMsg("using highcolor mode\n");
		spanfunc = basespanfunc = R_DrawSpan_16;
		transcolfunc = R_DrawTranslatedColumn_16;
		transtransfunc = R_DrawTranslucentColumn_16; // No 16bit operation for this function

		colfunc = basecolfunc = R_DrawColumn_16;
		shadecolfunc = NULL; // detect error if used somewhere..
		fuzzcolfunc = R_DrawTranslucentColumn_16;
		walldrawerfunc = R_DrawWallColumn_16;
	}*/
	else
		I_Error("unknown bytes per pixel mode %d\n", vid.bpp);
/*
	if (SCR_IsAspectCorrect(vid.width, vid.height))
		CONS_Alert(CONS_WARNING, M_GetText("Resolution is not aspect-correct!\nUse a multiple of %dx%d\n"), BASEVIDWIDTH, BASEVIDHEIGHT);
*/
}

void SCR_SetMode(void)
{
	if (dedicated)
		return;

	if (!(setmodeneeded || setrenderneeded) || WipeInAction)
		return; // should never happen and don't change it during a wipe, BAD!

	// Lactozilla: Renderer switching
	if (setrenderneeded)
	{
		// stop recording movies (APNG only)
		if (setrenderneeded && (moviemode == MM_APNG))
			M_StopMovie();

		// VID_SetMode will call VID_CheckRenderer itself,
		// so no need to do this in here.
		if (!setmodeneeded)
			VID_CheckRenderer();

		vid.recalc = 1;
	}

	// Set the video mode in the video interface.
	if (setmodeneeded)
		VID_SetMode(setmodeneeded - 1);

	V_SetPalette(0);

	SCR_SetDrawFuncs();

	// set the apprpriate drawer for the sky (tall or INT16)
	setmodeneeded = 0;
	setrenderneeded = 0;
}

// do some initial settings for the game loading screen
//
void SCR_Startup(void)
{
	const CPUInfoFlags *RCpuInfo = I_CPUInfo();
	if (!M_CheckParm("-NOCPUID") && RCpuInfo)
	{
#if defined (__i386__) || defined (_M_IX86) || defined (__WATCOMC__)
		R_486 = true;
#endif
		if (RCpuInfo->RDTSC)
			R_586 = true;
		if (RCpuInfo->MMX)
			R_MMX = true;
		if (RCpuInfo->AMD3DNow)
			R_3DNow = true;
		if (RCpuInfo->MMXExt)
			R_MMXExt = true;
		if (RCpuInfo->SSE)
			R_SSE = true;
		if (RCpuInfo->SSE2)
			R_SSE2 = true;
		CONS_Printf("CPU Info: 486: %i, 586: %i, MMX: %i, 3DNow: %i, MMXExt: %i, SSE2: %i\n", R_486, R_586, R_MMX, R_3DNow, R_MMXExt, R_SSE2);
	}

	if (M_CheckParm("-486"))
		R_486 = true;
	if (M_CheckParm("-586"))
		R_586 = true;
	if (M_CheckParm("-MMX"))
		R_MMX = true;
	if (M_CheckParm("-3DNow"))
		R_3DNow = true;
	if (M_CheckParm("-MMXExt"))
		R_MMXExt = true;

	if (M_CheckParm("-SSE"))
		R_SSE = true;
	if (M_CheckParm("-noSSE"))
		R_SSE = false;

	if (M_CheckParm("-SSE2"))
		R_SSE2 = true;

	M_SetupMemcpy();

	if (dedicated)
	{
		V_Init();
		V_SetPalette(0);
		return;
	}

	vid.modenum = 0;

	V_Init();
	V_Recalc();

	CV_RegisterVar(&cv_ticrate);
	CV_RegisterVar(&cv_constextsize);

	V_SetPalette(0);
}

// Called at new frame, if the video mode has changed
//
void SCR_Recalc(void)
{
	if (dedicated)
		return;

	// bytes per pixel quick access
	scr_bpp = vid.bpp;

	V_Recalc();

	// toggle off (then back on) the automap because some screensize-dependent values will
	// be calculated next time the automap is activated.
	if (automapactive)
	{
		am_recalc = true;
		AM_Start();
	}

	// set the screen[x] ptrs on the new vidbuffers
	V_Init();

	// scr_viewsize doesn't change, neither detailLevel, but the pixels
	// per screenblock is different now, since we've changed resolution.
	R_SetViewSize(); //just set setsizeneeded true now ..

	// vid.recalc lasts only for the next refresh...
	con_recalc = true;
	am_recalc = true;

#ifdef HWRENDER
	// Shoot! The screen texture was flushed!
	if ((rendermode == render_opengl) && (gamestate == GS_INTERMISSION))
		usebuffer = false;
#endif
}

// Check for screen cmd-line parms: to force a resolution.
//
// Set the video mode to set at the 1st display loop (setmodeneeded)
//

void SCR_CheckDefaultMode(void)
{
	INT32 scr_forcex, scr_forcey; // resolution asked from the cmd-line

	if (dedicated)
		return;

	// 0 means not set at the cmd-line
	scr_forcex = scr_forcey = 0;

	if (M_CheckParm("-width") && M_IsNextParm())
		scr_forcex = atoi(M_GetNextParm());

	if (M_CheckParm("-height") && M_IsNextParm())
		scr_forcey = atoi(M_GetNextParm());

	if (scr_forcex && scr_forcey)
	{
		CONS_Printf(M_GetText("Using resolution: %d x %d\n"), scr_forcex, scr_forcey);
		// returns -1 if not found, thus will be 0 (no mode change) if not found
		setmodeneeded = VID_GetModeForSize(scr_forcex, scr_forcey) + 1;
	}
	else
	{
		CONS_Printf(M_GetText("Default resolution: %d x %d\n"), cv_scr_width.value, cv_scr_height.value);
		CONS_Printf(M_GetText("Windowed resolution: %d x %d\n"), cv_scr_width_w.value, cv_scr_height_w.value);
		CONS_Printf(M_GetText("Default bit depth: %d bits\n"), cv_scr_depth.value);
		if (cv_fullscreen.value)
			setmodeneeded = VID_GetModeForSize(cv_scr_width.value, cv_scr_height.value) + 1; // see note above
		else
			setmodeneeded = VID_GetModeForSize(cv_scr_width_w.value, cv_scr_height_w.value) + 1; // see note above

		if (setmodeneeded <= 0)
			CONS_Alert(CONS_WARNING, "Invalid resolution given, defaulting to base resolution\n");
	}

	if (cv_renderer.value != (signed)rendermode)
	{
		if (chosenrendermode == render_none) // nothing set at command line
			SCR_ChangeRenderer();
		else
		{
			// Set cv_renderer to the current render mode
			CV_StealthSetValue(&cv_renderer, rendermode);
		}
	}
}

// sets the modenum as the new default video mode to be saved in the config file
void SCR_SetDefaultMode(void)
{
	CV_SetValue(cv_fullscreen.value ? &cv_scr_width : &cv_scr_width_w, vid.width);
	CV_SetValue(cv_fullscreen.value ? &cv_scr_height : &cv_scr_height_w, vid.height);
}

// Change fullscreen on/off according to cv_fullscreen
void SCR_ChangeFullscreen(void)
{
#ifdef DIRECTFULLSCREEN
	// allow_fullscreen is set by VID_PrepareModeList
	// it is used to prevent switching to fullscreen during startup
	if (!allow_fullscreen)
		return;

	if (graphics_started)
	{
		VID_PrepareModeList();
		if (cv_fullscreen.value)
			setmodeneeded = VID_GetModeForSize(cv_scr_width.value, cv_scr_height.value) + 1;
		else
			setmodeneeded = VID_GetModeForSize(cv_scr_width_w.value, cv_scr_height_w.value) + 1;

		if (setmodeneeded <= 0) // hacky safeguard
		{
			CONS_Alert(CONS_WARNING, "Invalid resolution given, defaulting to base resolution.\n");
			setmodeneeded = VID_GetModeForSize(BASEVIDWIDTH, BASEVIDHEIGHT) + 1;
		}
	}
	return;
#endif
}

void SCR_ChangeRenderer(void)
{
	if (chosenrendermode != render_none
	|| (signed)rendermode == cv_renderer.value)
		return;

#ifdef HWRENDER
	// Check if OpenGL loaded successfully (or wasn't disabled) before switching to it.
	if ((vid.glstate == VID_GL_LIBRARY_ERROR)
	&& (cv_renderer.value == render_opengl))
	{
		if (M_CheckParm("-nogl"))
			CONS_Alert(CONS_ERROR, "OpenGL rendering was disabled!\n");
		else
			CONS_Alert(CONS_ERROR, "OpenGL never loaded\n");
		return;
	}

	if (rendermode == render_opengl && (vid.glstate == VID_GL_LIBRARY_LOADED)) // Clear these out before switching to software
		HWR_ClearAllTextures();

#endif

	// STAR STUFF //
	TSoURDt3rdPlayers[consoleplayer].loadingScreens.loadCount = TSoURDt3rdPlayers[consoleplayer].loadingScreens.loadPercentage = 0; // reset loading status
	TSoURDt3rdPlayers[consoleplayer].loadingScreens.bspCount = 0; // reset bsp count

	TSoURDt3rdPlayers[consoleplayer].loadingScreens.softwareLoadComplete = false; // reset software loading status
	// END THE LOADING STUFF PLEASE //

	// Set the new render mode
	setrenderneeded = cv_renderer.value;
}

boolean SCR_IsAspectCorrect(INT32 width, INT32 height)
{
	return
	 (  width % BASEVIDWIDTH == 0
	 && height % BASEVIDHEIGHT == 0
	 && width / BASEVIDWIDTH == height / BASEVIDHEIGHT
	 );
}

double averageFPS = 0.0f;

#define USE_FPS_SAMPLES

#ifdef USE_FPS_SAMPLES
#define MAX_FRAME_TIME 0.05
#define NUM_FPS_SAMPLES (16) // Number of samples to store

static double total_frame_time = 0.0;
static int frame_index;
#endif

static boolean fps_init = false;
static precise_t fps_enter = 0;

void SCR_CalculateFPS(void)
{
	precise_t fps_finish = 0;

	double frameElapsed = 0.0;

	if (fps_init == false)
	{
		fps_enter = I_GetPreciseTime();
		fps_init = true;
	}

	fps_finish = I_GetPreciseTime();
	frameElapsed = (double)((INT64)(fps_finish - fps_enter)) / I_GetPrecisePrecision();
	fps_enter = fps_finish;

#ifdef USE_FPS_SAMPLES
	total_frame_time += frameElapsed;
	if (frame_index++ >= NUM_FPS_SAMPLES || total_frame_time >= MAX_FRAME_TIME)
	{
		averageFPS = 1.0 / (total_frame_time / frame_index);
		total_frame_time = 0.0;
		frame_index = 0;
	}
#else
	// Direct, unsampled counter.
	averageFPS = 1.0 / frameElapsed;
#endif
}

// STAR STUFF //
// Remember This From st_stuff.c? Yeah, I Reworked It!
UINT16 positiveTicRateColor[16] = {
	[0] = V_GREENMAP,
	V_MAGENTAMAP,
	V_YELLOWMAP,
	V_BLUEMAP,
	V_REDMAP,
	V_GRAYMAP,
	V_ORANGEMAP,
	V_SKYMAP,
	V_PURPLEMAP,
	V_AQUAMAP,
	V_PERIDOTMAP,
	V_AZUREMAP,
	V_BROWNMAP,
	V_ROSYMAP,
	V_INVERTMAP,
};
// BEP BEP //

void SCR_DisplayTicRate(void)
{
	INT32 ticcntcolor = 0;
	const INT32 h = vid.height-(8*vid.dupy);
	UINT32 cap = R_GetFramerateCap();
	double fps = round(averageFPS);

	// STAR STUFF //
	// TPS Specific
	tic_t i;
	tic_t ontic = I_GetTime();
	tic_t totaltics = 0;
	static tic_t lasttic;
	static boolean ticsgraph[TICRATE];
	INT32 tpscntcolor = 0;
	// END THAT FOR NOW //

	if (gamestate == GS_NULL)
		return;

	// FPS
	if (cv_ticrate.value)
	{
		if (cap > 0)
		{
			if (fps <= cap / 2.0) ticcntcolor = V_REDMAP;
			else if (fps <= cap * 0.90) ticcntcolor = V_YELLOWMAP;
			else {
				ticcntcolor = positiveTicRateColor[cv_fpscountercolor.value];
			}
		}
		else
		{
			ticcntcolor = positiveTicRateColor[cv_fpscountercolor.value];
		}

		if (cv_ticrate.value == 2) // compact counter
		{			
			V_DrawRightAlignedString(vid.width, h,
				ticcntcolor|V_NOSCALESTART|V_USERHUDTRANS, va("%04.2f", averageFPS)); // use averageFPS directly
		}
		else // full counter
		{
			const char *drawnstr;
			INT32 width;

			// The highest assignable cap is < 1000, so 3 characters is fine.
			if (cap > 0)
				drawnstr = va("%3.0f/%3u", fps, cap);
			else
				drawnstr = va("%4.2f", averageFPS);

			width = V_StringWidth(drawnstr, V_NOSCALESTART);

			V_DrawString(vid.width - ((7 * 8 * vid.dupx) + V_StringWidth("FPS: ", V_NOSCALESTART)), h,
				menuColor[cv_menucolor.value]|V_NOSCALESTART|V_USERHUDTRANS, "FPS:");
			V_DrawString(vid.width - width, h,
				ticcntcolor|V_NOSCALESTART|V_USERHUDTRANS, drawnstr);
		}
	}

	// STAR STUFF //
	// TPS
	if (cv_tpsrate.value)
	{
		for (i = lasttic + 1; i < TICRATE+lasttic && i < ontic; ++i)
			ticsgraph[i % TICRATE] = false;
		
		ticsgraph[ontic % TICRATE] = true;

		for (i = 0; i < TICRATE; ++i)
			if (ticsgraph[i])
				++totaltics;

		if (totaltics <= TICRATE/2) tpscntcolor = V_REDMAP;
		else if (totaltics <= TICRATE-8) tpscntcolor = V_YELLOWMAP;
		else {
			tpscntcolor = positiveTicRateColor[cv_tpscountercolor.value];
		}

		if (cv_tpsrate.value == 2) // compact counter
			V_DrawRightAlignedString(vid.width, h-(8*vid.dupy),
				tpscntcolor|V_NOSCALESTART|V_USERHUDTRANS, va("%02d", totaltics));
		else // full counter
		{
			const char *drawntpsStr;
			INT32 tpswidth;
			
			drawntpsStr = va("%02d/ %02u", totaltics, TICRATE);
			tpswidth = V_StringWidth(drawntpsStr, V_NOSCALESTART);

			V_DrawString(vid.width - ((7 * 8 * vid.dupx) + V_StringWidth("TPS: ", V_NOSCALESTART)), h-(8*vid.dupy),
				menuColor[cv_menucolor.value]|V_NOSCALESTART|V_USERHUDTRANS, "TPS:");
			V_DrawString(vid.width - tpswidth, h-(8*vid.dupy),
				tpscntcolor|V_NOSCALESTART|V_USERHUDTRANS, drawntpsStr);
		}

		lasttic = ontic;
	}
	// END THAT //
}

void SCR_DisplayLocalPing(void)
{
	UINT32 ping = playerpingtable[consoleplayer];	// consoleplayer's ping is everyone's ping in a splitnetgame :P
	if (cv_showping.value == 1 || (cv_showping.value == 2 && servermaxping && ping > servermaxping))	// only show 2 (warning) if our ping is at a bad level
	{
		INT32 dispy = (((cv_ticrate.value && cv_tpsrate.value) || cv_tpsrate.value) ? 171 : (cv_ticrate.value ? 180 : 189)); // STAR NOTE: i was here
		HU_drawPing(307, dispy, ping, true, V_SNAPTORIGHT | V_SNAPTOBOTTOM);
	}
}


void SCR_ClosedCaptions(void)
{
	UINT8 i;
	boolean gamestopped = (paused || P_AutoPause());
	INT32 basey = BASEVIDHEIGHT;

	if (gamestate != wipegamestate)
		return;

	if (gamestate == GS_LEVEL)
	{
		if (promptactive)
			basey -= 42;
		else if (splitscreen)
			basey -= 8;
		else if ((modeattacking == ATTACKING_NIGHTS)
		|| (!(maptol & TOL_NIGHTS)
		&& ((cv_powerupdisplay.value == 2) // "Always"
		 || (cv_powerupdisplay.value == 1 && !camera.chase)))) // "First-person only"
			basey -= 16;
	}

	for (i = 0; i < NUMCAPTIONS; i++)
	{
		INT32 flags, y;
		char dot;
		boolean music;

		if (!closedcaptions[i].s)
			continue;

		music = (closedcaptions[i].s-S_sfx == sfx_None);

		if (music && !gamestopped && (closedcaptions[i].t < flashingtics) && (closedcaptions[i].t & 1))
			continue;

		flags = V_SNAPTORIGHT|V_SNAPTOBOTTOM|V_ALLOWLOWERCASE;
		y = basey-((i + 2)*10);

		if (closedcaptions[i].b)
		{
			y -= closedcaptions[i].b * vid.dupy;
			if (renderisnewtic)
			{
				closedcaptions[i].b--;
			}
		}

		if (closedcaptions[i].t < CAPTIONFADETICS)
			flags |= (((CAPTIONFADETICS-closedcaptions[i].t)/2)*V_10TRANS);

		if (music)
			dot = '\x19';
		else if (closedcaptions[i].c && closedcaptions[i].c->origin)
			dot = '\x1E';
		else
			dot = ' ';

		V_DrawRightAlignedString(BASEVIDWIDTH - 20, y, flags,
			va("%c [%s]", dot, (closedcaptions[i].s->caption[0] ? closedcaptions[i].s->caption : closedcaptions[i].s->name)));
	}
}

void SCR_DisplayMarathonInfo(void)
{
	INT32 flags = V_SNAPTOBOTTOM;
	static tic_t entertic, oldentertics = 0, antisplice[2] = {48,0};
	const char *str;
#if 0 // eh, this probably isn't going to be a problem
	if (((signed)marathontime) < 0)
	{
		flags |= V_REDMAP;
		str = "No waiting out the clock to submit a bogus time.";
	}
	else
#endif
	{
		entertic = I_GetTime();
		if (gamecomplete)
			flags |= menuColor[cv_menucolor.value]; // STAR NOTE: i was also here lol
		else if (marathonmode & MA_INGAME)
			; // see also G_Ticker
		else if (marathonmode & MA_INIT)
			marathonmode &= ~MA_INIT;
		else
			marathontime += entertic - oldentertics;

		// Create a sequence of primes such that their LCM is nice and big.
#define PRIMEV1 13
#define PRIMEV2 17 // I can't believe it! I'm on TV!
		antisplice[0] += (entertic - oldentertics)*PRIMEV2;
		antisplice[0] %= PRIMEV1*((vid.width/vid.dupx)+1);
		antisplice[1] += (entertic - oldentertics)*PRIMEV1;
		antisplice[1] %= PRIMEV1*((vid.width/vid.dupx)+1);
		str = va("%i:%02i:%02i.%02i",
			G_TicsToHours(marathontime),
			G_TicsToMinutes(marathontime, false),
			G_TicsToSeconds(marathontime),
			G_TicsToCentiseconds(marathontime));
		oldentertics = entertic;
	}
	V_DrawFill((antisplice[0]/PRIMEV1)-1, BASEVIDHEIGHT-8, 1, 8, V_SNAPTOBOTTOM|V_SNAPTOLEFT);
	V_DrawFill((antisplice[0]/PRIMEV1),   BASEVIDHEIGHT-8, 1, 8, V_SNAPTOBOTTOM|V_SNAPTOLEFT|31);
	V_DrawFill(BASEVIDWIDTH-((antisplice[1]/PRIMEV1)-1), BASEVIDHEIGHT-8, 1, 8, V_SNAPTOBOTTOM|V_SNAPTORIGHT);
	V_DrawFill(BASEVIDWIDTH-((antisplice[1]/PRIMEV1)),   BASEVIDHEIGHT-8, 1, 8, V_SNAPTOBOTTOM|V_SNAPTORIGHT|31);
#undef PRIMEV1
#undef PRIMEV2
	V_DrawPromptBack(-8, cons_backcolor.value);
	V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-8, flags, str);
}
