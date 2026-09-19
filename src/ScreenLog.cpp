#include "ScreenLog.h"
#include "SKSEMenuFramework.h"

void ScreenLog::Install() {

    if (!SKSEMenuFramework::IsInstalled()) {
        return;
    }
    SKSEMenuFramework::AddHudElement(Render);
}

void ScreenLog::Add(std::string message) {
    auto now = RE::GetDurationOfApplicationRunTime(); 
	Logs.push_back(LogItem{now, message});
    while (Logs.size() > 50) Logs.erase(Logs.begin());
}


void ScreenLog::Render() {
    auto fg = ImGuiMCP::GetForegroundDrawList();
    float y = 10;
    for (auto it = Logs.rbegin(); it != Logs.rend(); ++it) {
        y += 30;
        ImGuiMCP::ImDrawListManager::AddText(fg, {50 + 1, y + 1}, IM_COL32(0, 0, 0, 255), it->message.c_str());
        ImGuiMCP::ImDrawListManager::AddText(fg, {50, y}, IM_COL32(255, 255, 255, 255), it->message.c_str());
    }
}