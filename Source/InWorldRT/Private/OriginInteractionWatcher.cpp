/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "OriginInteractionWatcher.h"

void UOriginInteractionWatcher::BeginWatch(UInworldCharacterComponent* CharacterToStartWatching)
{
	if (CharacterToStartWatching == nullptr || WatchedCharacter.IsValid())
	{
		return;
	}

	WatchedCharacter = CharacterToStartWatching;

	UInworldCharacterPlaybackHistory* History = Cast<UInworldCharacterPlaybackHistory>(WatchedCharacter->GetPlayback(UInworldCharacterPlaybackHistory::StaticClass()));
	History->OnInteractionsChanged.AddDynamic(this, &UOriginInteractionWatcher::OnWatchedCharacterInteractionsChanged);
}

void UOriginInteractionWatcher::EndWatch()
{
	if (!WatchedCharacter.IsValid())
	{
		return;
	}

	NumProcessedInteractions = 0;

	UInworldCharacterPlaybackHistory* History = Cast<UInworldCharacterPlaybackHistory>(WatchedCharacter->GetPlayback(UInworldCharacterPlaybackHistory::StaticClass()));
	History->OnInteractionsChanged.RemoveDynamic(this, &UOriginInteractionWatcher::OnWatchedCharacterInteractionsChanged);

	WatchedCharacter = nullptr;
}

void UOriginInteractionWatcher::OnWatchedCharacterInteractionsChanged(const TArray<FInworldCharacterInteraction>& Interactions)
{
	for (int32 i = NumProcessedInteractions; i < Interactions.Num(); ++i)
	{
		const FInworldCharacterInteraction& Interaction = Interactions[i];
		if (Interaction.Message.bTextFinal)
		{
			OnOriginInteraction.Broadcast(WatchedCharacter.Get(), Interaction.bPlayerInteraction, Interaction.Message.InteractionId, Interaction.Message.Text);
			NumProcessedInteractions = i + 1;
		}
		else
		{
			break;
		}
	}
}
