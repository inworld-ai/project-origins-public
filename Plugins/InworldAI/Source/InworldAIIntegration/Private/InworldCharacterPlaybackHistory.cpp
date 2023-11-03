// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#include "InworldCharacterPlaybackHistory.h"
#include "InworldCharacterMessage.h"

void FInworldCharacterInteractionHistory::Add(const FString& InteractionId, const FString& UtteranceId, const FString& Text, bool bPlayerInteraction
	// ORIGINS MODIFY
	, bool bInTextFinal
	// END ORIGINS MODIFY
)
{
	if (IsInteractionCanceled(InteractionId))
	{
		return;
	}

	auto* Interaction = Interactions.FindByPredicate([UtteranceId](const auto& I) { return I.UtteranceId == UtteranceId; });
	if (Interaction)
	{
		*Interaction = FInworldCharacterInteraction(InteractionId, UtteranceId, Text, bPlayerInteraction
		// ORIGINS MODIFY
		, bInTextFinal
		// END ORIGINS MODIFY
		);
		return;
	}

	Interactions.Emplace(InteractionId, UtteranceId, Text, bPlayerInteraction
		// ORIGINS MODIFY
		, bInTextFinal
		// END ORIGINS MODIFY
	);

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

void FInworldCharacterInteractionHistory::CancelUtterance(const FString& InteractionId, const FString& UtteranceId)
{
	CanceledInteractions.Add(InteractionId);
	Interactions.RemoveAll([&UtteranceId, &InteractionId](const auto& I) { return I.UtteranceId == UtteranceId && I.InteractionId == InteractionId; });
}

bool FInworldCharacterInteractionHistory::IsInteractionCanceled(const FString& InteractionId) const
{
	int32 Idx;
	return CanceledInteractions.Find(InteractionId, Idx);
}

void FInworldCharacterInteractionHistory::ClearCanceledInteraction(const FString& InteractionId)
{
	CanceledInteractions.Remove(InteractionId);
}

void UInworldCharacterPlaybackHistory::BeginPlay_Implementation()
{
	Super::BeginPlay_Implementation();

	InteractionHistory.SetMaxEntries(InteractionHistoryMaxEntries);

}

void UInworldCharacterPlaybackHistory::OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message)
{
	InteractionHistory.Add(Message);
	OnInteractionsChanged.Broadcast(InteractionHistory.GetInteractions());
}

void UInworldCharacterPlaybackHistory::OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message)
{
	InteractionHistory.CancelUtterance(Message.InteractionId, Message.UtteranceId);
	OnInteractionsChanged.Broadcast(InteractionHistory.GetInteractions());
}

void UInworldCharacterPlaybackHistory::OnCharacterPlayerTalk_Implementation(const FCharacterMessagePlayerTalk& Message)
{
	InteractionHistory.Add(Message);
	OnInteractionsChanged.Broadcast(InteractionHistory.GetInteractions());
}

void UInworldCharacterPlaybackHistory::OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message)
{
	InteractionHistory.ClearCanceledInteraction(Message.InteractionId);
}
