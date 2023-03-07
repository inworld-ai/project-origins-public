/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterPlaybackAudio.h"
#include "InworldUtils.h"
#include <Components/AudioComponent.h>

void UInworldCharacterPlaybackAudio::BeginPlay_Implementation()
{
	Super::BeginPlay_Implementation();

	auto AudioComponents = OwnerActor->GetComponentsByTag(UAudioComponent::StaticClass(), TEXT("Voice"));
	if (AudioComponents.Num() != 0)
	{
		AudioComponent = Cast<UAudioComponent>(AudioComponents[0]);
	}
	else
	{
		AudioComponent = Cast<UAudioComponent>(OwnerActor->GetComponentByClass(UAudioComponent::StaticClass()));
	}

	if (ensureMsgf(AudioComponent.IsValid(), TEXT("UInworldCharacterPlaybackAudio owner doesn't contain AudioComponent")))
	{
		AudioPlaybackPercentHandle = AudioComponent->OnAudioPlaybackPercentNative.AddUObject(this, &UInworldCharacterPlaybackAudio::OnAudioPlaybackPercent);
	}
}

void UInworldCharacterPlaybackAudio::EndPlay_Implementation()
{
	if (AudioComponent.IsValid())
	{
		AudioComponent->OnAudioPlaybackPercentNative.Remove(AudioPlaybackPercentHandle);
	}

	Super::EndPlay_Implementation();
}

bool UInworldCharacterPlaybackAudio::Update()
{
	EState CurState = EState::Idle;
	if (!SilenceTimer.IsExpired(OwnerActor->GetWorld()))
	{
		CurState = EState::Silence;
	}
	else if (AudioComponent->IsPlaying())
	{
		CurState = EState::Audio;
	}

	if (CurState == EState::Idle)
	{
		if (State == EState::Audio)
		{
			OnUtteranceStopped.Broadcast();
		}
		else if (State == EState::Silence)
		{
			OnSilenceStopped.Broadcast();
		}
	}

	State = CurState;

	return State == EState::Idle;
}

void UInworldCharacterPlaybackAudio::Visit(const Inworld::FCharacterMessageUtterance& Event)
{
	if (!ensure(State == EState::Idle))
	{
		return;
	}

	float Duration = 0.f;
	if (!Event.AudioData.empty())
	{
		USoundWave* SoundWave = Inworld::Utils::StringToSoundWave(Event.AudioData);
		if (ensure(SoundWave))
		{
			SoundWave->SourceEffectChain = EffectSourcePresetChain;
			AudioComponent->SetVolumeMultiplier(VolumeMultiplier);
			AudioComponent->SetSound(SoundWave);
			State = EState::Audio;
			PlayAudio(SoundWave, Event.AudioData);
			Duration = SoundWave->GetDuration();
			OnCharacterInteraction.Broadcast(Event.Text, Duration);
		}
	}
	OnUtteranceStarted.Broadcast(Duration, Event.CustomGesture);
}

void UInworldCharacterPlaybackAudio::Visit(const Inworld::FCharacterMessageSilence& Event)
{
	if (!ensure(State == EState::Idle))
	{
		return;
	}

	SilenceTimer.SetOneTime(OwnerActor->GetWorld(), Event.Duration);
	OnSilenceStarted.Broadcast(Event.Duration);
}

float UInworldCharacterPlaybackAudio::GetRemainingTimeForCurrentUtterance() const
{
	if (!AudioComponent.IsValid() || !AudioComponent->Sound || !AudioComponent->IsPlaying())
	{
		return 0.f;
	}

	return (1.f - CurrentAudioPlaybackPercent) * AudioComponent->Sound->Duration;
}

void UInworldCharacterPlaybackAudio::HandlePlayerTalking(const Inworld::FCharacterMessageUtterance& Message)
{
	auto CurrentMessage = GetCurrentMessage();
	if (CurrentMessage && CurrentMessage->InteractionId != Message.InteractionId && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
		OnUtteranceInterrupted.Broadcast();
	}
}

void UInworldCharacterPlaybackAudio::PlayAudio(USoundWave* SoundWave, const std::string& AudioData)
{
	AudioComponent->Play();
}

void UInworldCharacterPlaybackAudio::OnAudioPlaybackPercent(const UAudioComponent* InAudioComponent, const USoundWave* InSoundWave, float Percent)
{
	CurrentAudioPlaybackPercent = Percent;
}
