/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OriginBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class INWORLDRT_API UOriginBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Logging")
	static bool LogToFile(FString Content);

	UFUNCTION(BlueprintCallable, Category = "Utility", meta = (WorldContext = "WorldContextObject"))
	static void GetAllInworldCharacterComponents(const UObject* WorldContextObject, TArray<UInworldCharacterComponent*>& OutInworldCharacterComponents);

};
