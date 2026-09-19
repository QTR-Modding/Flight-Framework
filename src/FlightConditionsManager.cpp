#include "FlightConditionsManager.h"
#include <nlohmann/json.hpp>
#define PATH ".\\Data"
using json = nlohmann::json;

std::vector<RE::BGSPerk*> perks;

std::vector<RE::BGSPerk*>& FlightConditionsManager::GetPerks() { return perks; }

void FlightConditionsManager::Install() {
    for (const auto& entry : std::filesystem::directory_iterator(PATH)) {
        if (!entry.is_regular_file()) continue;
        const auto& path = entry.path();
        const auto name = path.filename().string();
        if (name.size() >= 23 && name.ends_with(".FLIGHT_CONDITION.json")) {
            std::ifstream file(path);
            json j = json::parse(file);
            if (j.contains("ModName") && j.contains("LocalId")) {
                std::string name = j.at("ModName").get<std::string>();
                std::string idHex = j.at("LocalId").get<std::string>();
                RE::FormID id = std::stoul(idHex, nullptr, 16);
                auto fid = RE::TESDataHandler::GetSingleton()->LookupFormID(id, name);
                auto perk = RE::TESForm::LookupByID<RE::BGSPerk>(fid);
                if (perk) {
                    logger::info("Added perk {}::{:x} -> {:x}", name, id, fid);
                    perks.push_back(perk);
                } else {
                    logger::error("Object not found {}::{:x} -> {:x}", name, id, fid);
                }

            } else {
                logger::error("Failed to parse condition file {}", name);
            }
        }
    }
}
