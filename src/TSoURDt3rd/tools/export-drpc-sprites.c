// SONIC ROBO BLAST 2; TSOURDT3RD
// PORTED FROM DR. ROBOTNIK'S RING RACERS
//-----------------------------------------------------------------------------
// Original Copyright (C) 2024 by James Robert Roman
// Copyright (C) 2024 by Star "Guy Who Names Scripts After Him" ManiaKG
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------

// #!/bin/sh
// for f in /tmp/rpcphotos/*.dat; do
// 	convert -size 16x16 -depth 8 "rgb:$f" \
// 		-crop 14x14+1+1 -scale 518x518 \
// 		"/tmp/rpcphotos/$(basename $f .dat).png"
// done

#if 1
#include "../../doomstat.h"
#include "../../r_draw.h"
#include "../../r_skins.h"
#include "../../v_video.h"
#include "../../z_zone.h"
#endif

static void expo(size_t skin)
{
	lumpnum_t lumpnum = skins[skin].sprites[SPR2_XTRA].spriteframes[0].lumppat[0];
	patch_t *pat = W_CacheSoftwarePatchNum(lumpnum, PU_STATIC);
	FILE *fp = fopen(va("/tmp/rpcphotos/%s.dat", skins[skin].name), "w");
	int w = pat->width;
	int h = pat->height;
	int size = w * h * 3;
	UINT8 *bitmap = calloc(1, size);
	const UINT8 *colormap = R_GetTranslationColormap(TC_DEFAULT, skins[skin].prefcolor, GTC_CACHE);
	for (int i = 0; i < w * h; ++i)
	{
		UINT8 *p = Picture_GetPatchPixel(pat, PICFMT_PATCH, i % w, i / h, 0);
		if (p)
		{
			byteColor_t col = pMasterPalette[colormap[*p]].s;
			bitmap[i * 3] = col.red;
			bitmap[i * 3 + 1] = col.green;
			bitmap[i * 3 + 2] = col.blue;
		}
	}
	fwrite(bitmap, 1, size, fp);
	fclose(fp);
	free(bitmap);
}
