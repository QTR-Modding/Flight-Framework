#pragma once

#include <string>
#include <string_view>

namespace Localization {
    void LoadTranslations();
    const std::string& GetTranslation(std::string_view key);
}
