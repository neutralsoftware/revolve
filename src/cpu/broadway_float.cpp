#include "cpu/broadway.h"
#include <algorithm>
#include <bit>
#include <cfenv>
#include <cmath>
#include <limits>

#pragma STDC FENV_ACCESS ON

namespace {
constexpr uint32_t FPSCR_FX = 0x80000000u;
constexpr uint32_t FPSCR_VXSNAN = 0x01000000u;
constexpr uint32_t FPSCR_VXVC = 0x00080000u;
constexpr uint32_t FPSCR_VXCVI = 0x00000100u;

bool signalingNaN(uint64_t bits) {
    return (bits & 0x7FF0000000000000ull) == 0x7FF0000000000000ull &&
           (bits & 0x000FFFFFFFFFFFFFull) != 0 &&
           !(bits & 0x0008000000000000ull);
}

void refreshFPSCR(BroadwayState &state) {
    bool invalid = (state.fpscr & 0x01F80700u) != 0;
    state.fpscr = (state.fpscr & ~0x20000000u) | (invalid ? 0x20000000u : 0);
    bool enabled = ((state.fpscr >> 22) & state.fpscr & 0xF8) != 0;
    state.fpscr = (state.fpscr & ~0x40000000u) | (enabled ? 0x40000000u : 0);
}

void setFPException(BroadwayState &state, uint32_t flags) {
    if ((state.fpscr & flags) != flags)
        state.fpscr |= FPSCR_FX;
    state.fpscr |= flags;
    refreshFPSCR(state);
}

void classifyFP(BroadwayState &state, double value, bool single) {
    uint32_t field;
    if (std::isnan(value))
        field = 0x11;
    else if (value == 0)
        field = std::signbit(value) ? 0x12 : 2;
    else if (std::isinf(value))
        field = std::signbit(value) ? 9 : 5;
    else {
        field = std::signbit(value) ? 8 : 4;
        double minimum = single ? std::numeric_limits<float>::min()
                                : std::numeric_limits<double>::min();
        if (std::abs(value) < minimum)
            field |= 0x10;
    }
    state.fpscr = (state.fpscr & ~0x0001F000u) | (field << 12);
}

int roundingMode(uint32_t fpscr) {
    switch (fpscr & 3) {
    case 1:
        return FE_TOWARDZERO;
    case 2:
        return FE_UPWARD;
    case 3:
        return FE_DOWNWARD;
    default:
        return FE_TONEAREST;
    }
}

double estimateFP(double value, bool squareRoot) {
    static constexpr uint32_t reciprocalBase[32] = {
        0x7FF800, 0x783800, 0x70EA00, 0x6A0800, 0x638800, 0x5D6200, 0x579000,
        0x520800, 0x4CC800, 0x47CA00, 0x430800, 0x3E8000, 0x3A2C00, 0x360800,
        0x321400, 0x2E4A00, 0x2AA800, 0x272C00, 0x23D600, 0x209E00, 0x1D8800,
        0x1A9000, 0x17AE00, 0x14F800, 0x124400, 0x0FBE00, 0x0D3800, 0x0ADE00,
        0x088400, 0x065000, 0x041C00, 0x020C00};
    static constexpr uint32_t reciprocalStep[32] = {
        0x3E1, 0x3A7, 0x371, 0x340, 0x313, 0x2EA, 0x2C4, 0x2A0,
        0x27F, 0x261, 0x245, 0x22A, 0x212, 0x1FB, 0x1E5, 0x1D1,
        0x1BE, 0x1AC, 0x19B, 0x18B, 0x17C, 0x16E, 0x15B, 0x15B,
        0x143, 0x143, 0x12D, 0x12D, 0x11A, 0x11A, 0x108, 0x106};
    static constexpr uint32_t squareRootBase[32] = {
        0x1A7E800, 0x17CB800, 0x1552800, 0x130C000, 0x10F2000, 0x0EFF000,
        0x0D2E000, 0x0B7C000, 0x09E5000, 0x0867000, 0x06FF000, 0x05AB800,
        0x046A000, 0x0339800, 0x0218800, 0x0105800, 0x3FFA000, 0x3C29000,
        0x38AA000, 0x3572000, 0x3279000, 0x2FB7000, 0x2D26000, 0x2AC0000,
        0x2881000, 0x2665000, 0x2468000, 0x2287000, 0x20C1000, 0x1F12000,
        0x1D79000, 0x1BF4000};
    static constexpr uint32_t squareRootStep[32] = {
        0x568, 0x4F3, 0x48D, 0x435, 0x3E7, 0x3A2, 0x365, 0x32E,
        0x2FC, 0x2D0, 0x2A8, 0x283, 0x261, 0x243, 0x226, 0x20B,
        0x7A4, 0x700, 0x670, 0x5F2, 0x584, 0x524, 0x4CC, 0x47E,
        0x43A, 0x3FA, 0x3C2, 0x38E, 0x35E, 0x332, 0x30A, 0x2E6};
    uint64_t bits = std::bit_cast<uint64_t>(value);
    uint64_t sign = bits & (1ull << 63);
    if (std::isnan(value))
        return std::bit_cast<double>(bits | 0x0008000000000000ull);
    if (value == 0)
        return std::copysign(std::numeric_limits<double>::infinity(), value);
    if (squareRoot && value < 0)
        return std::numeric_limits<double>::quiet_NaN();
    if (std::isinf(value))
        return std::copysign(0.0, value);
    int exponent = static_cast<int>((bits >> 52) & 2047);
    uint64_t fraction = bits & 0x000FFFFFFFFFFFFFull;
    if (!squareRoot) {
        if (exponent < 895)
            return std::copysign(
                static_cast<double>(std::numeric_limits<float>::max()), value);
        if (exponent >= 1149)
            return std::copysign(0.0, value);
        uint32_t index = fraction >> 47;
        uint32_t offset = (fraction >> 37) & 1023;
        uint64_t estimate =
            reciprocalBase[index] - (reciprocalStep[index] * offset + 1) / 2;
        return std::bit_cast<double>(
            sign | (static_cast<uint64_t>(2045 - exponent) << 52) |
            (estimate << 29));
    }
    if (exponent == 0) {
        int power;
        double normalized = std::frexp(value, &power) * 2;
        exponent = power + 1022;
        fraction = std::bit_cast<uint64_t>(normalized) & 0x000FFFFFFFFFFFFFull;
    }
    uint32_t index = ((exponent & 1) << 4) | (fraction >> 48);
    uint32_t offset = (fraction >> 37) & 2047;
    uint64_t estimate = squareRootBase[index] - squareRootStep[index] * offset;
    uint64_t outputExponent = (3068 - exponent) / 2;
    return std::bit_cast<double>((outputExponent << 52) | (estimate << 26));
}

double roundMultiplier(double value) {
    if (!std::isfinite(value) || value == 0)
        return value;
    int exponent;
    double fraction = std::frexp(std::abs(value), &exponent);
    double rounded = std::floor(std::ldexp(fraction, 25) + 0.5);
    return std::copysign(std::ldexp(rounded, exponent - 25), value);
}

double calculateFP(BroadwayState &state, uint32_t xo, double a, double b,
                   double c, bool single, bool &suppress) {
    int oldRounding = std::fegetround();
    int oldExceptions = std::fetestexcept(FE_ALL_EXCEPT);
    std::fesetround(roundingMode(state.fpscr));
    std::feclearexcept(FE_ALL_EXCEPT);
    double result = 0;
    uint32_t invalid = 0;
    bool useA = xo != 12 && xo != 24 && xo != 26;
    bool useB = xo != 25;
    bool useC = xo == 25 || xo >= 28;
    if ((useA && signalingNaN(std::bit_cast<uint64_t>(a))) ||
        (useB && signalingNaN(std::bit_cast<uint64_t>(b))) ||
        (useC && signalingNaN(std::bit_cast<uint64_t>(c))))
        invalid |= FPSCR_VXSNAN;
    if (single && useC)
        c = roundMultiplier(c);
    switch (xo) {
    case 12:
        result = b;
        break;
    case 18:
        if (a == 0 && b == 0)
            invalid |= 0x00200000;
        if (std::isinf(a) && std::isinf(b))
            invalid |= 0x00400000;
        result = a / b;
        break;
    case 20:
        if (std::isinf(a) && a == b)
            invalid |= 0x00800000;
        result = a - b;
        break;
    case 21:
        if (std::isinf(a) && a == -b)
            invalid |= 0x00800000;
        result = a + b;
        break;
    case 24:
        result = estimateFP(b, false);
        if (b == 0)
            std::feraiseexcept(FE_DIVBYZERO);
        break;
    case 25:
        if ((a == 0 && std::isinf(c)) || (c == 0 && std::isinf(a)))
            invalid |= 0x00100000;
        result = a * c;
        break;
    case 26:
        if (b < 0)
            invalid |= 0x00000200;
        result = estimateFP(b, true);
        if (b == 0)
            std::feraiseexcept(FE_DIVBYZERO);
        break;
    case 28:
    case 29:
    case 30:
    case 31: {
        double addend = (xo == 28 || xo == 30) ? -b : b;
        if ((a == 0 && std::isinf(c)) || (c == 0 && std::isinf(a)))
            invalid |= 0x00100000;
        if ((std::isinf(a) || std::isinf(c)) && std::isinf(addend) &&
            !std::isnan(a) && !std::isnan(c) &&
            ((std::signbit(a) != std::signbit(c)) != std::signbit(addend)))
            invalid |= 0x00800000;
        result = std::fma(a, c, addend);
        if (xo >= 30 && !std::isnan(result))
            result = -result;
        break;
    }
    }
    if (std::isnan(result)) {
        double input = useA && std::isnan(a)   ? a
                       : useB && std::isnan(b) ? b
                       : useC && std::isnan(c)
                           ? c
                           : std::numeric_limits<double>::quiet_NaN();
        result = std::bit_cast<double>(std::bit_cast<uint64_t>(input) |
                                       0x0008000000000000ull);
    }
    double unrounded = result;
    if ((state.fpscr & 4) &&
        std::abs(result) < (single ? std::numeric_limits<float>::min()
                                   : std::numeric_limits<double>::min()))
        result = std::copysign(0.0, result);
    if (single)
        result = static_cast<double>(static_cast<float>(result));
    int exceptions = std::fetestexcept(FE_ALL_EXCEPT);
    std::feclearexcept(FE_ALL_EXCEPT);
    std::fesetround(oldRounding);
    std::feraiseexcept(oldExceptions);
    suppress = (invalid && (state.fpscr & 0x80)) ||
               ((exceptions & FE_DIVBYZERO) && (state.fpscr & 0x10));
    if (invalid)
        setFPException(state, invalid);
    if (exceptions & FE_DIVBYZERO)
        setFPException(state, 0x04000000);
    if (exceptions & FE_OVERFLOW)
        setFPException(state, 0x10000000);
    if (exceptions & FE_UNDERFLOW)
        setFPException(state, 0x08000000);
    if (exceptions & FE_INEXACT)
        setFPException(state, 0x02000000);
    state.fpscr &= ~0x00060000u;
    if (exceptions & FE_INEXACT)
        state.fpscr |= 0x00020000;
    if (std::abs(result) > std::abs(unrounded))
        state.fpscr |= 0x00040000;
    return result;
}

void compareFP(BroadwayState &state, uint32_t field, uint64_t left,
               uint64_t right, bool ordered) {
    double a = std::bit_cast<double>(left);
    double b = std::bit_cast<double>(right);
    bool unordered = std::isnan(a) || std::isnan(b);
    uint32_t result = unordered ? 1 : a < b ? 8 : a > b ? 4 : 2;
    bool signaling = signalingNaN(left) || signalingNaN(right);
    if (signaling)
        setFPException(state, FPSCR_VXSNAN);
    if (ordered && unordered && (!signaling || !(state.fpscr & 0x80)))
        setFPException(state, FPSCR_VXVC);
    state.setCRField(field, result);
    state.fpscr = (state.fpscr & ~0xF000u) | (result << 12);
}
}

void Broadway::executeXFLType(uint32_t instruction) {
    uint32_t b = (instruction >> 11) & 31;
    uint32_t fields = (instruction >> 17) & 255;
    for (uint32_t i = 0; i < 8; ++i) {
        if (fields & (128u >> i)) {
            uint32_t mask = 15u << (28 - i * 4);
            state.fpscr = (state.fpscr & ~mask) |
                          (static_cast<uint32_t>(state.fpr[b]) & mask);
        }
    }
    refreshFPSCR(state);
    if (instruction & 1)
        state.setCRField(1, state.fpscr >> 28);
}

void Broadway::executeAType(uint32_t instruction) {
    uint32_t op = instruction >> 26;
    uint32_t d = (instruction >> 21) & 31;
    uint32_t a = (instruction >> 16) & 31;
    uint32_t b = (instruction >> 11) & 31;
    uint32_t c = (instruction >> 6) & 31;
    uint32_t xo = (instruction >> 1) & 1023;
    double av = std::bit_cast<double>(state.fpr[a]);
    double bv = std::bit_cast<double>(state.fpr[b]);
    double cv = std::bit_cast<double>(state.fpr[c]);
    bool single = op == 59;
    if (op == 63) {
        bool handled = true;
        switch (xo) {
        case 0:
        case 32:
            compareFP(state, d >> 2, state.fpr[a], state.fpr[b], xo == 32);
            return;
        case 40:
            state.fpr[d] = state.fpr[b] ^ (1ull << 63);
            break;
        case 72:
            state.fpr[d] = state.fpr[b];
            break;
        case 136:
            state.fpr[d] = state.fpr[b] | (1ull << 63);
            break;
        case 264:
            state.fpr[d] = state.fpr[b] & ~(1ull << 63);
            break;
        case 583:
            state.fpr[d] = 0xFFF8000000000000ull | state.fpscr;
            break;
        case 711:
            executeXFLType(instruction);
            return;
        case 38:
        case 70:
            if (xo == 38) {
                uint32_t mask = 1u << (31 - d);
                if (mask & 0x1FF80700u)
                    setFPException(state, mask);
                else
                    state.fpscr |= mask;
            } else
                state.fpscr &= ~(1u << (31 - d));
            refreshFPSCR(state);
            break;
        case 134: {
            uint32_t shift = 28 - (d >> 2) * 4;
            state.fpscr = (state.fpscr & ~(15u << shift)) |
                          (((instruction >> 12) & 15) << shift);
            refreshFPSCR(state);
            break;
        }
        case 64: {
            uint32_t shift = 28 - (a >> 2) * 4;
            state.setCRField(d >> 2, state.fpscr >> shift);
            state.fpscr &= ~((15u << shift) & 0x9FF80700u);
            refreshFPSCR(state);
            return;
        }
        case 14:
        case 15: {
            int old = std::fegetround();
            std::fesetround(xo == 15 ? FE_TOWARDZERO
                                     : roundingMode(state.fpscr));
            double rounded = std::nearbyint(bv);
            std::fesetround(old);
            int32_t result;
            bool invalid = std::isnan(rounded) || rounded > 2147483647.0 ||
                           rounded < -2147483648.0;
            if (signalingNaN(state.fpr[b]))
                setFPException(state, FPSCR_VXSNAN);
            if (invalid) {
                result = rounded > 0 ? std::numeric_limits<int32_t>::max()
                                     : std::numeric_limits<int32_t>::min();
                setFPException(state, FPSCR_VXCVI);
                state.fpscr &= ~0x60000u;
            } else {
                result = static_cast<int32_t>(rounded);
                state.fpscr &= ~0x60000u;
                if (rounded != bv) {
                    setFPException(state, 0x02000000);
                    state.fpscr |= 0x20000;
                    if (std::abs(rounded) > std::abs(bv))
                        state.fpscr |= 0x40000;
                }
            }
            if (!invalid || !(state.fpscr & 0x80))
                state.fpr[d] =
                    0xFFF8000000000000ull | static_cast<uint32_t>(result) |
                    (result == 0 && std::signbit(bv) ? 0x100000000ull : 0);
            break;
        }
        default:
            handled = false;
            break;
        }
        if (handled) {
            if (instruction & 1)
                state.setCRField(1, state.fpscr >> 28);
            return;
        }
    }
    xo &= 31;
    if (op == 63 && xo == 23) {
        state.fpr[d] = av >= 0 ? state.fpr[c] : state.fpr[b];
    } else {
        bool valid = xo == 18 || xo == 20 || xo == 21 || xo == 25 || xo >= 28 ||
                     (single && xo == 24) || (!single && xo == 26) ||
                     (op == 63 && ((instruction >> 1) & 1023) == 12);
        if (!valid) {
            raiseException(0x700, 0x80000);
            return;
        }
        single |= xo == 12;
        bool suppress = false;
        double result = calculateFP(state, xo, av, bv, cv, single, suppress);
        if (!suppress) {
            state.fpr[d] = std::bit_cast<uint64_t>(result);
            if (single)
                state.ps1[d] = state.fpr[d];
            classifyFP(state, result, single);
        }
    }
    if (instruction & 1)
        state.setCRField(1, state.fpscr >> 28);
}

void Broadway::executePaired(uint32_t instruction) {
    uint32_t d = (instruction >> 21) & 31;
    uint32_t a = (instruction >> 16) & 31;
    uint32_t b = (instruction >> 11) & 31;
    uint32_t c = (instruction >> 6) & 31;
    uint32_t xo = (instruction >> 1) & 1023;
    uint64_t left[2] = {state.fpr[a], state.ps1[a]};
    uint64_t right[2] = {state.fpr[b], state.ps1[b]};
    uint64_t third[2] = {state.fpr[c], state.ps1[c]};
    uint64_t result[2]{};
    bool arithmetic = false;
    bool suppress = false;
    switch (xo) {
    case 0:
    case 32:
    case 64:
    case 96:
        compareFP(state, d >> 2, left[xo >> 6], right[xo >> 6], (xo & 32) != 0);
        return;
    case 40:
    case 72:
    case 136:
    case 264:
        for (uint32_t i = 0; i < 2; ++i) {
            result[i] = right[i];
            if (xo == 40)
                result[i] ^= 1ull << 63;
            if (xo == 136)
                result[i] |= 1ull << 63;
            if (xo == 264)
                result[i] &= ~(1ull << 63);
        }
        break;
    case 528:
    case 560:
    case 592:
    case 624:
        result[0] = left[(xo >> 6) & 1];
        result[1] = right[(xo >> 5) & 1];
        break;
    case 1014: {
        uint32_t address = ((a == 0 ? 0 : state.gpr[a]) + state.gpr[b]) & ~31u;
        for (uint32_t i = 0; i < 32; i += 4)
            Bus::write32(address + i, 0);
        return;
    }
    default: {
        xo &= 31;
        if (!(xo == 10 || xo == 11 || xo == 12 || xo == 13 || xo == 14 ||
              xo == 15 || xo == 18 || xo == 20 || xo == 21 || xo == 23 ||
              xo == 24 || xo == 25 || xo == 26 || xo >= 28)) {
            raiseException(0x700, 0x80000);
            return;
        }
        arithmetic = xo != 23;
        for (uint32_t i = 0; i < 2; ++i) {
            double av = std::bit_cast<double>(left[i]);
            double bv = std::bit_cast<double>(right[i]);
            double cv = std::bit_cast<double>(third[i]);
            if (xo == 23) {
                result[i] = av >= 0 ? third[i] : right[i];
                continue;
            }
            uint32_t operation = xo;
            if (xo == 10 || xo == 11) {
                if (i != xo - 10) {
                    result[i] = std::bit_cast<uint64_t>(
                        static_cast<double>(static_cast<float>(cv)));
                    continue;
                }
                av = std::bit_cast<double>(left[0]);
                bv = std::bit_cast<double>(right[1]);
                operation = 21;
            } else if (xo >= 12 && xo <= 15) {
                cv = std::bit_cast<double>(third[xo & 1]);
                operation = xo < 14 ? 25 : 29;
            }
            bool laneSuppressed = false;
            result[i] = std::bit_cast<uint64_t>(calculateFP(
                state, operation, av, bv, cv, true, laneSuppressed));
            suppress |= laneSuppressed;
        }
        break;
    }
    }
    if (!suppress) {
        state.fpr[d] = result[0];
        state.ps1[d] = result[1];
    }
    if (arithmetic && !suppress)
        classifyFP(state, std::bit_cast<double>(result[xo == 11 ? 1 : 0]),
                   true);
    if (instruction & 1)
        state.setCRField(1, state.fpscr >> 28);
}

void Broadway::executePSQ_DType(uint32_t instruction) {
    executeQuantized(instruction, false);
}

void Broadway::executePSQ_XType(uint32_t instruction) {
    uint32_t xo = (instruction >> 1) & 63;
    if (xo == 6 || xo == 7 || xo == 38 || xo == 39)
        executeQuantized(instruction, true);
    else
        executePaired(instruction);
}

void Broadway::executeQuantized(uint32_t instruction, bool indexed) {
    uint32_t d = (instruction >> 21) & 31;
    uint32_t a = (instruction >> 16) & 31;
    uint32_t op = indexed ? (instruction >> 1) & 63 : instruction >> 26;
    bool store = indexed ? (op & 1) != 0 : (op & 4) != 0;
    bool update = indexed ? (op & 32) != 0 : (op & 1) != 0;
    bool one = (instruction >> (indexed ? 10 : 15)) & 1;
    uint32_t gqr = (instruction >> (indexed ? 7 : 12)) & 7;
    uint32_t format = state.spr[SPR::GQR0 + gqr] >> (store ? 0 : 16);
    uint32_t type = format & 7;
    int32_t scale = signExtend((format >> 8) & 63, 6);
    if ((update && a == 0) || (type != 0 && type < 4)) {
        raiseException(0x700, 0x80000);
        return;
    }
    uint32_t offset =
        indexed ? state.gpr[(instruction >> 11) & 31]
                : static_cast<uint32_t>(signExtend(instruction & 4095, 12));
    uint32_t address = (a == 0 ? 0 : state.gpr[a]) + offset;
    uint32_t size = type == 0 ? 4 : (type & 1) ? 2 : 1;
    uint64_t values[2] = {state.fpr[d], state.ps1[d]};
    for (uint32_t i = 0; i < (one ? 1u : 2u); ++i) {
        uint32_t current = address + i * size;
        if (store) {
            double value = std::bit_cast<float>(singleStoreBits(values[i]));
            if (type == 0) {
                uint32_t bits = singleStoreBits(values[i]);
                if (std::abs(value) < std::numeric_limits<float>::min())
                    bits &= 0x80000000u;
                Bus::write32(current, bits);
            } else {
                value = std::ldexp(value, scale);
                double minimum = type == 6 ? -128 : type == 7 ? -32768 : 0;
                double maximum = type == 4   ? 255
                                 : type == 5 ? 65535
                                 : type == 6 ? 127
                                             : 32767;
                value = std::isnan(value) ? minimum
                                          : std::clamp(value, minimum, maximum);
                int32_t quantized = static_cast<int32_t>(value);
                if (size == 1)
                    Bus::write8(current, static_cast<uint8_t>(quantized));
                else
                    Bus::write16(current, static_cast<uint16_t>(quantized));
            }
        } else {
            double value;
            switch (type) {
            case 0:
                value = std::bit_cast<float>(Bus::read32(current));
                break;
            case 4:
                value = Bus::read8(current);
                break;
            case 5:
                value = Bus::read16(current);
                break;
            case 6:
                value = signExtend(Bus::read8(current), 8);
                break;
            default:
                value = signExtend(Bus::read16(current), 16);
                break;
            }
            if (type != 0)
                value = static_cast<float>(std::ldexp(value, -scale));
            values[i] = std::bit_cast<uint64_t>(value);
        }
    }
    if (!store) {
        state.fpr[d] = values[0];
        state.ps1[d] = one ? std::bit_cast<uint64_t>(1.0) : values[1];
    }
    if (update)
        state.gpr[a] = address;
}
