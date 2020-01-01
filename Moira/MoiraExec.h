// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// Saves information to stack for group 0 exceptions
void saveToStackDetailed(u16 sr, u32 addr, u16 code);

// Saves information to stack for group 1 and group 2 exceptions
void saveToStackBrief(u16 sr);

// Emulates an address error
void execAddressError(u32 addr);

// Emulates the execution of unimplemented and illegal instructions
void execUnimplemented(int nr);

// Emulates a trap or priviledge exception
void execTrapException(int nr);
void execPrivilegeException();

// Emulates an interrupt exception
void execIrqException(int level);
