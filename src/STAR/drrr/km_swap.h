// SONIC ROBO BLAST 2; TSOURDT3RD
// PORTED FROM DR. ROBOTNIK'S RING RACERS
//-----------------------------------------------------------------------------
// Copyright (C) 2024 by Kart Krew.
// Copyright (C) 2020 by Sonic Team Junior.
// Copyright (C) 2000 by DooM Legacy Team.
// Copyright (C) 1996 by id Software, Inc.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  km_swap.h
/// \brief Endianess handling, swapping 16bit and 32bit

#ifndef __KM_SWAP__
#define __KM_SWAP__

#include "../../endian.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SWAP_SHORT(x) ((INT16)(\
(((UINT16)(x) & (UINT16)0x00ffU) << 8) \
| \
(((UINT16)(x) & (UINT16)0xff00U) >> 8))) \

#define SWAP_LONG(x) ((INT32)(\
(((UINT32)(x) & (UINT32)0x000000ffUL) << 24) \
| \
(((UINT32)(x) & (UINT32)0x0000ff00UL) <<  8) \
| \
(((UINT32)(x) & (UINT32)0x00ff0000UL) >>  8) \
| \
(((UINT32)(x) & (UINT32)0xff000000UL) >> 24)))

// Endianess handling.
// WAD files are stored little endian.
#ifdef SRB2_BIG_ENDIAN
#if 0
	#define SHORT SWAP_SHORT
	#define LONG SWAP_LONG
#endif
#define MSBF_SHORT(x) ((INT16)(x))
#define MSBF_LONG(x) ((INT32)(x))
#else
#if 0
	#define SHORT(x) ((INT16)(x))
	#define LONG(x)	((INT32)(x))
#endif
#define MSBF_SHORT SWAP_SHORT
#define MSBF_LONG SWAP_LONG
#endif

#if 0
// Big to little endian
#ifdef SRB2_LITTLE_ENDIAN
	#define BIGENDIAN_LONG(x) ((INT32)(((x)>>24)&0xff)|(((x)<<8)&0xff0000)|(((x)>>8)&0xff00)|(((x)<<24)&0xff000000))
	#define BIGENDIAN_SHORT(x) ((INT16)(((x)>>8)|((x)<<8)))
#else
	#define BIGENDIAN_LONG(x) ((INT32)(x))
	#define BIGENDIAN_SHORT(x) ((INT16)(x))
#endif
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __KM_SWAP__