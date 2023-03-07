/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldUtils.h"
#include "Components/SynthComponent.h"
#include "EchoCancellationFilter.h"

#include "InworldPlayerAudioCaptureComponent.generated.h"

class UInworldPlayerComponent;
class UInworldCharacterComponent;
class UInworldApiSubsystem;
class USoundWave;
class UAudioCaptureComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerAudioCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UInworldPlayerAudioCaptureComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void StartVoiceCapture();
    void StopVoiceCapture();
    void StartVoiceChunkCapture();
    void StopVoiceChunkCapture();

    void UpdateVoiceCapture();

    void OnAudioEnvelope(const class UAudioComponent* AudioComponent, const float Value);

private:
    void OnTargetChanged(UInworldCharacterComponent* Target);

	UPROPERTY(EditAnywhere, Category = "Performance")
	float NotifyVoiceVolumeThreshold = 2.f;

    FEchoCancellationFilter EchoFilter;

	TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;
    TWeakObjectPtr<UInworldPlayerComponent> PlayerComponent;
    TWeakObjectPtr<USynthComponent> AudioCaptureComponent;

    FDelegateHandle TargetChangeHandle;
    FDelegateHandle AudioEnvelopeHandle;

	Inworld::Utils::FWorldTimer SendSoundMessageTimer = Inworld::Utils::FWorldTimer(0.1f);
	Inworld::Utils::FWorldTimer NotifyVoiceTimer = Inworld::Utils::FWorldTimer(0.2f);

    TAtomic<bool> bCapturingVoice = false;
    TAtomic<bool> bCapturingAudio = false;

    struct FPlayerVoiceCaptureInfo
    {
        FPlayerVoiceCaptureInfo()
            : MicSoundWave(nullptr)
            , OutputSoundWave(nullptr)
        {}

        FPlayerVoiceCaptureInfo(USoundWave* InMicSoundWave, USoundWave* InOutputSoundWave)
            : MicSoundWave(InMicSoundWave)
            , OutputSoundWave(InOutputSoundWave)
        {}

        USoundWave* MicSoundWave = nullptr;
        USoundWave* OutputSoundWave = nullptr;
    };

    TQueue<FPlayerVoiceCaptureInfo> OutgoingPlayerVoiceCaptureQueue;
	bool bNotifyingVoice = false;
};
