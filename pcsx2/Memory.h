/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "VirtualMemory.h"
#include "vtlb.h"

#define PSM(mem)	(vtlb_GetPhyPtr((mem)&0x1fffffff)) //pcsx2 is a competition.The one with most hacks wins :D

#define psHu8(mem)	(*(u8 *)&eeHw[(mem) & 0xffff])
#define psHu16(mem)	(*(u16*)&eeHw[(mem) & 0xffff])
#define psHu32(mem)	(*(u32*)&eeHw[(mem) & 0xffff])
#define psHu64(mem)	(*(u64*)&eeHw[(mem) & 0xffff])
#define psHu128(mem)(*(u128*)&eeHw[(mem) & 0xffff])

#define psSu32(mem)	(*(u32 *)&eeMem->Scratch[(mem) & 0x3fff])
#define psSu64(mem)	(*(u64 *)&eeMem->Scratch[(mem) & 0x3fff])
#define psSu128(mem)	(*(u128*)&eeMem->Scratch[(mem) & 0x3fff])

#define memRead8 vtlb_memRead8
#define memRead16 vtlb_memRead16
#define memRead32 vtlb_memRead32
#define memRead64 vtlb_memRead64

#define memWrite8 vtlb_memWrite8
#define memWrite16 vtlb_memWrite16
#define memWrite32 vtlb_memWrite32
#define memWrite64 vtlb_memWrite64

// This is a table of default virtual map addresses for ps2vm components.  These locations
// are provided and used to assist in debugging and possibly hacking; as it makes it possible
// for a programmer to know exactly where to look (consistently!) for the base address of
// the various virtual machine components.  These addresses can be keyed directly into the
// debugger's disasm window to get disassembly of recompiled code, and they can be used to help
// identify recompiled code addresses in the callstack.

// All of these areas should be reserved as soon as possible during program startup, and its
// important that none of the areas overlap.  In all but superVU's case, failure due to overlap
// or other conflict will result in the operating system picking a preferred address for the mapping.

/* The host memory map and the VM memory owner. Both are C++: a namespace
 * of constants and a class holding the reserves. C reaches the memory
 * through the macros and vtlb entry points above and below. */
#ifdef __cplusplus

namespace HostMemoryMap
{
	//////////////////////////////////////////////////////////////////////////
	// Main
	//////////////////////////////////////////////////////////////////////////
	static const u32 MainSize = 0x14000000;

	// PS2 main memory, SPR, and ROMs (approximately 40.5MB, but we round up to 64MB for simplicity).
	static const u32 EEmemOffset   = 0x00000000;

	// IOP main memory and ROMs
	static const u32 IOPmemOffset  = 0x04000000;

	// VU0 and VU1 memory.
	static const u32 VUmemOffset   = 0x08000000;

	// Bump allocator for any other small allocations
	// size: Difference between it and HostMemoryMap::Size, so nothing should allocate higher than it!
	static const u32 bumpAllocatorOffset = 0x10000000;

	//////////////////////////////////////////////////////////////////////////
	// Code
	//////////////////////////////////////////////////////////////////////////
	// Each recompiler's code cache, one after another. A cache that fills is
	// cleared and refilled, so a smaller one costs the occasional reset and
	// nothing else.
#if defined(__PROSPERO__)
	// PS5: the code area is anonymous memory, charged in full to the title's
	// flexible-memory budget when it is mapped -- about 450MB for the whole
	// title, RetroArch and the core image included -- where upstream's 305MB
	// would not fit. VU0's microprograms are small; VU1's are what games run.
	static const u32 EErecSize        = 0x02800000; // 40mb
	static const u32 IOPrecSize       = 0x00C00000; // 12mb
	static const u32 VIFrecSize       = 0x00600000; // 6mb each
	static const u32 mVU0recSize      = 0x01000000; // 16mb
	static const u32 mVU1recSize      = 0x03000000; // 48mb
	static const u32 VIFUnpackRecSize = 0x00100000; // 1mb
	static const u32 SWrecSize        = 0x02000000; // 32mb
#else
	static const u32 EErecSize        = 0x04000000; // 64mb
	static const u32 IOPrecSize       = 0x02000000; // 32mb
	static const u32 VIFrecSize       = 0x00800000; // 8mb each
	static const u32 mVU0recSize      = 0x04000000; // 64mb
	static const u32 mVU1recSize      = 0x04000000; // 64mb
	static const u32 VIFUnpackRecSize = 0x00100000; // 1mb
	static const u32 SWrecSize        = 0x04000000; // 64mb
#endif

	// EE recompiler code cache area
	static const u32 EErecOffset   = 0x00000000;

	// IOP recompiler code cache area
	static const u32 IOPrecOffset  = EErecOffset + EErecSize;

	// newVif0 recompiler code cache area
	static const u32 VIF0recOffset = IOPrecOffset + IOPrecSize;

	// newVif1 recompiler code cache area
	static const u32 VIF1recOffset = VIF0recOffset + VIFrecSize;

	// microVU1 recompiler code cache area (upstream's names are swapped:
	// microVU.cpp gives VU1 this one)
	static const u32 mVU0recOffset = VIF1recOffset + VIFrecSize;

	// microVU0 recompiler code cache area
	static const u32 mVU1recOffset = mVU0recOffset + mVU1recSize;

	// SSE-optimized VIF unpack functions
	static const u32 VIFUnpackRecOffset = mVU1recOffset + mVU0recSize;

	// Software Renderer JIT buffer
	static const u32 SWrecOffset = VIFUnpackRecOffset + VIFUnpackRecSize;

	static const u32 CodeSize = SWrecOffset + SWrecSize;
}

// --------------------------------------------------------------------------------------
//  SysMainMemory
// --------------------------------------------------------------------------------------
// This class provides the main memory for the virtual machines.
class SysMainMemory final
{
protected:
	const VirtualMemoryManagerPtr m_mainMemory;
	const VirtualMemoryManagerPtr m_codeMemory;

	VirtualMemoryBumpAllocator m_bumpAllocator;

	eeMemoryReserve m_ee;
	iopMemoryReserve m_iop;
	vuMemoryReserve m_vu;

public:
	SysMainMemory();
	~SysMainMemory();

	const VirtualMemoryManagerPtr& MainMemory() { return m_mainMemory; }
	const VirtualMemoryManagerPtr& CodeMemory() { return m_codeMemory; }

	VirtualMemoryBumpAllocator& BumpAllocator() { return m_bumpAllocator; }

	const vuMemoryReserve& VUMemory() const { return m_vu; }

	bool Allocate();
	void Reset();
	void Release();
};

extern SysMainMemory& GetVmMemory();

#endif /* __cplusplus */

extern void memBindConditionalHandlers(void);

static __fi void memRead128(u32 mem, mem128_t* out)        { r128_store(out, vtlb_memRead128(mem)); }
#if PCSX2_MINGW_R128_BY_PTR
static __fi void memWrite128(u32 mem, const mem128_t* val) { vtlb_memWrite128(mem, (const r128*)(val)); }
#else
static __fi void memWrite128(u32 mem, const mem128_t* val) { vtlb_memWrite128(mem, r128_load(val)); }
#endif
