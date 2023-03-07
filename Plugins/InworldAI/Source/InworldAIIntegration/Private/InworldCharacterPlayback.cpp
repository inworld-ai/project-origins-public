/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterPlayback.h"
#include "InworldCharacterComponent.h"

void UInworldCharacterPlayback::BeginPlay_Implementation()
{

}

void UInworldCharacterPlayback::EndPlay_Implementation()
{

}

void UInworldCharacterPlayback::SetCharacterComponent(UInworldCharacterComponent* InCharacterComponent)
{
	CharacterComponent = InCharacterComponent;
}

void UInworldCharacterPlayback::HandleMessage(const TSharedPtr<Inworld::FCharacterMessage>& Message)
{
	Message->Accept(*this);
}

const TSharedPtr<Inworld::FCharacterMessage> UInworldCharacterPlayback::GetCurrentMessage() const
{
	return CharacterComponent.IsValid() ? CharacterComponent->GetCurrentMessage() : nullptr;
}
