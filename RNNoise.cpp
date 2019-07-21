#include "RNNoise.h"
#include "IPlug_include_in_plug_src.h"

RNNoise::RNNoise(IPlugInstanceInfo instanceInfo)
: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  GetParam(kNoiseFloor)->InitDouble("Noise Floor", 0.2, 0., 1., 0.01, ""); // TODO: default
  
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
    rnnoise_set_param(sts, RNNOISE_PARAM_MAX_ATTENUATION, GetParam(kNoiseFloor)->Value()); // TODO: DB mapping correct?
    rnnoise_set_param(sts, RNNOISE_PARAM_SAMPLE_RATE, GetSampleRate());
  }
}

void RNNoise::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  for (int s = 0; s < nFrames; s++) {
    // Input is in floatingpoint -1.0 to 1.0 range. Rnnoise uses float, but expects
    // int16_t's range [-32768, 32767] so convert.
    ibuffer[frameCount++] = ((inputs[0][s] + inputs[1][s]) / 2.f) * 32768;
    
    if(frameCount == kNumFrames) {
      rnnoise_process_frame(sts, &obuffer[outFrameCount], ibuffer);
      outFrameCount += frameCount;
      frameCount = 0;
    }
  }
    
  int framesToOutput = std::min(nFrames, outFrameCount);
  for (int s = 0; s < framesToOutput; s++) {
     outputs[0][s] = outputs[1][s] = obuffer[s] / 32768;
  }
  int left = outFrameCount - framesToOutput;
  if (left > 0) {
    memmove(obuffer, obuffer + framesToOutput, left * sizeof(obuffer[0]));
  }
  outFrameCount -= framesToOutput;
}

void RNNoise::OnParamChange(int paramIdx)
{
  if(sts)
    rnnoise_set_param(sts, RNNOISE_PARAM_MAX_ATTENUATION, GetParam(kNoiseFloor)->Value()); // TODO: DB mapping correct?
}
#endif
