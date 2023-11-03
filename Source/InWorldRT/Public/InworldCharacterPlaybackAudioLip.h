// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterPlaybackAudio.h"
#include "InworldCharacterPlaybackAudioLip.generated.h"

class UOVRLipSyncFrameSequence;
class UOVRLipSyncPlaybackActorComponent;

UCLASS(BlueprintType, Blueprintable)
class INWORLDRT_API UInworldCharacterPlaybackAudioLip : public UInworldCharacterPlaybackAudio
{
	GENERATED_BODY()
public:
	virtual void BeginPlay_Implementation() override;

protected:
	virtual void OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message) override;

private:
	UOVRLipSyncFrameSequence* SoundWaveToLipSyncSequence(USoundWave* SoundWaveToSync, uint8* Data, int32 Num);

	TWeakObjectPtr<UOVRLipSyncPlaybackActorComponent> LipSyncComponent;

	UFUNCTION()
	void OnTimeoutTimer();

	FTimerHandle TimeoutHandle;
};
