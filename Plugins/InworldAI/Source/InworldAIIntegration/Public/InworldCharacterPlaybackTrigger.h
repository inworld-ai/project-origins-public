/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterPlayback.h"

#include "InworldCharacterPlaybackTrigger.generated.h"

UCLASS(BlueprintType, Blueprintable)
class INWORLDAIINTEGRATION_API UInworldCharacterPlaybackTrigger : public UInworldCharacterPlayback
{
	GENERATED_BODY()

public:
	virtual void Visit(const Inworld::FCharacterMessageTrigger& Event) override;
	virtual void Visit(const Inworld::FCharacterMessageInteractionEnd& Event) override;

private:
	TMap<FName, Inworld::FCharacterMessageTrigger> PendingTriggers;
	TSet<FName> FinalizedInteractions;
};

