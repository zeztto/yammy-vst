#include "PluginProcessor.h"
#include "PluginEditor.h"

YAMMYAudioProcessor::YAMMYAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
#endif
      apvts(*this, nullptr, "Parameters", createParameterLayout()) {
}

YAMMYAudioProcessor::~YAMMYAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
YAMMYAudioProcessor::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  layout.add(std::make_unique<juce::AudioParameterFloat>("PITCH", "Pitch",
                                                         -24.0f, 24.0f, 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", 0.0f,
                                                         1.0f, 1.0f));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("BYPASS", "Bypass", false));

  return layout;
}

const juce::String YAMMYAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool YAMMYAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool YAMMYAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool YAMMYAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double YAMMYAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int YAMMYAudioProcessor::getNumPrograms() {
  return 1; // 참고: 일부 호스트는 프로그램이 0개라고 알리면 잘 처리하지
            // 못하므로, 실제로 프로그램을 구현하지 않더라도 최소 1이어야
            // 합니다.
}

int YAMMYAudioProcessor::getCurrentProgram() { return 0; }

void YAMMYAudioProcessor::setCurrentProgram(int index) {}

const juce::String YAMMYAudioProcessor::getProgramName(int index) { return {}; }

void YAMMYAudioProcessor::changeProgramName(int index,
                                            const juce::String &newName) {}

void YAMMYAudioProcessor::prepareToPlay(double sampleRate,
                                        int samplesPerBlock) {
  pitchShifter.prepare(sampleRate, samplesPerBlock);
}

void YAMMYAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool YAMMYAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // 입력 레이아웃이 출력 레이아웃과 일치하는지 확인합니다.
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void YAMMYAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                       juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // 파라미터 업데이트
  float pitch = *apvts.getRawParameterValue("PITCH");
  float mix = *apvts.getRawParameterValue("MIX");
  bool bypass = *apvts.getRawParameterValue("BYPASS") > 0.5f;

  if (bypass)
    return;

  // 피치 시프팅 처리
  // 믹스를 위해 원음(Dry) 복사본이 필요합니다.
  juce::AudioBuffer<float> dryBuffer;
  dryBuffer.makeCopyOf(buffer);

  pitchShifter.setPitch(pitch);
  pitchShifter.process(buffer);

  // Dry/Wet 믹스
  for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
    auto *dry = dryBuffer.getReadPointer(channel);
    auto *wet = buffer.getWritePointer(channel);

    for (int i = 0; i < buffer.getNumSamples(); ++i) {
      wet[i] = dry[i] * (1.0f - mix) + wet[i] * mix;
    }
  }
}

bool YAMMYAudioProcessor::hasEditor() const {
  return true; // (에디터를 제공하지 않으려면 false로 변경하세요)
}

juce::AudioProcessorEditor *YAMMYAudioProcessor::createEditor() {
  return new YAMMYAudioProcessorEditor(*this);
}

void YAMMYAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void YAMMYAudioProcessor::setStateInformation(const void *data,
                                              int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// 플러그인의 새 인스턴스를 생성합니다..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new YAMMYAudioProcessor();
}
