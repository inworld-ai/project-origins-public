/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldPlayerTargetingComponent.h"
#include "Camera/CameraComponent.h"
#include "InworldPlayerComponent.h"
#include "InworldCharacterComponent.h"
#include "InworldApi.h"


UInworldPlayerTargetingComponent::UInworldPlayerTargetingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInworldPlayerTargetingComponent::BeginPlay()
{
    Super::BeginPlay();

    InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
    PlayerComponent = Cast<UInworldPlayerComponent>(GetOwner()->GetComponentByClass(UInworldPlayerComponent::StaticClass()));
}

void UInworldPlayerTargetingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTargetCharacter();
}

void UInworldPlayerTargetingComponent::SetPermanentTargetCharacter(UInworldCharacterComponent* Character)
{
    UnsetPermanentTargetCharacter(Character);
    PermanentTargetCharacterPriorityList.Add(Character);
}

void UInworldPlayerTargetingComponent::UnsetPermanentTargetCharacter(UInworldCharacterComponent* Character)
{
    PermanentTargetCharacterPriorityList.Remove(Character);
}

UInworldCharacterComponent* UInworldPlayerTargetingComponent::GetFocusTargetCharacter() const
{
    const auto& CharacterComponents = InworldSubsystem->GetCharacterComponents();
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

    for (auto& Character : CharacterComponents)
    {
        if (!Character)
        {
            continue;
        }

        auto CandidateCharacter = static_cast<UInworldCharacterComponent*>(Character);
        const FVector CharacterLocation = CandidateCharacter->GetComponentOwner()->GetActorLocation();
        const FVector CharacterForward = CandidateCharacter->GetComponentOwner()->GetActorForwardVector();

        FVector ToOwner = Location - CharacterLocation;
        ToOwner.Normalize();
        const float ToOwnerFoV = (180.f) / PI * FMath::Acos(FVector::DotProduct(ToOwner, CharacterForward));
        if (ToOwnerFoV <= -90.f || ToOwnerFoV >= 90.f)
        {
            continue;
        }

        FVector ToCharacter = CharacterLocation - Location;
        ToCharacter.Normalize();
        const float ToCharacterFoV = (180.f) / PI * FMath::Acos(FVector::DotProduct(ToCharacter, Forward));
        if (ToCharacterFoV <= -90.f || ToCharacterFoV >= 90.f)
        {
            continue;
        }

        const float DistSq = FVector::DistSquared(Location, CharacterLocation);
        if (DistSq > (CandidateCharacter->bIsMainCharacter ? MinDistSq : AltMinDistSq))
        {
            continue;
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

        if (Dot < (CandidateCharacter->bIsMainCharacter ? InteractionDotThreshold : AltInteractionDotThreshold))
        {
            continue;
        }

        if (CandidateCharacter->bIsMainCharacter && Dot < CurMaxDot)
        {
            if (Dot < CurMaxDot)
            {
                continue;
            }
            CurMaxDot = Dot;
        }

        if (!CandidateCharacter->bIsMainCharacter)
        {
            if (Dot < AltCurMaxDot)
            {
                continue;
            }
            AltCurMaxDot = Dot;
        }

        if (!CandidateCharacter->bIsMainCharacter && ClosestCharacter.IsValid() && ClosestCharacter->bIsMainCharacter)
        {
            continue;
        }

        ClosestCharacter = CandidateCharacter;
    }

    return ClosestCharacter.Get();
}

UInworldCharacterComponent* UInworldPlayerTargetingComponent::GetPermanentTargetCharacter() const
{
    return PermanentTargetCharacterPriorityList.Num() > 0 ? PermanentTargetCharacterPriorityList.Last().Get() : nullptr;
}

void UInworldPlayerTargetingComponent::UpdateTargetCharacter()
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

    if (PlayerComponent->GetTargetCharacter() && (!TargetCharacter.IsValid() || PlayerComponent->GetTargetCharacter() != TargetCharacter.Get()))
    {
        ClearTargetCharacter();
    }

    if (TargetCharacter.IsValid() && !PlayerComponent->GetTargetCharacter())
    {
        SetTargetCharacter(TargetCharacter);
    }
}

void UInworldPlayerTargetingComponent::SetTargetCharacter(TWeakObjectPtr<UInworldCharacterComponent> Character)
{
    if (ChangeTargetCharacterTimer.CheckPeriod(GetWorld()))
    {
        PlayerComponent->SetTargetCharacter(Character.Get());
    }
}

void UInworldPlayerTargetingComponent::ClearTargetCharacter()
{
    if (PlayerComponent->GetTargetCharacter() && ChangeTargetCharacterTimer.CheckPeriod(GetWorld()))
    {
        PlayerComponent->ClearTargetCharacter();
    }
}

