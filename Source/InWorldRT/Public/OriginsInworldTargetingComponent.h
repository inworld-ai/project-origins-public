// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InworldPlayerTargetingComponent.h"
#include "OriginsInworldTargetingComponent.generated.h"

UCLASS(ClassGroup = (Origins), meta = (BlueprintSpawnableComponent))
class INWORLDRT_API UOriginsInworldTargetingComponent : public UInworldPlayerTargetingComponent
{
	GENERATED_BODY()
	
public:

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float AltInteractionDistance = 250.f;

    UPROPERTY(EditAnywhere, Category = "Interaction")
    float BeginInteractionDotThreshold = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Interaction")
    float MaintainInteractionDotThreshold = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Interaction")
    float AltBeginInteractionDotThreshold = 0.75f;

    UPROPERTY(EditAnywhere, Category = "Interaction")
    float AltMaintainInteractionDotThreshold = 0.75f;

protected:
    virtual void UpdateTargetCharacter() override;

private:
    TWeakObjectPtr<UInworldCharacterComponent> FocusTargetCharacter;
    TWeakObjectPtr<UInworldCharacterComponent> PermanentTargetCharacter;
    TArray<TWeakObjectPtr<UInworldCharacterComponent>> PermanentTargetCharacterPriorityList;
};
