// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterComponent.h"
#include "OriginsInworldCharacterComponent.generated.h"


UCLASS(ClassGroup = (Origins), meta = (BlueprintSpawnableComponent))
class INWORLDRT_API UOriginsInworldCharacterComponent : public UInworldCharacterComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Inworld")
	bool bIsMainCharacter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inworld")
	bool bHandlePlayerTalking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inworld")
	bool bCanInteractNow = true;
	
};
