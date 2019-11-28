// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


// Reads a value from memory
template<Size S> u32 read(u32 addr);

// Writes a value to memory
template<Size S> void write(u32 addr, u32 value);

// Computea an effective address
template<Mode M, Size S> u32 computeEA(u32 n, u32 dis = 0, u32 idx = 0);

// Reads a value from an effective address
template<Mode M, Size S> u32 readEA(u32 n, u32 dis = 0, u32 idx = 0);


// Read immediate value
template<Size S> u32 readImm();

template<Size S, Instr I> u32 shift(u32 cnt, u32 data);
template<Size S> u32 add(u32 op1, u32 op2);
