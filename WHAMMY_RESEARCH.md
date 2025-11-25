# Whammy 스타일 피치 시프터 제작 연구 문서

> h4ppy Labs 3번째 플러그인 개발을 위한 기술 조사 문서

## 1. 개요

### DigiTech Whammy란?
- 1989년 출시된 최초의 실시간 피치 시프팅 기타 페달
- 익스프레션 페달로 피치를 실시간 제어
- 최대 2옥타브 업/다운 가능
- IVL Technologies의 피치 감지 알고리즘 라이센스 사용

### 특징적인 사운드
- 모노포닉 처리로 인한 특유의 "글리치" 아티팩트
- 코드 입력 시 발생하는 워블링이 오히려 시그니처 사운드가 됨
- Tom Morello, Jack White 등 유명 기타리스트들이 애용

---

## 2. 피치 시프팅의 기술적 과제

### 핵심 딜레마
```
피치 UP = 빠르게 재생 = 길이 단축
피치 DOWN = 느리게 재생 = 길이 연장

→ 실시간 처리에서는 길이(시간)를 유지하면서 피치만 변경해야 함
```

### 기타 신호의 어려움
- **폴리포닉**: 여러 음이 동시에 울림 (코드)
- **복잡한 하모닉스**: 기타는 배음 구조가 복잡
- **제로 레이턴시 요구**: 연주자가 지연을 체감하면 안 됨
- **트랜지언트**: 피킹 어택의 날카로운 시작점 보존 필요

---

## 3. 주요 알고리즘

### 3.1 Time-Domain 방식

#### Granular Pitch Shifting
```
1. 오디오를 작은 조각(grain, ~2000 samples)으로 분할
2. 두 개의 읽기 포인터를 버퍼 크기의 절반만큼 떨어뜨려 배치
3. 원하는 속도로 과거 샘플을 읽음
4. 버퍼 경계에서 페이드 인/아웃으로 크로스페이드
```
- 장점: 빠름, CPU 부하 낮음
- 단점: 클릭/글리치 발생 가능, 저주파 손실 위험

#### SOLA (Synchronous Overlap-Add)
- **Whammy가 사용하는 것으로 추정되는 알고리즘**
- 윈도우 크기를 입력 주파수의 정확한 배수로 맞춤
- 연속적인 파형 유지 가능
```
핵심: 피치 감지 → 주기 파악 → 윈도우 크기 동적 조정
```

#### PSOLA (Pitch Synchronous Overlap-Add)
- 음성 처리에서 유래
- 피치 주기를 감지하여 처리
- 단음에 적합, 코드에 약함

#### WSOLA (Waveform Synchronized Overlap-Add)
- PSOLA의 개선 버전
- 파형 유사도 기반으로 오버랩 위치 결정
- 더 자연스러운 결과

### 3.2 Frequency-Domain 방식

#### Phase Vocoder (STFT 기반)
```
1. FFT로 주파수 영역 변환
2. 각 주파수 빈의 위상과 크기 분석
3. 주파수 빈을 이동시켜 피치 변경
4. IFFT로 시간 영역 복원
```
- 장점: 깨끗한 결과물, 폴리포닉 처리 가능
- 단점: 높은 레이턴시 (수십~수백 ms), CPU 부하

#### Spectral Processing
- 포먼트 보존 가능
- 더 복잡한 피치 조작 가능

---

## 4. 공개 라이브러리 및 리소스

### 4.1 Signalsmith Stretch
- **URL**: https://github.com/Signalsmith-Audio/signalsmith-stretch
- **라이센스**: MIT
- **언어**: C++11 (헤더 온리)
- **특징**:
  - 여러 옥타브 피치 시프팅 가능
  - ADC22 발표 "Four Ways To Write A Pitch-Shifter" 기반
  - Web Audio, Python, Rust 바인딩 존재
- **문서**: https://signalsmith-audio.co.uk/code/stretch/

### 4.2 stftPitchShift (추천)
- **URL**: https://github.com/jurihock/stftPitchShift
- **라이센스**: MIT
- **언어**: C++, Python
- **특징**:
  - STFT 기반 실시간 피치 시프터
  - 폴리포닉 피치 시프팅 지원
  - 포먼트 보존 기능
  - 실시간 파이프라인 임베드 가능
  - CLI 도구 포함

### 4.3 Chowdhury-DSP PitchShift
- **URL**: https://github.com/Chowdhury-DSP/PitchShift
- **라이센스**: BSD 3-clause
- **언어**: C++ (JUCE)
- **특징**:
  - Bungee 라이브러리 사용
  - JUCE 플러그인 예제로 바로 참고 가능

### 4.4 기타 리소스
- **SoundTouch**: 가벼운 시간 영역 라이브러리
- **Rubber Band**: 고품질이나 CPU 부하 높음
- **Perry Cook's STK**: 간단한 피치 시프터 포함
- **DSP Dimension**: http://www.dspdimension.com (알고리즘 설명 및 소스코드)

---

## 5. 구현 전략

### 5.1 Whammy 스타일 (글리치 허용)

**목표**: 깨끗한 사운드보다 특유의 캐릭터 우선

```cpp
// 기본 구조
class WhammyProcessor {
    CircularBuffer buffer;
    float readPos1, readPos2;  // 두 개의 읽기 포인터
    float pitchRatio;          // 1.0 = 원음, 2.0 = 옥타브 업

    void process(float* input, float* output, int numSamples) {
        for (int i = 0; i < numSamples; i++) {
            // 쓰기
            buffer.write(input[i]);

            // 두 포인터에서 읽기 (크로스페이드)
            float sample1 = buffer.readInterpolated(readPos1);
            float sample2 = buffer.readInterpolated(readPos2);

            // 크로스페이드 계산
            float fade = calculateCrossfade(readPos1, readPos2);
            output[i] = sample1 * fade + sample2 * (1.0 - fade);

            // 읽기 포인터 이동
            readPos1 += pitchRatio;
            readPos2 += pitchRatio;

            // 포인터 랩어라운드 처리
            wrapPointers();
        }
    }
};
```

### 5.2 깨끗한 사운드 (STFT 기반)

**목표**: 최소한의 아티팩트

- stftPitchShift 라이브러리 통합
- 레이턴시 트레이드오프 필요

### 5.3 하이브리드 접근

1. **모노 모드**: 단음 입력 시 SOLA 기반 (낮은 레이턴시)
2. **폴리 모드**: 코드 입력 시 STFT 기반 (높은 품질)
3. 입력 신호 분석으로 자동 전환

---

## 6. 기능 설계 (안)

### 기본 컨트롤
| 파라미터 | 설명 | 범위 |
|---------|------|------|
| Pitch | 피치 시프트 양 | -24 ~ +24 반음 |
| Mix | Dry/Wet 비율 | 0 ~ 100% |
| Expression | 익스프레션 페달 입력 | 0 ~ 100% |

### 모드
- **Whammy**: 익스프레션으로 피치 스윕
- **Harmony**: 고정 인터벌 (3도, 5도 등)
- **Detune**: 미세 디튜닝 (코러스 효과)
- **Octave**: 옥타브 업/다운

### 고급 옵션
- **Glitch Amount**: 아티팩트 양 조절
- **Tracking Speed**: 피치 감지 속도
- **Latency Mode**: Low/High Quality

---

## 7. 개발 로드맵

### Phase 1: 프로토타입
- [ ] 기본 그래뉼러 피치 시프터 구현
- [ ] JUCE 프로젝트 설정
- [ ] 단순 피치 업/다운 테스트

### Phase 2: 핵심 기능
- [ ] 익스프레션 페달 (MIDI CC) 연동
- [ ] 크로스페이드 최적화
- [ ] 프리셋 모드 구현

### Phase 3: UI/UX
- [ ] 페달 스타일 UI 디자인
- [ ] 피치 시각화
- [ ] h4ppy Labs 브랜딩 적용

### Phase 4: 최적화
- [ ] 레이턴시 최소화
- [ ] CPU 사용량 최적화
- [ ] 다양한 샘플레이트 지원

---

## 8. 참고 자료

### 포럼/토론
- [KVR: How to pitch shift UP in real time?](https://www.kvraudio.com/forum/viewtopic.php?t=476173)
- [Cycling '74: Whammy pedal style pitch shifting](https://cycling74.com/forums/how-to-get-glitchy-whammy-pedal-style-pitch-shifting)
- [JUCE Forum: Pitch Shift Algorithms](https://forum.juce.com/t/pitch-shift-slow-down-algorithms/19219)

### 기술 문서
- [Wikipedia: DigiTech Whammy](https://en.wikipedia.org/wiki/DigiTech_Whammy)
- [Analog Devices: Gamechanger Audio Bigsby Pitch-Shifting Pedal](https://www.analog.com/en/resources/technical-articles/gamechanger-audio-bigsby-pitch-shifting-pedal.html)
- [WPI Lab: Real-Time Pitch Shifting](https://schaumont.dyn.wpi.edu/ece4703b22/lab5x.html)

### 영상/발표
- ADC22: "Four Ways To Write A Pitch-Shifter" (Signalsmith Audio)

---

## 9. 제품명 후보

| 이름 | 의미 |
|------|------|
| **WHAMMA** | Whammy + h4ppy Labs 스타일 |
| **PITCHA** | Pitch + a |
| **BENDA** | Bend + a (와미바 벤딩) |
| **SHIFTA** | Shift + a |
| **WARPA** | Warp + a |

---

*문서 작성일: 2025-11-25*
*h4ppy Labs*
