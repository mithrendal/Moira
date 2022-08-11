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
                writeMS<C, MEM_DATA, Word>((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS<C, MEM_DATA, Word>((reg.sp + 0) & ~1, sr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 2) & ~1, pc >> 16);
            }
            break;

        case M68010:
        case M68020:

            if constexpr (MIMIC_MUSASHI) {

                push<C, Word>(4 * nr);
                push<C, Long>(pc);
                push<C, Word>(sr);

            } else {

                reg.sp -= 8;
                writeMS<C, MEM_DATA, Word>((reg.sp + 6) & ~1, 4 * nr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS<C, MEM_DATA, Word>((reg.sp + 0) & ~1, sr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 2) & ~1, pc >> 16);
            }
            break;
    }
}

void
Moira::saveToStack2(u16 nr, u16 sr, u32 pc)
{
    switch (core) {

        case M68000: saveToStack2<M68000>(nr, sr, pc); break;
        case M68010: saveToStack2<M68010>(nr, sr, pc); break;
        case M68020: saveToStack2<M68020>(nr, sr, pc); break;

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
                writeMS<C, MEM_DATA, Word>((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS<C, MEM_DATA, Word>((reg.sp + 0) & ~1, sr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 2) & ~1, pc >> 16);
            }
            break;

        case M68010:

            // SAME AS saveToStack1? IF YES, CALL IT
            if constexpr (MIMIC_MUSASHI) {

                push<C, Word>(4 * nr);
                push<C, Long>(pc);
                push<C, Word>(sr);

            } else {

                reg.sp -= 8;
                writeMS<C, MEM_DATA, Word>((reg.sp + 6) & ~1, 4 * nr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS<C, MEM_DATA, Word>((reg.sp + 0) & ~1, sr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 2) & ~1, pc >> 16);
            }
            break;

        case M68020:

            if constexpr (MIMIC_MUSASHI) {

                // printf("Pushing %x %x %x %x\n", pc, 0x2000 | nr << 2, reg.pc, sr);
                push<C, Long>(reg.pc0);
                push<C, Word>(0x2000 | nr << 2);
                push<C, Long>(pc);
                push<C, Word>(sr);

            } else {

                // TODO
                assert(false);
                reg.sp -= 8;
                writeMS<C, MEM_DATA, Word>((reg.sp + 6) & ~1, 4 * nr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS<C, MEM_DATA, Word>((reg.sp + 0) & ~1, sr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 2) & ~1, pc >> 16);
            }
            break;
    }
}

template <Core C> void
Moira::writeStackFrame0000(u16 sr, u32 pc, u16 nr)
{
    switch (C) {

        case M68000:

            if constexpr (MIMIC_MUSASHI) {

                push<C, Long>(pc);
                push<C, Word>(sr);

            } else {

                reg.sp -= 6;
                writeMS<C, MEM_DATA,Word>((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS<C, MEM_DATA,Word>((reg.sp + 0) & ~1, sr);
                writeMS<C, MEM_DATA,Word>((reg.sp + 2) & ~1, pc >> 16);
            }
            break;

        case M68010:
        case M68020:

            if constexpr (MIMIC_MUSASHI) {

                push<C, Word>(nr << 2);
                push<C, Long>(pc);
                push<C, Word>(sr);

            } else {

                reg.sp -= 8;
                writeMS<C, MEM_DATA, Word>((reg.sp + 6) & ~1, 4 * nr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS<C, MEM_DATA, Word>((reg.sp + 0) & ~1, sr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 2) & ~1, pc >> 16);
            }
            break;
    }
}

template <Core C> void
Moira::writeStackFrame0001(u16 sr, u32 pc, u16 nr)
{
    assert(C == M68020);

    push<C, Word>(0x1000 | nr << 2);
    push<C, Long>(pc);
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame0010(u16 sr, u32 pc, u32 ia, u16 nr)
{
    assert(C == M68020);

    push<C, Long>(ia);
    push<C, Word>(0x2000 | nr << 2);
    push<C, Long>(pc);
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame1000(u16 sr, u32 pc, u16 nr)
{

}

template <Core C> void
Moira::writeStackFrame1001(u16 sr, u32 pc, u32 ia, u16 nr)
{

}

template <Core C> void
Moira::writeStackFrame1010(u16 sr, u32 pc, u16 nr){

}

template <Core C> void
Moira::writeStackFrame1011(u16 sr, u32 pc, u16 nr)
{

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

    // Call the delegate
    willExecute(EXC_ADDRESS_ERROR, 3);

    // Emulate additional delay
    sync(delay);
    
    // Enter supervisor mode
    setSupervisorMode(true);
    
    // Disable tracing
    clearTraceFlags();
    flags &= ~CPU_TRACE_EXCEPTION;
    SYNC(8);

    // A misaligned stack pointer will cause a "double fault"
    bool doubleFault = misaligned<C>(reg.sp);

    if (!doubleFault) {
        
        // Write stack frame
        saveToStack0(frame);
        SYNC(2);
        jumpToVector<C>(3);
    }

    // Halt the CPU if a double fault occurred
    if (doubleFault) halt();

    // Call the delegate
    didExecute(EXC_ADDRESS_ERROR, 3);
}

void
Moira::execException(ExceptionType exc, int nr)
{
    switch (core) {

        case M68000: execException<M68000>(exc, nr); break;
        case M68010: execException<M68010>(exc, nr); break;
        case M68020: execException<M68020>(exc, nr); break;

        default:
            assert(false);
    }
}

template <Core C> void
Moira::execException(ExceptionType exc, int nr)
{
    u16 status = getSR();

    // Determine the exception vector number
    u16 vector = u16(exc) + (exc == EXC_TRAP ? nr : 0);

    // Call the delegate
    willExecute(exc, vector);

    switch (exc) {

        case EXC_ILLEGAL:
        case EXC_LINEA:
        case EXC_LINEF:

            // Enter supervisor mode
            setSupervisorMode(true);

            // Disable tracing
            clearTraceFlags();
            flags &= ~CPU_TRACE_EXCEPTION;

            // Write stack frame
            SYNC(4);
            saveToStack1<C>(vector, status, reg.pc - 2);

            jumpToVector<C, AE_SET_CB3>(vector);
            break;

        case EXC_DIVIDE_BY_ZERO:
        case EXC_CHK:
        case EXC_TRAPV:

            // Enter supervisor mode
            setSupervisorMode(true);

            // Disable tracing, but keep the CPU_TRACE_EXCEPTION flag
            clearTraceFlags();

            // Write exception information to stack
            saveToStack2<C>(u16(vector), status, reg.pc);

            jumpToVector<C>(vector);
            break;

        case EXC_PRIVILEGE_VIOLATION:

            // Enter supervisor mode
            setSupervisorMode(true);

            // Disable tracing
            clearTraceFlags();
            flags &= ~CPU_TRACE_EXCEPTION;

            // Write exception information to stack
            SYNC(4);
            saveToStack1<C>(vector, status, reg.pc - 2);

            jumpToVector<C,AE_SET_CB3>(vector);
            break;

        case EXC_TRACE:

            // Recover from stop state
            flags &= ~CPU_IS_STOPPED;

            // Enter supervisor mode
            setSupervisorMode(true);

            // Disable tracing
            clearTraceFlags();
            flags &= ~CPU_TRACE_EXCEPTION;

            // Write stack frame
            SYNC(4);
            saveToStack1<C>(vector, status, reg.pc);

            jumpToVector<C>(vector);
            break;

        case EXC_FORMAT_ERROR:

            // Enter supervisor mode
            setSupervisorMode(true);

            // Disable tracing
            clearTraceFlags();
            flags &= ~CPU_TRACE_EXCEPTION;

            // Write stack frame
            SYNC(4);
            saveToStack1(14, status, reg.pc);

            jumpToVector<C, AE_SET_CB3>(vector);
            break;

        case EXC_TRAP:

            // Enter supervisor mode
            setSupervisorMode(true);

            // Disable tracing, but keep the CPU_TRACE_EXCEPTION flag
            clearTraceFlags();

            // Write stack frame
            writeStackFrame0000<C>(status, reg.pc, u16(vector));

            jumpToVector<C>(vector);
            break;
            
        default:
            break;
    }

    // Call the delegate
    didExecute(exc, vector);
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
    clearTraceFlags();
    flags &= ~CPU_TRACE_EXCEPTION;

    // Write exception information to stack
    SYNC(4);
    saveToStack1<C>(u16(nr), status, reg.pc - 2);

    jumpToVector<C,AE_SET_CB3>(nr);
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
    clearTraceFlags();
    flags &= ~CPU_TRACE_EXCEPTION;

    if constexpr (C == M68000) {

        SYNC(6);
        reg.sp -= 6;
        writeMS<C, MEM_DATA, Word>(reg.sp + 4, reg.pc & 0xFFFF);

        SYNC(4);
        queue.ird = getIrqVector(level);

        SYNC(4);
        writeMS<C, MEM_DATA, Word>(reg.sp + 0, status);
        writeMS<C, MEM_DATA, Word>(reg.sp + 2, reg.pc >> 16);
    }

    if constexpr (C == M68010) {

        SYNC(6);
        reg.sp -= 8;
        writeMS<C, MEM_DATA, Word>(reg.sp + 4, reg.pc & 0xFFFF);

        SYNC(4);
        queue.ird = getIrqVector(level);

        SYNC(4);
        writeMS<C, MEM_DATA, Word>(reg.sp + 0, status);
        writeMS<C, MEM_DATA, Word>(reg.sp + 2, reg.pc >> 16);

        writeMS<C, MEM_DATA, Word>(reg.sp + 6, 4 * queue.ird);
    }

    jumpToVector<C, AE_SET_CB3>(queue.ird);
}
