/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldUtils.h"
#include "InworldPlayerTargetingComponent.generated.h"

class UInworldPlayerComponent;
class UInworldCharacterComponent;
class UInworldApiSubsystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UInworldPlayerTargetingComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetPermanentTargetCharacter(UInworldCharacterComponent* Character);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void UnsetPermanentTargetCharacter(UInworldCharacterComponent* Character);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    UInworldCharacterComponent* GetPermanentTargetCharacter() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    UInworldCharacterComponent* GetFocusTargetCharacter() const;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetCharacterChanged, class UInworldCharacterComponent*, TargetCharacter);

    UPROPERTY(BlueprintAssignable, Category = "Target")
    FOnTargetCharacterChanged OnFocusTargetCharacterChanged;

    UPROPERTY(BlueprintAssignable, Category = "Target")
    FOnTargetCharacterChanged OnPermanentTargetCharacterChanged;

private:
    void UpdateTargetCharacter();
    void SetTargetCharacter(TWeakObjectPtr<UInworldCharacterComponent> ClosestCharacter);
    void ClearTargetCharacter();

public:
	/** Minimum distance to start interacting with a character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float AltInteractionDistance = 250.f;

private:

    /** How close must player be facing the direction of the character to interact */
    UPROPERTY(EditAnywhere, Category = "Interaction")
	float BeginInteractionDotThreshold = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Interaction")
    float MaintainInteractionDotThreshold = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Interaction")
    float AltBeginInteractionDotThreshold = 0.75f;

    UPROPERTY(EditAnywhere, Category = "Interaction")
    float AltMaintainInteractionDotThreshold = 0.75f;

	TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;
	TWeakObjectPtr<UInworldPlayerComponent> PlayerComponent;

    TWeakObjectPtr<UInworldCharacterComponent> FocusTargetCharacter;
    TWeakObjectPtr<UInworldCharacterComponent> PermanentTargetCharacter;
    TArray<TWeakObjectPtr<UInworldCharacterComponent>> PermanentTargetCharacterPriorityList;

    Inworld::Utils::FWorldTimer ChangeTargetCharacterTimer = Inworld::Utils::FWorldTimer(0.5f);

};
