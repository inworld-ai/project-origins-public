/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterMessage.h"
#include "InworldCharacterPlayback.h"

#include "InworldCharacterPlaybackHistory.generated.h"

USTRUCT(BlueprintType)
struct FInworldCharacterInteraction
{
	GENERATED_BODY();

	FInworldCharacterInteraction() = default;
	FInworldCharacterInteraction(const Inworld::FCharacterMessageUtterance& InMessage, bool InPlayerInteraction);

	UPROPERTY(BlueprintReadOnly, Category = "Text")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bPlayerInteraction = false;

	Inworld::FCharacterMessageUtterance Message;
};

USTRUCT(BlueprintType)
struct FInworldCharacterInteractionHistory
{
	GENERATED_BODY();

	void Add(const Inworld::FCharacterMessageUtterance& Message, bool bPlayerInteraction);
	void Clear();

	void SetMaxEntries(uint32 Val);

	TArray<FInworldCharacterInteraction>& GetInteractions() { return Interactions; }

	void CancelUtterance(const FName& InteractionId, const FName& UtteranceId);
	bool IsInteractionCanceled(const FName& InteractionId) const;
	void ClearCanceledInteraction(const FName& InteractionId);

private:
	TArray<FInworldCharacterInteraction> Interactions;
	TArray<FName> CanceledInteractions;

	int32 MaxEntries = 50;
};

UCLASS(BlueprintType, Blueprintable)
class INWORLDAIINTEGRATION_API UInworldCharacterPlaybackHistory : public UInworldCharacterPlayback
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterInteractionsChanged, const TArray<FInworldCharacterInteraction>&, Interactions);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterInteractionsChanged OnInteractionsChanged;

	UFUNCTION(BlueprintPure, Category = "Interactions")
	const TArray<FInworldCharacterInteraction>& GetInteractions() { return InteractionHistory.GetInteractions(); }

	virtual void HandlePlayerTalking(const Inworld::FCharacterMessageUtterance& Message) override;

	virtual void BeginPlay_Implementation() override;

	virtual void Visit(const Inworld::FCharacterMessageUtterance& Event) override;
	virtual void Visit(const Inworld::FCharacterMessageInteractionEnd& Event) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (UIMin = "1", UIMax = "500"))
	int32 InteractionHistoryMaxEntries = 50;

private:

	FInworldCharacterInteractionHistory InteractionHistory;
};

