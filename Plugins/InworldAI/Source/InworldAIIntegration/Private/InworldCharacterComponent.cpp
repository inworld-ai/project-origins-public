/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterComponent.h"
#include "proto/ProtoDisableWarning.h"
#include "InworldApi.h"
#include "Packets.h"
#include "InworldUtils.h"
#include "Engine/EngineBaseTypes.h"

UInworldCharacterComponent::UInworldCharacterComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInworldCharacterComponent::BeginPlay()
{
    Super::BeginPlay();

    Register();

    for (auto& Type : PlaybackTypes)
    {
        auto* Pb = NewObject<UInworldCharacterPlayback>(this, Type);
        Pb->SetOwnerActor(GetOwner());
        Pb->SetCharacterComponent(this);
        Playbacks.Add(Pb);
    }

    for (auto* Pb : Playbacks)
    {
        Pb->BeginPlay();
    }
}

void UInworldCharacterComponent::EndPlay(EEndPlayReason::Type Reason)
{
    for (auto* Pb : Playbacks)
    {
        Pb->EndPlay();
    }

    Unregister();

    Super::EndPlay(Reason);
}

void UInworldCharacterComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    bool bUnqueueNextMessage = true;
    for (auto* Pb : Playbacks)
    {
        bUnqueueNextMessage &= Pb->Update();
    }

    if (!bUnqueueNextMessage)
    {
        return;
    }

	CurrentMessage = nullptr;

	if (PendingMessages.Num() == 0 || !PendingMessages[0]->IsValid())
	{
        return;
	}

	CurrentMessage = PendingMessages[0];
	PendingMessages.RemoveAt(0);

    if (CancelledInteractions.Find(CurrentMessage.Get()->InteractionId) != INDEX_NONE)
    {
        if (CurrentMessage->IsSkippable())
        {
            CurrentMessage = nullptr;
            return;
        }
    }

	for (auto* Pb : Playbacks)
	{
		Pb->HandleMessage(CurrentMessage);
	}
}

UInworldCharacterPlayback* UInworldCharacterComponent::GetPlayback(TSubclassOf<UInworldCharacterPlayback> Class) const
{
    for (auto* Pb : Playbacks)
    {
        if (Pb->GetClass()->IsChildOf(Class.Get()))
        {
            return Pb;
        }
    }
    return nullptr;
}

void UInworldCharacterComponent::SendTextMessage(const FString& Text)
{
    if (ensure(!AgentId.IsNone()))
    {
        GetWorld()->GetSubsystem<UInworldApiSubsystem>()->SendTextMessage(AgentId, Text);
    }
}

void UInworldCharacterComponent::HandlePacket(TSharedPtr<Inworld::FInworldPacket> Packet)
{
    if (ensure(Packet))
	{
		Packet->Accept(*this);
    }
}

void UInworldCharacterComponent::HandlePlayerTalking(const Inworld::FTextEvent& Event)
{
    if (CurrentMessage.IsValid() && CurrentMessage->InteractionId != Event.PacketId.InteractionId)
    {
        CancelCurrentInteraction();
    }

    Inworld::FCharacterMessageUtterance Message;
    Message.InteractionId = Event.PacketId.InteractionId;
    Message.UtteranceId = Event.PacketId.UtteranceId;
    Message.Text = Event.GetText();
    Message.bTextFinal = Event.IsFinal();

    for (auto* Pb : Playbacks)
    {
        Pb->HandlePlayerTalking(Message);
    }

    OnPlayerVoiceUpdated.Broadcast(Event.GetText());
}

void UInworldCharacterComponent::HandlePlayerInteraction(bool bInteracting)
{
	bInteractingWithPlayer = bInteracting;
	OnPlayerInteractionStateChanged.Broadcast(bInteractingWithPlayer);
}

void UInworldCharacterComponent::CancelCurrentInteraction()
{
    if (!CurrentMessage.IsValid())
    {
        return;
    }

	TArray<FName> Utterances;
	Utterances.Reserve(PendingMessages.Num() + 1);
    Utterances.Add(CurrentMessage->UtteranceId);
	for (auto& Message : PendingMessages)
	{
		if (CurrentMessage->InteractionId == Message->InteractionId)
		{
			Utterances.Add(Message->UtteranceId);
		}

		if (!Message->IsSkippable())
		{
			for (auto* Pb : Playbacks)
			{
				Pb->HandleMessage(Message);
			}
		}
	}

	if (Utterances.Num() > 0)
	{
		GetWorld()->GetSubsystem<UInworldApiSubsystem>()->CancelResponse(AgentId, CurrentMessage->InteractionId, Utterances);
	}

	PendingMessages.Empty();
}

void UInworldCharacterComponent::SendCustomEvent(const FString& Name) const
{
    GetWorld()->GetSubsystem<UInworldApiSubsystem>()->SendCustomEvent(AgentId, Name);
}

bool UInworldCharacterComponent::Register()
{
    if (bRegistered)
    {
        return false;
    }

    if (BrainName.IsEmpty())
    {
        return false;
    }

	auto* InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (!ensure(InworldSubsystem))
	{
        return false;
	}

    InworldSubsystem->RegisterCharacterComponent(this);

    bRegistered = true;

    return true;
}

bool UInworldCharacterComponent::Unregister()
{
	if (!bRegistered)
	{
		return false;
	}

	auto* InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (!ensure(InworldSubsystem))
	{
		return false;
	}

    InworldSubsystem->UnregisterCharacterComponent(this);

    bRegistered = false;

    return true;
}

bool UInworldCharacterComponent::IsCustomGesture(const FString& CustomEventName) const
{
    return CustomEventName.Find("gesture") == 0;
}

void UInworldCharacterComponent::Visit(const Inworld::FTextEvent& Event)
{
    const Inworld::FActor& FromActor = Event.Routing.Source;
    const Inworld::FActor& ToActor = Event.Routing.Target;

    if (ToActor.Type == ai::inworld::packets::Actor_Type_AGENT)
    {
        if (Event.IsFinal())
        {
            Inworld::Utils::Log(FString::Printf(TEXT("%s to %s: %s"), *FromActor.Name.ToString(), *ToActor.Name.ToString(), *Event.GetText()), true, Inworld::Utils::FTextColors::Player());
        }

        if (bHandlePlayerTalking)
        {
            HandlePlayerTalking(Event);
        }
    }

    if (FromActor.Type == ai::inworld::packets::Actor_Type_AGENT)
    {
        if (Event.IsFinal())
        {
            Inworld::Utils::Log(FString::Printf(TEXT("%s to %s: %s"), *FromActor.Name.ToString(), *ToActor.Name.ToString(), *Event.GetText()), true, Inworld::Utils::FTextColors::Character());
        }

        if (auto Message = FindOrAddMessage<Inworld::FCharacterMessageUtterance>(Event.PacketId.InteractionId, Event.PacketId.UtteranceId))
        {
            Message->Text = Event.GetText();
            Message->bTextFinal = Event.IsFinal();
        }
    }
}

void UInworldCharacterComponent::Visit(const Inworld::FDataEvent& Event)
{
	if (auto Message = FindOrAddMessage<Inworld::FCharacterMessageUtterance>(Event.PacketId.InteractionId, Event.PacketId.UtteranceId))
	{
        Message->AudioData = Event.GetDataChunk();
	}
}

void UInworldCharacterComponent::Visit(const Inworld::FSilenceEvent& Event)
{
	if (auto Message = FindOrAddMessage<Inworld::FCharacterMessageSilence>(Event.PacketId.InteractionId, Event.PacketId.UtteranceId))
	{
        Message->Duration = Event.GetDuration();
	}
}

void UInworldCharacterComponent::Visit(const Inworld::FControlEvent& Event)
{
    if (Event.GetControlAction() == ai::inworld::packets::ControlEvent_Action_INTERACTION_END)
    {
		FindOrAddMessage<Inworld::FCharacterMessageInteractionEnd>(Event.PacketId.InteractionId, Event.PacketId.UtteranceId);
    }
}

void UInworldCharacterComponent::Visit(const Inworld::FEmotionEvent& Event)
{
    EInworldCharacterEmotionalState State = EInworldCharacterEmotionalState::Idle;

    const auto& EmotionState = Event.GetEmotionalState();

    constexpr float Tolerance = 0.1f;
    if (FMath::IsNearlyEqual(EmotionState.Joy, 1.f, Tolerance))
    {
        State = EInworldCharacterEmotionalState::Joy;
    }
    else if (FMath::IsNearlyEqual(EmotionState.Joy, -1.f, Tolerance))
    {
        State = EInworldCharacterEmotionalState::Sadness;
    }
    else if (FMath::IsNearlyEqual(EmotionState.Fear, 1.f, Tolerance))
    {
        State = EInworldCharacterEmotionalState::Fear;
    }
    else if (FMath::IsNearlyEqual(EmotionState.Fear, -1.f, Tolerance))
    {
        State = EInworldCharacterEmotionalState::Anger;
    }
    else if (FMath::IsNearlyEqual(EmotionState.Trust, 1.f, Tolerance))
    {
        State = EInworldCharacterEmotionalState::Trust;
    }
    else if (FMath::IsNearlyEqual(EmotionState.Trust, -1.f, Tolerance))
    {
        State = EInworldCharacterEmotionalState::Disgust;
    }
    else if (FMath::IsNearlyEqual(EmotionState.Surprise, 1.f, Tolerance))
    {
        State = EInworldCharacterEmotionalState::Surprise;
    }
    else if (FMath::IsNearlyEqual(EmotionState.Surprise, -1.f, Tolerance))
    {
        State = EInworldCharacterEmotionalState::Anticipation;
    }

    EmotionStrength = static_cast<EInworldCharacterEmotionStrength>(Event.GetStrength());

    if (EmotionalState != State)
    {
        EmotionalState = State;
        OnEmotionalStateChanged.Broadcast(EmotionalState, EmotionStrength);
    }

    const auto Behavior = static_cast<EInworldCharacterEmotionalBehavior>(Event.GetEmotionalBehavior());
    if (Behavior != EmotionalBehavior)
    {
        EmotionalBehavior = Behavior;
        OnEmotionalBehaviorChanged.Broadcast(EmotionalBehavior, EmotionStrength);
    }
}

void UInworldCharacterComponent::Visit(const Inworld::FCustomEvent& Event)
{
    if (IsCustomGesture(Event.GetName()))
    {
        auto Message = FindMessage<Inworld::FCharacterMessageUtterance>(Event.PacketId.InteractionId, Event.PacketId.UtteranceId);
        if (Message.IsValid())
        {
            Message->CustomGesture = Event.GetName();
        }
        else
        {
            Message = MakeShared<Inworld::FCharacterMessageUtterance>();
            Message->InteractionId = Event.PacketId.InteractionId;
            Message->UtteranceId = Event.PacketId.UtteranceId;
            Message->CustomGesture = Event.GetName();
            PendingMessages.EmplaceAt(0, Message);
        }
    }
    else
    {
		if (auto Message = FindOrAddMessage<Inworld::FCharacterMessageTrigger>(Event.PacketId.InteractionId, Event.PacketId.UtteranceId))
		{
            TriggerInteraction = Event.PacketId.InteractionId;
            TriggerUtterance = Event.PacketId.UtteranceId;
			Message->Name = Event.GetName();
		}

        Inworld::Utils::Log(FString::Printf(TEXT("CustomEvent arrived: %s - %s"), *Event.GetName(), *Event.PacketId.InteractionId.ToString()), true);
    }
}