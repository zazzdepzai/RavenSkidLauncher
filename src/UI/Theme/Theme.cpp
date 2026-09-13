#include "Theme.h"

namespace UI
{
    static ImVec4 RGBA(int r, int g, int b, float a)
    {
        return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a);
    }

    Theme::Theme()
    {
        m_Colors.Background = RGBA(5, 10, 20, 1.0f);
        m_Colors.InputSurface = RGBA(30, 95, 170, 0.105f);
        m_Colors.InputSurfaceHover = RGBA(50, 130, 220, 0.145f);
        m_Colors.InputSurfaceFocus = RGBA(70, 155, 255, 0.19f);
        m_Colors.TextPrimary = RGBA(242, 242, 245, 1.0f);
        m_Colors.TextSecondary = RGBA(124, 124, 134, 1.0f);
        m_Colors.TextPlaceholder = RGBA(92, 92, 101, 1.0f);
        m_Colors.TextDisabled = RGBA(88, 88, 95, 1.0f);
        m_Colors.Accent = RGBA(74, 157, 255, 1.0f);
        m_Colors.AccentHover = RGBA(112, 190, 255, 1.0f);
        m_Colors.AccentPressed = RGBA(42, 118, 235, 1.0f);
        m_Colors.AccentText = RGBA(245, 250, 255, 1.0f);
        m_Colors.AccentDisabled = RGBA(75, 155, 255, 0.12f);
        m_Colors.Border = RGBA(100, 180, 255, 0.10f);
        m_Colors.BorderFocus = RGBA(105, 190, 255, 0.32f);
        m_Colors.ControlHover = RGBA(70, 145, 235, 0.16f);
        m_Colors.CloseHover = RGBA(232, 78, 71, 0.90f);
        m_Colors.SnowParticle = RGBA(180, 225, 255, 1.0f);
        m_Colors.CardSurface = RGBA(20, 50, 90, 0.42f);
        m_Colors.CardSurfaceHover = RGBA(35, 90, 155, 0.48f);
        m_Colors.CardBorder = RGBA(90, 170, 255, 0.16f);
        m_Colors.OutlineSurface = RGBA(10, 35, 65, 0.52f);
        m_Colors.OutlineSurfaceHover = RGBA(45, 110, 190, 0.48f);
        m_Colors.OutlineBorder = RGBA(100, 190, 255, 0.25f);
        m_Colors.SidebarActive = RGBA(40, 120, 235, 0.30f);
        m_Colors.Divider = RGBA(80, 160, 235, 0.10f);
        m_Colors.Danger = RGBA(232, 96, 88, 1.0f);
        m_Colors.GlassSurface = RGBA(20, 55, 100, 0.46f);
        m_Colors.GlassSheen = RGBA(130, 205, 255, 0.11f);
        m_Colors.GlassBorder = RGBA(115, 200, 255, 0.22f);
        m_Colors.Shadow = RGBA(0, 0, 0, 1.0f);

        m_Metrics.ContentWidth = 252.0f;
        m_Metrics.InputHeight = 38.0f;
        m_Metrics.InputRadius = 9.0f;
        m_Metrics.InputPaddingX = 13.0f;
        m_Metrics.ButtonHeight = 44.0f;
        m_Metrics.ButtonRadius = 13.0f;
        m_Metrics.IconButtonSize = 34.0f;
        m_Metrics.WindowControlSize = 26.0f;
        m_Metrics.TitleBarHeight = 42.0f;

        m_Animation.HoverSeconds = 0.10f;
        m_Animation.FocusSeconds = 0.12f;
        m_Animation.PressSeconds = 0.06f;
        m_Animation.TransitionSeconds = 0.17f;
        m_Animation.ScreenOutSeconds = 0.11f;
        m_Animation.ScreenInSeconds = 0.19f;

        m_Snow.ParticleCount = 95;
        m_Snow.MinSize = 1.1f;
        m_Snow.MaxSize = 2.7f;
        m_Snow.MinFallSpeed = 7.0f;
        m_Snow.MaxFallSpeed = 22.0f;
        m_Snow.MinOpacity = 0.05f;
        m_Snow.MaxOpacity = 0.26f;
        m_Snow.DriftAmount = 12.0f;
        m_Snow.DriftSpeed = 0.32f;
        m_Snow.BlurredFraction = 0.35f;
    }

    Theme& Theme::Get()
    {
        static Theme instance;
        return instance;
    }
}
