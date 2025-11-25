#pragma once

#include <JuceHeader.h>

namespace H4ppyLabs {
namespace Colors {
const juce::Colour Background =
    juce::Colour::fromString("FF1A1A1A");                        // 다크 그레이
const juce::Colour Panel = juce::Colour::fromString("FF2A2A2A"); // 밝은 그레이
const juce::Colour Accent =
    juce::Colour::fromString("FFFF6B6B"); // 소프트 레드/오렌지
const juce::Colour Accent2 = juce::Colour::fromString("FF4ECDC4"); // 청록색
const juce::Colour Text = juce::Colour::fromString("FFEEEEEE"); // 오프 화이트
const juce::Colour KnobBase = juce::Colour::fromString("FF333333");
const juce::Colour KnobIndicator = Accent;
} // namespace Colors

class YAMMYLookAndFeel : public juce::LookAndFeel_V4 {
public:
  YAMMYLookAndFeel() {
    setColour(juce::Slider::thumbColourId, Colors::Accent);
    setColour(juce::Slider::rotarySliderFillColourId, Colors::Accent);
    setColour(juce::Slider::rotarySliderOutlineColourId, Colors::KnobBase);
    setColour(juce::ToggleButton::textColourId, Colors::Text);
    setColour(juce::ToggleButton::tickColourId, Colors::Text);
  }

  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle,
                        juce::Slider &slider) override {
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;

    // 배경 아크
    auto arcThickness = 6.0f;
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(Colors::KnobBase);
    g.strokePath(backgroundArc, juce::PathStrokeType(
                                    arcThickness, juce::PathStrokeType::curved,
                                    juce::PathStrokeType::rounded));

    // 값 아크
    juce::Path valueArc;
    auto angle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    valueArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                           rotaryStartAngle, angle, true);
    g.setColour(Colors::Accent);
    g.strokePath(valueArc, juce::PathStrokeType(arcThickness,
                                                juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // 썸 (흰색 점)
    auto thumbRadius = 6.0f; // 아크 두께보다 약간 큼
    auto thumbX =
        centreX + radius * std::cos(angle - juce::MathConstants<float>::halfPi);
    auto thumbY =
        centreY + radius * std::sin(angle - juce::MathConstants<float>::halfPi);

    // 좌표계에 대한 올바른 각도 계산
    // addCentredArc에서 0은 상단(12시)인가요? 아니요, -pi/2만큼 회전하면 0이
    // 상단입니다. juce::MathConstants<float>::halfPi는 90도입니다. 간단한
    // 극좌표를 사용해 봅시다. 각도는 라디안 단위이며, 올바르게 구성된 경우 JUCE
    // 로터리 슬라이더 컨텍스트에서 0은 보통 12시 방향이지만 표준 삼각법에서는
    // 0이 3시 방향입니다. JUCE rotaryStartAngle은 보통 7시 방향입니다. 썸
    // 위치를 정확하게 다시 계산해 봅시다.

    thumbX =
        centreX + radius * std::cos(angle - juce::MathConstants<float>::halfPi);
    thumbY =
        centreY + radius * std::sin(angle - juce::MathConstants<float>::halfPi);

    // 사실, addCentredArc 0은 12시 방향입니다.
    // std::cos(0)은 1이고, std::sin(0)은 0입니다. (3시 방향)
    // 따라서 0을 12시 방향에 맞추려면 pi/2를 빼야 합니다.

    g.setColour(juce::Colours::white);
    g.fillEllipse(thumbX - thumbRadius, thumbY - thumbRadius,
                  thumbRadius * 2.0f, thumbRadius * 2.0f);
  }

  void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override {
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    auto cornerSize = 6.0f;

    g.setColour(Colors::Panel);
    g.drawRoundedRectangle(bounds, cornerSize, 2.0f);

    if (button.getToggleState()) {
      g.setColour(Colors::Panel.brighter(0.2f));
      g.fillRoundedRectangle(bounds, cornerSize);
      g.setColour(Colors::Accent); // 활성 상태일 때 텍스트 색상
    } else {
      g.setColour(Colors::Text); // 비활성 상태일 때 텍스트 색상
    }

    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawFittedText(button.getButtonText(), button.getLocalBounds(),
                     juce::Justification::centred, 1);
  }
};
} // namespace H4ppyLabs
