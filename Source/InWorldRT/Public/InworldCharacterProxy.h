// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InworldTypes.h"
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

	void SetBestInworldCharacterComponent(class UInworldCharacterComponent* InworldCharacterComponent);

	class UInworldCharacterComponent* GetBestInworldCharacterComponent() const { return MostRecentInworldCharacterComponent.Get(); }

	void Possess(const FInworldAgentInfo& AgentInfo);
	void Unpossess();

	const TArray<class UInworldCharacterComponent*>& GetManagedCharacterComponents() const { return RegisteredCharacterComponents; }

private:

	void RegisterProxyCharacterComponent(UInworldCharacterComponent* Character) { RegisteredCharacterComponents.AddUnique(Character); }
	void UnregisterProxyCharacterComponent(UInworldCharacterComponent* Character) { RegisteredCharacterComponents.Remove(Character); }

protected:
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

	UPROPERTY()
	TArray<class UInworldCharacterComponent*> RegisteredCharacterComponents;

};
