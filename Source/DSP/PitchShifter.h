#pragma once

#include <JuceHeader.h>

class PitchShifter {
public:
  PitchShifter();
  ~PitchShifter();

  void prepare(double sampleRate, int samplesPerBlock);
  void reset();
  void setPitch(float semitones);
  void process(juce::AudioBuffer<float> &buffer);

private:
  double sampleRate = 44100.0;

  // 원형 버퍼 파라미터
  juce::AudioBuffer<float> delayBuffer;
  int writePos = 0;
  float readPos = 0.0f;
  int bufferSize = 0;

  // 피치 시프팅 파라미터
  float currentPitch = 0.0f;
  float pitchRatio = 1.0f;

  // 윈도우 처리
  static constexpr int windowSize = 4096; // 레이턴시 대 부드러움 조절
  static constexpr int crossfadeSize = 1024;

  void updatePitchRatio();
};
