// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterComponent.h"
#include "InworldCharacterProxyComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INWORLDRT_API UInworldCharacterProxyComponent : public UInworldCharacterComponent
{
	GENERATED_BODY()

public:
	virtual void SetAgentId(const FName& InAgentId) override;
	virtual void SetGivenName(const FString& InGivenName) override;

protected:
	virtual void HandlePacket(TSharedPtr<Inworld::FInworldPacket> Packet) override;
	virtual void HandlePlayerTalking(const Inworld::FTextEvent& Event) override;
	virtual void HandlePlayerInteraction(bool bInteracting) override;

private:
	class AInworldCharacterProxy* GetOwnerAsInworldCharacterProxy() const;
};
