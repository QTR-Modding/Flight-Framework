#pragma once

namespace Bar {
    
    enum BarPosition{
        TopLeft = 0,
        TopCenter = 1,
        TopRight = 2,
        CenterLeft = 3,
        CenterCenter = 4,
        CenterRight = 5,
        BottomLeft = 6,
        BottomCenter = 7,
        BottomRight = 8
    };

    void Render(float scale, float progress, BarPosition position, float barDistance);
}