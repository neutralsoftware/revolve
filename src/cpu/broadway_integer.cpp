#include "cpu/broadway.h"
#include <bit>
#include <limits>

void Broadway::executeADD(uint32_t rt, uint32_t ra, uint32_t rb, bool oe,
                          bool rc) {
    executeXOType((31u << 26) | (rt << 21) | (ra << 16) | (rb << 11) |
                  (static_cast<uint32_t>(oe) << 10) | (266 << 1) | rc);
}

void Broadway::executeXOType(uint32_t instruction) {
    uint32_t d = (instruction >> 21) & 31;
    uint32_t a = state.gpr[(instruction >> 16) & 31];
    uint32_t b = state.gpr[(instruction >> 11) & 31];
    uint32_t xo = (instruction >> 1) & 511;
    int64_t sa = static_cast<int32_t>(a);
    int64_t sb = static_cast<int32_t>(b);
    uint64_t result = 0;
    int64_t signedResult = 0;
    bool carry = state.getCA();
    bool setCarry = false;
    bool overflow = false;
    switch (xo) {
    case 266:
    case 10:
    case 138:
    case 234:
    case 202: {
        uint32_t operand = xo == 234 ? 0xFFFFFFFFu : xo == 202 ? 0 : b;
        uint32_t extra = (xo == 138 || xo == 234 || xo == 202) ? carry : 0;
        result = static_cast<uint64_t>(a) + operand + extra;
        signedResult = sa + static_cast<int32_t>(operand) + extra;
        setCarry = xo != 266;
        break;
    }
    case 40:
    case 8:
    case 136:
    case 232:
    case 200: {
        uint32_t operand = xo == 232 ? 0xFFFFFFFFu : xo == 200 ? 0 : b;
        uint32_t extra = (xo == 40 || xo == 8) ? 1 : carry;
        result = static_cast<uint64_t>(~a) + operand + extra;
        signedResult = -sa + static_cast<int32_t>(operand) + extra - 1;
        setCarry = xo != 40;
        break;
    }
    case 104:
        result = 0u - a;
        signedResult = -sa;
        break;
    case 235:
        signedResult = sa * sb;
        result = static_cast<uint64_t>(signedResult);
        break;
    case 75:
        result = static_cast<uint64_t>(sa * sb) >> 32;
        break;
    case 11:
        result = (static_cast<uint64_t>(a) * b) >> 32;
        break;
    case 491: {
        overflow = b == 0 || (a == 0x80000000u && b == 0xFFFFFFFFu);

        if (overflow) {
            result = static_cast<int32_t>(a) < 0 ? 0xFFFFFFFFu : 0u;
        } else {
            result = static_cast<uint32_t>(static_cast<int32_t>(a) /
                                           static_cast<int32_t>(b));
        }
        break;
    }
    case 459:
        overflow = b == 0;
        result = overflow ? 0 : a / b;
        break;
    default:
        raiseException(0x700, 0x80000);
        return;
    }
    state.gpr[d] = static_cast<uint32_t>(result);
    if (setCarry)
        state.setCA((result >> 32) != 0);
    if ((instruction & 0x400) && xo != 75 && xo != 11) {
        overflow |= signedResult > std::numeric_limits<int32_t>::max() ||
                    signedResult < std::numeric_limits<int32_t>::min();
        state.setOV(overflow);
        if (overflow)
            state.setSO(true);
    }
    if (instruction & 1)
        state.updateCR0(state.gpr[d]);
}

void Broadway::executeXFXType(uint32_t instruction) {
    uint32_t d = (instruction >> 21) & 31;
    uint32_t xo = (instruction >> 1) & 1023;
    uint32_t spr = ((instruction >> 16) & 31) | ((instruction >> 6) & 992);
    if ((state.msr & 0x4000) && (xo == 339 || xo == 467) &&
        spr != SPR::XER && spr != SPR::LR && spr != SPR::CTR &&
        !(xo == 339 && (spr == SPR::TBL || spr == SPR::TBU))) {
        raiseException(0x700, 0x40000);
        return;
    }
    switch (xo) {
    case 19:
        state.gpr[d] = state.cr;
        break;
    case 144:
        for (uint32_t i = 0; i < 8; ++i) {
            if (instruction & (1u << (19 - i)))
                state.setCRField(i, state.gpr[d] >> (28 - i * 4));
        }
        break;
    case 339:
        state.gpr[d] = readSPR(spr);
        break;
    case 371: {
        uint32_t tbr = spr;

        if (tbr != SPR::TBL && tbr != SPR::TBU) {
            raiseException(0x700, 0x80000);
            return;
        }

        if (tbr == SPR::TBL) {
            state.gpr[d] = static_cast<uint32_t>(state.timeBase);
        } else {
            state.gpr[d] = static_cast<uint32_t>(state.timeBase >> 32);
        }

        break;
    }
    case 467: {
        if (spr == SPR::PVR)
            return;
        const uint32_t oldValue = state.spr[spr];
        spr = spr == 284 ? 268 : spr == 285 ? 269 : spr;
        state.spr[spr] = state.gpr[d];
        if (spr == SPR::HID0)
            state.spr[spr] &= ~0x00000C00u;
        if (spr == SPR::SDR1 ||
            (spr >= SPR::IBAT0U && spr <= SPR::DBAT3L) ||
            (spr >= SPR::IBAT4U && spr <= SPR::DBAT7L))
            invalidateTranslationCache();
        if (spr == SPR::DEC) {
            if (!(oldValue & 0x80000000u) && (state.gpr[d] & 0x80000000u))
                state.decrementerPending = true;
        } else if (spr == SPR::TBL)
            state.timeBase =
                (state.timeBase & 0xFFFFFFFF00000000ull) | state.gpr[d];
        else if (spr == SPR::TBU)
            state.timeBase = (state.timeBase & 0xFFFFFFFFull) |
                             (static_cast<uint64_t>(state.gpr[d]) << 32);
        break;
    }
    default:
        raiseException(0x700, 0x80000);
        break;
    }
}

void Broadway::executeXLType(uint32_t instruction) {
    uint32_t d = (instruction >> 21) & 31;
    uint32_t a = (instruction >> 16) & 31;
    uint32_t b = (instruction >> 11) & 31;
    uint32_t xo = (instruction >> 1) & 1023;
    bool av = (state.cr >> (31 - a)) & 1;
    bool bv = (state.cr >> (31 - b)) & 1;
    bool result = false;
    switch (xo) {
    case 0:
        state.setCRField(d >> 2, state.getCRField(a >> 2));
        return;
    case 16:
    case 528: {
        if (xo == 528 && !(d & 4)) {
            raiseException(0x700, 0x80000);
            return;
        }
        uint32_t target = state.spr[xo == 16 ? SPR::LR : SPR::CTR] & ~3u;
        if (branchCondition(d, a, xo == 16))
            state.nia = target;
        if (instruction & 1)
            state.spr[SPR::LR] = state.cia + 4;
        return;
    }
    case 50:
        if (state.msr & 0x4000) {
            raiseException(0x700, 0x40000);
            return;
        }
        state.msr =
            (state.msr & ~0x87C0FFFFu) | (state.spr[SPR::SRR1] & 0x87C0FFFFu);
        state.msr &= ~0x40000u;
        state.nia = state.spr[SPR::SRR0] & ~3u;
        return;
    case 150:
        return;
    case 257:
        result = av && bv;
        break;
    case 129:
        result = av && !bv;
        break;
    case 289:
        result = av == bv;
        break;
    case 225:
        result = !(av && bv);
        break;
    case 33:
        result = !(av || bv);
        break;
    case 449:
        result = av || bv;
        break;
    case 417:
        result = av || !bv;
        break;
    case 193:
        result = av != bv;
        break;
    default:
        raiseException(0x700, 0x80000);
        return;
    }
    uint32_t mask = 1u << (31 - d);
    state.cr = (state.cr & ~mask) | (result ? mask : 0);
}

void Broadway::executeXType(uint32_t instruction) {
    if ((instruction >> 26) != 31) {
        raiseException(0x700, 0x80000);
        return;
    }
    uint32_t d = (instruction >> 21) & 31;
    uint32_t a = (instruction >> 16) & 31;
    uint32_t b = (instruction >> 11) & 31;
    uint32_t xo = (instruction >> 1) & 1023;
    uint32_t source = state.gpr[d];
    uint32_t operand = state.gpr[b];
    uint32_t address = (a == 0 ? 0 : state.gpr[a]) + operand;
    uint32_t result = 0;
    switch (xo) {
    case 28:
        result = source & operand;
        break;
    case 60:
        result = source & ~operand;
        break;
    case 444:
        result = source | operand;
        break;
    case 124:
        result = ~(source | operand);
        break;
    case 316:
        result = source ^ operand;
        break;
    case 412:
        result = source | ~operand;
        break;
    case 476:
        result = ~(source & operand);
        break;
    case 284:
        result = ~(source ^ operand);
        break;
    case 26:
        result = std::countl_zero(source);
        break;
    case 922:
        result = signExtend(source & 0xFFFF, 16);
        break;
    case 954:
        result = signExtend(source & 0xFF, 8);
        break;
    case 24:
        result = (operand & 32) ? 0 : source << (operand & 31);
        break;
    case 536:
        result = (operand & 32) ? 0 : source >> (operand & 31);
        break;
    case 792:
    case 824: {
        uint32_t shift = xo == 824 ? b : operand & 63;
        int32_t signedSource = static_cast<int32_t>(source);
        result =
            static_cast<uint32_t>(signedSource >> (shift > 31 ? 31 : shift));
        uint32_t mask = shift >= 32 ? 0xFFFFFFFFu : (1u << shift) - 1;
        state.setCA(signedSource < 0 && (source & mask) != 0);
        break;
    }
    case 0:
    case 32: {
        int64_t left = xo == 0 ? static_cast<int32_t>(state.gpr[a])
                               : static_cast<int64_t>(state.gpr[a]);
        int64_t right = xo == 0 ? static_cast<int32_t>(operand)
                                : static_cast<int64_t>(operand);
        state.setCRField(d >> 2, (left < right   ? 8u
                                  : left > right ? 4u
                                                 : 2u) |
                                     state.getSO());
        return;
    }
    case 4: {
        int32_t left = static_cast<int32_t>(state.gpr[a]);
        int32_t right = static_cast<int32_t>(operand);
        if (((d & 16) && left < right) || ((d & 8) && left > right) ||
            ((d & 4) && left == right) || ((d & 2) && state.gpr[a] < operand) ||
            ((d & 1) && state.gpr[a] > operand))
            raiseException(0x700, 0x20000);
        return;
    }
    case 512:
        state.setCRField(d >> 2, state.spr[SPR::XER] >> 28);
        state.spr[SPR::XER] &= ~(XER_SO | XER_OV | XER_CA);
        return;
    case 83:
    case 146:
    case 210:
    case 242:
    case 595:
    case 659:
    case 306:
    case 566:
    case 470:
        if (state.msr & 0x4000) {
            raiseException(0x700, 0x40000);
            return;
        }
        switch (xo) {
        case 83:
            state.gpr[d] = state.msr;
            break;
        case 146:
            state.msr = source;
            break;
        case 210:
            state.sr[a & 15] = source;
            invalidateTranslationCache();
            break;
        case 242:
            state.sr[operand >> 28] = source;
            invalidateTranslationCache();
            break;
        case 595:
            state.gpr[d] = state.sr[a & 15];
            break;
        case 659:
            state.gpr[d] = state.sr[operand >> 28];
            break;
        case 306:
            invalidateTranslationPage(operand);
            break;
        }
        return;
    case 54:
    case 86:
    case 246:
    case 278:
    case 758:
    case 598:
    case 854:
        return;
    case 982:
        invalidateTranslationPage(address);
        return;
    case 1014:
        for (uint32_t i = 0; i < 32; i += 4)
            Bus::write32((address & ~31u) + i, 0);
        return;
    case 20:
        if (address & 3) {
            state.spr[SPR::DAR] = address;
            state.spr[SPR::DSISR] = 0;
            raiseException(0x600);
            return;
        }
        state.gpr[d] = Bus::read32(address);
        state.reservationAddress = translateAddress(address, MemoryAccess::Read);
        state.reservationValid = true;
        return;
    case 150: {
        if (address & 3) {
            state.reservationValid = false;
            state.spr[SPR::DAR] = address;
            state.spr[SPR::DSISR] = 0x02000000;
            raiseException(0x600);
            return;
        }
        const uint32_t physical = translateAddress(address, MemoryAccess::Write);
        const bool success = state.reservationValid &&
                             state.reservationAddress == physical;
        state.reservationValid = false;
        if (success)
            Bus::write32(address, source);
        state.setCRField(0, (success ? 2u : 0u) | state.getSO());
        return;
    }
    case 534:
        state.gpr[d] = __builtin_bswap32(Bus::read32(address));
        return;
    case 790:
        state.gpr[d] = __builtin_bswap16(Bus::read16(address));
        return;
    case 662:
        Bus::write32(address, __builtin_bswap32(source));
        return;
    case 918:
        Bus::write16(address, __builtin_bswap16(static_cast<uint16_t>(source)));
        return;
    case 983:
        Bus::write32(address, static_cast<uint32_t>(state.fpr[d]));
        return;
    case 310:
    case 438:
        if (address & 3) {
            state.spr[SPR::DAR] = address;
            state.spr[SPR::DSISR] = xo == 438 ? 0x02000000 : 0;
            raiseException(0x600);
            return;
        }
        if (!(state.spr[282] & 0x80000000u)) {
            state.spr[SPR::DAR] = address;
            state.spr[SPR::DSISR] = 0x00100000u | (xo == 438 ? 0x02000000u : 0);
            raiseException(0x300);
            return;
        }
        if (xo == 310)
            state.gpr[d] = Bus::read32(address);
        else
            Bus::write32(address, source);
        return;
    case 533:
    case 597:
    case 661:
    case 725: {
        bool immediate = xo == 597 || xo == 725;
        bool load = xo == 533 || xo == 597;
        uint32_t count =
            immediate ? (b == 0 ? 32 : b) : state.spr[SPR::XER] & XER_BC_MASK;
        if (immediate)
            address = a == 0 ? 0 : state.gpr[a];
        for (uint32_t i = 0; i < count; ++i) {
            uint32_t reg = (d + i / 4) & 31;
            uint32_t shift = 24 - (i & 3) * 8;
            if (load) {
                if (!(i & 3))
                    state.gpr[reg] = 0;
                state.gpr[reg] |= static_cast<uint32_t>(Bus::read8(address + i))
                                  << shift;
            } else {
                Bus::write8(address + i,
                            static_cast<uint8_t>(state.gpr[reg] >> shift));
            }
        }
        return;
    }
    case 23:
        executeLWZ(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 55:
        executeLWZU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 87:
        executeLBZ(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 119:
        executeLBZU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 279:
        executeLHZ(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 311:
        executeLHZU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 343:
        executeLHA(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 375:
        executeLHAU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 151:
        executeSTW(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 183:
        executeSTWU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 215:
        executeSTB(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 247:
        executeSTBU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 407:
        executeSTH(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 439:
        executeSTHU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 535:
        executeLFS(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 567:
        executeLFSU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 599:
        executeLFD(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 631:
        executeLFDU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 663:
        executeSTFS(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 695:
        executeSTFSU(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 727:
        executeSTFD(d, a, operand, static_cast<int32_t>(operand));
        return;
    case 759:
        executeSTFDU(d, a, operand, static_cast<int32_t>(operand));
        return;
    default:
        raiseException(0x700, 0x80000);
        return;
    }
    state.gpr[a] = result;
    if (instruction & 1)
        state.updateCR0(result);
}
