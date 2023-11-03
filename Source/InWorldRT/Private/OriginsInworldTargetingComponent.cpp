// Fill out your copyright notice in the Description page of Project Settings.


#include "OriginsInworldTargetingComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "InworldPlayerComponent.h"
#include "OriginsInworldCharacterComponent.h"
#include "OriginsInworldPlayerComponent.h"
#include "InworldCharacterProxyComponent.h"
#include "InworldCharacterProxy.h"
#include "Kismet/GameplayStatics.h"

void UOriginsInworldTargetingComponent::SetPermanentTargetCharacter(UInworldCharacterComponent* Character)
{
    UnsetPermanentTargetCharacter(Character);
    PermanentTargetCharacterPriorityList.Add(Character);
}

void UOriginsInworldTargetingComponent::UnsetPermanentTargetCharacter(UInworldCharacterComponent* Character)
{
    PermanentTargetCharacterPriorityList.Remove(Character);
}

UInworldCharacterComponent* UOriginsInworldTargetingComponent::GetFocusTargetCharacter() const
{
    const auto& CharacterComponents = InworldSubsystem->GetCharacterComponents();

    TArray<UInworldCharacterComponent*> ProxyComponents;
    TArray<AActor*> CharacterProxies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInworldCharacterProxy::StaticClass(), CharacterProxies);
    for (const auto& CharacterProxy : CharacterProxies)
    {
        AInworldCharacterProxy* CastCharacterProxy = Cast<AInworldCharacterProxy>(CharacterProxy);
        ProxyComponents.Append(CastCharacterProxy->GetManagedCharacterComponents());
    }

    TWeakObjectPtr<UInworldCharacterComponent> ClosestCharacter;
    const float MinDistSq = InteractionDistance * InteractionDistance;
    const float AltMinDistSq = AltInteractionDistance * AltInteractionDistance;
    float AltCurMaxDot = -1.f;
    float CurMaxDot = -1.f;
    const FVector Location = GetOwner()->GetActorLocation();
    const FVector Forward = GetOwner()->GetActorForwardVector();

    if (Location == FVector::ZeroVector)
    {
        return nullptr;
    }

    const auto Eval = [&](Inworld::ICharacterComponent* Character)
        {
            if (!Character)
            {
                return;
            }

            bool bIsMainCharacter = false;

            UInworldCharacterComponent* CandidateCharacter = static_cast<UInworldCharacterComponent*>(Character);

            if (CandidateCharacter->IsA<UOriginsInworldCharacterComponent>())
            {
                UOriginsInworldCharacterComponent* OriginsCandidateCharacter = Cast<UOriginsInworldCharacterComponent>(CandidateCharacter);
                if (OriginsCandidateCharacter != nullptr)
                {
                    if(!OriginsCandidateCharacter->bCanInteractNow)
                    {
                        return;
                    }
                    bIsMainCharacter = OriginsCandidateCharacter->bIsMainCharacter;
                }
            }

            AActor* OwningCharacterActor = CandidateCharacter->GetComponentOwner();
            if (!OwningCharacterActor)
            {
                return;
            }
            ACharacter* OwningCharacter = Cast<ACharacter>(OwningCharacterActor);

            const FVector CharacterLocation = OwningCharacter ? OwningCharacter->GetMesh()->GetComponentLocation() : OwningCharacterActor->GetActorLocation();
            const FVector CharacterForward = OwningCharacterActor->GetActorForwardVector();

            FVector ToOwner = Location - CharacterLocation;
            ToOwner.Normalize();
            const float ToOwnerFoV = (180.f) / PI * FMath::Acos(FVector::DotProduct(ToOwner, CharacterForward));
            if (ToOwnerFoV <= -90.f || ToOwnerFoV >= 90.f)
            {
                return;
            }

            FVector ToCharacter = CharacterLocation - Location;
            ToCharacter.Normalize();
            const float ToCharacterFoV = (180.f) / PI * FMath::Acos(FVector::DotProduct(ToCharacter, Forward));
            if (ToCharacterFoV <= -90.f || ToCharacterFoV >= 90.f)
            {
                return;
            }

            const float DistSq = FVector::DistSquared(Location, CharacterLocation);
            if (DistSq > (bIsMainCharacter ? MinDistSq : AltMinDistSq))
            {
                return;
            }

            const FVector2D Direction2D = FVector2D(CharacterLocation - Location).GetSafeNormal();
            FVector2D Forward2D;
            if (auto* CameraComponent = Cast<UCameraComponent>(GetOwner()->GetComponentByClass(UCameraComponent::StaticClass())))
            {
                Forward2D = FVector2D(CameraComponent->K2_GetComponentRotation().Vector());
            }
            else
            {
                Forward2D = FVector2D(GetOwner()->GetActorRotation().Vector());
            }

            const float Dot = FVector2D::DotProduct(Forward2D, Direction2D);

            const float InteractionDotThreshold = CandidateCharacter == PlayerComponent->GetTargetCharacter() ? MaintainInteractionDotThreshold : BeginInteractionDotThreshold;
            const float AltInteractionDotThreshold = CandidateCharacter == PlayerComponent->GetTargetCharacter() ? AltMaintainInteractionDotThreshold : AltBeginInteractionDotThreshold;

            if (Dot < (bIsMainCharacter ? InteractionDotThreshold : AltInteractionDotThreshold))
            {
                return;
            }

            if (bIsMainCharacter && Dot < CurMaxDot)
            {
                if (Dot < CurMaxDot)
                {
                    return;
                }
                CurMaxDot = Dot;
            }

            if (!bIsMainCharacter)
            {
                if (Dot < AltCurMaxDot)
                {
                    return;
                }
                AltCurMaxDot = Dot;
            }

            UOriginsInworldCharacterComponent* ClosestOriginsCharacter = Cast<UOriginsInworldCharacterComponent>(ClosestCharacter.Get());
            bool bClosestIsMainCharacter = false;
            if (ClosestOriginsCharacter != nullptr)
            {
                bClosestIsMainCharacter = ClosestOriginsCharacter->bIsMainCharacter;
            }

            if (!bIsMainCharacter && ClosestCharacter.IsValid() && bClosestIsMainCharacter)
            {
                return;
            }

            ClosestCharacter = CandidateCharacter;
        };

    for (Inworld::ICharacterComponent* Character : CharacterComponents)
    {
        Eval(Character);
    }

    for (auto& Character : ProxyComponents)
    {
        Eval(Character);
    }

    return ClosestCharacter.Get();
}

UInworldCharacterComponent* UOriginsInworldTargetingComponent::GetPermanentTargetCharacter() const
{
    return PermanentTargetCharacterPriorityList.Num() > 0 ? PermanentTargetCharacterPriorityList.Last().Get() : nullptr;
}

void UOriginsInworldTargetingComponent::UpdateTargetCharacter()
{
    TWeakObjectPtr<UInworldCharacterComponent> PreviousFocusTargetCharacter = FocusTargetCharacter.Get();

    FocusTargetCharacter = GetFocusTargetCharacter();
    if (PreviousFocusTargetCharacter != FocusTargetCharacter)
    {
        OnFocusTargetCharacterChanged.Broadcast(FocusTargetCharacter.Get());
    }

    TWeakObjectPtr<UInworldCharacterComponent> PreviousPermanentTargetCharacter = PermanentTargetCharacter.Get();

    PermanentTargetCharacter = GetPermanentTargetCharacter();
    if (PreviousPermanentTargetCharacter != PermanentTargetCharacter)
    {
        OnPermanentTargetCharacterChanged.Broadcast(PermanentTargetCharacter.Get());
    }

    TWeakObjectPtr<UInworldCharacterComponent> TargetCharacter = PermanentTargetCharacter.IsValid() ? PermanentTargetCharacter : FocusTargetCharacter;
    if (TargetCharacter.IsValid())
    {
        // Swap with proxy component if one exists, and mark the fake one as the pass-thru
        TWeakObjectPtr<UInworldCharacterComponent> ResolvedTargetCharacter = static_cast<UInworldCharacterComponent*>(InworldSubsystem->GetCharacterComponentByAgentId(TargetCharacter->GetAgentId()));
        if (ResolvedTargetCharacter != nullptr && ResolvedTargetCharacter->IsA<UInworldCharacterProxyComponent>())
        {
            UInworldCharacterProxyComponent* ProxyComponent = Cast<UInworldCharacterProxyComponent>(ResolvedTargetCharacter.Get());
            ProxyComponent->GetOwnerAsInworldCharacterProxy()->SetBestInworldCharacterComponent(TargetCharacter.Get());
            TargetCharacter = ResolvedTargetCharacter;
        }
    }

    if (PlayerComponent->GetTargetCharacter() && (!TargetCharacter.IsValid() || PlayerComponent->GetTargetCharacter() != TargetCharacter.Get()))
    {
        ClearTargetCharacter();
    }

    if (TargetCharacter.IsValid() && !PlayerComponent->GetTargetCharacter())
    {
        SetTargetCharacter(TargetCharacter);
    }
}
