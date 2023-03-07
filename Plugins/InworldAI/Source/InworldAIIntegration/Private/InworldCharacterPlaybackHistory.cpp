/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterPlaybackHistory.h"
#include "InworldCharacterMessage.h"
#include "InworldCharacterPlaybackAudio.h"

void FInworldCharacterInteractionHistory::Add(const Inworld::FCharacterMessageUtterance& Message, bool bPlayerInteraction)
{
	if (IsInteractionCanceled(Message.InteractionId))
	{
		return;
	}

	auto* Interaction = Interactions.FindByPredicate([&Message](const auto& I) { return I.Message.UtteranceId == Message.UtteranceId; });
	if (Interaction)
	{
		*Interaction = FInworldCharacterInteraction(Message, bPlayerInteraction);
		return;
	}

	Interactions.Emplace(Message, bPlayerInteraction);

	if (Interactions.Num() > MaxEntries)
	{
		Interactions.RemoveAt(0, 1, false);
	}
}

void FInworldCharacterInteractionHistory::Clear()
{
	Interactions.Empty();
}

void FInworldCharacterInteractionHistory::SetMaxEntries(uint32 Val)
{
	MaxEntries = Val;
	Interactions.Reserve(MaxEntries);
}

void FInworldCharacterInteractionHistory::CancelUtterance(const FName& InteractionId, const FName& UtteranceId)
{
	CanceledInteractions.Add(InteractionId);
	Interactions.RemoveAll([&UtteranceId, &InteractionId](const auto& I) { return I.Message.UtteranceId == UtteranceId && I.Message.InteractionId == InteractionId; });
}

bool FInworldCharacterInteractionHistory::IsInteractionCanceled(const FName& InteractionId) const
{
	int32 Idx;
	return CanceledInteractions.Find(InteractionId, Idx);
}

void FInworldCharacterInteractionHistory::ClearCanceledInteraction(const FName& InteractionId)
{
	CanceledInteractions.Remove(InteractionId);
}

FInworldCharacterInteraction::FInworldCharacterInteraction(const Inworld::FCharacterMessageUtterance& InMessage, bool InPlayerInteraction)
	: Text(InMessage.Text)
	, bPlayerInteraction(InPlayerInteraction)
	, Message(InMessage)
{

}

void UInworldCharacterPlaybackHistory::HandlePlayerTalking(const Inworld::FCharacterMessageUtterance& Message)
{
	auto CurrentMessage = GetCurrentMessage();
	if (CurrentMessage.IsValid() && CurrentMessage->InteractionId != Message.InteractionId)
	{
		InteractionHistory.CancelUtterance(CurrentMessage->InteractionId, CurrentMessage->UtteranceId);
	}

	InteractionHistory.Add(Message, true);
	OnInteractionsChanged.Broadcast(InteractionHistory.GetInteractions());
}

void UInworldCharacterPlaybackHistory::BeginPlay_Implementation()
{
	Super::BeginPlay_Implementation();

	InteractionHistory.SetMaxEntries(InteractionHistoryMaxEntries);
}

void UInworldCharacterPlaybackHistory::Visit(const Inworld::FCharacterMessageUtterance& Message)
{
	InteractionHistory.Add(Message, false);
	OnInteractionsChanged.Broadcast(InteractionHistory.GetInteractions());
}

void UInworldCharacterPlaybackHistory::Visit(const Inworld::FCharacterMessageInteractionEnd& Message)
{
	InteractionHistory.ClearCanceledInteraction(Message.InteractionId);
}
