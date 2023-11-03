// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#include "InworldCharacterPlaybackTrigger.h"
#include "InworldApi.h"

#include "InworldCharacterPlaybackTrigger.h"
#include "InworldApi.h"

void UInworldCharacterPlaybackTrigger::OnCharacterTrigger_Implementation(const FCharacterMessageTrigger& Message)
{
	Triggers.Add(Message.Name);
}

void UInworldCharacterPlaybackTrigger::OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message)
{
	FlushTriggers();
}

void UInworldCharacterPlaybackTrigger::FlushTriggers()
{
	TArray<FString> TriggersToFlush = Triggers;
	Triggers = {};
	for (const FString& Trigger : TriggersToFlush)
	{
		OwnerActor->GetWorld()->GetSubsystem<UInworldApiSubsystem>()->NotifyCustomTrigger(Trigger);
	}
}
