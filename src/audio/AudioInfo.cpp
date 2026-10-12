#include "AudioInfo.h"
#include "miniaudio.h"                    // only the declarations. The code lives in miniaudioBackend.cpp

double AudioInfo::seconds(const std::string& path)
{
    ma_decoder decoder;

    if (ma_decoder_init_file(path.c_str(), nullptr, &decoder) != MA_SUCCESS)
        return 0.0;

    ma_uint64 frames = 0;
    double result = 0.0;

    if (ma_decoder_get_length_in_pcm_frames(&decoder, &frames) == MA_SUCCESS && decoder.outputSampleRate > 0)
        result = (double)frames / (double)decoder.outputSampleRate;

    ma_decoder_uninit(&decoder);
    return result;
}
