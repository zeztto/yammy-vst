#include "PitchShifter.h"

PitchShifter::PitchShifter() {}

PitchShifter::~PitchShifter() {}

void PitchShifter::prepare(double sr, int samplesPerBlock) {
  sampleRate = sr;
  // 윈도우를 위해 충분한 크기 또는 몇 초 분량을 담을 수 있도록 버퍼 크기 설정
  // 단순 피치 시프팅을 위해 충분한 히스토리가 필요합니다.
  // 합리적으로 큰 버퍼를 사용합니다.
  bufferSize = (int)(sampleRate * 2.0);
  delayBuffer.setSize(2, bufferSize);
  delayBuffer.clear();

  reset();
}

void PitchShifter::reset() {
  delayBuffer.clear();
  writePos = 0;
  readPos = 0.0f;
}

void PitchShifter::setPitch(float semitones) {
  if (currentPitch != semitones) {
    currentPitch = semitones;
    updatePitchRatio();
  }
}

void PitchShifter::updatePitchRatio() {
  // 반음에서 피치 비율 계산
  // 비율 = 2^(반음 / 12)
  pitchRatio = std::pow(2.0f, currentPitch / 12.0f);
}

void PitchShifter::process(juce::AudioBuffer<float> &buffer) {
  // 단순 딜레이 라인 기반 피치 시프터 (Whammy 스타일)
  // 가변 속도 테이프 루프의 단순화된 구현입니다.
  // 일정한 길이를 유지하려면 쓰기 포인터보다 빠르거나 느리게 움직이는 읽기
  // 포인터를 처리해야 합니다.

  // 참고: 진정한 "Whammy" 스타일은 읽기 포인터가 쓰기 포인터를 추월할 때
  // 클릭음을 방지하기 위해 SOLA와 유사한 기술이나 두 읽기 헤드 간의 단순
  // 크로스페이딩을 자주 사용합니다.

  // 그 "글리치한" 사운드를 위해 기본적인 듀얼 읽기 헤드 그래뉼러 스타일 접근
  // 방식을 구현해 봅시다.

  const int numSamples = buffer.getNumSamples();
  const int numChannels = buffer.getNumChannels();

  // 두 채널을 모두 처리합니다. 스테레오 일관성을 위해 두 채널 모두 동일한 읽기
  // 포인터 로직을 사용해야 합니다. 따라서 인덱스를 한 번 계산하고 모든 채널에
  // 적용합니다.

  auto *channelDataL = buffer.getWritePointer(0);
  auto *channelDataR = (numChannels > 1) ? buffer.getWritePointer(1) : nullptr;

  // 딜레이 버퍼 포인터
  auto *delayDataL = delayBuffer.getWritePointer(0);
  auto *delayDataR =
      (numChannels > 1) ? delayBuffer.getWritePointer(1) : nullptr;

  for (int i = 0; i < numSamples; ++i) {
    // 1. 원형 버퍼에 입력 쓰기
    float inL = channelDataL[i];
    float inR = (channelDataR != nullptr) ? channelDataR[i] : inL;

    delayDataL[writePos] = inL;
    if (delayDataR)
      delayDataR[writePos] = inR;

    // 2. 크로스페이딩으로 원형 버퍼에서 읽기
    // 윈도우 방식을 사용합니다.
    // 이상적으로는 읽기 포인터가 'pitchRatio' 속도로 움직이기를 원합니다.
    // 상대 속도 = pitchRatio - 1.0
    // pitchRatio = 1이면 readPos는 1.0(쓰기와 동일)으로 이동하고 거리는
    // 일정하게 유지됩니다. 잠깐, 타임 스트레칭 없는 실시간 피치 시프팅의 경우
    // 단순히 리샘플링합니다. 하지만 타이밍을 동일하게 유지하려면 주기적으로
    // 읽기 포인터를 재설정해야 합니다.

    // 윈도우 처리를 위해 간단한 페이저 접근 방식을 사용해 봅시다.
    // 이것은 효과적으로 2개의 그레인을 사용하는 그래뉼러 접근 방식입니다.

    // 사실, 연구 문서에 설명된 로직을 구현해 봅시다:
    // 두 개의 읽기 포인터, 크로스페이딩.

    // 첫 번째 패스에서의 단순성을 위해 기본 회전 테이프 헤드 모델을 시도해
    // 봅시다. 이것은 종종 "회전 테이프 헤드" 또는 "윈도우 딜레이"라고 불립니다.

    // 쓰기 위치에 대한 읽기 위치 계산
    // 딜레이가 변하기를 원합니다.
    // delay(t) = delay(t-1) + (1 - pitchRatio)

    // 더 간단한 로직을 사용해 봅시다:
    // 0에서 1로 가는 '위상'이 있습니다.
    // 읽기 포인터 1은 위상에 있습니다.
    // 읽기 포인터 2는 위상 + 0.5(래핑됨)에 있습니다.
    // 위상에 대한 삼각형 윈도우를 기반으로 크로스페이드합니다.

    // 주파수 차이에 따라 위상 증가
    // 비율 = (1 - pitchRatio) * 주파수 / 샘플 레이트?
    // 아니요, 딜레이 변조로 봅시다.

    // 가능하다면 "Whammy" 특정 로직을 고수합시다.
    // 연구 문서 언급: "버퍼 크기의 절반만큼 떨어진 두 개의 읽기 포인터".

    // "윈도우 크기"(예: 50ms)를 정의해 봅시다.
    // 이 윈도우를 통해 이동합니다.

    // 유효 로직:
    // 델타 = 1.0 - pitchRatio;
    // readPos += 델타;

    // readPos가 너무 멀어지면 윈도우 크기 주위로 래핑합니다.

    // 구현해 봅시다:
    // 'delay' 변수를 유지합니다.
    // delay += (1.0 - pitchRatio);
    // delay < 0 또는 delay > maxDelay이면 래핑해야 합니다.
    // 클릭음을 피하기 위해 두 개의 탭을 사용합니다.

    // 윈도우 처리를 위해 전역 '페이저'를 사용해 봅시다.
    // 페이저는 (1.0 - pitchRatio) / windowSize 만큼 증가합니다.

    float relativeSpeed = 1.0f - pitchRatio;

    // 쓰기 포인터에 대해 읽기 포인터를 전진시켜야 합니다.
    // readPos는 샘플 단위의 딜레이입니다.
    readPos += relativeSpeed;

    // 윈도우 내에서 readPos 래핑
    // 윈도우가 2048 샘플이라고 가정해 봅시다.
    float windowLen = 2048.0f;

    // 'readPos'가 딜레이 양이 되기를 원합니다.
    // 이 딜레이를 원형 버퍼의 위치에 매핑합니다.
    // 실제 읽기 인덱스 = writePos - readPos

    // 래핑을 부드럽게 처리하기 위해 두 개의 탭을 사용합니다.
    // 탭 1: 딜레이 = readPos (모듈로 windowLen)
    // 탭 2: 딜레이 = readPos + windowLen/2 (모듈로 windowLen)

    // 잠깐, 더 좋은 방법은:
    // 그레인 윈도우 내의 위치를 나타내는 메인 "페이저" 0..1이 있습니다. 이
    // 페이저의 속도는 피치 시프트 양에 의해 결정됩니다. 속도 = (pitchRatio
    // - 1.0) * 샘플 레이트 / 윈도우 크기? 사실, 속도 = (1.0 - pitchRatio) /
    // 윈도우 크기 * 샘플 레이트는 정확하지 않습니다. 버퍼 순회 속도는
    // pitchRatio입니다. 쓰기 속도는 1.0입니다. 차이는 (1.0 - pitchRatio)입니다.
    // 따라서 샘플당 (1.0 - pitchRatio) 샘플을 누적합니다.

    // 이것을 'delayWalk'라고 부릅시다.
    // delayWalk += (1.0 - pitchRatio);

    // delayWalk를 [0, windowLen]으로 래핑합니다.
    // 하지만 단순히 래핑하면 클릭음이 발생합니다.
    // 그래서 삼각형 윈도우를 사용합니다.

    // 탭 1 딜레이: delayWalk
    // 탭 2 딜레이: delayWalk + windowLen/2 (래핑됨)

    // 게인 1: 딜레이가 windowLen/2일 때 피크인 삼각형 윈도우?
    // 보통:
    // 딜레이가 0이면 쓰기 포인터에 있습니다(레이턴시 0).
    // 딜레이가 windowLen이면 훨씬 뒤에 있습니다.
    // 가장자리(0과 windowLen)에서는 페이드 아웃하고 중앙에서는 페이드 인하기를
    // 원합니다.

    while (readPos < 0)
      readPos += windowLen;
    while (readPos >= windowLen)
      readPos -= windowLen;

    float delay1 = readPos;
    float delay2 = readPos + windowLen * 0.5f;
    if (delay2 >= windowLen)
      delay2 -= windowLen;

    // 게인 계산 (삼각형 윈도우)
    // 0 -> 0, windowLen/2 -> 1, windowLen -> 0
    auto getGain = [&](float d) {
      float x = d / windowLen; // 0..1
      if (x < 0.5f)
        return x * 2.0f;
      else
        return (1.0f - x) * 2.0f;
    };

    float gain1 = getGain(delay1);
    float gain2 = getGain(delay2);

    // 버퍼에서 읽기
    auto getSample = [&](float delay, int channel) {
      float rPos = (float)writePos - delay;
      while (rPos < 0)
        rPos += bufferSize;
      while (rPos >= bufferSize)
        rPos -= bufferSize;

      int i0 = (int)rPos;
      int i1 = (i0 + 1) % bufferSize;
      float frac = rPos - i0;

      float *d = (channel == 0) ? delayDataL : delayDataR;
      return d[i0] + frac * (d[i1] - d[i0]); // 선형 보간
    };

    float outL = getSample(delay1, 0) * gain1 + getSample(delay2, 0) * gain2;
    float outR =
        (numChannels > 1)
            ? (getSample(delay1, 1) * gain1 + getSample(delay2, 1) * gain2)
            : outL;

    // 게인 정규화?
    // 0.5만큼 오프셋된 삼각형 윈도우의 게인 합은 상수 1.0인가요?
    // 확인해 봅시다:
    // x가 0..0.5일 때: g1 = 2x.
    // x2 = x + 0.5 (0.5..1 범위). g2 = (1 - (x+0.5))*2 = (0.5 - x)*2 = 1 - 2x.
    // g1 + g2 = 2x + 1 - 2x = 1.
    // 네, 합이 일정합니다.

    channelDataL[i] = outL;
    if (channelDataR)
      channelDataR[i] = outR;

    // 쓰기 포인터 전진
    writePos++;
    if (writePos >= bufferSize)
      writePos = 0;
  }
}
