#include "RNNoise.h"
#include "IPlug_include_in_plug_src.h"

RNNoise::RNNoise(IPlugInstanceInfo instanceInfo)
: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  GetParam(kNoiseFloor)->InitGain("Noise Floor", 0., -20., 0.); // TODO: default
  
  sts = rnnoise_create(nullptr);
  
  MakeDefaultPreset();
}

RNNoise::~RNNoise()
{
  if(sts)
    rnnoise_destroy(sts);
}

#if IPLUG_DSP
void RNNoise::OnReset()
{
  if(mPreviousSR != GetSampleRate())
  {
    mPreviousSR = GetSampleRate();
    
    if(sts)
      rnnoise_destroy(sts);
    
    sts = rnnoise_create(nullptr);
    rnnoise_set_param(sts, RNNOISE_PARAM_MAX_ATTENUATION, GetParam(kNoiseFloor)->DBToAmp()); // TODO: DB mapping correct?
    rnnoise_set_param(sts, RNNOISE_PARAM_SAMPLE_RATE, GetSampleRate());
  }
}

void RNNoise::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  for (int s = 0; s < nFrames; s++) {
    ibuffer[frameCount] = (inputs[0][s] + inputs[1][s]) / 2.f;
    outputs[0][s] = outputs[1][s] = obuffer[frameCount++];
    
    if(frameCount == kNumFrames) {
      rnnoise_process_frame(sts, obuffer, ibuffer);
      frameCount = 0;
    }
  }
}

void RNNoise::OnParamChange(int paramIdx)
{
  if(sts)
    rnnoise_set_param(sts, RNNOISE_PARAM_MAX_ATTENUATION, GetParam(kNoiseFloor)->Value()); // TODO: DB mapping correct?
}
#endif
