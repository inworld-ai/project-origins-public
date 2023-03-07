/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Packets.h"
#include "InworldCharacterMessage.h"
#include "InworldCharacterEnums.h"

#include "InworldCharacterPlayback.generated.h"

class UInworldCharacterComponent;

UCLASS(BlueprintType, Blueprintable, Abstract)
class INWORLDAIINTEGRATION_API UInworldCharacterPlayback : public UObject, public Inworld::FCharacterMessageVisitor
{
	GENERATED_BODY()

public:
	/**
	 * Handle message from player(interrupt audio, update history etc)
	 * @param text event
	 */
	virtual void HandlePlayerTalking(const Inworld::FCharacterMessageUtterance& Message) {}
	
	/**
	 * Handle start/stop interacting with player
	 * @param Interaction state
	 */
	virtual void HandlePlayerInteraction(bool bInteracting) {}

	/**
	 * Update playback
	 * @return is playback ready to handle next message
	 */
	virtual bool Update() { return true; }

	/**
	 * Handle character emotion change
	 */
	virtual void HandleEmotionChange(EInworldCharacterEmotionalBehavior Emotion, EInworldCharacterEmotionStrength Strength) { }

	/**
	 * Use for initializing
	 */
	UFUNCTION(BlueprintNativeEvent)
	void BeginPlay();

	/**
	 * Use for deinitializing
	 */
	UFUNCTION(BlueprintNativeEvent)
	void EndPlay();

	void HandleMessage(const TSharedPtr<Inworld::FCharacterMessage>& Message);

	void SetOwnerActor(AActor* InOwnerActor) { OwnerActor = InOwnerActor; }
	void SetCharacterComponent(UInworldCharacterComponent* InCharacterComponent);

protected:

	UFUNCTION(BlueprintPure)
	const UInworldCharacterComponent* GetCharacterComponent() const 
	{ 
		return CharacterComponent.Get();
	};

	UFUNCTION(BlueprintPure)
	const AActor* GetOwner() const
	{
		return OwnerActor.Get();
	};

	const TSharedPtr<Inworld::FCharacterMessage> GetCurrentMessage() const;

	TWeakObjectPtr<AActor> OwnerActor;
	TWeakObjectPtr<UInworldCharacterComponent> CharacterComponent;
};