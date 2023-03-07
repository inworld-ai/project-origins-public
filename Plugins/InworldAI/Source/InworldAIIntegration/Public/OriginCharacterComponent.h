/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterComponent.h"

#include "OriginCharacterComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UOriginCharacterComponent : public UInworldCharacterComponent
{
	GENERATED_BODY()
};
