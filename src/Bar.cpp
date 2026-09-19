#include "Bar.h"
#include "SKSEMenuFramework.h"

void RennderBarElement(std::string path, ImGuiMCP::ImVec2 position, ImGuiMCP::ImVec2 size, ImGuiMCP::ImVec2 resolution, float alpha) {
    ImGuiMCP::ImDrawList* draw = ImGuiMCP::GetForegroundDrawList();
    auto texture = SKSEMenuFramework::LoadTexture("Data\\interface\\" + path, resolution);
    ImGuiMCP::ImDrawListManager::AddImage(draw, texture, position, ImGuiMCP::ImVec2(position.x + size.x, position.y + size.y), ImGuiMCP::ImVec2(0, 0), ImGuiMCP::ImVec2(1, 1), IM_COL32(255, 255, 255, alpha));
}

void Bar::Render(float scale, float progress, BarPosition position, float barDistance) {

    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    float bgWidth = 424.15f * scale;
    float bgHeight = 29.4f * scale;
    float barWidth = 366.4f * scale;
    float barHeight = 25.4f * scale;

    float screenWidth = ImGuiMCP::GetIO()->DisplaySize.x;
    float screenHeight = ImGuiMCP::GetIO()->DisplaySize.y;

    float xBase = 0;
    float yBase = 0;

    switch (position) {
        case TopLeft:      xBase = barDistance; yBase = barDistance; break;
        case TopCenter:    xBase = (screenWidth - bgWidth) / 2; yBase = barDistance; break;
        case TopRight:     xBase = screenWidth - bgWidth - barDistance; yBase = barDistance; break;
        case CenterLeft:   xBase = barDistance; yBase = (screenHeight - bgHeight) / 2; break;
        case CenterCenter: xBase = (screenWidth - bgWidth) / 2; yBase = (screenHeight - bgHeight) / 2; break;
        case CenterRight:  xBase = screenWidth - bgWidth - barDistance; yBase = (screenHeight - bgHeight) / 2; break;
        case BottomLeft:   xBase = barDistance; yBase = screenHeight - bgHeight - barDistance; break;
        case BottomCenter: xBase = (screenWidth - bgWidth) / 2; yBase = screenHeight - bgHeight - barDistance; break;
        case BottomRight:  xBase = screenWidth - bgWidth - barDistance; yBase = screenHeight - bgHeight - barDistance; break;
    }

    float xBarOffset = 0;

    switch (position) {
        case TopRight:
        case CenterRight:
        case BottomRight:
            xBarOffset = barWidth * (1.0f - progress);
            break;
        case TopCenter:
        case CenterCenter:
        case BottomCenter:
            xBarOffset = barWidth * 0.5f * (1.0f - progress);
            break;
    }

    RennderBarElement("barBg.svg", ImGuiMCP::ImVec2(xBase, yBase), ImGuiMCP::ImVec2(bgWidth, bgHeight), ImGuiMCP::ImVec2(bgWidth, bgHeight), 255);
    RennderBarElement("speedBar.svg", ImGuiMCP::ImVec2(xBase + 29 * scale + xBarOffset, yBase + 2 * scale), ImGuiMCP::ImVec2(progress * barWidth, barHeight), ImGuiMCP::ImVec2(barWidth, barHeight), 255);
    RennderBarElement("barOverlay.png", ImGuiMCP::ImVec2(xBase + 29 * scale + xBarOffset, yBase - 2 * scale), ImGuiMCP::ImVec2(progress * barWidth, barHeight), ImGuiMCP::ImVec2(0, 0), 200);
}
