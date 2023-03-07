/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "InworldStudioUserData.generated.h"

USTRUCT(BlueprintType)
struct FInworldStudioUserCharacterData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString ShortName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString RpmModelUri;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString RpmImageUri;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString RpmPortraitUri;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString RpmPostureUri;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool bMale = false;

	mutable FString RpmModelData;
};

USTRUCT(BlueprintType)
struct FInworldStudioUserSceneData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString ShortName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TArray<FString> Characters;
};

USTRUCT(BlueprintType)
struct FInworldStudioUserApiKeyData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString Key;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString Secret;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool IsActive;
};

USTRUCT(BlueprintType)
struct FInworldStudioUserWorkspaceData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString ShortName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TArray<FInworldStudioUserCharacterData> Characters;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TArray<FInworldStudioUserSceneData> Scenes;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TArray<FInworldStudioUserApiKeyData> ApiKeys;
};

USTRUCT(BlueprintType)
struct FInworldStudioUserData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TArray<FInworldStudioUserWorkspaceData> Workspaces;
};

USTRUCT()
struct FRefreshTokenRequestData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString grant_type;

	UPROPERTY()
	FString refresh_token;
};

USTRUCT()
struct FRefreshTokenResponseData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString access_token;

	UPROPERTY()
	int32 expires_in;
	
	UPROPERTY()
	FString token_type;

	UPROPERTY()
	FString refresh_token;

	UPROPERTY()
	FString id_token;
	
	UPROPERTY()
	FString user_id;

	UPROPERTY()
	FString project_id;
};
