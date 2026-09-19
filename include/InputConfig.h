#pragma once

#include <optional>
#include <string>
#include <vector>

struct InputBinding {
    RE::INPUT_DEVICE device;
    std::string keyName;
    std::optional<std::string> modifier;
    bool doublePress = false;
};

struct ResolvedInputBinding {
    RE::INPUT_DEVICE device;
    uint32_t key;
    std::optional<uint32_t> modifier;
    bool doublePress = false;
};

struct ActionBindings {
    std::string action;
    std::vector<InputBinding> bindings;
};

namespace InputConfig {
    void Install();
    std::vector<ResolvedInputBinding> GetBindings(const std::string& action);
    std::vector<std::pair<RE::INPUT_DEVICE, uint32_t>> Get(const std::string& key);
    std::optional<uint32_t> Get(const std::string& key, RE::INPUT_DEVICE device);
    std::vector<ActionBindings> GetEditableBindings();
    bool SaveEditableBindings(const std::vector<ActionBindings>& bindings, std::string& errorMessage);

inline const char* keyboardKeys[] = {
    "escape",
    "num1",
    "num2",
    "num3",
    "num4",
    "num5",
    "num6",
    "num7",
    "num8",
    "num9",
    "num0",
    "minus",
    "equals",
    "backspace",
    "tab",
    "q",
    "w",
    "e",
    "r",
    "t",
    "y",
    "u",
    "i",
    "o",
    "p",
    "bracketleft",
    "bracketright",
    "enter",
    "leftcontrol",
    "a",
    "s",
    "d",
    "f",
    "g",
    "h",
    "j",
    "k",
    "l",
    "semicolon",
    "apostrophe",
    "tilde",
    "leftshift",
    "backslash",
    "z",
    "x",
    "c",
    "v",
    "b",
    "n",
    "m",
    "comma",
    "period",
    "slash",
    "rightshift",
    "kp_multiply",
    "leftalt",
    "spacebar",
    "capslock",
    "f1",
    "f2",
    "f3",
    "f4",
    "f5",
    "f6",
    "f7",
    "f8",
    "f9",
    "f10",
    "numlock",
    "scrolllock",
    "kp_7",
    "kp_8",
    "kp_9",
    "kp_subtract",
    "kp_4",
    "kp_5",
    "kp_6",
    "kp_plus",
    "kp_1",
    "kp_2",
    "kp_3",
    "kp_0",
    "kp_decimal",
    "f11",
    "f12",
    "kp_enter",
    "rightcontrol",
    "kp_divide",
    "printscreen",
    "rightalt",
    "pause",
    "home",
    "up",
    "pageup",
    "left",
    "right",
    "end",
    "down",
    "pagedown",
    "insert",
    "delete",
    "leftwin",
    "rightwin"
};

inline const char* mouseKeys[] = {
    "leftbutton",
    "rightbutton",
    "middlebutton",
    "button3",
    "button4",
    "button5",
    "button6",
    "button7",
    "wheelup",
    "wheeldown"
};

inline const char* gamepadKeys[] = {
    "up",
    "down",
    "left",
    "right",
    "start",
    "back",
    "leftthumb",
    "rightthumb",
    "leftshoulder",
    "rightshoulder",
    "a",
    "b",
    "x",
    "y",
    "lefttrigger",
    "righttrigger"
};
inline constexpr std::size_t keyboardKeysLength = 103;
inline constexpr std::size_t mouseKeysLength = 10;
inline constexpr std::size_t gamepadKeysLength = 16;
}
