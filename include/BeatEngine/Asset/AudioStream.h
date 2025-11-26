#pragma once

#include "BeatEngine/Base/Asset.h"

#include <miniaudio.h>
// #include <samplerate.h>

class AudioStream : Base::Asset {
private:
	ma_decoder m_Decoder;
	
public:
	AudioStream();
	~AudioStream() override;
};
