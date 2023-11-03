// Fill out your copyright notice in the Description page of Project Settings.


#include "InworldCharacterProxy.h"
#include "InworldApi.h"
#include "InworldCharacterProxyComponent.h"
#include "InworldCharacterComponent.h"
#include "InworldPlayerComponent.h"

AInworldCharacterProxy::AInworldCharacterProxy()
	: Super()
{
	CharacterProxyComponent = CreateDefaultSubobject<UInworldCharacterProxyComponent>(TEXT("ProxyCharacterComponent"));
}

void AInworldCharacterProxy::SetBestInworldCharacterComponent(class UInworldCharacterComponent* InworldCharacterComponent)
{
	MostRecentInworldCharacterComponent = InworldCharacterComponent;
}

void AInworldCharacterProxy::EnableManagedActors()
{
	auto* InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (!ensure(InworldSubsystem))
	{
		return;
	}
	for (AActor* InworldCharacterActor : ActorsToManage)
	{
		if (!InworldCharacterActor)
		{
			continue;
		}
		UInworldCharacterComponent* InworldCharacterComponent = Cast<UInworldCharacterComponent>(InworldCharacterActor->GetComponentByClass(UInworldCharacterComponent::StaticClass()));
		if (InworldCharacterComponent)
		{
			ManagedInworldCharacterComponents.Add(InworldCharacterComponent);

			if (InworldCharacterComponent->GetBrainName().IsEmpty())
			{
				RegisterProxyCharacterComponent(InworldCharacterComponent);
			}
		}
	}
	CharacterProxyComponent->SetBrainName(ProxyBrainName);
	CharacterProxyComponent->Register();
}

void AInworldCharacterProxy::DisableManagedActors()
{
	auto* InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (!ensure(InworldSubsystem))
	{
		return;
	}
	for (AActor* InworldCharacterActor : ActorsToManage)
	{
		if (!InworldCharacterActor)
		{
			continue;
		}
		UInworldCharacterComponent* InworldCharacterComponent = Cast<UInworldCharacterComponent>(InworldCharacterActor->GetComponentByClass(UInworldCharacterComponent::StaticClass()));
		if (InworldCharacterComponent)
		{
			ManagedInworldCharacterComponents.Remove(InworldCharacterComponent);

			if (InworldCharacterComponent->GetBrainName().IsEmpty())
			{
				UnregisterProxyCharacterComponent(InworldCharacterComponent);
			}
		}
	}
	CharacterProxyComponent->SetBrainName(ProxyBrainName);
	CharacterProxyComponent->Unregister();
}

void AInworldCharacterProxy::Possess(const FInworldAgentInfo& AgentInfo)
{
	for (auto Character : ManagedInworldCharacterComponents)
	{
		Character->Possess(AgentInfo);
	}
}

void AInworldCharacterProxy::Unpossess()
{
	for (auto Character : ManagedInworldCharacterComponents)
	{
		Character->Unpossess();
	}
}
