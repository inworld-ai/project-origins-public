// Fill out your copyright notice in the Description page of Project Settings.


#include "InworldCharacterProxyComponent.h"
#include "InworldCharacterProxy.h"

void UInworldCharacterProxyComponent::SetAgentId(const FName& InAgentId)
{
	AInworldCharacterProxy* InworldCharacterProxyOwner = GetOwnerAsInworldCharacterProxy();
	if (!InworldCharacterProxyOwner)
	{
		return;
	}

	InworldCharacterProxyOwner->SetAgentId(InAgentId);
}

void UInworldCharacterProxyComponent::SetGivenName(const FString& InGivenName)
{
	AInworldCharacterProxy* InworldCharacterProxyOwner = GetOwnerAsInworldCharacterProxy();
	if (!InworldCharacterProxyOwner)
	{
		return;
	}

	InworldCharacterProxyOwner->SetGivenName(InGivenName);
}

void UInworldCharacterProxyComponent::HandlePacket(TSharedPtr<Inworld::FInworldPacket> Packet)
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

void UInworldCharacterProxyComponent::HandlePlayerTalking(const Inworld::FTextEvent& Event)
{
	AInworldCharacterProxy* InworldCharacterProxyOwner = GetOwnerAsInworldCharacterProxy();
	if (!InworldCharacterProxyOwner)
	{
		return;
	}

	UInworldCharacterComponent* InworldCharacterComponent = InworldCharacterProxyOwner->GetBestInworldCharacterComponent();
	if (InworldCharacterComponent)
	{
		InworldCharacterComponent->HandlePlayerTalking(Event);
	}
}

void UInworldCharacterProxyComponent::HandlePlayerInteraction(bool bInteracting)
{
	AInworldCharacterProxy* InworldCharacterProxyOwner = GetOwnerAsInworldCharacterProxy();
	if (!InworldCharacterProxyOwner)
	{
		return;
	}

	UInworldCharacterComponent* InworldCharacterComponent = InworldCharacterProxyOwner->GetBestInworldCharacterComponent();
	if (InworldCharacterComponent)
	{
		InworldCharacterComponent->HandlePlayerInteraction(bInteracting);
	}
}

AInworldCharacterProxy* UInworldCharacterProxyComponent::GetOwnerAsInworldCharacterProxy() const
{
	return Cast<AInworldCharacterProxy>(GetOwner());
}
