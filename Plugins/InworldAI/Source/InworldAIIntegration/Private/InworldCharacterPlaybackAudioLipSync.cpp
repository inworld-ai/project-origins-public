/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterPlaybackAudioLipSync.h"

#if INWORLD_OVR_LIPSYNC

#include "OVRLipSyncFrame.h"
#include "OVRLipSyncContextWrapper.h"
#include "OVRLipSync.h"
#include "OVRLipSyncPlaybackActorComponent.h"

void UInworldCharacterPlaybackAudioLipSync::BeginPlay_Implementation()
{
	Super::BeginPlay_Implementation();

	LipSyncComponent = Cast<UOVRLipSyncPlaybackActorComponent>(OwnerActor->GetComponentByClass(UOVRLipSyncPlaybackActorComponent::StaticClass()));
	ensureMsgf(LipSyncComponent.IsValid(), TEXT("UInworldCharacterPlaybackAudioLipSync owner doesn't contain OVRLipSyncPlaybackActorComponent"));
}

void UInworldCharacterPlaybackAudioLipSync::PlayAudio(USoundWave* SoundWave, const std::string& AudioData)
{
	if (LipSyncComponent.IsValid())
	{
		auto* LipSyncSequence = SoundWaveToLipSyncSequence(SoundWave, (uint8*)AudioData.data(), AudioData.size());
		if (LipSyncSequence)
		{
			LipSyncComponent->Stop();
			LipSyncComponent->Start(AudioComponent.Get(), Cast<UOVRLipSyncFrameSequence>(LipSyncSequence));
		}
	}
}

UOVRLipSyncFrameSequence* UInworldCharacterPlaybackAudioLipSync::SoundWaveToLipSyncSequence(USoundWave* SoundWave, uint8* Data, int32 Num)
{
	constexpr float SequenceDuration = 0.01f;

	const int32 NumChannels = SoundWave->NumChannels;
	const int32 SampleRate = SoundWave->GetSampleRateForCurrentPlatform();
	const int32 PCMDataSize = Num / sizeof(int16);
	const int16* PCMData = reinterpret_cast<int16*>(Data);
	const int32 ChunkSizeSamples = static_cast<int32>(SampleRate * SequenceDuration);
	const int32 ChunkSize = NumChannels * ChunkSizeSamples;

	float LaughterScore = 0.f;
	int32 FrameDelayInMs = 0;
	TArray<float> Visemes;

	UOVRLipSyncContextWrapper Context(ovrLipSyncContextProvider_Enhanced, SampleRate, 4096, FString());
	TArray<int16> Samples;
	Samples.SetNumZeroed(ChunkSize);
	Context.ProcessFrame(Samples.GetData(), ChunkSizeSamples, Visemes, LaughterScore, FrameDelayInMs, NumChannels > 1);

	const int32 FrameOffset = static_cast<int32>(FrameDelayInMs * SampleRate / 1000 * NumChannels);

	auto Sequence = NewObject<UOVRLipSyncFrameSequence>(UOVRLipSyncFrameSequence::StaticClass());
	for (int32 Offset = 0; Offset < PCMDataSize + FrameOffset; Offset += ChunkSize)
	{
		const int32 RemainingSamples = PCMDataSize - Offset;
		if (RemainingSamples >= ChunkSize)
		{
			Context.ProcessFrame(PCMData + Offset, ChunkSizeSamples, Visemes, LaughterScore, FrameDelayInMs, NumChannels > 1);
		}
		else
		{
			if (RemainingSamples > 0)
			{
				FMemory::Memcpy(Samples.GetData(), PCMData + Offset, sizeof(int16) * RemainingSamples);
				FMemory::Memset(Samples.GetData() + RemainingSamples, 0, ChunkSize - RemainingSamples);
			}
			else
			{
				FMemory::Memset(Samples.GetData(), 0, ChunkSize);
			}
			Context.ProcessFrame(Samples.GetData(), ChunkSizeSamples, Visemes, LaughterScore, FrameDelayInMs, NumChannels > 1);
		}

		if (Offset >= FrameOffset)
		{
			Sequence->Add(Visemes, LaughterScore);
		}
	}

	return Sequence;
}

#endif