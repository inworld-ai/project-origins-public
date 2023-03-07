/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldUtils.h"
#include "Components/AudioComponent.h"
#include "InworldCharacterPlayback.h"

#include "InworldCharacterPlaybackAudio.generated.h"

UCLASS(BlueprintType, Blueprintable)
class INWORLDAIINTEGRATION_API UInworldCharacterPlaybackAudio : public UInworldCharacterPlayback
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldCharacterInteraction, FString, Text, float, Duration);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterInteraction OnCharacterInteraction;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldCharacterUtteranceStarted, float, Duration, FString,  CustomGesture);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterUtteranceStarted OnUtteranceStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterUtteranceStopped);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterUtteranceStopped OnUtteranceStopped;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterUtteranceInterrupted);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterUtteranceInterrupted OnUtteranceInterrupted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterSilenceStarted, float, Duration);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterSilenceStarted OnSilenceStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterSilenceStopped);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterSilenceStopped OnSilenceStopped;

	virtual void BeginPlay_Implementation() override;
	virtual void EndPlay_Implementation() override;

	virtual bool Update() override;

	virtual void Visit(const Inworld::FCharacterMessageUtterance& Event) override;
	virtual void Visit(const Inworld::FCharacterMessageSilence& Event) override;

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	float GetRemainingTimeForCurrentUtterance() const;

	virtual void HandlePlayerTalking(const Inworld::FCharacterMessageUtterance& Message) override;

	enum class EState
	{
		Idle,
		Audio,
		Silence,
	};

	EState GetState() const 
	{
		return State;
	}

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetVoiceEffect(USoundEffectSourcePresetChain* InEffectSourcePresetChain, float InVolumeMultiplier)
	{
		EffectSourcePresetChain = InEffectSourcePresetChain;
		VolumeMultiplier = InVolumeMultiplier;
	}

protected:

	virtual void PlayAudio(USoundWave* SoundWave, const std::string& AudioData);

	TWeakObjectPtr<UAudioComponent> AudioComponent;

	EState State = EState::Idle;

private:
	void OnAudioPlaybackPercent(const UAudioComponent* InAudioComponent, const USoundWave* InSoundWave, float Percent);

	FDelegateHandle AudioPlaybackPercentHandle;
	float CurrentAudioPlaybackPercent = 0.f;

	UPROPERTY()
	USoundEffectSourcePresetChain* EffectSourcePresetChain;

	float VolumeMultiplier = 1.f;

	Inworld::Utils::FWorldTimer SilenceTimer = Inworld::Utils::FWorldTimer(0.f);
};