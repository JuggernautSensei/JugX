#pragma once
#include "EnumFlags.h"

namespace jug
{

enum class eKey
{
    None = 0,

    // ===========================================
    //  Alphabet
    // ===========================================

    A = 4,
    B = 5,
    C = 6,
    D = 7,
    E = 8,
    F = 9,
    G = 10,
    H = 11,
    I = 12,
    J = 13,
    K = 14,
    L = 15,
    M = 16,
    N = 17,
    O = 18,
    P = 19,
    Q = 20,
    R = 21,
    S = 22,
    T = 23,
    U = 24,
    V = 25,
    W = 26,
    X = 27,
    Y = 28,
    Z = 29,

    // ===========================================
    //  Number
    // ===========================================

    Num1 = 30,
    Num2 = 31,
    Num3 = 32,
    Num4 = 33,
    Num5 = 34,
    Num6 = 35,
    Num7 = 36,
    Num8 = 37,
    Num9 = 38,
    Num0 = 39,

    // ===========================================
    //  Control
    // ===========================================

    Return    = 40,
    Escape    = 41,
    Backspace = 42,
    Tab       = 43,
    Space     = 44,

    // ===========================================
    //  Punctuation
    // ===========================================

    Minus        = 45,
    Equals       = 46,
    LeftBracket  = 47,
    RightBracket = 48,
    Backslash    = 49,
    Semicolon    = 51,
    Apostrophe   = 52,
    Grave        = 53,
    Comma        = 54,
    Period       = 55,
    Slash        = 56,

    CapsLock = 57,

    // ===========================================
    //  Function
    // ===========================================

    F1  = 58,
    F2  = 59,
    F3  = 60,
    F4  = 61,
    F5  = 62,
    F6  = 63,
    F7  = 64,
    F8  = 65,
    F9  = 66,
    F10 = 67,
    F11 = 68,
    F12 = 69,
    F13 = 104,
    F14 = 105,
    F15 = 106,
    F16 = 107,
    F17 = 108,
    F18 = 109,
    F19 = 110,
    F20 = 111,
    F21 = 112,
    F22 = 113,
    F23 = 114,
    F24 = 115,

    // ===========================================
    //  Navigation
    // ===========================================

    PrintScreen = 70,
    ScrollLock  = 71,
    Pause       = 72,
    Insert      = 73,
    Home        = 74,
    PageUp      = 75,
    Delete      = 76,
    End         = 77,
    PageDown    = 78,
    Right       = 79,
    Left        = 80,
    Down        = 81,
    Up          = 82,

    // ===========================================
    //  Keypad
    // ===========================================

    NumLock    = 83,
    KpDivide   = 84,
    KpMultiply = 85,
    KpMinus    = 86,
    KpPlus     = 87,
    KpEnter    = 88,
    Kp1        = 89,
    Kp2        = 90,
    Kp3        = 91,
    Kp4        = 92,
    Kp5        = 93,
    Kp6        = 94,
    Kp7        = 95,
    Kp8        = 96,
    Kp9        = 97,
    Kp0        = 98,
    KpPeriod   = 99,
    KpEquals   = 103,

    Application = 101,
    Menu        = 118,

    // ===========================================
    //  Modifier
    // ===========================================

    LCtrl  = 224,
    LShift = 225,
    LAlt   = 226,
    LGui   = 227,
    RCtrl  = 228,
    RShift = 229,
    RAlt   = 230,
    RGui   = 231,

    Mode = 257,

    Count = 512,
};

// ===========================================
//  eMouse
// ===========================================

enum class eMouse
{
    None,
    Left,
    Middle,
    Right,
    X1,
    X2,
};

// ================================================================
//  eKeyMod
// ================================================================

enum class eKeyMod
{
    None   = 0,
    Shift  = 1 << 0,
    Ctrl   = 1 << 1,
    Alt    = 1 << 2,
    Gui    = 1 << 3,
    Num    = 1 << 4,
    Caps   = 1 << 5,
    Scroll = 1 << 6,
    Mode   = 1 << 7,
};

// ===========================================
//  eGamepadButton / eGamepadAxis
// ===========================================

enum class eGamepadButton
{
    South = 0,
    East,
    West,
    North,
    Back,
    Guide,
    Start,
    LeftStick,
    RightStick,
    LeftShoulder,
    RightShoulder,
    DpadUp,
    DpadDown,
    DpadLeft,
    DpadRight,
};

enum class eGamepadAxis
{
    LeftX,
    LeftY,
    RightX,
    RightY,
    LeftTrigger,
    RightTrigger,
};

}   // namespace jug
