#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>

#include "UI/StyleSheet.h"

class YAMMYAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  YAMMYAudioProcessorEditor(YAMMYAudioProcessor &);
  ~YAMMYAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  YAMMYAudioProcessor &audioProcessor;

  H4ppyLabs::YAMMYLookAndFeel lookAndFeel;

  juce::Slider pitchSlider;
  juce::Slider mixSlider;
  juce::ToggleButton bypassButton;

  juce::Label pitchLabel;
  juce::Label mixLabel;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      pitchAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      mixAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      bypassAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(YAMMYAudioProcessorEditor)
};
