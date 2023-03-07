/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldApi.h"
#include "proto/ProtoDisableWarning.h"
#include "Packets.h"
#include "InworldComponentInterface.h"
#include "InworldUtils.h"
#include <Engine/Engine.h>

const FString DefaultAuthUrl = "api-studio.inworld.ai";
const FString DefaultTargetUrl = "api-engine.inworld.ai:443";

UInworldApiSubsystem::UInworldApiSubsystem()
    : UWorldSubsystem()
{
    Client.InitClient(
        [this](EInworldConnectionState ConnectionState)
        {
            if (ensureMsgf(PlayerComponent, TEXT("You must add InworldPlayerComponent to your Pawn.")))
            {
                PlayerComponent->HandleConnectionStateChanged(ConnectionState);
            }

            if (ConnectionState == EInworldConnectionState::Connected)
            {
                CurrentRetryConnectionTime = 0.f;
            }

            if (ConnectionState == EInworldConnectionState::Disconnected)
            {
                if (CurrentRetryConnectionTime == 0.f)
                {
                    ResumeSession();
                }
                else
                {
                    GetWorld()->GetTimerManager().SetTimer(RetryConnectionTimerHandle, this, &UInworldApiSubsystem::ResumeSession, CurrentRetryConnectionTime);
                }
                CurrentRetryConnectionTime += FMath::Min(CurrentRetryConnectionTime + RetryConnectionIntervalTime, MaxRetryConnectionTime);
            }

            OnConnectionStateChanged.Broadcast(ConnectionState);
        },
        [this](TSharedPtr<Inworld::FInworldPacket> Packet)
        {
            if (!Packet)
            {
                return;
            }

            auto* SourceComponentPtr = CharacterComponentByAgentId.Find(Packet->Routing.Source.Name);
            if (SourceComponentPtr)
            {
                (*SourceComponentPtr)->HandlePacket(Packet);
            }
            auto* TargetComponentPtr = CharacterComponentByAgentId.Find(Packet->Routing.Target.Name);
            if (TargetComponentPtr)
            {
                (*TargetComponentPtr)->HandlePacket(Packet);
            }
        });
}

void UInworldApiSubsystem::StartSession(const FString& SceneName, const FString& PlayerName, const FString& ApiKey, const FString& ApiSecret, const FString& AuthUrlOverride, const FString& TargetUrlOverride)
{
    if (ApiKey.IsEmpty())
    {
        Inworld::Utils::ErrorLog("Can't Start Session, ApiKey is empty", true);
        return;
    }
    if (ApiSecret.IsEmpty())
    {
        Inworld::Utils::ErrorLog("Can't Start Session, ApiSecret is empty", true);
        return;
    }

    Inworld::FClientOptions Options;
    Options.AuthUrl = !AuthUrlOverride.IsEmpty() ? AuthUrlOverride : DefaultAuthUrl;
    Options.LoadSceneUrl = !TargetUrlOverride.IsEmpty() ? TargetUrlOverride : DefaultTargetUrl;
    Options.SceneName = SceneName;
    Options.ApiKey = ApiKey;
    Options.ApiSecret = ApiSecret;
    Options.PlayerName = PlayerName;
    Options.SentryDSN = SentryDSN;
    Options.SentryTransactionName = SentryTransactionName;
    Options.SentryTransactionOperation = SentryTransactionOperation;

    Client.StartClient(Options,
        [this](const auto& AgentInfos)
        {
            for (const auto& Info : AgentInfos)
            {
                auto* ComponentPtr = CharacterComponentRegistry.FindByPredicate([&Info](const auto C) { return C->GetBrainName() == Info.BrainName; });
                if (ComponentPtr)
                {
                    auto& Component = *ComponentPtr;
                    CharacterComponentByAgentId.Add(Info.AgentId, Component);
                    Component->SetAgentId(Info.AgentId);
                    Component->SetGivenName(Info.GivenName);
                }
                AgentInfoByBrain.Add(Info.BrainName, Info);
            }

            CharacterComponentRegistry.RemoveAll([&](const auto C) {
                auto* AgentInfoPtr = AgentInfoByBrain.Find(C->GetBrainName());
                return !ensureMsgf(AgentInfoPtr, TEXT("Couldn't register component with brain name '%s'"), *C->GetBrainName());
                });

            bCharactersInitialized = true;
        });
}

void UInworldApiSubsystem::PauseSession()
{
    Client.PauseClient();
}

void UInworldApiSubsystem::ResumeSession()
{
    Client.ResumeClient();
}

void UInworldApiSubsystem::StopSession()
{
    Client.StopClient();
}

void UInworldApiSubsystem::RegisterCharacterComponent(Inworld::ICharacterComponent* Component)
{
    if (!bCharactersInitialized)
    {
		CharacterComponentRegistry.Add(Component);
    }
    else
    {
        auto* AgentInfoPtr = AgentInfoByBrain.Find(Component->GetBrainName());
        if (ensureMsgf(AgentInfoPtr, TEXT("Couldn't register component with brain name '%s'"), *Component->GetBrainName()))
        {
            CharacterComponentRegistry.Add(Component);
            CharacterComponentByAgentId.Add(AgentInfoPtr->AgentId, Component);
            Component->SetAgentId(AgentInfoPtr->AgentId);
            Component->SetGivenName(AgentInfoPtr->GivenName);
        }
    }
}

void UInworldApiSubsystem::UnregisterCharacterComponent(Inworld::ICharacterComponent* Component)
{
    CharacterComponentRegistry.Remove(Component);
    CharacterComponentByAgentId.Remove(Component->GetAgentId());
}

void UInworldApiSubsystem::RegisterPlayerComponent(Inworld::IPlayerComponent* Component)
{
    PlayerComponent = Component;
}

void UInworldApiSubsystem::UnregisterPlayerComponent()
{
    PlayerComponent = nullptr;
}

void UInworldApiSubsystem::SendTextMessage(const FName& AgentId, const FString& Text)
{
    if (!ensureMsgf(!AgentId.IsNone(), TEXT("AgentId must be valid!")))
    {
        return;
    }

    auto Packet = Client.SendTextMessage(AgentId, Text);
    auto* AgentComponentPtr = CharacterComponentByAgentId.Find(AgentId);
    if (AgentComponentPtr)
    {
        (*AgentComponentPtr)->HandlePacket(Packet);
    }
}

void UInworldApiSubsystem::SendCustomEvent(FName AgentId, const FString& Name)
{
	if (!ensureMsgf(!AgentId.IsNone(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client.SendCustomEvent(AgentId, Name);
}

void UInworldApiSubsystem::SendCustomEventForTargetAgent(const FString& Name)
{
    if (ensureMsgf(PlayerComponent, TEXT("PlayerComponent is not initialized. You must call RegisterPlayerComponent.")))
    {
        if (auto* Character = PlayerComponent->GetTargetCharacter())
        {
            SendCustomEvent(Character->GetAgentId(), Name);
        }
    }
}

void UInworldApiSubsystem::SendAudioMessage(const FName& AgentId, USoundWave* SoundWave)
{
	if (!ensureMsgf(!AgentId.IsNone(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    std::string Data;
    if (ensure(Inworld::Utils::SoundWaveToString(SoundWave, Data)))
    {
        SendAudioDataMessage(AgentId, Data);
    }
}

void UInworldApiSubsystem::SendAudioDataMessage(const FName& AgentId, const std::string& Data)
{
	if (!ensureMsgf(!AgentId.IsNone(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client.SendSoundMessage(AgentId, Data);
}

void UInworldApiSubsystem::StartAudioSession(const FName& AgentId)
{
	if (!ensureMsgf(!AgentId.IsNone(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client.StartAudioSession(AgentId);
}

void UInworldApiSubsystem::StopAudioSession(const FName& AgentId)
{
	if (!ensureMsgf(!AgentId.IsNone(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client.StopAudioSession(AgentId);
}

void UInworldApiSubsystem::NotifyVoiceStarted()
{
    Client.NotifyVoiceStarted();
}

void UInworldApiSubsystem::NotifyVoiceStopped()
{
    Client.NotifyVoiceStopped();
}

void UInworldApiSubsystem::CancelResponse(const FName& AgentId, const FName& InteractionId, const TArray<FName>& UtteranceIds)
{
	if (!ensureMsgf(!AgentId.IsNone(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client.CancelResponse(AgentId, InteractionId, UtteranceIds);
}

bool UInworldApiSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UInworldApiSubsystem::Deinitialize()
{
    Client.DestroyClient();
}
