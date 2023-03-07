#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "InworldGameConfig.generated.h"

UCLASS(BlueprintType, config = Game)
class INWORLDAIINTEGRATION_API UInworldGameConfigSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override { return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;  }

	UFUNCTION(BlueprintCallable)
		void Save() { SaveConfig(CPF_Config, *GGameIni); }

	UPROPERTY(config, BlueprintReadWrite)
	FString PlayerName = "Detective";
	
	UPROPERTY(config, BlueprintReadWrite)
	bool bEnableDebugInfo = true;

	UPROPERTY(config, BlueprintReadWrite)
	bool bEnableDialogHistory = false;
};
