#pragma once
#include "../dependencies/imgui/imgui.h"

namespace colors
{
    inline ImVec4 bg_color = ImColor(24, 24, 25, 255);
    inline ImVec4 nav_color = ImColor(24, 24, 25, 255);
    inline ImVec4 tab_color = ImColor(24, 24, 25, 255);
    inline ImVec4 tab_section_text = ImColor(99, 99, 99, 255);
    inline ImVec4 widget_background = ImColor(24, 24, 25, 255);

    namespace tab
    {
        inline ImVec4 text_col = ImColor(201, 201, 201, 255);
        inline ImVec4 text_col_disabled = ImColor(123, 123, 123, 255);
    }

    namespace user_info
    {
        inline ImVec4 separator = ImColor(47, 47, 47, 255);
        inline ImVec4 text = ImColor(201, 201, 201, 255);
        inline ImVec4 till_text = ImColor(99, 99, 99, 255);
    }

    namespace icon_text_button
    {
        inline ImVec4 back = ImColor(27, 27, 45, 255);
        inline ImVec4 front = ImColor(12, 21, 31, 255);
        inline ImVec4 front_active = ImColor(13, 25, 36, 255);
        inline ImVec4 text = ImColor(112, 112, 112, 255);
        inline ImVec4 text_active = ImColor(201, 201, 201, 255);
    }

    namespace menu_button
    {
        inline ImVec4 text = ImColor(112, 112, 112, 255);
        inline ImVec4 text_active = ImColor(201, 201, 201, 255);
        namespace popup
        {
            inline ImVec4 background = ImColor(12, 21, 31, 255);
        }
    }

    namespace cchild
    {
        inline ImVec4 background = ImColor(17, 17, 36, 255);
        inline ImVec4 title_text = ImColor(163, 163, 163, 255);
        inline ImVec4 title_background = ImColor(23, 24, 54, 255);
        inline ImVec4 title_gradient_background = ImColor(129, 133, 255, 255);
        inline ImVec4 title_gradient_background_after = ImColor(23, 24, 54, 255);
        inline ImVec4 gradient_start = ImColor(3, 4, 16, 255);
        inline ImVec4 gradient_end = ImColor(5, 8, 28, 255);
    }

    namespace checkbox
    {
        inline ImVec4 text_hovered = ImColor(220, 220, 220, 255);
        inline ImVec4 text = ImColor(163, 163, 163, 255);
        inline ImVec4 cb_active = ImColor(129, 133, 255, 255);
        inline ImVec4 cb_inactive = ImColor(40, 42, 83, 255);
        inline ImVec4 checkmark = ImColor(255, 255, 255, 255);
        inline ImVec4 checkmark_inactive = ImColor(255, 255, 255, 0);
    }

    namespace slider
    {
        inline ImVec4 text_hovered = ImColor(220, 220, 220, 255);
        inline ImVec4 text = ImColor(163, 163, 163, 255);
        inline ImVec4 tail = ImColor(129, 133, 255, 255);
        inline ImVec4 grab = ImColor(217, 217, 217, 255);
        inline ImVec4 background = ImColor(217, 217, 217, 255);

        inline ImVec4 input_bg = ImColor(40, 42, 83, 255);
    }

    namespace toggleswitch
    {
        inline ImVec4 text_hovered = ImColor(220, 220, 220, 255);
        inline ImVec4 text = ImColor(163, 163, 163, 255);
        inline ImVec4 toggle_bg = ImColor(129, 133, 255, 255);
        inline ImVec4 toggle_bg_unchecked = ImColor(16, 16, 16, 255);
        inline ImVec4 circle_bg = ImColor(205, 205, 205, 255);
    }

    namespace button
    {
        inline ImVec4 text_hovered = ImColor(220, 220, 220, 255);
        inline ImVec4 text = ImColor(163, 163, 163, 255);
        inline ImVec4 background_clicked = ImColor(35, 37, 78, 255);
        inline ImVec4 background_hovered = ImColor(29, 28, 64, 255);
        inline ImVec4 background = ImColor(23, 24, 54, 255);
    }

    namespace picker
    {
        inline ImVec4 text = ImColor(163, 163, 163, 255);
    }

    namespace scrollbar
    {
        inline ImVec4 background = ImColor(23, 24, 54, 255);
    }

    namespace combo
    {
        inline ImVec4 text = ImColor(163, 163, 163, 255);
        inline ImVec4 button = ImColor(23, 23, 23, 255);
        inline ImVec4 circle = ImColor(129, 133, 255, 255);
        inline ImVec4 popup_bg = ImColor(18, 18, 18, 240);
    }

    namespace selectable
    {
        inline ImVec4 text_selected = ImColor(220, 220, 220, 255);
        inline ImVec4 text = ImColor(163, 163, 163, 255);
        inline ImVec4 circle = ImColor(129, 133, 255, 255);
        inline ImVec4 background = ImColor(55, 56, 84, 255);
        inline ImVec4 background_inactive = ImColor(55, 56, 84, 0);
    }
}

namespace params
{
    inline float textAnimSpeed = 6.0f;
    inline ImVec2 widgets_padding = ImVec2(11.f, 0.f);

    namespace tab
    {
        inline float animSpeed = 6.0f; // bigger number = faster
    }

    namespace icon_text_button
    {
        inline float animSpeed = 10.0f; // bigger number = faster
    }

    namespace menu_button
    {
        inline float animSpeed = 10.f; // bigger number = faster
        namespace popup
        {
            inline float animSpeed = 6.f; // bigger number = faster
            inline float rounding = 15.f;
        }
    }

    namespace checkbox
    {
        inline float animSpeed = 6.0f;
    }

    namespace slider
    {
        inline float animSpeed = 10.0f;
    }

    namespace toggleswitch
    {
        inline float animSpeed = 8.0f;
    }

    namespace button
    {
        inline float animSpeed = 6.0f;
    }

    namespace picker
    {
        inline float radiusAnimSpeed = 10.f;
        inline float circleAnimSpeed = 18.f;
        inline float hueAnimSpeed = 16.f;
        inline float alphaAnimSpeed = 16.f;
    }

    namespace combo
    {
        inline float animSpeed = 7.0f;
    }

    namespace selectable
    {
        inline float backgroundAnimSpeed = 8.f;
    }

    namespace input
    {
        inline float cursorAnimSpeed = 30.f;
    }
}
