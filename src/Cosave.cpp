#include "Cosave.h"
#include "Serializer.h"
#include "PromptManager.h"
#define VERSION 1

namespace {
    bool loadedStatePending = false;
}

void SaveCallback(SKSE::SerializationInterface* a_intfc) {
    logger::info("SAVE CAllBACK");
    if (a_intfc->OpenRecord('SAVG', VERSION)) {
        logger::info("WRITE");
        auto serializer = Serializer(a_intfc);

        serializer.Write<bool>(PromptManager::canFly);
        serializer.Write<bool>(PromptManager::isFlying);
        serializer.Write<bool>(PromptManager::isVisible);
    }

}

void LoadCallback(SKSE::SerializationInterface* a_intfc) {
    logger::info("LOAD CAllBACK");

    uint32_t type;
    uint32_t version;
    uint32_t length;
    loadedStatePending = false;

    while (a_intfc->GetNextRecordInfo(type, version, length)) {
        if (type == 'SAVG' && version == VERSION) {
            auto serializer = Serializer(a_intfc);
            PromptManager::canFly = serializer.Read<bool>();
            PromptManager::isFlying = serializer.Read<bool>();
            PromptManager::isVisible = serializer.Read<bool>();
            logger::info(
                "Loaded prompt state: canFly={}, isFlying={}, isVisible={}.",
                PromptManager::canFly,
                PromptManager::isFlying,
                PromptManager::isVisible);
            loadedStatePending = true;
        }
    }
}


void Cosave::Install() {
    auto serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID('QTFM');
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
}

void Cosave::ApplyLoadedState()
{
    if (!loadedStatePending) {
        return;
    }

    loadedStatePending = false;
    SKSE::GetTaskInterface()->AddTask([] { PromptManager::Update(); });
}
