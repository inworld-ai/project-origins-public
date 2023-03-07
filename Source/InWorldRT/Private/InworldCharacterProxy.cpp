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
			InworldCharacterComponent->OnPlayerInteractionStateChanged.AddDynamic(this, &AInworldCharacterProxy::OnPlayerInteractionStateChanged);

			if (InworldCharacterComponent->GetBrainName().IsEmpty())
			{
				InworldCharacterComponent->SetBrainName(ProxyBrainName);
				InworldCharacterComponent->Register();
				InworldCharacterComponent->SetBrainName(FString());
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
			InworldCharacterComponent->OnPlayerInteractionStateChanged.RemoveDynamic(this, &AInworldCharacterProxy::OnPlayerInteractionStateChanged);

			if (InworldCharacterComponent->GetBrainName().IsEmpty())
			{
				InworldCharacterComponent->SetBrainName(ProxyBrainName);
				InworldCharacterComponent->Unregister();
				InworldCharacterComponent->SetBrainName(FString());
			}
		}
	}
	CharacterProxyComponent->SetBrainName(ProxyBrainName);
	CharacterProxyComponent->Unregister();

	SetAgentId(FName());
	SetGivenName(FString());
}

void AInworldCharacterProxy::SetAgentId(const FName& InAgentId)
{
	for (auto Character : ManagedInworldCharacterComponents)
	{
		Character->SetAgentId(InAgentId);
	}
}

void AInworldCharacterProxy::SetGivenName(const FString& InGivenName)
{
	for (auto Character : ManagedInworldCharacterComponents)
	{
		Character->SetGivenName(InGivenName);
	}
}

void AInworldCharacterProxy::OnPlayerInteractionStateChanged(bool bIsInteracting)
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController<APlayerController>();
	if (!PlayerController)
	{
		return;
	}

	APawn* PlayerPawn = PlayerController->GetPawn<APawn>();
	if (!PlayerPawn)
	{
		return;
	}

	UInworldPlayerComponent* InworldPlayerComponent = Cast<UInworldPlayerComponent>(PlayerPawn->GetComponentByClass(UInworldPlayerComponent::StaticClass()));
	if (!InworldPlayerComponent)
	{
		return;
	}

	UInworldCharacterComponent* InteractingCharacterComponent = static_cast<UInworldCharacterComponent*>(InworldPlayerComponent->GetTargetCharacter());

	if (bIsInteracting && ManagedInworldCharacterComponents.Contains(InteractingCharacterComponent))
	{
		UInworldCharacterComponent* PreviousComponent = MostRecentInworldCharacterComponent.Get();
		if (PreviousComponent)
		{
			PreviousComponent->CancelCurrentInteraction();
		}
		MostRecentInworldCharacterComponent = InteractingCharacterComponent;
	}
}
