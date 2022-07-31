// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

void
Moira::saveToStack0(AEStackFrame &frame)
{
    switch (core) {

        case M68000: saveToStack0 <M68000> (frame); break;
        case M68010: saveToStack0 <M68010> (frame); break;
        case M68020: saveToStack0 <M68020> (frame); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::saveToStack0(AEStackFrame &frame)
{
    // Push PC
    push <C,Word> ((u16)frame.pc);
    push <C,Word> (frame.pc >> 16);
    
    // Push SR and IRD
    push <C,Word> (frame.sr);
    push <C,Word> (frame.ird);
    
    // Push address
    push <C,Word> ((u16)frame.addr);
    push <C,Word> (frame.addr >> 16);
    
    // Push memory access type and function code
    push <C,Word> (frame.code);
}

void
Moira::saveToStack1(u16 nr, u16 sr, u32 pc)
{
    switch (core) {

        case M68000: saveToStack1 <M68000> (nr, sr, pc); break;
        case M68010: saveToStack1 <M68010> (nr, sr, pc); break;
        case M68020: saveToStack1 <M68020> (nr, sr, pc); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::saveToStack1(u16 nr, u16 sr, u32 pc)
{
    switch (C) {

        case M68000:

            if constexpr (MIMIC_MUSASHI) {

                push <C,Long> (pc);
                push <C,Word> (sr);

            } else {

                reg.sp -= 6;
                writeMS <C,MEM_DATA,Word> ((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 0) & ~1, sr);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 2) & ~1, pc >> 16);
            }
            break;

        case M68010:
        case M68020:

            if constexpr (MIMIC_MUSASHI) {

                push <C,Word> (4 * nr);
                push <C,Long> (pc);
                push <C,Word> (sr);

            } else {

                reg.sp -= 8;
                writeMS <C,MEM_DATA,Word> ((reg.sp + 6) & ~1, 4 * nr);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 0) & ~1, sr);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 2) & ~1, pc >> 16);
            }
            break;
    }
}

void
Moira::saveToStack2(u16 nr, u16 sr, u32 pc)
{
    switch (core) {

        case M68000: saveToStack2 <M68000> (nr, sr, pc); break;
        case M68010: saveToStack2 <M68010> (nr, sr, pc); break;
        case M68020: saveToStack2 <M68020> (nr, sr, pc); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::saveToStack2(u16 nr, u16 sr, u32 pc)
{
    switch (C) {

        case M68000:

            // SAME AS saveToStack1? IF YES, CALL IT
            if constexpr (MIMIC_MUSASHI) {

                push <C,Long> (pc);
                push <C,Word> (sr);

            } else {

                reg.sp -= 6;
                writeMS <C,MEM_DATA,Word> ((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 0) & ~1, sr);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 2) & ~1, pc >> 16);
            }
            break;

        case M68010:

            // SAME AS saveToStack1? IF YES, CALL IT
            if constexpr (MIMIC_MUSASHI) {

                push <C,Word> (4 * nr);
                push <C,Long> (pc);
                push <C,Word> (sr);

            } else {

                reg.sp -= 8;
                writeMS <C,MEM_DATA,Word> ((reg.sp + 6) & ~1, 4 * nr);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 0) & ~1, sr);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 2) & ~1, pc >> 16);
            }
            break;

        case M68020:

            if constexpr (MIMIC_MUSASHI) {

                printf("Pushing %x %x %x %x\n", pc, 0x2000 | nr << 2, reg.pc, sr);
                push <C,Long> (reg.pc0);
                push <C,Word> (0x2000 | nr << 2);
                push <C,Long> (pc);
                push <C,Word> (sr);

            } else {

                // TODO
                assert(false);
                reg.sp -= 8;
                writeMS <C,MEM_DATA,Word> ((reg.sp + 6) & ~1, 4 * nr);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 0) & ~1, sr);
                writeMS <C,MEM_DATA,Word> ((reg.sp + 2) & ~1, pc >> 16);
            }
            break;
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
        saveToStack0(frame);
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
    saveToStack1(14, status, reg.pc);

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
    saveToStack1<C>(u16(nr), status, reg.pc - 2);

    jumpToVector<C,AE_SET_CB3>(nr);
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
    saveToStack1<C>(9, status, reg.pc);

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
    saveToStack2<C>(u16(nr), status, reg.pc);

    jumpToVector<C>(nr);
}

void
Moira::execPrivilegeException()
{
    switch (core) {

        case M68000: execPrivilegeException<M68000>(); break;
        case M68010: execPrivilegeException<M68010>(); break;
        case M68020: execPrivilegeException<M68020>(); break;

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
    saveToStack1<C>(8, status, reg.pc - 2);

    jumpToVector<C,AE_SET_CB3>(8);
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
        writeMS <C,MEM_DATA,Word> (reg.sp + 4, reg.pc & 0xFFFF);

        SYNC(4);
        queue.ird = getIrqVector(level);

        SYNC(4);
        writeMS <C,MEM_DATA,Word> (reg.sp + 0, status);
        writeMS <C,MEM_DATA,Word> (reg.sp + 2, reg.pc >> 16);
    }

    if constexpr (C == M68010) {

        SYNC(6);
        reg.sp -= 8;
        writeMS <C,MEM_DATA,Word> (reg.sp + 4, reg.pc & 0xFFFF);

        SYNC(4);
        queue.ird = getIrqVector(level);

        SYNC(4);
        writeMS <C,MEM_DATA,Word> (reg.sp + 0, status);
        writeMS <C,MEM_DATA,Word> (reg.sp + 2, reg.pc >> 16);

        writeMS <C,MEM_DATA,Word> (reg.sp + 6, 4 * queue.ird);
    }

    jumpToVector <C,AE_SET_CB3> (queue.ird);
}
