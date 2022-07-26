// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

void
Moira::saveToStack(AEStackFrame &frame)
{
    switch (core) {

        case M68000: saveToStack <M68000> (frame); break;
        case M68010: saveToStack <M68010> (frame); break;
        case M68020: saveToStack <M68020> (frame); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::saveToStack(AEStackFrame &frame)
{
    // Push PC
    push <Word> ((u16)frame.pc);
    push <Word> (frame.pc >> 16);
    
    // Push SR and IRD
    push <Word> (frame.sr);
    push <Word> (frame.ird);
    
    // Push address
    push <Word> ((u16)frame.addr);
    push <Word> (frame.addr >> 16);
    
    // Push memory access type and function code
    push <Word> (frame.code);
}

void
Moira::saveToStackBrief(u16 nr, u16 sr, u32 pc)
{
    switch (core) {

        case M68000: saveToStackBrief <M68000> (nr, sr, pc); break;
        case M68010: saveToStackBrief <M68010> (nr, sr, pc); break;
        case M68020: saveToStackBrief <M68020> (nr, sr, pc); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::saveToStackBrief(u16 nr, u16 sr, u32 pc)
{
    if constexpr (C == M68000) {

        if constexpr (MIMIC_MUSASHI) {

            push <Long> (pc);
            push <Word> (sr);

        } else {

            reg.sp -= 6;
            writeMS <MEM_DATA, Word> ((reg.sp + 4) & ~1, pc & 0xFFFF);
            writeMS <MEM_DATA, Word> ((reg.sp + 0) & ~1, sr);
            writeMS <MEM_DATA, Word> ((reg.sp + 2) & ~1, pc >> 16);
        }
    }

    if constexpr (C == M68010) {

        if constexpr (MIMIC_MUSASHI) {

            push <Word> (4 * nr);
            push <Long> (pc);
            push <Word> (sr);

        } else {

            reg.sp -= 8;
            writeMS <MEM_DATA, Word> ((reg.sp + 6) & ~1, 4 * nr);
            writeMS <MEM_DATA, Word> ((reg.sp + 4) & ~1, pc & 0xFFFF);
            writeMS <MEM_DATA, Word> ((reg.sp + 0) & ~1, sr);
            writeMS <MEM_DATA, Word> ((reg.sp + 2) & ~1, pc >> 16);
        }
    }
}

void
Moira::execAddressError(AEStackFrame frame, int delay)
{
    switch (core) {

        case M68000: execAddressError <M68000> (frame, delay); break;
        case M68010: execAddressError <M68010> (frame, delay); break;
        case M68020: execAddressError <M68020> (frame, delay); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::execAddressError(AEStackFrame frame, int delay)
{
    assert(frame.addr & 1);
    
    // Emulate additional delay
    sync(delay);
    
    // Enter supervisor mode
    setSupervisorMode(true);
    
    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;
    SYNC(8);

    // A misaligned stack pointer will cause a "double fault"
    bool doubleFault = misaligned<Word>(reg.sp);

    if (!doubleFault) {
        
        // Write stack frame
        saveToStack(frame);
        SYNC(2);
        jumpToVector<C>(3);
    }
    
    // Inform the delegate
    signalAddressError(frame);
    
    // Halt the CPU if a double fault occurred
    if (doubleFault) halt();
}

void
Moira::execFormatError()
{
    switch (core) {

        case M68000: assert(false); break;
        case M68010: execFormatError <M68010> (); break;
        case M68020: execFormatError <M68020> (); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::execFormatError()
{
    u16 status = getSR();

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;

    // Write exception information to stack
    SYNC(4);
    saveToStackBrief(14, status, reg.pc);

    jumpToVector <C, AE_SET_CB3> (14);
}

void
Moira::execUnimplemented(int nr)
{
    switch (core) {

        case M68000: execUnimplemented <M68000> (nr); break;
        case M68010: execUnimplemented <M68010> (nr); break;
        case M68020: execUnimplemented <M68020> (nr); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::execUnimplemented(int nr)
{
    u16 status = getSR();
    
    // Enter supervisor mode
    setSupervisorMode(true);
    
    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;

    // Write exception information to stack
    SYNC(4);
    saveToStackBrief(u16(nr), status, reg.pc - 2);

    jumpToVector <C, AE_SET_CB3> (nr);
}

void
Moira::execTraceException()
{
    switch (core) {

        case M68000: execTraceException <M68000> (); break;
        case M68010: execTraceException <M68010> (); break;
        case M68020: execTraceException <M68020> (); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::execTraceException()
{
    signalTraceException();
    
    u16 status = getSR();

    // Recover from stop state
    flags &= ~CPU_IS_STOPPED;

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;

    // Write exception information to stack
    SYNC(4);
    saveToStackBrief(9, status, reg.pc);

    jumpToVector<C>(9);
}

void
Moira::execTrapException(int nr)
{
    switch (core) {

        case M68000: execTrapException <M68000> (nr); break;
        case M68010: execTrapException <M68010> (nr); break;
        case M68020: execTrapException <M68020> (nr); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::execTrapException(int nr)
{
    signalTrapException();
    
    u16 status = getSR();

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing, but keep the CPU_TRACE_EXCEPTION flag
    clearTraceFlag();

    // Write exception information to stack
    saveToStackBrief(u16(nr), status, reg.pc);

    jumpToVector<C>(nr);
}

void
Moira::execPrivilegeException()
{
    switch (core) {

        case M68000: execPrivilegeException <M68000> (); break;
        case M68010: execPrivilegeException <M68010> (); break;
        case M68020: execPrivilegeException <M68020> (); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::execPrivilegeException()
{
    signalPrivilegeViolation();
    
    u16 status = getSR();

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;

    // Write exception information to stack
    SYNC(4);
    saveToStackBrief(8, status, reg.pc - 2);

    jumpToVector <C,AE_SET_CB3> (8);
}

void
Moira::execIrqException(u8 level)
{
    switch (core) {

        case M68000: execIrqException <M68000> (level); break;
        case M68010: execIrqException <M68010> (level); break;
        case M68020: execIrqException <M68020> (level); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::execIrqException(u8 level)
{
    assert(level < 8);
    
    // Notify delegate
    signalInterrupt(level);
    
    // Remember the current value of the status register
    u16 status = getSR();

    // Recover from stop state and terminate loop mode
    flags &= ~(CPU_IS_STOPPED | CPU_IS_LOOPING);

    // Clear the polled IPL value
    reg.ipl = 0;

    // Temporarily raise the interrupt threshold
    reg.sr.ipl = level;
    
    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;

    if constexpr (C == M68000) {

        SYNC(6);
        reg.sp -= 6;
        writeMS <MEM_DATA, Word> (reg.sp + 4, reg.pc & 0xFFFF);

        SYNC(4);
        queue.ird = getIrqVector(level);

        SYNC(4);
        writeMS <MEM_DATA, Word> (reg.sp + 0, status);
        writeMS <MEM_DATA, Word> (reg.sp + 2, reg.pc >> 16);
    }

    if constexpr (C == M68010) {

        SYNC(6);
        reg.sp -= 8;
        writeMS <MEM_DATA, Word> (reg.sp + 4, reg.pc & 0xFFFF);

        SYNC(4);
        queue.ird = getIrqVector(level);

        SYNC(4);
        writeMS <MEM_DATA, Word> (reg.sp + 0, status);
        writeMS <MEM_DATA, Word> (reg.sp + 2, reg.pc >> 16);

        writeMS <MEM_DATA, Word> (reg.sp + 6, 4 * queue.ird);
    }

    jumpToVector <C,AE_SET_CB3> (queue.ird);
}
