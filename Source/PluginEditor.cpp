#include "PluginEditor.h"
#include "PluginProcessor.h"

YAMMYAudioProcessorEditor::YAMMYAudioProcessorEditor(YAMMYAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setLookAndFeel(&lookAndFeel);

  // 피치 슬라이더
  pitchSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  pitchSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  addAndMakeVisible(pitchSlider);

  pitchLabel.setText("PITCH", juce::dontSendNotification);
  pitchLabel.setJustificationType(juce::Justification::centred);
  pitchLabel.setColour(juce::Label::textColourId, H4ppyLabs::Colors::Text);
  addAndMakeVisible(pitchLabel);

  pitchAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "PITCH", pitchSlider);

  // 믹스 슬라이더
  mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  addAndMakeVisible(mixSlider);

  mixLabel.setText("MIX", juce::dontSendNotification);
  mixLabel.setJustificationType(juce::Justification::centred);
  mixLabel.setColour(juce::Label::textColourId, H4ppyLabs::Colors::Text);
  addAndMakeVisible(mixLabel);

  mixAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "MIX", mixSlider);

  // 바이패스 버튼
  bypassButton.setButtonText("BYPASS");
  bypassButton.setColour(juce::ToggleButton::textColourId,
                         H4ppyLabs::Colors::Text);
  addAndMakeVisible(bypassButton);

  bypassAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "BYPASS", bypassButton);

  setSize(300, 400);
}

YAMMYAudioProcessorEditor::~YAMMYAudioProcessorEditor() {
  setLookAndFeel(nullptr);
}

void YAMMYAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(H4ppyLabs::Colors::Background);

  // 헤더
  auto headerArea = getLocalBounds().removeFromTop(60);

  // 제목
  g.setColour(juce::Colours::white);
  g.setFont(
      juce::Font("Futura", 40.0f, juce::Font::bold)); // 또는 유사한 볼드 폰트
  g.drawFittedText("YAMMY", headerArea.removeFromTop(40),
                   juce::Justification::centred, 1);

  // 부제목
  g.setColour(H4ppyLabs::Colors::Accent);
  g.setFont(14.0f);
  g.drawFittedText("Whammy Style Pitch Shifter", headerArea,
                   juce::Justification::centredTop, 1);

  // 푸터
  g.setColour(H4ppyLabs::Colors::Text.withAlpha(0.5f));
  g.setFont(12.0f);
  g.drawFittedText("h4ppy Labs", getLocalBounds().removeFromBottom(20),
                   juce::Justification::centred, 1);

  // 장식용 나사 (Duckaverb와 유사)
  float screwSize = 10.0f;
  float margin = 10.0f;
  g.setColour(H4ppyLabs::Colors::Panel);
  g.fillEllipse(margin, margin, screwSize, screwSize); // 좌상단
  g.fillEllipse(getWidth() - margin - screwSize, margin, screwSize,
                screwSize); // 우상단
  g.fillEllipse(margin, getHeight() - margin - screwSize, screwSize,
                screwSize); // 좌하단
  g.fillEllipse(getWidth() - margin - screwSize,
                getHeight() - margin - screwSize, screwSize,
                screwSize); // 우하단

  // 나사 슬롯
  g.setColour(H4ppyLabs::Colors::Background);
  g.drawLine(margin, margin + screwSize / 2, margin + screwSize,
             margin + screwSize / 2, 2.0f);
  g.drawLine(getWidth() - margin - screwSize, margin + screwSize / 2,
             getWidth() - margin, margin + screwSize / 2, 2.0f);
  g.drawLine(margin, getHeight() - margin - screwSize / 2, margin + screwSize,
             getHeight() - margin - screwSize / 2, 2.0f);
  g.drawLine(getWidth() - margin - screwSize,
             getHeight() - margin - screwSize / 2, getWidth() - margin,
             getHeight() - margin - screwSize / 2, 2.0f);
}

void YAMMYAudioProcessorEditor::resized() {
  auto area = getLocalBounds();
  auto header = area.removeFromTop(80); // 제목 영역
  auto footer = area.removeFromBottom(30);

  // 메인 콘텐츠 영역
  auto contentArea = area.reduced(20);

  // 피치 노브 (중앙 및 대형)
  auto pitchArea = contentArea.removeFromTop(180);
  pitchLabel.setBounds(pitchArea.removeFromTop(30));
  pitchSlider.setBounds(pitchArea);

  // 믹스 노브 (작게, 피치 아래)
  auto mixArea = contentArea.removeFromTop(100);
  mixLabel.setBounds(mixArea.removeFromTop(20));
  mixSlider.setBounds(mixArea.withSizeKeepingCentre(80, 80));

  // 바이패스 버튼 (하단)
  bypassButton.setBounds(
      contentArea.removeFromBottom(50).withSizeKeepingCentre(120, 40));
}
