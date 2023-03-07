/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Packets.h"
#include "Components/ActorComponent.h"
#include "InworldComponentInterface.h"
#include "InworldCharacterPlayback.h"
#include "InworldCharacterMessage.h"
#include "InworldCharacterEnums.h"

#include "InworldCharacterComponent.generated.h"

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldCharacterComponent : public UActorComponent, public Inworld::FPacketVisitor, public Inworld::ICharacterComponent
{
	GENERATED_BODY()

public:
	UInworldCharacterComponent();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldCharacterEmotionalStateChanged, EInworldCharacterEmotionalState, EmotionalState, EInworldCharacterEmotionStrength, Strength);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterEmotionalStateChanged OnEmotionalStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldCharacterEmotionalBehaviorChanged, EInworldCharacterEmotionalBehavior, EmotionalBehavior, EInworldCharacterEmotionStrength, Strength);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterEmotionalBehaviorChanged OnEmotionalBehaviorChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInworldCharacterPlayerInteractionStateChanged, bool, bInteracting); 
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FInworldCharacterPlayerInteractionStateChanged OnPlayerInteractionStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerVoiceUpdated, const FString&, Text);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldPlayerVoiceUpdated OnPlayerVoiceUpdated;

    virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type Reason);
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
    
    UFUNCTION(BlueprintCallable, Category = "Inworld")
	virtual const FName& GetAgentId() const override { return AgentId; }
    virtual void SetAgentId(const FName& InAgentId) override { AgentId = InAgentId; }

    UFUNCTION(BlueprintCallable, Category = "Inworld")
    virtual const FString& GetGivenName() const override { return GivenName; }
    virtual void SetGivenName(const FString& InGivenName) override { GivenName = InGivenName; }

    virtual AActor* GetComponentOwner() const override { return GetOwner(); }

    UFUNCTION(BlueprintCallable, Category = "Inworld")
    const FString& GetUiName() const { return UiName; }
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void SetUiName(const FString& Name) { UiName = Name; }

	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (DeterminesOutputType = "Class"))
	UInworldCharacterPlayback* GetPlayback(TSubclassOf<UInworldCharacterPlayback> Class) const;

    void SendTextMessage(const FString& Text);

    virtual const FString& GetBrainName() const override { return BrainName; }

    virtual void HandlePacket(TSharedPtr<Inworld::FInworldPacket> Packet) override;

    virtual void HandleConnectionStateChanged(EInworldConnectionState State) override {}

	virtual void HandlePlayerTalking(const Inworld::FTextEvent& Event);
	virtual void HandlePlayerInteraction(bool bInteracting);

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	bool IsInteractingWithPlayer() const { return bInteractingWithPlayer; }

	UFUNCTION(BlueprintCallable, Category = "Emotions")
	EInworldCharacterEmotionalBehavior GetEmotionalBehavior() const { return EmotionalBehavior; }

	UFUNCTION(BlueprintPure, Category = "Emotions")
	EInworldCharacterEmotionStrength GetEmotionStrength() const { return EmotionStrength; }

	UFUNCTION(BlueprintCallable, Category = "Emotions")
	EInworldCharacterEmotionalState GetEmotionalState() const { return EmotionalState; }

	UFUNCTION(BlueprintCallable, Category = "Events")
	void SendCustomEvent(const FString& Name) const;

	UFUNCTION(BlueprintCallable, Category = "Brain")
	void SetBrainName(const FString& Name) { BrainName = Name; }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelCurrentInteraction();

	UFUNCTION(BlueprintCallable, Category = "Events")
	bool Register();

	UFUNCTION(BlueprintCallable, Category = "Events")
	bool Unregister();

	const TSharedPtr<Inworld::FCharacterMessage> GetCurrentMessage() const 
	{ 
		return CurrentMessage; 
	}

	template<class T>
	T* GetPlaybackNative()
	{
		for (auto* Pb : Playbacks)
		{
			if (auto* Playback = Cast<T>(Pb))
			{
				return Playback;
			}
		}
		return nullptr;
	}

	UPROPERTY(EditAnywhere, Category = "Inworld")
	TArray<TSubclassOf<UInworldCharacterPlayback>> PlaybackTypes;

	UPROPERTY(EditAnywhere, Category = "Inworld")
	bool bIsMainCharacter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inworld")
	bool bHandlePlayerTalking = true;

protected:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FString UiName = "Character";

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
	class UDataTable* AnimationDT;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
	class UDataTable* SemanticGestureDT;

private:

    virtual void Visit(const Inworld::FTextEvent& Event) override;
    virtual void Visit(const Inworld::FDataEvent& Event) override;
    virtual void Visit(const Inworld::FSilenceEvent& Event) override;
    virtual void Visit(const Inworld::FControlEvent& Event) override;
    virtual void Visit(const Inworld::FEmotionEvent& Event) override;
    virtual void Visit(const Inworld::FCustomEvent& Event) override;

	template<class T>
	TSharedPtr<T> FindOrAddMessage(const FName& InteractionId, const FName& UtteranceId)
	{
		TSharedPtr<T> Message = FindMessage<T>(InteractionId, UtteranceId);
		if (Message.IsValid())
		{
			return Message;
		}

		Message = MakeShared<T>();
		Message->InteractionId = InteractionId;
		Message->UtteranceId = UtteranceId;
		PendingMessages.Add(Message);
		return Message;
	}

	template<class T>
	TSharedPtr<T> FindMessage(const FName& InteractionId, const FName& UtteranceId)
	{
		if (auto* Message = PendingMessages.FindByPredicate([&InteractionId, &UtteranceId](const auto& U) { return U->InteractionId == InteractionId && U->UtteranceId == UtteranceId; }))
		{
			return StaticCastSharedPtr<T>(*Message);
		}

		return nullptr;
	}

	bool IsCustomGesture(const FString& CustomEventName) const;

    UPROPERTY(EditAnywhere, Category = "Inworld")
	FString BrainName;

	UPROPERTY()
	TArray<UInworldCharacterPlayback*> Playbacks;

	TArray<TSharedPtr<Inworld::FCharacterMessage>> PendingMessages;
	TArray<FName> CancelledInteractions;

	TSharedPtr<Inworld::FCharacterMessage> CurrentMessage;

	FName TriggerUtterance;
	FName TriggerInteraction;

    EInworldCharacterEmotionalBehavior EmotionalBehavior = EInworldCharacterEmotionalBehavior::NEUTRAL;
    EInworldCharacterEmotionStrength EmotionStrength = EInworldCharacterEmotionStrength::UNSPECIFIED;
	EInworldCharacterEmotionalState EmotionalState = EInworldCharacterEmotionalState::Idle;

    FName AgentId = NAME_None;
	
	FString GivenName;

	bool bInteractingWithPlayer = false;
    bool bRegistered = false;
};
