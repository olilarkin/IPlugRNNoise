#pragma once

#include "IPlug_include_in_plug_hdr.h"
extern "C" {
#include "rnnoise-nu.h"
}

const int kNumPrograms = 1;

enum EParams
{
  kNoiseFloor = 0,
  kNumParams
};

static constexpr const unsigned long kNumFrames = 480; // This is how much rnnoise expects. 10ms at 48khz

class RNNoise : public IPlug
{
public:
  RNNoise(IPlugInstanceInfo instanceInfo);
  ~RNNoise();
  
#if IPLUG_DSP // All DSP methods and member variables should be within an IPLUG_DSP guard, should you want distributed UI
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
private:
  DenoiseState* sts;
  int frameCount = 0;
  float ibuffer[kNumFrames] {};
  float obuffer[kNumFrames] {};
  double mPreviousSR = DEFAULT_SAMPLE_RATE;
#endif
};
