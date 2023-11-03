// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "TickableEditorObject.h"
#include "InworldStudio.h"
#include "InworldStudioTypes.h"
#include "InworldEditorClient.h"
#include "InworldEditorApi.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInworldEditorApiSubsystemOnLogin, bool, bSuccess, FInworldStudioUserData, Data);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCharacterStudioDataAction, const FInworldStudioUserCharacterData&, CharacterStudioData);

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnCharacterStudioDataPermission, const FInworldStudioUserCharacterData&, CharacterStudioData);

UCLASS(BlueprintType, Config = InworldAI)
class INWORLDAIEDITOR_API UInworldEditorApiSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FInworldEditorApiSubsystemOnLogin OnLogin;

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	const FString& GetSavedStudioAccessToken() const;

    UFUNCTION(BlueprintCallable, Category = "Inworld")
	void RequestStudioData(const FString& ExchangeToken);

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	void CancelRequestStudioData();

	void NotifyRestartRequired();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inworld")
	bool IsRequestInProgress() const { return EditorClient.IsRequestInProgress() || Studio.IsRequestInProgress(); }

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	UWorld* GetViewportWorld() const { return GetWorld(); }

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	TArray<FString> GetWorldActorNames() const;

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	void SetupActor(const FInworldStudioUserCharacterData& Data, const FString& Name, const FString& PreviousName);

	UFUNCTION(BlueprintPure, Category = "Inworld")
	const FString& GetError() { return !EditorClient.GetError().IsEmpty() ? EditorClient.GetError() : Studio.GetError(); }

	UFUNCTION(BlueprintPure, Category = "Inworld")
	const FInworldStudioUserData& GetCachedStudioData() const;

	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	void BindActionForCharacterData(const FName& Name, FOnCharacterStudioDataPermission Permission, FOnCharacterStudioDataAction Action);

	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	void UnbindActionForCharacterData(const FName& Name);

	UFUNCTION(BlueprintPure, Category = "Inworld")
	void GetCharacterDataActions(TArray<FName>& OutKeys) const;

	UFUNCTION(BlueprintPure, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	bool CanExecuteCharacterDataAction(const FName& Name, const FInworldStudioUserCharacterData& CharacterStudioData);
	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	void ExecuteCharacterDataAction(const FName& Name, const FInworldStudioUserCharacterData& CharacterStudioData);

	bool CanSetupAssetAsInworldPlayer(const FAssetData& AssetData, bool bLogErrors = false);
	void SetupAssetAsInworldPlayer(const FAssetData& AssetData);
	void SetupBlueprintAsInworldPlayer(UBlueprint* Blueprint);

	bool CanSetupAssetAsInworldCharacter(const FAssetData& AssetData, bool bLogErrors = false);
	void SetupAssetAsInworldCharacter(const FAssetData& AssetData);
	void SetupBlueprintAsInworldCharacter(UBlueprint* Blueprint);

	/** Subsystem interface */
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UBlueprint* CreateCharacterActorBP(const FInworldStudioUserCharacterData& CharacterData);
	void SavePackageToCharacterFolder(UObject* Object, const FInworldStudioUserCharacterData& CharacterData, const FString& NamePrefix, FString NameSuffix = "");

	UObject* AddNodeToBlueprint(UBlueprint* Blueprint, UClass* Class, const FString& NodeName);
	UObject* AddNodeToBlueprintNode(UBlueprint* Blueprint, const FString& ParentNodeName, UClass* Class, const FString& NodeName);
	UObject* GetNodeFromBlueprint(UBlueprint* Blueprint, const FString& NodeName);

	UFUNCTION()
	bool CanCreateInnequinActor(const FInworldStudioUserCharacterData& CharacterData);
	UFUNCTION()
	void CreateInnequinActor(const FInworldStudioUserCharacterData& CharacterData);

	UFUNCTION(BlueprintPure, Category = "Innequin")
	FString GetInnequinVersion() const { return InnequinPluginVersion; };

	FInworldEditorClient EditorClient;
	FInworldStudio Studio;

private:
	UPROPERTY(config)
	FString InnequinPluginVersion;

	void CacheStudioData(const FInworldStudioUserData& Data);

	struct FCharacterStudioDataFunctions
	{
	public:
		FOnCharacterStudioDataPermission Permission;
		FOnCharacterStudioDataAction Action;
	};

	TMap<FName, FCharacterStudioDataFunctions> CharacterStudioDataFunctionMap;

	TSharedPtr<class FInworldEditorRestartRequiredNotification> RestartRequiredNotification;
};
