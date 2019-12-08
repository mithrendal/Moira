// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

template <Size S> bool
CPU::addressError(u32 addr)
{
#ifdef MOIRA_EMULATE_ADDRESS_ERROR
    if (S != Byte && addr & 1) {
        execAddressError(addr);
        return true;
    }
#endif
    return false;
}

template<> u32
CPU::read<Byte>(u32 addr)
{
    // printf("read<Byte>(%x)\n", addr);
    return memory->moiraRead8(addr);
}

template<> u32
CPU::read<Word>(u32 addr)
{
    // printf("read<Word>(%x)\n", addr);
    /*
    if (addr & 1) {
        execAddressError(addr);
    }
    */
    return memory->moiraRead16(addr);
}

template<> u32
CPU::read<Long>(u32 addr)
{
    // printf("read<Long>(%x)\n", addr);
    return memory->moiraRead16(addr) << 16 | memory->moiraRead16(addr + 2);
}

template<> void
CPU::write<Byte>(u32 addr, u32 value)
{
    memory->moiraWrite8(addr & 0xFFFFFF, (u8)value);
}

template<> void
CPU::write<Word>(u32 addr, u32 value)
{
    memory->moiraWrite16(addr & 0xFFFFFF, (u16)value);
}

template<> void
CPU::write<Long>(u32 addr, u32 value)
{
    memory->moiraWrite16(addr & 0xFFFFFF, (u16)(value >> 16));
    memory->moiraWrite16((addr + 2) & 0xFFFFFF, (u16)value);
}

template<Mode M, Size S> u32
CPU::computeEA(u32 n, u32 dis, u32 idx) {

    u32 result;

    assert(n < 8);
    
    switch (M) {
            
        case 0: // Dn
        {
            assert(false);
            break;
        }
        case 1: // An
        {
            assert(false);
            break;
        }
        case 2: // (An)
        {
            result = readA(n);
            break;
        }
        case 3: // (An)+
        {
            u32 an = readA(n);
            bool addressError = (S != Byte) && (an & 1);
            u32 offset = (n == 7 && S == Byte) ? 2 : BYTES<S>();
            result = an;
            if (!addressError) writeA(n, an + offset);
            break;
        }
        case 4: // -(An)
        {
            u32 an = readA(n);
            bool addressError = (S != Byte) && (an & 1);
            u32 offset = (n == 7 && S == Byte) ? 2 : BYTES<S>();
            result = an - offset;
            if (!addressError) writeA(n, an - offset);
            break;
        }
        case 5: // (d,An)
        {
            u32 an = readA(n);
            i16  d = (i16)irc;

            result = d + an;
            readExtensionWord();
            break;
        }
        case 6: // (d,An,Xi)
        {
            i8   d = (i8)irc;
            i32 an = readA(n);
            i32 xi = readR((irc >> 12) & 0b1111);

            result = d + an + ((irc & 0x800) ? xi : (i16)xi);
            readExtensionWord();
            break;
        }
        case 7: // ABS.W
        {
            result = irc;
            readExtensionWord();
            break;
        }
        case 8: // ABS.L
        {
            result = irc << 16;
            readExtensionWord();
            result |= irc;
            readExtensionWord();
            break;
        }
        case 9: // (d,PC)
        {
            i16  d = (i16)irc;

            result = pc + d;
            readExtensionWord();
            break;
        }
        case 10: // (d,PC,Xi)
        {
            i8   d = (i8)irc;
            i32 xi = readR((irc >> 12) & 0b1111);

            result = d + pc + ((irc & 0x800) ? xi : (i16)xi);
            readExtensionWord();
            break;
        }
        case 11: // Imm
        {
            assert(false);
            return 0;
        }
        default:
            assert(false);
    }

    return result;
}

template<Mode M, Size S> bool
CPU::readOperand(int n, u32 &ea, u32 &result)
{
    switch (M) {

        case 0: // Dn
        {
            result = readD<S>(n);
            break;
        }
        case 1: // An
        {
            result = readA<S>(n);
            break;
        }
        case 2: // (An)
        {
            ea = readA(n);
            if (addressError<S>(ea)) return false;

            result = read<S>(ea);
            break;
        }
        case 3: // (An)+
        {
            ea = readA(n);
            if (addressError<S>(ea)) return false;
            result = read<S>(ea);
            ea += ((n == 7 && S == Byte) ? 2 : S);
            writeA(n, ea);
            break;
        }
        case 4: // -(An)
        {
            ea = readA(n);
            if (addressError<S>(ea)) return false;
            ea -= ((n == 7 && S == Byte) ? 2 : S);
            writeA(n, ea);
            result = read<S>(ea);
            break;
        }
        case 5: // (d,An)
        {
            i16 d = (i16)irc;
            ea = readA(n);
            result = ea + d;
            readExtensionWord();
            break;
        }
        case 6: // (d,An,Xi)
        {
            i8 d = (i8)irc;
            i32 xi = readR((irc >> 12) & 0b1111);
            ea = readA(n) + d + ((irc & 0x800) ? xi : (i16)xi);
            result = read<S>(ea);
            readExtensionWord();
            break;
        }
        case 7: // ABS.W
        {
            ea = irc;
            readExtensionWord();
            result = read<S>(ea);
            break;
        }
        case 8: // ABS.L
        {
            ea = irc << 16;
            readExtensionWord();
            ea |= irc;
            readExtensionWord();
            result = read<S>(ea);
            break;
        }
        case 9: // (d,PC)
        {
            i16 d = (i16)irc;
            result = pc + d;
            readExtensionWord();
            break;
        }
        case 10: // (d,PC,Xi)
        {
            i8  d = (i8)irc;
            i32 xi = readR((irc >> 12) & 0b1111);
            ea = pc + d + ((irc & 0x800) ? xi : (i16)xi);
            result = read<S>(ea);
            readExtensionWord();
            break;
        }
        case 11: // Imm
        {
            result = readImm<S>();
            break;
        }
        default:
            assert(false);
    }

    return true;
}

template<Mode M, Size S> void
CPU::writeOperand(int n, u32 ea, u32 value)
{
    assert(M < 11);

    switch (M) {

        case 0: // Dn
        {
            writeD<S>(n, value);
            break;
        }
        case 1: // An
        {
            writeA<S>(n, value);
            break;
        }
        default:
        {
            write<S>(ea, value);
            break;
        }
    }
}

template<Size S> u32
CPU::readImm()
{
    u32 result;

    switch (S) {
        case Byte:
            // printf("readImm (Byte)\n");
            result = (u8)irc;
            readExtensionWord();
            break;
        case Word:
            result = irc;
            // printf("readImm (Word) irc = %x\n", irc);
            readExtensionWord();
            break;
        case Long:
            // printf("readImm (Long)\n");
            result = irc << 16;
            readExtensionWord();
            result |= irc;
            readExtensionWord();
            break;
    }

    return result;
}

template<Instr I, Size S> u32
CPU::shift(int cnt, u64 data) {

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
                u64 shifted = SIGN<S>(data) >> 1;
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
CPU::arith(u32 op1, u32 op2)
{
    u64 result;

    switch(I) {

        case ADD:   // Numeric addition
        {
            result = (u64)op1 + (u64)op2;

            sr.x = sr.c = CARRY<S>(result);
            sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            sr.z = ZERO<S>(result);
            break;
        }
        case ADDX:  // Numeric addition with X flag
        {
            result = (u64)op1 + (u64)op2 + (u64)sr.x;

            sr.x = sr.c = CARRY<S>(result);
            sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            if (CLIP<S>(result)) sr.z = 0;
            break;
        }
        case ABCD:  // BCD addition
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
        case SUB:   // Numeric subtraction
        {
            result = (u64)op2 - (u64)op1;
            // printf("arith::SUB %x %x %llx \n", op1, op2, result);

            sr.x = sr.c = CARRY<S>(result);
            sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            sr.z = ZERO<S>(result);
            break;
        }
        case SUBX:  // Numeric subtraction with X flag
        {
            result = (u64)op2 - (u64)op1 - (u64)sr.x;

            sr.x = sr.c = CARRY<S>(result);
            sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            if (CLIP<S>(result)) sr.z = 0;
            break;
        }
        case SBCD:  // BCD subtraction
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

            printf("SBCD result = %llx c: %d x: %d\n", result, sr.c, sr.x);
            if (CLIP<Byte>(result)) sr.z = 0;
            sr.n = NBIT<Byte>(result);
            sr.v = ((tmp_result & 0x80) == 0x80) && ((result & 0x80) == 0);
            break;
        }
        case CMP:
        {
            result = (u64)op1 - (u64)op2;

            sr.c = CARRY<S>(result);
            sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            sr.z = ZERO<S>(result);
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

template<Instr I, Size S> u32
CPU::logic(u32 op)
{
    u32 result;

    switch(I) {

        case NOT:
        {
            result = ~op;
            break;
        }
        default:
        {
            assert(false);
        }
    }

    sr.c = 0;
    sr.v = 0;
    sr.n = NBIT<S>(result);
    sr.z = ZERO<S>(result);

    return result;
}

template<Instr I, Size S> u32
CPU::logic(u32 op1, u32 op2)
{
    u32 result;

    switch(I) {

        case AND:
        {
            result = op1 & op2;
            break;
        }
        case OR:
        {
            result = op1 | op2;
            break;
        }
        case EOR:
        {
            result = op1 ^ op2;
            break;
        }
        default:
        {
            assert(false);
        }
    }

    sr.c = 0;
    sr.v = 0;
    sr.n = NBIT<S>(result);
    sr.z = ZERO<S>(result);

    return result;
}

template <Instr I> u32
CPU::bitop(u32 op, u8 bit)
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

template <Size S> void
CPU::cmp(u32 op1, u32 op2)
{
    u64 result = (u64)op2 - (u64)op1;

    sr.c = NBIT<S>(result >> 1);
    sr.v = NBIT<S>((op2 ^ op1) & (op2 ^ result));
    sr.z = ZERO<S>(result);
    sr.n = NBIT<S>(result);
}

template <Instr I> bool
CPU::bcond() {

    switch(I) {

        case BT:  case DBT:  case ST:  return true;
        case BF:  case DBF:  case SF:  return false;
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
