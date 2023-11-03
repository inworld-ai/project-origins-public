// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterComponent.h"
#include "InworldCharacterProxyComponent.generated.h"


UCLASS(ClassGroup = (Origins), meta = (BlueprintSpawnableComponent))
class INWORLDRT_API UInworldCharacterProxyComponent : public UInworldCharacterComponent
{
	GENERATED_BODY()

public:
	virtual void Possess(const FInworldAgentInfo& AgentInfo) override;
	virtual void Unpossess() override;

protected:
	virtual void HandlePacket(TSharedPtr<FInworldPacket> Packet) override;
	virtual bool StartPlayerInteraction(UInworldPlayerComponent* Player) override;
	virtual bool StopPlayerInteraction(UInworldPlayerComponent* Player) override;

public:
	class AInworldCharacterProxy* GetOwnerAsInworldCharacterProxy() const;
};
