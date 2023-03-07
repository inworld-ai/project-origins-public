/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldState.h"
#include "Packets.h"
#include "InworldCharacterComponent.h"
#include "InworldUtils.h"

#include "InworldPlayerComponent.generated.h"

class UInworldApiSubsystem;
class UInworldCharacterComponent;

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerComponent : public UActorComponent, public Inworld::IPlayerComponent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetChange, UInworldCharacterComponent*);
    FOnInworldPlayerTargetChange OnTargetChange;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void HandleConnectionStateChanged(EInworldConnectionState State) override;

    virtual Inworld::ICharacterComponent* GetTargetCharacter() override { return TargetCharacter.Get(); }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetTargetCharacter(UInworldCharacterComponent* Character);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ClearTargetCharacter();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsInteracting() { return GetTargetCharacter() != nullptr; }

    void StartAudioSession();
    void StopAudioSession();
    void SendAudioDataMessage(const std::string& Data);
    void SendAudioMessage(USoundWave* SoundWave);

private:
    UPROPERTY(EditAnywhere, Category = "UI")
    FString UiName = "Player";

    TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;

    TWeakObjectPtr<UInworldCharacterComponent> TargetCharacter;
};
