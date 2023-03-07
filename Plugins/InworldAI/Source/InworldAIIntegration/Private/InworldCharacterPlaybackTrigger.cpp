/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterPlaybackTrigger.h"
#include "InworldApi.h"

void UInworldCharacterPlaybackTrigger::Visit(const Inworld::FCharacterMessageTrigger& Event)
{
	if (FinalizedInteractions.Contains(Event.InteractionId))
	{
		OwnerActor->GetWorld()->GetSubsystem<UInworldApiSubsystem>()->NotifyCustomTrigger(Event.Name);
	}
	else
	{
		PendingTriggers.Add(Event.InteractionId, Event);
	}
}

void UInworldCharacterPlaybackTrigger::Visit(const Inworld::FCharacterMessageInteractionEnd& Event)
{
	if (PendingTriggers.Contains(Event.InteractionId))
	{
		OwnerActor->GetWorld()->GetSubsystem<UInworldApiSubsystem>()->NotifyCustomTrigger(PendingTriggers[Event.InteractionId].Name);
		PendingTriggers.Remove(Event.InteractionId);
	}
	FinalizedInteractions.Add(Event.InteractionId);
}
