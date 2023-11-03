// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "InworldAudioComponent.generated.h"

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldAudioComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetVoiceEffect(USoundEffectSourcePresetChain* InEffectSourcePresetChain, float InVolumeMultiplier)
	{
		EffectSourcePresetChain = InEffectSourcePresetChain;
		SetVolumeMultiplier(VolumeMultiplier);
	}

	virtual void Play(float StartTime /* = 0.f*/) override;

private:
	UPROPERTY()
	USoundEffectSourcePresetChain* EffectSourcePresetChain = nullptr;
};
