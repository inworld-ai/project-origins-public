/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacterPlaybackAudioLip.h"

#include "InworldBlueprintFunctionLibrary.h"

#include "OVRLipSyncFrame.h"
#include "OVRLipSyncContextWrapper.h"
#include "OVRLipSync.h"
#include "OVRLipSyncPlaybackActorComponent.h"

void UInworldCharacterPlaybackAudioLip::BeginPlay_Implementation()
{
	Super::BeginPlay_Implementation();

	LipSyncComponent = Cast<UOVRLipSyncPlaybackActorComponent>(OwnerActor->GetComponentByClass(UOVRLipSyncPlaybackActorComponent::StaticClass()));
	ensureMsgf(LipSyncComponent.IsValid(), TEXT("UInworldCharacterPlaybackAudioLipSync owner doesn't contain OVRLipSyncPlaybackActorComponent"));

	AudioComponent->OnAudioFinishedNative.Remove(AudioFinishedHandle);
	AudioFinishedHandle = {};
}

void UInworldCharacterPlaybackAudioLip::OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message)
{
	SoundWave = nullptr;
	CurrentAudioPlaybackPercent = 0.f;
	SoundDuration = 0.f;
	GetWorld()->GetTimerManager().ClearTimer(TimeoutHandle);
	if (Message.SoundData.Num() > 0 && Message.bAudioFinal)
	{
		SoundWave = UInworldBlueprintFunctionLibrary::DataArrayToSoundWave(Message.SoundData);
		SoundDuration = SoundWave->GetDuration();

		if (SoundDuration > 0.f)
		{
			AudioComponent->SetSound(SoundWave);

			if (LipSyncComponent.IsValid())
			{
				auto* LipSyncSequence = SoundWaveToLipSyncSequence(SoundWave, (uint8*)Message.SoundData.GetData(), Message.SoundData.Num());
				if (LipSyncSequence)
				{
					LipSyncComponent->Stop();
					LipSyncComponent->Start(AudioComponent.Get(), Cast<UOVRLipSyncFrameSequence>(LipSyncSequence));
					GetWorld()->GetTimerManager().SetTimer(TimeoutHandle, this, &UInworldCharacterPlaybackAudioLip::OnTimeoutTimer, SoundDuration + 0.1f, false);
					LockMessageQueue();
				}
			}
		}
	}
	OnUtteranceStarted.Broadcast(SoundWave != nullptr ? SoundDuration : 0.f, Message.Text);
}

void UInworldCharacterPlaybackAudioLip::OnTimeoutTimer()
{
	VisemeBlends = FInworldCharacterVisemeBlends();
	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
	OnUtteranceStopped.Broadcast();
	UnlockMessageQueue();
}

UOVRLipSyncFrameSequence* UInworldCharacterPlaybackAudioLip::SoundWaveToLipSyncSequence(USoundWave* SoundWaveToSync, uint8* Data, int32 Num)
{
	constexpr float SequenceDuration = 0.01f;

	const int32 NumChannels = SoundWaveToSync->NumChannels;
	const int32 SampleRate = SoundWaveToSync->GetSampleRateForCurrentPlatform();
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
