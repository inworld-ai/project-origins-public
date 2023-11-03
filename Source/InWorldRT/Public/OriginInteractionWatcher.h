/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InworldCharacterComponent.h"
#include "InworldCharacterPlaybackHistory.h"
#include "OriginInteractionWatcher.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnOriginInteraction, class UInworldCharacterComponent*, InworldCharacter, bool, bPlayer, FString, InteractionId, FString, Text);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INWORLDRT_API UOriginInteractionWatcher : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Watch")
	void BeginWatch(UInworldCharacterComponent* CharacterToStartWatching);

	UFUNCTION(BlueprintCallable, Category = "Watch")
	void EndWatch();

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnOriginInteraction OnOriginInteraction;

private:
	UFUNCTION()
	void OnWatchedCharacterInteractionsChanged(const TArray<FInworldCharacterInteraction>& Interactions);

	UPROPERTY()
	TWeakObjectPtr<UInworldCharacterComponent> WatchedCharacter;

	int32 NumProcessedInteractions = 0;
};
