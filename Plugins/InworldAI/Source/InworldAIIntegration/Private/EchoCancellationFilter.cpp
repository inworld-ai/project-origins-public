/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "EchoCancellationFilter.h"
#include "webrtc/aec.h"
#include <GenericPlatform/GenericPlatformMath.h>

FEchoCancellationFilter::FEchoCancellationFilter()
#ifdef INWORLD_WEB_RTC
    : AecHandle(WebRtcAec3_Create(16000))
#endif
{

}

FEchoCancellationFilter::~FEchoCancellationFilter()
{
#ifdef INWORLD_WEB_RTC
	WebRtcAec3_Free(AecHandle);
#endif
}

std::vector<int16> FEchoCancellationFilter::FilterAudio(const std::vector<int16>& InAudio, const std::vector<int16>& OutAudio)
{
    std::vector<int16> FilteredAudio = InAudio;
    
#ifdef INWORLD_WEB_RTC
	constexpr int32 NumSamples = 160;
	const int32 MaxSamples = FMath::Min(InAudio.size(), OutAudio.size()) / NumSamples * NumSamples;

	for (int32 i = 0; i < MaxSamples; i += NumSamples)
	{
		WebRtcAec3_BufferFarend(AecHandle, OutAudio.data() + i);
		WebRtcAec3_Process(AecHandle, InAudio.data() + i, FilteredAudio.data() + i);
	}
#endif
	return FilteredAudio;
}
