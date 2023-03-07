/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "OriginBlueprintFunctionLibrary.h"

#include "InworldApi.h"
#include "InworldCharacterComponent.h"

bool UOriginBlueprintFunctionLibrary::LogToFile(FString Content)
{
	const static FString LOG_PATH = "Logs/";
	const static FString LOG_PREFIX = "Log_";
	const static FString LOG_SUFFIX = ".txt";
	return FFileHelper::SaveStringToFile(Content, *(FPaths::ProjectDir() + LOG_PATH + LOG_PREFIX + FDateTime::UtcNow().ToString() + LOG_SUFFIX));
}

void UOriginBlueprintFunctionLibrary::GetAllInworldCharacterComponents(const UObject* WorldContextObject, TArray<UInworldCharacterComponent*>& OutInworldCharacterComponents)
{
	auto* InworldSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (!ensure(InworldSubsystem))
	{
		return;
	}

	for (auto Character : InworldSubsystem->GetCharacterComponents())
	{
		UInworldCharacterComponent* InworldCharacterComponent = static_cast<UInworldCharacterComponent*>(Character);
		OutInworldCharacterComponents.Add(InworldCharacterComponent);
	}
}
