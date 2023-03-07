/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerComponent.h"
#include "proto/ProtoDisableWarning.h"
#include "InworldApi.h"
#include "InworldCharacterComponent.h"
#include "Packets.h"

void UInworldPlayerComponent::BeginPlay()
{
    Super::BeginPlay();

    InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
    InworldSubsystem->RegisterPlayerComponent(this);
}

void UInworldPlayerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (InworldSubsystem.IsValid())
    {
        InworldSubsystem->UnregisterPlayerComponent();
    }

    Super::EndPlay(EndPlayReason);
}

void UInworldPlayerComponent::HandleConnectionStateChanged(EInworldConnectionState State)
{
    
}

void UInworldPlayerComponent::SetTargetCharacter(UInworldCharacterComponent* Character)
{
    if (!ensureMsgf(Character && !Character->GetAgentId().IsNone(), TEXT("UInworldPlayerComponent::SetTargetCharacter: the Character must have valid AgentId")))
    {
        return;
    }

    TargetCharacter = Character;
    TargetCharacter->HandlePlayerInteraction(true);

    OnTargetChange.Broadcast(TargetCharacter.Get());
}

void UInworldPlayerComponent::ClearTargetCharacter()
{
    if (TargetCharacter.IsValid())
    {
        TargetCharacter->HandlePlayerInteraction(false);
        StopAudioSession();
    }
    TargetCharacter = nullptr;
     
    OnTargetChange.Broadcast(nullptr);
}

void UInworldPlayerComponent::StartAudioSession()
{
    if (TargetCharacter.IsValid())
    {
        InworldSubsystem->StartAudioSession(TargetCharacter->GetAgentId());
    }
}

void UInworldPlayerComponent::StopAudioSession()
{
	if (TargetCharacter.IsValid())
	{
		InworldSubsystem->StopAudioSession(TargetCharacter->GetAgentId());
	}
}

void UInworldPlayerComponent::SendAudioDataMessage(const std::string& Data)
{
    if (TargetCharacter.IsValid())
    {
        InworldSubsystem->SendAudioDataMessage(TargetCharacter->GetAgentId(), Data);
    }
}

void UInworldPlayerComponent::SendAudioMessage(USoundWave* SoundWave)
{
	if (TargetCharacter.IsValid())
	{
		InworldSubsystem->SendAudioMessage(TargetCharacter->GetAgentId(), SoundWave);
	}
}

