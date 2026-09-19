#include "Plugin.h"

#include "DrawDebug.h"
#include "FlightConditionsManager.h"
#include "Hooks.h"
#include "Localization.h"
#include "Persistence.h"
#include "ScreenLog.h"
#include "SkyPromptClient.h"
#include "InputConfig.h"
#include "FlightInput.h"
#include "Cosave.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    //DrawDebug::OnMessage(message);
    if (message->type == SKSE::MessagingInterface::kPreLoadGame ||
        message->type == SKSE::MessagingInterface::kNewGame) {
        FlightInput::Reset();
        SkyPromptClient::PrepareForLoad();
    }
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        Persistence::Install();
        UI::Register();
        SkyPromptClient::Install();
        FlightInput::Install();
        FlightConditionsManager::Install();
    }
    if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
        Cosave::ApplyLoadedState();
    }
    if (message->type == SKSE::MessagingInterface::kPostLoad) {
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    SetupLog();
    logger::info("Plugin loaded");
    Localization::LoadTranslations();
    Hooks::Install();
    ScreenLog::Install();
    InputConfig::Install();
    Cosave::Install();
    return true;
}

