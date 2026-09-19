#include "InputConfig.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <format>
#include <map>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace InputConfig {
    constexpr const char* inputConfigFile = "Data/SKSE/Plugins/FlightFrameworkKeyBindings.json";
    static inline std::map<std::string, std::vector<InputBinding>> editable_configs;
    static inline std::map<std::string, std::vector<ResolvedInputBinding>> input_configs;
    static inline std::vector<std::string> action_order;
}

namespace {
    const std::map<std::string, RE::INPUT_DEVICE> deviceMap = {
    {"keyboard", RE::INPUT_DEVICE::kKeyboard},
    {"mouse", RE::INPUT_DEVICE::kMouse},
    {"gamepad", RE::INPUT_DEVICE::kGamepad},
};

    const std::map<std::string, RE::BSKeyboardDevice::Key> keyboardMap = {
        {"escape", RE::BSKeyboardDevice::Key::kEscape},
        {"num1", RE::BSKeyboardDevice::Key::kNum1},
        {"num2", RE::BSKeyboardDevice::Key::kNum2},
        {"num3", RE::BSKeyboardDevice::Key::kNum3},
        {"num4", RE::BSKeyboardDevice::Key::kNum4},
        {"num5", RE::BSKeyboardDevice::Key::kNum5},
        {"num6", RE::BSKeyboardDevice::Key::kNum6},
        {"num7", RE::BSKeyboardDevice::Key::kNum7},
        {"num8", RE::BSKeyboardDevice::Key::kNum8},
        {"num9", RE::BSKeyboardDevice::Key::kNum9},
        {"num0", RE::BSKeyboardDevice::Key::kNum0},
        {"minus", RE::BSKeyboardDevice::Key::kMinus},
        {"equals", RE::BSKeyboardDevice::Key::kEquals},
        {"backspace", RE::BSKeyboardDevice::Key::kBackspace},
        {"tab", RE::BSKeyboardDevice::Key::kTab},
        {"q", RE::BSKeyboardDevice::Key::kQ},
        {"w", RE::BSKeyboardDevice::Key::kW},
        {"e", RE::BSKeyboardDevice::Key::kE},
        {"r", RE::BSKeyboardDevice::Key::kR},
        {"t", RE::BSKeyboardDevice::Key::kT},
        {"y", RE::BSKeyboardDevice::Key::kY},
        {"u", RE::BSKeyboardDevice::Key::kU},
        {"i", RE::BSKeyboardDevice::Key::kI},
        {"o", RE::BSKeyboardDevice::Key::kO},
        {"p", RE::BSKeyboardDevice::Key::kP},
        {"bracketleft", RE::BSKeyboardDevice::Key::kBracketLeft},
        {"bracketright", RE::BSKeyboardDevice::Key::kBracketRight},
        {"enter", RE::BSKeyboardDevice::Key::kEnter},
        {"leftcontrol", RE::BSKeyboardDevice::Key::kLeftControl},
        {"a", RE::BSKeyboardDevice::Key::kA},
        {"s", RE::BSKeyboardDevice::Key::kS},
        {"d", RE::BSKeyboardDevice::Key::kD},
        {"f", RE::BSKeyboardDevice::Key::kF},
        {"g", RE::BSKeyboardDevice::Key::kG},
        {"h", RE::BSKeyboardDevice::Key::kH},
        {"j", RE::BSKeyboardDevice::Key::kJ},
        {"k", RE::BSKeyboardDevice::Key::kK},
        {"l", RE::BSKeyboardDevice::Key::kL},
        {"semicolon", RE::BSKeyboardDevice::Key::kSemicolon},
        {"apostrophe", RE::BSKeyboardDevice::Key::kApostrophe},
        {"tilde", RE::BSKeyboardDevice::Key::kTilde},
        {"leftshift", RE::BSKeyboardDevice::Key::kLeftShift},
        {"backslash", RE::BSKeyboardDevice::Key::kBackslash},
        {"z", RE::BSKeyboardDevice::Key::kZ},
        {"x", RE::BSKeyboardDevice::Key::kX},
        {"c", RE::BSKeyboardDevice::Key::kC},
        {"v", RE::BSKeyboardDevice::Key::kV},
        {"b", RE::BSKeyboardDevice::Key::kB},
        {"n", RE::BSKeyboardDevice::Key::kN},
        {"m", RE::BSKeyboardDevice::Key::kM},
        {"comma", RE::BSKeyboardDevice::Key::kComma},
        {"period", RE::BSKeyboardDevice::Key::kPeriod},
        {"slash", RE::BSKeyboardDevice::Key::kSlash},
        {"rightshift", RE::BSKeyboardDevice::Key::kRightShift},
        {"kp_multiply", RE::BSKeyboardDevice::Key::kKP_Multiply},
        {"leftalt", RE::BSKeyboardDevice::Key::kLeftAlt},
        {"spacebar", RE::BSKeyboardDevice::Key::kSpacebar},
        {"capslock", RE::BSKeyboardDevice::Key::kCapsLock},
        {"f1", RE::BSKeyboardDevice::Key::kF1},
        {"f2", RE::BSKeyboardDevice::Key::kF2},
        {"f3", RE::BSKeyboardDevice::Key::kF3},
        {"f4", RE::BSKeyboardDevice::Key::kF4},
        {"f5", RE::BSKeyboardDevice::Key::kF5},
        {"f6", RE::BSKeyboardDevice::Key::kF6},
        {"f7", RE::BSKeyboardDevice::Key::kF7},
        {"f8", RE::BSKeyboardDevice::Key::kF8},
        {"f9", RE::BSKeyboardDevice::Key::kF9},
        {"f10", RE::BSKeyboardDevice::Key::kF10},
        {"numlock", RE::BSKeyboardDevice::Key::kNumLock},
        {"scrolllock", RE::BSKeyboardDevice::Key::kScrollLock},
        {"kp_7", RE::BSKeyboardDevice::Key::kKP_7},
        {"kp_8", RE::BSKeyboardDevice::Key::kKP_8},
        {"kp_9", RE::BSKeyboardDevice::Key::kKP_9},
        {"kp_subtract", RE::BSKeyboardDevice::Key::kKP_Subtract},
        {"kp_4", RE::BSKeyboardDevice::Key::kKP_4},
        {"kp_5", RE::BSKeyboardDevice::Key::kKP_5},
        {"kp_6", RE::BSKeyboardDevice::Key::kKP_6},
        {"kp_plus", RE::BSKeyboardDevice::Key::kKP_Plus},
        {"kp_1", RE::BSKeyboardDevice::Key::kKP_1},
        {"kp_2", RE::BSKeyboardDevice::Key::kKP_2},
        {"kp_3", RE::BSKeyboardDevice::Key::kKP_3},
        {"kp_0", RE::BSKeyboardDevice::Key::kKP_0},
        {"kp_decimal", RE::BSKeyboardDevice::Key::kKP_Decimal},
        {"f11", RE::BSKeyboardDevice::Key::kF11},
        {"f12", RE::BSKeyboardDevice::Key::kF12},
        {"kp_enter", RE::BSKeyboardDevice::Key::kKP_Enter},
        {"rightcontrol", RE::BSKeyboardDevice::Key::kRightControl},
        {"kp_divide", RE::BSKeyboardDevice::Key::kKP_Divide},
        {"printscreen", RE::BSKeyboardDevice::Key::kPrintScreen},
        {"rightalt", RE::BSKeyboardDevice::Key::kRightAlt},
        {"pause", RE::BSKeyboardDevice::Key::kPause},
        {"home", RE::BSKeyboardDevice::Key::kHome},
        {"up", RE::BSKeyboardDevice::Key::kUp},
        {"pageup", RE::BSKeyboardDevice::Key::kPageUp},
        {"left", RE::BSKeyboardDevice::Key::kLeft},
        {"right", RE::BSKeyboardDevice::Key::kRight},
        {"end", RE::BSKeyboardDevice::Key::kEnd},
        {"down", RE::BSKeyboardDevice::Key::kDown},
        {"pagedown", RE::BSKeyboardDevice::Key::kPageDown},
        {"insert", RE::BSKeyboardDevice::Key::kInsert},
        {"delete", RE::BSKeyboardDevice::Key::kDelete},
        {"leftwin", RE::BSKeyboardDevice::Key::kLeftWin},
        {"rightwin", RE::BSKeyboardDevice::Key::kRightWin},
    };

    const std::map<std::string, RE::BSWin32MouseDevice::Key> mouseMap = {
    {"leftbutton", RE::BSWin32MouseDevice::Key::kLeftButton},
    {"rightbutton", RE::BSWin32MouseDevice::Key::kRightButton},
    {"middlebutton", RE::BSWin32MouseDevice::Key::kMiddleButton},
    {"button3", RE::BSWin32MouseDevice::Key::kButton3},
    {"button4", RE::BSWin32MouseDevice::Key::kButton4},
    {"button5", RE::BSWin32MouseDevice::Key::kButton5},
    {"button6", RE::BSWin32MouseDevice::Key::kButton6},
    {"button7", RE::BSWin32MouseDevice::Key::kButton7},
    {"wheelup", RE::BSWin32MouseDevice::Key::kWheelUp},
        {"wheeldown", RE::BSWin32MouseDevice::Key::kWheelDown},
    };

    const std::map<std::string, RE::BSWin32GamepadDevice::Key> gamepadMap = {
    {"up", RE::BSWin32GamepadDevice::Key::kUp},
    {"down", RE::BSWin32GamepadDevice::Key::kDown},
    {"left", RE::BSWin32GamepadDevice::Key::kLeft},
    {"right", RE::BSWin32GamepadDevice::Key::kRight},
    {"start", RE::BSWin32GamepadDevice::Key::kStart},
    {"back", RE::BSWin32GamepadDevice::Key::kBack},
    {"leftthumb", RE::BSWin32GamepadDevice::Key::kLeftThumb},
    {"rightthumb", RE::BSWin32GamepadDevice::Key::kRightThumb},
    {"leftshoulder", RE::BSWin32GamepadDevice::Key::kLeftShoulder},
    {"rightshoulder", RE::BSWin32GamepadDevice::Key::kRightShoulder},
    {"a", RE::BSWin32GamepadDevice::Key::kA},
    {"b", RE::BSWin32GamepadDevice::Key::kB},
    {"x", RE::BSWin32GamepadDevice::Key::kX},
    {"y", RE::BSWin32GamepadDevice::Key::kY},
    {"lefttrigger", RE::BSWin32GamepadDevice::Key::kLeftTrigger},
    {"righttrigger", RE::BSWin32GamepadDevice::Key::kRightTrigger},
};

    constexpr std::array<std::string_view, 5> preferredActionOrder = {
        "skyprompt.fly",
        "skyprompt.land",
        "skyprompt.fall",
        "ui.flyUp",
        "ui.flyDown",
    };

    std::filesystem::path GetInputConfigPath()
    {
        return InputConfig::inputConfigFile;
    }

    std::string NormalizeToken(std::string value)
    {
        value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), value.end());
        std::ranges::transform(value, value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }

    std::string ToDeviceToken(RE::INPUT_DEVICE device)
    {
        switch (device) {
        case RE::INPUT_DEVICE::kKeyboard:
            return "keyboard";
        case RE::INPUT_DEVICE::kMouse:
            return "mouse";
        case RE::INPUT_DEVICE::kGamepad:
            return "gamepad";
        default:
            return "unknown";
        }
    }

    std::optional<uint32_t> ResolveKeyCode(RE::INPUT_DEVICE device, const std::string& keyName)
    {
        const auto normalizedKey = NormalizeToken(keyName);
        if (device == RE::INPUT_DEVICE::kKeyboard) {
            if (const auto it = keyboardMap.find(normalizedKey); it != keyboardMap.end()) {
                return static_cast<uint32_t>(it->second);
            }
            return std::nullopt;
        }

        if (device == RE::INPUT_DEVICE::kMouse) {
            if (const auto it = mouseMap.find(normalizedKey); it != mouseMap.end()) {
                return static_cast<uint32_t>(it->second);
            }
            return std::nullopt;
        }

        if (device == RE::INPUT_DEVICE::kGamepad) {
            if (const auto it = gamepadMap.find(normalizedKey); it != gamepadMap.end()) {
                return static_cast<uint32_t>(it->second);
            }
            return std::nullopt;
        }

        return std::nullopt;
    }

    std::vector<std::string> BuildActionOrder(const std::map<std::string, std::vector<InputBinding>>& configs)
    {
        std::vector<std::string> orderedActions;

        for (const auto& action : preferredActionOrder) {
            if (configs.find(std::string(action)) != configs.end()) {
                orderedActions.emplace_back(action);
            }
        }

        for (const auto& [action, _] : configs) {
            if (std::find(orderedActions.begin(), orderedActions.end(), action) == orderedActions.end()) {
                orderedActions.push_back(action);
            }
        }

        return orderedActions;
    }

    bool CompileBindings(
        const std::map<std::string, std::vector<InputBinding>>& sourceConfigs,
        std::map<std::string, std::vector<ResolvedInputBinding>>& compiledConfigs,
        std::string* errorMessage = nullptr)
    {
        compiledConfigs.clear();

        for (const auto& [action, bindings] : sourceConfigs) {
            auto& compiledBindings = compiledConfigs[action];

            for (const auto& binding : bindings) {
                const auto normalizedKey = NormalizeToken(binding.keyName);
                if (normalizedKey.empty()) {
                    if (errorMessage) {
                        *errorMessage = std::format("{} has an empty {} binding.", action, (int)binding.device);
                    }
                    return false;
                }

                const auto keyCode = ResolveKeyCode(binding.device, normalizedKey);
                if (!keyCode) {
                    if (errorMessage) {
                        *errorMessage = std::format(
                            "{} uses an unknown {} key token '{}'.", action, (int)binding.device,
                            binding.keyName);
                    }
                    return false;
                }

                std::optional<uint32_t> modifierCode;
                if (binding.modifier) {
                    // Back/View is the only delayed modifier: other buttons must
                    // retain their original press/hold behavior.
                    if (binding.device != RE::INPUT_DEVICE::kGamepad ||
                        NormalizeToken(*binding.modifier) != "back" || normalizedKey == "back") {
                        if (errorMessage) {
                            *errorMessage = std::format(
                                "{}: modifier must be gamepad 'back', with a different main button.", action);
                        }
                        return false;
                    }
                    modifierCode = ResolveKeyCode(binding.device, *binding.modifier);
                }
                if (binding.doublePress && (binding.device != RE::INPUT_DEVICE::kGamepad ||
                    normalizedKey == "back" || (action != "skyprompt.fly" &&
                    action != "skyprompt.land" && action != "skyprompt.fall"))) {
                    if (errorMessage) {
                        *errorMessage = std::format(
                            "{}: double press supports gamepad takeoff, land, and fall, using a button other than Back/View.", action);
                    }
                    return false;
                }
                compiledBindings.push_back({ binding.device, *keyCode, modifierCode, binding.doublePress });
            }
        }

        return true;
    }

    bool SaveBindingsToDisk(const std::map<std::string, std::vector<InputBinding>>& sourceConfigs)
    {
        const auto path = GetInputConfigPath();

        json root = json::object();
        for (const auto& action : BuildActionOrder(sourceConfigs)) {
            const auto it = sourceConfigs.find(action);
            if (it == sourceConfigs.end()) {
                continue;
            }

            json bindings = json::array();
            for (const auto& binding : it->second) {
                json entry = {
                    { "device", ToDeviceToken(binding.device) },
                    { "key", NormalizeToken(binding.keyName) },
                };
                if (binding.modifier) {
                    entry["modifier"] = NormalizeToken(*binding.modifier);
                }
                if (binding.doublePress) {
                    entry["doublePress"] = true;
                }
                bindings.push_back(std::move(entry));
            }

            root[action] = std::move(bindings);
        }

        std::filesystem::create_directories(path.parent_path());

        std::ofstream file(path);
        if (!file.is_open()) {
            logger::error("Failed to open input configuration file for writing: {}", path.string());
            return false;
        }

        file << root.dump(4) << '\n';
        if (!file.good()) {
            logger::error("Failed to write input configuration file: {}", path.string());
            return false;
        }

        return true;
    }
}

void InputConfig::Install()
{
    editable_configs.clear();
    input_configs.clear();
    action_order.clear();

    const auto path = GetInputConfigPath();
    std::ifstream file(path);
    if (!file.is_open()) {
        logger::error("Failed to open input configuration file: {}", path.string());
        return;
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        logger::error("Error parsing JSON: {}", e.what());
        return;
    }

    logger::trace("Reading key bindings from: {}", path.string());
    if (!j.is_object()) {
        logger::warn("Key bind JSON must be an object: {}", path.string());
        return;
    }

    for (const auto& [key, value] : j.items()) {
        const std::string actionName = key;

        try {
            if (!value.is_array()) {
                logger::warn("Value for '{}' is not an array", actionName);
                continue;
            }

            std::vector<InputBinding> bindings;
            for (const auto& item : value) {
                if (!item.is_object()) {
                    logger::warn("Item in array for '{}' is not an object", actionName);
                    continue;
                }

                std::string deviceStr = item.value("device", "");
                std::string keyStr = item.value("key", "");

                if (deviceStr.empty() || keyStr.empty()) {
                    logger::warn("Missing 'device' or 'key' in item for '{}'", actionName);
                    continue;
                }

                deviceStr = NormalizeToken(deviceStr);
                keyStr = NormalizeToken(keyStr);

                const auto deviceIt = deviceMap.find(deviceStr);
                if (deviceIt == deviceMap.end()) {
                    logger::warn("Unknown device: {}", deviceStr);
                    continue;
                }

                const auto keyCode = ResolveKeyCode(deviceIt->second, keyStr);
                if (!keyCode) {
                    logger::warn("Unknown {} key token: {}", (int)deviceIt->second, keyStr);
                    continue;
                }

                bindings.push_back({
                    .device = deviceIt->second,
                    .keyName = keyStr,
                    .modifier = item.contains("modifier")
                        ? std::optional<std::string>(item.at("modifier").get<std::string>())
                        : std::nullopt,
                    .doublePress = item.value("doublePress", false),
                });
            }

            if (!bindings.empty()) {
                editable_configs[actionName] = std::move(bindings);
            }
        } catch (const std::exception& e) {
            logger::error("Error processing action '{}': {}", actionName, e.what());
        }
    }

    std::string errorMessage;
    if (!CompileBindings(editable_configs, input_configs, &errorMessage)) {
        logger::error("Failed to compile key bindings from {}: {}", path.string(), errorMessage);
        editable_configs.clear();
        input_configs.clear();
        return;
    }

    action_order = BuildActionOrder(editable_configs);
}

std::vector<std::pair<RE::INPUT_DEVICE, uint32_t>> InputConfig::Get(const std::string& key)
{
    auto it = input_configs.find(key);
    if (it != input_configs.end()) {
        std::vector<std::pair<RE::INPUT_DEVICE, uint32_t>> keys;
        for (const auto& binding : it->second) {
            keys.emplace_back(binding.device, binding.key);
        }
        return keys;
    }
    return {};
}

std::vector<ResolvedInputBinding> InputConfig::GetBindings(const std::string& action)
{
    const auto it = input_configs.find(action);
    return it != input_configs.end() ? it->second : std::vector<ResolvedInputBinding>{};
}

std::optional<uint32_t> InputConfig::Get(const std::string& key, RE::INPUT_DEVICE device)
{
    auto it = input_configs.find(key);
    if (it != input_configs.end()) {
        for (const auto& binding : it->second) {
            if (binding.device == device) {
                return binding.key;
            }
        }
    }

    return std::nullopt;
}

std::vector<ActionBindings> InputConfig::GetEditableBindings()
{
    std::vector<ActionBindings> bindings;
    bindings.reserve(action_order.size());

    for (const auto& action : action_order) {
        if (const auto it = editable_configs.find(action); it != editable_configs.end()) {
            bindings.push_back({
                .action = action,
                .bindings = it->second,
            });
        }
    }

    return bindings;
}




bool InputConfig::SaveEditableBindings(const std::vector<ActionBindings>& bindings, std::string& errorMessage)
{
    std::map<std::string, std::vector<InputBinding>> nextEditableConfigs;
    std::vector<std::string> nextActionOrder;

    for (const auto& actionBindings : bindings) {
        std::vector<InputBinding> normalizedBindings;
        normalizedBindings.reserve(actionBindings.bindings.size());

        for (const auto& binding : actionBindings.bindings) {
            normalizedBindings.push_back({
                .device = binding.device,
                .keyName = NormalizeToken(binding.keyName),
                .modifier = binding.modifier
                    ? std::optional<std::string>(NormalizeToken(*binding.modifier)) : std::nullopt,
                .doublePress = binding.doublePress,
            });
        }

        nextActionOrder.push_back(actionBindings.action);
        nextEditableConfigs[actionBindings.action] = std::move(normalizedBindings);
    }

    std::map<std::string, std::vector<ResolvedInputBinding>> nextCompiledConfigs;
    if (!CompileBindings(nextEditableConfigs, nextCompiledConfigs, &errorMessage)) {
        return false;
    }

    try {
        if (!SaveBindingsToDisk(nextEditableConfigs)) {
            errorMessage = "Failed to save key bindings to Data/SKSE/Plugins/FlightFrameworkKeyBindings.json.";
            return false;
        }
    } catch (const std::exception& e) {
        errorMessage = std::format("Failed to save key bindings: {}", e.what());
        return false;
    }

    editable_configs = std::move(nextEditableConfigs);
    input_configs = std::move(nextCompiledConfigs);
    action_order = std::move(nextActionOrder);
    return true;
}

