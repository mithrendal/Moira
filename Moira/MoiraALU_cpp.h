// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

template<Size S> u32 MSBIT() {
    if (S == Byte) return 0x00000080;
    if (S == Word) return 0x00008000;
    if (S == Long) return 0x80000000;
}

template<Size S> u32 CLIP(u64 data) {
    if (S == Byte) return data & 0x000000FF;
    if (S == Word) return data & 0x0000FFFF;
    if (S == Long) return data & 0xFFFFFFFF;
}

template<Size S> u32 CLEAR(u64 data) {
    if (S == Byte) return data & 0xFFFFFF00;
    if (S == Word) return data & 0xFFFF0000;
    if (S == Long) return data & 0x00000000;
}

template<Size S> i32 SEXT(u64 data) {
    if (S == Byte) return (i8)data;
    if (S == Word) return (i16)data;
    if (S == Long) return (i32)data;
}

template<Size S> bool NBIT(u64 data) {
    if (S == Byte) return data & 0x00000080;
    if (S == Word) return data & 0x00008000;
    if (S == Long) return data & 0x80000000;
}

template<Size S> bool CARRY(u64 data) {
    if (S == Byte) return data & 0x000000100;
    if (S == Word) return data & 0x000010000;
    if (S == Long) return data & 0x100000000;
}

template<Size S> bool ZERO(u64 data) {
    if (S == Byte) return !(data & 0x000000FF);
    if (S == Word) return !(data & 0x0000FFFF);
    if (S == Long) return !(data & 0xFFFFFFFF);
}

template<Size S> u32 WRITE(u32 d1, u32 d2) {
    if (S == Byte) return (d1 & 0xFFFFFF00) | (d2 & 0x000000FF);
    if (S == Word) return (d1 & 0xFFFF0000) | (d2 & 0x0000FFFF);
    if (S == Long) return d2;
}

template<Instr I, Size S> u32
Moira::shift(int cnt, u64 data) {

    switch(I) {
            
        case ASL:
        {
            bool carry = false;
            u32 changed = 0;
            for (int i = 0; i < cnt; i++) {
                carry = NBIT<S>(data);
                u64 shifted = data << 1;
                changed |= data ^ shifted;
                data = shifted;
            }
            if (cnt) sr.x = carry;
            sr.c = carry;
            sr.v = NBIT<S>(changed);
            break;
        }
        case ASR:
        {
            bool carry = false;
            u32 changed = 0;
            for (int i = 0; i < cnt; i++) {
                carry = data & 1;
                u64 shifted = SEXT<S>(data) >> 1;
                changed |= data ^ shifted;
                data = shifted;
            }
            if (cnt) sr.x = carry;
            sr.c = carry;
            sr.v = NBIT<S>(changed);
            break;
        }
        case LSL:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = NBIT<S>(data);
                data = data << 1;
            }
            if (cnt) sr.x = carry;
            sr.c = carry;
            sr.v = 0;
            break;
        }
        case LSR:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = data & 1;
                data = data >> 1;
            }
            if (cnt) sr.x = carry;
            sr.c = carry;
            sr.v = 0;
            break;
        }
        case ROL:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = NBIT<S>(data);
                data = data << 1 | carry;
            }
            sr.c = carry;
            sr.v = 0;
            break;
        }
        case ROR:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = data & 1;
                data >>= 1;
                if (carry) data |= MSBIT<S>();
            }
            sr.c = carry;
            sr.v = 0;
            break;
        }
        case ROXL:
        {
            bool carry = sr.x;
            for (int i = 0; i < cnt; i++) {
                bool extend = carry;
                carry = NBIT<S>(data);
                data = data << 1 | extend;
            }

            sr.x = carry;
            sr.c = carry;
            sr.v = 0;
            break;
        }
        case ROXR:
        {
            bool carry = sr.x;
            for (int i = 0; i < cnt; i++) {
                bool extend = carry;
                carry = data & 1;
                data >>= 1;
                if (extend) data |= MSBIT<S>();
            }

            sr.x = carry;
            sr.c = carry;
            sr.v = 0;
            break;
        }
        default:
        {
            assert(false);
        }
    }
    sr.n = NBIT<S>(data);
    sr.z = ZERO<S>(data);
    return CLIP<S>(data);
}

template<Instr I, Size S> u32
Moira::addsub(u32 op1, u32 op2)
{
    u64 result;

    switch(I) {

        case ADD:
        case ADDI:
        case ADDQ:
        {
            result = (u64)op1 + (u64)op2;

            sr.x = sr.c = CARRY<S>(result);
            sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            sr.z = ZERO<S>(result);
            break;
        }
        case ADDX:
        {
            result = (u64)op1 + (u64)op2 + (u64)sr.x;

            sr.x = sr.c = CARRY<S>(result);
            sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            if (CLIP<S>(result)) sr.z = 0;
            break;
        }
        case SUB:
        case SUBI:
        case SUBQ:
        {
            result = (u64)op2 - (u64)op1;

            sr.x = sr.c = CARRY<S>(result);
            sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            sr.z = ZERO<S>(result);
            break;
        }
        case SUBX:
        {
            result = (u64)op2 - (u64)op1 - (u64)sr.x;

            sr.x = sr.c = CARRY<S>(result);
            sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            if (CLIP<S>(result)) sr.z = 0;
            break;
        }
        default:
        {
            assert(false);
        }
    }
    sr.n = NBIT<S>(result);
    return (u32)result;
}

template <Instr I> u32
Moira::mul(u32 op1, u32 op2)
{
    u32 result;

    switch (I) {

         case MULS:
         {
             result = (i16)op1 * (i16)op2;
             break;
         }
         case MULU:
         {
             result = (u16)op1 * (u16)op2;
             break;
         }
     }

    sr.n = NBIT<Long>(result);
    sr.z = ZERO<Long>(result);
    sr.v = 0;
    sr.c = 0;

    sync(cyclesMul<I>(op1));
    return result;
}

template <Instr I> u32
Moira::div(u32 op1, u32 op2)
{
    u32 result;
    bool overflow;

    sr.n = sr.z = sr.v = sr.c = 0;

    switch (I) {

        case DIVS: // Signed division
        {
            i64 quotient  = (i64)(i32)op1 / (i16)op2;
            i16 remainder = (i64)(i32)op1 % (i16)op2;

            result = (quotient & 0xffff) | remainder << 16;
            overflow = ((quotient & 0xffff8000) != 0 &&
                        (quotient & 0xffff8000) != 0xffff8000);
            overflow |= op1 == 0x80000000 && (i16)op2 == -1;
            break;
        }
        case DIVU: // Unsigned division
        {
            i64 quotient  = op1 / op2;
            u16 remainder = op1 % op2;

            result = (quotient & 0xffff) | remainder << 16;
            overflow = quotient > 0xFFFF;
            break;
        }
    }
    sr.v   = overflow ? 1    : sr.v;
    sr.n   = overflow ? 1    : NBIT<Word>(result);
    sr.z   = overflow ? sr.z : ZERO<Word>(result);

    sync(cyclesDiv<I>(op1, op2) - 4);
    return overflow ? op1 : result;
}

template<Instr I, Size S> u32
Moira::bcd(u32 op1, u32 op2)
{
    u64 result;

    switch(I) {

        case ABCD:
        {
            // Extract nibbles
            u16 op1_hi = op1 & 0xF0, op1_lo = op1 & 0x0F;
            u16 op2_hi = op2 & 0xF0, op2_lo = op2 & 0x0F;

            // From portable68000
            u16 resLo = op1_lo + op2_lo + sr.x;
            u16 resHi = op1_hi + op2_hi;
            u64 tmp_result;
            result = tmp_result = resHi + resLo;
            if (resLo > 9) result += 6;
            sr.x = sr.c = (result & 0x3F0) > 0x90;
            if (sr.c) result += 0x60;
            if (CLIP<Byte>(result)) sr.z = 0;
            sr.n = NBIT<Byte>(result);
            sr.v = ((tmp_result & 0x80) == 0) && ((result & 0x80) == 0x80);
            break;
        }
        case SBCD:
        {
            // Extract nibbles
            u16 op1_hi = op1 & 0xF0, op1_lo = op1 & 0x0F;
            u16 op2_hi = op2 & 0xF0, op2_lo = op2 & 0x0F;

            // From portable68000
            u16 resLo = op2_lo - op1_lo - sr.x;
            u16 resHi = op2_hi - op1_hi;
            u64 tmp_result;
            result = tmp_result = resHi + resLo;
            int bcd = 0;
            if (resLo & 0xf0) {
                bcd = 6;
                result -= 6;
            }
            if (((op2 - op1 - sr.x) & 0x100) > 0xff) result -= 0x60;
            sr.c = sr.x = ((op2 - op1 - bcd - sr.x) & 0x300) > 0xff;

            if (CLIP<Byte>(result)) sr.z = 0;
            sr.n = NBIT<Byte>(result);
            sr.v = ((tmp_result & 0x80) == 0x80) && ((result & 0x80) == 0);
            break;
        }
        default:
        {
            assert(false);
        }
    }
    sr.n = NBIT<S>(result);
    return (u32)result;
}

template <Size S> void
Moira::cmp(u32 op1, u32 op2)
{
    u64 result = (u64)op2 - (u64)op1;

    sr.c = NBIT<S>(result >> 1);
    sr.v = NBIT<S>((op2 ^ op1) & (op2 ^ result));
    sr.z = ZERO<S>(result);
    sr.n = NBIT<S>(result);
}

template<Instr I, Size S> u32
Moira::logic(u32 op)
{
    u32 result;

    switch(I) {

        case NOT:
        {
            result = ~op;
            sr.n = NBIT<S>(result);
            sr.z = ZERO<S>(result);
            sr.v = 0;
            sr.c = 0;
            break;
        }
        case NEG:
        {
            result = addsub<SUB,S>(op, 0);
            break;
        }
        case NEGX:
        {
            result = addsub<SUBX,S>(op, 0);
            break;
        }
        default:
        {
            assert(false);
        }
    }
    return result;
}

template<Instr I, Size S> u32
Moira::logic(u32 op1, u32 op2)
{
    u32 result;

    switch(I) {

        case AND: case ANDI: case ANDICCR: case ANDISR:
        {
            result = op1 & op2;
            break;
        }
        case OR: case ORI: case ORICCR: case ORISR:
        {
            result = op1 | op2;
            break;
        }
        case EOR: case EORI: case EORICCR: case EORISR:
        {
            result = op1 ^ op2;
            break;
        }
        default:
        {
            assert(false);
        }
    }
    
    sr.n = NBIT<S>(result);
    sr.z = ZERO<S>(result);
    sr.v = 0;
    sr.c = 0;
    return result;
}

template <Instr I> u32
Moira::bit(u32 op, u8 bit)
{
    switch (I) {
        case BCHG:
        {
            sr.z = 1 ^ ((op >> bit) & 1);
            op ^= (1 << bit);
            break;
        }
        case BSET:
        {
            sr.z = 1 ^ ((op >> bit) & 1);
            op |= (1 << bit);
            break;
        }
        case BCLR:
        {
            sr.z = 1 ^ ((op >> bit) & 1);
            op &= ~(1 << bit);
            break;
        }
        case BTST:
        {
            sr.z = 1 ^ ((op >> bit) & 1);
            break;
        }
        default:
        {
            assert(false);
        }
    }
    return op;
}

template <Instr I> bool
Moira::cond() {

    switch(I) {

        case BRA: case DBT:  case ST:  return true;
        case DBF: case SF:             return false;
        case BHI: case DBHI: case SHI: return !sr.c && !sr.z;
        case BLS: case DBLS: case SLS: return sr.c || sr.z;
        case BCC: case DBCC: case SCC: return !sr.c;
        case BCS: case DBCS: case SCS: return sr.c;
        case BNE: case DBNE: case SNE: return !sr.z;
        case BEQ: case DBEQ: case SEQ: return sr.z;
        case BVC: case DBVC: case SVC: return !sr.v;
        case BVS: case DBVS: case SVS: return sr.v;
        case BPL: case DBPL: case SPL: return !sr.n;
        case BMI: case DBMI: case SMI: return sr.n;
        case BGE: case DBGE: case SGE: return sr.n == sr.v;
        case BLT: case DBLT: case SLT: return sr.n != sr.v;
        case BGT: case DBGT: case SGT: return sr.n == sr.v && !sr.z;
        case BLE: case DBLE: case SLE: return sr.n != sr.v || sr.z;
    }
}

template <Instr I> int
Moira::cyclesBit(u8 bit)
{
    switch (I)
    {
        case BTST: return 2;
        case BCLR: return (bit > 15 ? 6 : 4);
        case BSET:
        case BCHG: return (bit > 15 ? 4 : 2);
    }

    assert(false);
    return 0;
}

template <Instr I> int
Moira::cyclesMul(u16 data)
{
    int mcycles = 17;

    switch (I)
    {
        case MULU:
        {
            for (; data; data >>= 1) if (data & 1) mcycles++;
            return 2 * mcycles;
        }
        case MULS:
        {
            data = ((data << 1) ^ data) & 0xFFFF;
            for (; data; data >>= 1) if (data & 1) mcycles++;
            return 2 * mcycles;
        }
    }
}

template <Instr I> int
Moira::cyclesDiv(u32 op1, u16 op2)
{
    switch (I)
    {
        case DIVU:
        {
            u32 dividend = op1;
            u16 divisor  = op2;
            int mcycles  = 38;

            // Check if quotient is larger than 16 bit
            if ((dividend >> 16) >= divisor) return 10;
            u32 hdivisor = divisor << 16;

            for (int i = 0; i < 15; i++) {
                if ((i32)dividend < 0) {
                    dividend <<= 1;
                    dividend -= hdivisor;
                } else {
                    dividend <<= 1;
                    if (dividend >= hdivisor) {
                        dividend -= hdivisor;
                        mcycles += 1;
                    } else {
                        mcycles += 2;
                    }
                }
            }
            return 2 * mcycles;
        }
        case DIVS:
        {
            i32 dividend = (i32)op1;
            i16 divisor  = (i16)op2;
            int mcycles  = (dividend < 0) ? 7 : 6;

            // Check if quotient is larger than 16 bit
            if ((abs(dividend) >> 16) >= abs(divisor))
                return (mcycles + 2) * 2;

            mcycles += 55;

            if (divisor >= 0) {
                mcycles += (dividend < 0) ? 1 : -1;
            }

            u32 aquot = abs(dividend) / abs(divisor);
            for (int i = 0; i < 15; i++) {
                if ( (i16)aquot >= 0) mcycles++;
                aquot <<= 1;
            }
            return 2 * mcycles;
        }
    }
}