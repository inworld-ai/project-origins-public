// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InworldCharacterProxy.generated.h"

UCLASS()
class INWORLDRT_API AInworldCharacterProxy : public AActor
{
	GENERATED_BODY()
	
public:
	AInworldCharacterProxy();

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	void EnableManagedActors();

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	void DisableManagedActors();

	class UInworldCharacterComponent* GetBestInworldCharacterComponent() const { return MostRecentInworldCharacterComponent.Get(); }

	void SetAgentId(const FName& InAgentId);
	void SetGivenName(const FString& InGivenName);

private:
	UFUNCTION()
	void OnPlayerInteractionStateChanged(bool bIsInteracting);

public:
	UPROPERTY(EditAnywhere, Category = "Proxy")
	class UInworldCharacterProxyComponent* CharacterProxyComponent;

	UPROPERTY(EditAnywhere, Category = "Proxy")
	FString ProxyBrainName;

	UPROPERTY(EditAnywhere, Category = "Proxy")
	TArray<AActor*> ActorsToManage;

	UPROPERTY()
	TArray<TWeakObjectPtr<class UInworldCharacterComponent>> ManagedInworldCharacterComponents;

	UPROPERTY()
	TWeakObjectPtr<class UInworldCharacterComponent> MostRecentInworldCharacterComponent;

};
