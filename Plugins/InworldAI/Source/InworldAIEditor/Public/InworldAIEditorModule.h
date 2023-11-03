// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserDelegates.h"
#include "InworldStudioTypes.h"
#include "InworldEditorUtilityWidget.h"

INWORLDAIEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogInworldAIEditor, Log, All);

DECLARE_DELEGATE_OneParam(FAssetAction, const FAssetData&);
DECLARE_DELEGATE_RetVal_OneParam(bool, FAssetActionPermission, const FAssetData&);

class INWORLDAIEDITOR_API FInworldAIEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void SetStudioData(const FInworldStudioUserData& Data) 
	{
		StudioData = Data;
	}

	const FInworldStudioUserData& GetStudioData() const
	{
		return StudioData;
	}

	void SetStudioWidgetState(const FInworldEditorUtilityWidgetState& Data)
	{
		StudioWidgetState = Data;
	}

	const FInworldEditorUtilityWidgetState& GetStudioWidgetState() const
	{
		return StudioWidgetState;
	}

private:
	void AssetExtenderFunc(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets);
	TSharedRef<FExtender> OnExtendAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);

	static bool CanSetupAssetAsInworldPlayer(const FAssetData& AssetData);
	static void SetupAssetAsInworldPlayer(const FAssetData& AssetData);

	void OpenInworldStudioWidget(TSharedPtr<SWindow>, bool);

	FDelegateHandle OpenInworldStudioWidgetDelHandle;

	FInworldStudioUserData StudioData;
	FInworldEditorUtilityWidgetState StudioWidgetState;

public:
	void BindMenuAssetAction(const FName& Name, const FName& Section, FText Label, FText Tooltip, FAssetAction Action, FAssetActionPermission ActionPermission);
	void UnbindMenuAssetAction(const FName& Name);

private:
	struct FAssetActionMenuFunction
	{
	public:
		FText Label;
		FText Tooltip;
		FSlateIcon Icon;
		FAssetAction Action;
		FAssetActionPermission ActionPermission;
	};

	struct FAssetActionMenuSection
	{
		TMap<FName, FAssetActionMenuFunction> FunctionMap;
	};

	struct FAssetActionMenu
	{
	public:
		TMap<FName, FAssetActionMenuSection> SectionMap;
		TMap<FName, FName> NameToSectionMap;
		TSet<FName> Sections;
	};

	FAssetActionMenu AssetActionMenu;
};
