// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterComponent.h"
#include "InworldGameplayDebuggerCategory.h"

#include "InworldPlayerComponent.generated.h"

class UInworldApiSubsystem;
class UInworldCharacterComponent;

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerComponent : public UActorComponent, public Inworld::IPlayerComponent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetChange, UInworldCharacterComponent*);
    FOnInworldPlayerTargetChange OnTargetSet;
    FOnInworldPlayerTargetChange OnTargetClear;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "GetTargetCharacter"))
    UInworldCharacterComponent* GetTargetInworldCharacter() { return static_cast<UInworldCharacterComponent*>(GetTargetCharacter()); }

    virtual Inworld::ICharacterComponent* GetTargetCharacter() override;

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "SetTargetCharacter"))
    void SetTargetInworldCharacter(UInworldCharacterComponent* Character);

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "ClearTargetCharacter"))
    void ClearTargetInworldCharacter();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsInteracting() { return !TargetCharacterAgentId.IsEmpty(); }

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Interaction")
    void SendTextMessageToTarget(const FString& Message);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Interaction")
    void SendTextMessage(const FString& Message, const FString& AgentId);

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (AutoCreateRefTerm = "Params"))
    void SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params);
    [[deprecated("UInworldPlayerComponent::SendCustomEventToTarget is deprecated, please use UInworldPlayerComponent::SendTriggerToTarget")]]
    void SendCustomEventToTarget(const FString& Name) { SendTriggerToTarget(Name, {}); }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartAudioSessionWithTarget();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StopAudioSessionWithTarget();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SendAudioMessageToTarget(USoundWave* SoundWave);
    void SendAudioDataMessageToTarget(const TArray<uint8>& Data);
    void SendAudioDataMessageWithAECToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

private:
	UFUNCTION()
	void OnRep_TargetCharacterAgentId(FString OldAgentId);

    FDelegateHandle CharacterTargetUnpossessedHandle;

    UPROPERTY(EditAnywhere, Category = "UI")
    FString UiName = "Player";

    TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;

	UPROPERTY(ReplicatedUsing = OnRep_TargetCharacterAgentId)
	FString TargetCharacterAgentId;

	friend class FInworldGameplayDebuggerCategory;
};
