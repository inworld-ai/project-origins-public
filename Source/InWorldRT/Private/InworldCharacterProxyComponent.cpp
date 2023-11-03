// Fill out your copyright notice in the Description page of Project Settings.


#include "InworldCharacterProxyComponent.h"
#include "InworldCharacterProxy.h"

void UInworldCharacterProxyComponent::Possess(const FInworldAgentInfo& AgentInfo)
{
	AInworldCharacterProxy* InworldCharacterProxyOwner = GetOwnerAsInworldCharacterProxy();
	if (!InworldCharacterProxyOwner)
	{
		return;
	}

	InworldCharacterProxyOwner->Possess(AgentInfo);
	Super::Possess(AgentInfo);
}

void UInworldCharacterProxyComponent::Unpossess()
{
	AInworldCharacterProxy* InworldCharacterProxyOwner = GetOwnerAsInworldCharacterProxy();
	if (!InworldCharacterProxyOwner)
	{
		return;
	}

	InworldCharacterProxyOwner->Unpossess();
	Super::Unpossess();
}

bool UInworldCharacterProxyComponent::StartPlayerInteraction(UInworldPlayerComponent* Player)
{
	Super::StartPlayerInteraction(Player);

	AInworldCharacterProxy* InworldCharacterProxyOwner = GetOwnerAsInworldCharacterProxy();
	if (!InworldCharacterProxyOwner)
	{
		return false;
	}

	UInworldCharacterComponent* InworldCharacterComponent = InworldCharacterProxyOwner->GetBestInworldCharacterComponent();
	if (InworldCharacterComponent)
	{
		return InworldCharacterComponent->StartPlayerInteraction(Player);
	}
	return false;
}

bool UInworldCharacterProxyComponent::StopPlayerInteraction(UInworldPlayerComponent* Player)
{
	Super::StopPlayerInteraction(Player);

	AInworldCharacterProxy* InworldCharacterProxyOwner = GetOwnerAsInworldCharacterProxy();
	if (!InworldCharacterProxyOwner)
	{
		return false;
	}

	UInworldCharacterComponent* InworldCharacterComponent = InworldCharacterProxyOwner->GetBestInworldCharacterComponent();
	if (InworldCharacterComponent)
	{
		return InworldCharacterComponent->StopPlayerInteraction(Player);
	}
	return false;
}

void UInworldCharacterProxyComponent::HandlePacket(TSharedPtr<FInworldPacket> Packet)
{
	AInworldCharacterProxy* InworldCharacterProxyOwner = GetOwnerAsInworldCharacterProxy();
	if (!InworldCharacterProxyOwner)
	{
		return;
	}

	UInworldCharacterComponent* InworldCharacterComponent = InworldCharacterProxyOwner->GetBestInworldCharacterComponent();
	if (InworldCharacterComponent)
	{
		InworldCharacterComponent->HandlePacket(Packet);
	}
}

AInworldCharacterProxy* UInworldCharacterProxyComponent::GetOwnerAsInworldCharacterProxy() const
{
	return Cast<AInworldCharacterProxy>(GetOwner());
}
