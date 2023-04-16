// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

namespace chronicle {

template <typename T> struct PlatformEvent {
    T data = {};
};

struct CursorPositionData {
    double positionX;
    double positionY;
};

enum class CursorMode { normal = 0, hidden = 1, disabled = 2 };

enum class CursorType { system = 0, arrow = 1, ibeam = 2, crosshair = 3, hand = 4, hresize = 5, vresize = 6 };

enum class MouseButton {
    button1 = 0,
    button2 = 1,
    button3 = 2,
    button4 = 3,
    button5 = 4,
    button6 = 5,
    button7 = 6,
    button8 = 7,

    buttonLeft = button1,
    buttonRight = button2,
    buttonMiddle = button3,

    count = 8
};

enum class GamepadButton {
    butttonA = 0,
    butttonB = 1,
    butttonX = 2,
    butttonY = 3,
    butttonLeftBumper = 4,
    butttonRightBumper = 5,
    butttonBack = 6,
    butttonStart = 7,
    butttonGuide = 8,
    butttonLeftThumb = 9,
    butttonRightTumb = 10,
    butttonDPadUp = 11,
    butttonDPadRight = 12,
    butttonDPadDown = 13,
    butttonDPadLeft = 14,

    butttonCross = butttonA,
    butttonCircle = butttonB,
    butttonSquare = butttonX,
    butttonTriangle = butttonY,

    count = 15
};

enum class GamepadAxis {
    leftX = 0,
    leftY = 1,
    rightX = 2,
    rightY = 3,
    leftTrigger = 4,
    rightTrigger = 5,

    count = 6
};

enum class Gamepad {
    pad1 = 0,
    pad2 = 1,
    pad3 = 2,
    pad4 = 3,

    count = 4
};

enum class ButtonAction : uint8_t { release = 0, press = 1, repeat = 2 };

enum class KeyModifier {
    shift = 0x01,
    control = 0x02,
    alt = 0x08,
    super = 0x08,
    capsLock = 0x10,
    numLock = 0x20,

    _entt_enum_as_bitmask
};

enum class Key {
    unknown = -1,
    space = 32,
    apostrophe = 39,
    comma = 44,
    minus = 45,
    period = 46,
    slash = 47,
    number0 = 48,
    number1 = 49,
    number2 = 50,
    number3 = 51,
    number4 = 52,
    number5 = 53,
    number6 = 54,
    number7 = 55,
    number8 = 56,
    number9 = 57,
    semicolon = 59,
    equal = 61,
    a = 65,
    b = 66,
    c = 67,
    d = 68,
    e = 69,
    f = 70,
    g = 71,
    h = 72,
    i = 73,
    j = 74,
    k = 75,
    l = 76,
    m = 77,
    n = 78,
    o = 79,
    p = 80,
    q = 81,
    r = 82,
    s = 83,
    t = 84,
    u = 85,
    v = 86,
    w = 87,
    x = 88,
    y = 89,
    z = 90,
    leftBracket = 91,
    backslash = 92,
    rightBracket = 93,
    graveAccent = 96,
    world1 = 161,
    world2 = 162,
    escape = 256,
    enter = 257,
    tab = 258,
    backspace = 259,
    insert = 260,
    del = 261,
    right = 262,
    left = 263,
    down = 264,
    up = 265,
    pageUp = 266,
    pageDown = 267,
    home = 268,
    end = 269,
    capsLock = 280,
    scrollLock = 281,
    numLock = 282,
    printScreen = 283,
    pause = 284,
    f1 = 290,
    f2 = 291,
    f3 = 292,
    f4 = 293,
    f5 = 294,
    f6 = 295,
    f7 = 296,
    f8 = 297,
    f9 = 298,
    f10 = 299,
    f11 = 300,
    f12 = 301,
    f13 = 302,
    f14 = 303,
    f15 = 304,
    f16 = 305,
    f17 = 306,
    f18 = 307,
    f19 = 308,
    f20 = 309,
    f21 = 310,
    f22 = 311,
    f23 = 312,
    f24 = 313,
    f25 = 314,
    keyPad0 = 320,
    keyPad1 = 321,
    keyPad2 = 322,
    keyPad3 = 323,
    keyPad4 = 324,
    keyPad5 = 325,
    keyPad6 = 326,
    keyPad7 = 327,
    keyPad8 = 328,
    keyPad9 = 329,
    keyPadDecimal = 330,
    keyPadDivide = 331,
    keyPadMultiply = 332,
    keyPadSubtract = 333,
    keyPadAdd = 334,
    keyPadEnter = 335,
    keyPadEqual = 336,
    leftShift = 340,
    leftControl = 341,
    leftAlt = 342,
    leftSuper = 343,
    rightShift = 344,
    rightControl = 345,
    rightAlt = 346,
    rightSuper = 347,
    menu = 348,
};

struct MouseButtonData {
    MouseButton button;
    ButtonAction action;
    KeyModifier modifier;
};

struct MouseEnterData {
    bool entered;
};

struct MouseScrollData {
    double offsetX;
    double offsetY;
};

struct KeyData {
    Key key;
    ButtonAction action;
    KeyModifier modifier;
};

struct CharData {
    wchar_t character;
};

struct GamepadData {
    std::array<ButtonAction, static_cast<int>(GamepadButton::count)> buttons;
    std::array<float, static_cast<int>(GamepadAxis::count)> axes;
};

struct GamepadConnectedData {
    Gamepad gamepad;
    bool connected;
};

struct PathDropData {
    std::vector<std::string> paths;
};

using CursorPositionEvent = PlatformEvent<CursorPositionData>;
using MouseButtonEvent = PlatformEvent<MouseButtonData>;
using MouseEnterEvent = PlatformEvent<MouseEnterData>;
using MouseScrollEvent = PlatformEvent<MouseScrollData>;
using KeyEvent = PlatformEvent<KeyData>;
using CharEvent = PlatformEvent<CharData>;
using GamepadConnectedEvent = PlatformEvent<GamepadConnectedData>;
using PathDropEvent = PlatformEvent<PathDropData>;

template <class T> class PlatformI;

#ifdef GLFW_PLATFORM
class GLFWPlatform;

using Platform = PlatformI<GLFWPlatform>;
#endif

} // namespace chronicle