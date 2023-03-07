/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterPlaybackAudio.h"

#include "InworldCharacterPlaybackAudioLipSync.generated.h"

#if INWORLD_OVR_LIPSYNC
class UOVRLipSyncFrameSequence;
class UOVRLipSyncPlaybackActorComponent;
#endif

UCLASS(BlueprintType, Blueprintable)
class INWORLDAIINTEGRATION_API UInworldCharacterPlaybackAudioLipSync : public UInworldCharacterPlaybackAudio
{
	GENERATED_BODY()

#if INWORLD_OVR_LIPSYNC
public:
	virtual void BeginPlay_Implementation() override;

protected:
	virtual void PlayAudio(USoundWave* SoundWave, const std::string& AudioData) override;

private:
	UOVRLipSyncFrameSequence* SoundWaveToLipSyncSequence(USoundWave* SoundWave, uint8* Data, int32 Num);

	TWeakObjectPtr<UOVRLipSyncPlaybackActorComponent> LipSyncComponent;
#endif

};
