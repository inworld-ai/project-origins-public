/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerAudioCaptureComponent.h"
#include "InworldPlayerComponent.h"
#include "AudioCaptureComponent.h"
#include "AudioMixerBlueprintLibrary.h"
#include "AudioMixerDevice.h"
#include "AudioMixerSubmix.h"
#include "InworldPlayerAudioCaptureComponent.h"
#include "InworldApi.h"

static TAutoConsoleVariable<bool> CVarEnableEchoFilter(
	TEXT("Inworld.Experimental.EnableEchoFilter"),
	false,
	TEXT(""));

UInworldPlayerAudioCaptureComponent::UInworldPlayerAudioCaptureComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bTickEvenWhenPaused = true;
}

void UInworldPlayerAudioCaptureComponent::BeginPlay()
{
    Super::BeginPlay();

	InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();

    PlayerComponent = Cast<UInworldPlayerComponent>(GetOwner()->GetComponentByClass(UInworldPlayerComponent::StaticClass()));
    if (ensureMsgf(PlayerComponent.IsValid(), TEXT("UInworldPlayerAudioCaptureComponent::BeginPlay: add InworldPlayerComponent.")))
    {
        TargetChangeHandle = PlayerComponent->OnTargetChange.AddUObject(this, &UInworldPlayerAudioCaptureComponent::OnTargetChanged);
    }

    AudioCaptureComponent = Cast<USynthComponent>(GetOwner()->GetComponentByClass(USynthComponent::StaticClass()));
    
    if (ensureMsgf(AudioCaptureComponent.IsValid() && AudioCaptureComponent->SoundSubmix, TEXT("UInworldPlayerAudioCaptureComponent::BeginPlay: AudioCaptureComponent is invalid. Ensure it set up with valid base submix.")))
    {
        AudioEnvelopeHandle = AudioCaptureComponent->OnAudioEnvelopeValueNative.AddUObject(this, &UInworldPlayerAudioCaptureComponent::OnAudioEnvelope);
    }
}

void UInworldPlayerAudioCaptureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (PlayerComponent.IsValid())
    {
        PlayerComponent->OnTargetChange.Remove(TargetChangeHandle);
    }

	if (AudioCaptureComponent.IsValid())
	{
		AudioCaptureComponent->OnAudioEnvelopeValueNative.Remove(AudioEnvelopeHandle);
	}

    Super::EndPlay(EndPlayReason);
}

void UInworldPlayerAudioCaptureComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (TickType == ELevelTick::LEVELTICK_PauseTick || InworldSubsystem->GetConnectionState() != EInworldConnectionState::Connected)
    {
        StopVoiceCapture();
        return;
    }

    // restore voice capture after pause
	if (PlayerComponent->GetTargetCharacter() && !bCapturingVoice)
	{
        StartVoiceCapture();
	}

	UpdateVoiceCapture();
}

void UInworldPlayerAudioCaptureComponent::StartVoiceCapture()
{
    if (bCapturingVoice)
    {
        return;
    }

    if (!ensure(PlayerComponent->GetTargetCharacter()))
    {
        Inworld::Utils::ErrorLog("UInworldPlayerAudioCaptureComponent::StartVoiceCapture: TargetAgentId is empty.", true);
        return;
    }

	if (!ensure(AudioCaptureComponent.IsValid()))
	{
		Inworld::Utils::ErrorLog("UInworldPlayerAudioCaptureComponent::StartVoiceCapture: AudioCaptureComponent is empty.", true);
		return;
	}

    PlayerComponent->StartAudioSession();
    bCapturingVoice = true;
}

void UInworldPlayerAudioCaptureComponent::StopVoiceCapture()
{
    if (!bCapturingVoice)
    {
        return;
    }

    bCapturingVoice = false;
    OnAudioEnvelope(nullptr, 0.f);
    PlayerComponent->StopAudioSession();
}

void UInworldPlayerAudioCaptureComponent::StartVoiceChunkCapture()
{
    UAudioMixerBlueprintLibrary::StartRecordingOutput(this, SendSoundMessageTimer.GetThreshold(), Cast<USoundSubmix>(AudioCaptureComponent->SoundSubmix));
    
    //if (CVarEnableEchoFilter.GetValueOnGameThread())
    //{
        UAudioMixerBlueprintLibrary::StartRecordingOutput(this, SendSoundMessageTimer.GetThreshold());
    //}
}

void UInworldPlayerAudioCaptureComponent::StopVoiceChunkCapture()
{
    USoundWave* SoundWaveMic = UAudioMixerBlueprintLibrary::StopRecordingOutput(this, EAudioRecordingExportType::SoundWave, "InworldVoiceCapture", "InworldVoiceCapture", Cast<USoundSubmix>(AudioCaptureComponent->SoundSubmix));
    if (SoundWaveMic)
    {
        //if (CVarEnableEchoFilter.GetValueOnGameThread())
        //{
            USoundWave* SoundWaveOutput = UAudioMixerBlueprintLibrary::StopRecordingOutput(this, EAudioRecordingExportType::SoundWave, "InworldOutputCapture", "InworldOutputCapture");
            OutgoingPlayerVoiceCaptureQueue.Enqueue({ SoundWaveMic, SoundWaveOutput });
        //}
        //else
        //{
            //OutgoingPlayerVoiceCaptureQueue.Enqueue({ SoundWaveMic, nullptr });
        //}
    }
}

void UInworldPlayerAudioCaptureComponent::UpdateVoiceCapture()
{
    if (SendSoundMessageTimer.CheckPeriod(GetWorld()))
    {
        if (bCapturingVoice)
        {
            FAudioThread::RunCommandOnAudioThread([this]()
            {
                if (!bCapturingAudio)
                {
                    AudioCaptureComponent->Start();
                    bCapturingAudio = true;
                }
                StopVoiceChunkCapture();
                StartVoiceChunkCapture();
            });
        }
        else
        {
            if (bCapturingAudio)
            {
                FAudioThread::RunCommandOnAudioThread([this]()
                {
                    if (bCapturingAudio)
                    {
                        AudioCaptureComponent->Stop();
                        bCapturingAudio = false;
                        StopVoiceChunkCapture();
                    }
                });
            }
        }
    }

    if (!bCapturingVoice)
    {
        OutgoingPlayerVoiceCaptureQueue.Empty();
    }

    FPlayerVoiceCaptureInfo NextPlayerAudioChunk;
    if (OutgoingPlayerVoiceCaptureQueue.Dequeue(NextPlayerAudioChunk))
    {
        if (PlayerComponent->GetTargetCharacter() && NextPlayerAudioChunk.MicSoundWave)
        {
            if (NextPlayerAudioChunk.OutputSoundWave)
            {
                std::vector<int16> MicData, OutputData;
                if (Inworld::Utils::SoundWaveToVec(NextPlayerAudioChunk.MicSoundWave, MicData) && Inworld::Utils::SoundWaveToVec(NextPlayerAudioChunk.OutputSoundWave, OutputData))
                {
                    std::vector<int16> FilteredData = EchoFilter.FilterAudio(MicData, OutputData);

                    std::string StrData;
                    StrData.resize(FilteredData.size() * sizeof(int16));
                    FMemory::Memcpy((void*)StrData.data(), (void*)FilteredData.data(), StrData.size());

                    PlayerComponent->SendAudioDataMessage(StrData);
                }
            }
            else
            {
                PlayerComponent->SendAudioMessage(NextPlayerAudioChunk.MicSoundWave);
            }
        }
    }
}

void UInworldPlayerAudioCaptureComponent::OnAudioEnvelope(const class UAudioComponent* AudioComponent, const float Value)
{
    const float Volume = Value * 100.f;
    const bool bTimeThresholdExpired = NotifyVoiceTimer.IsExpired(GetWorld());
    if (Volume > NotifyVoiceVolumeThreshold && !bNotifyingVoice)
    {
        if (bTimeThresholdExpired)
        {
            InworldSubsystem->NotifyVoiceStarted();
            bNotifyingVoice = true;
            NotifyVoiceTimer.CheckPeriod(GetWorld());
        }
		return;
    }

    if (Volume < NotifyVoiceVolumeThreshold && bNotifyingVoice)
	{
        if (bTimeThresholdExpired)
        {
            InworldSubsystem->NotifyVoiceStopped();
            bNotifyingVoice = false;
            NotifyVoiceTimer.CheckPeriod(GetWorld());
        }
        return;
    }

    NotifyVoiceTimer.CheckPeriod(GetWorld());
}

void UInworldPlayerAudioCaptureComponent::OnTargetChanged(UInworldCharacterComponent* Target)
{
    if (Target)
    {
        StartVoiceCapture();
    }
    else
    {
        StopVoiceCapture();
    }
}

