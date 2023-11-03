// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.


#include "DownloadInnequinPluginAction.h"

#include "InworldEditorApi.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/MonitoredProcess.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

UDownloadInnequinPluginAction* UDownloadInnequinPluginAction::DownloadInnequinPlugin(FOnDownloadInnequinLog InLogCallback)
{
	UDownloadInnequinPluginAction* BlueprintNode = NewObject<UDownloadInnequinPluginAction>();
	BlueprintNode->LogCallback = InLogCallback;
	return BlueprintNode;
}

void UDownloadInnequinPluginAction::Activate()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
		{
			bool bSuccess = false;
			if (auto* World = GEditor->GetEditorWorldContext().World())
			{
				auto* InworldEditorApi = World->GetSubsystem<UInworldEditorApiSubsystem>();

				const FString TempPath = FDesktopPlatformModule::Get()->GetUserTempPath();
				const FString ZipLocation = FString::Format(TEXT("{0}{1}"), { TempPath, TEXT("InworldInnequin.zip") });
				const FString ZipURL = FString::Format(TEXT("https://storage.googleapis.com/assets-inworld-ai/models/innequin/unreal/InworldInnequin_{0}.zip"), { InworldEditorApi->GetInnequinVersion() });
				FStringFormatOrderedArguments GetZipFormattedArgs;
				GetZipFormattedArgs.Add(FStringFormatArg(TEXT("--output")));
				GetZipFormattedArgs.Add(FStringFormatArg(ZipLocation));
				GetZipFormattedArgs.Add(FStringFormatArg(ZipURL));
				const FString GetZipCommandLineArgs = FString::Format(TEXT("{0} \"{1}\" \"{2}\""), GetZipFormattedArgs);
				TSharedPtr<FMonitoredProcess> GetZipProcess = MakeShareable(new FMonitoredProcess(TEXT("curl"), GetZipCommandLineArgs, true));
				NotifyLog(TEXT("Downloading..."));
				GetZipProcess->OnOutput().BindLambda([this](const FString& Output)
					{
						NotifyLog(Output);
					}
				);
				if (!GetZipProcess->Launch())
				{
					NotifyComplete(false);
					return;
				}
				while (GetZipProcess->Update())
				{
					FPlatformProcess::Sleep(0.01f);
				}
				const int GetZipRetCode = GetZipProcess->GetReturnCode();
				if (GetZipRetCode != 0)
				{
					NotifyComplete(false);
					return;
				}
				NotifyLog(TEXT("Downloading Complete!"));

				const FString PluginLocation = FPaths::ProjectPluginsDir();
				FStringFormatOrderedArguments UnzipFormattedArgs;
				UnzipFormattedArgs.Add(FStringFormatArg(TEXT("-xf")));
				UnzipFormattedArgs.Add(FStringFormatArg(ZipLocation));
				UnzipFormattedArgs.Add(FStringFormatArg(TEXT("-C")));
				UnzipFormattedArgs.Add(FStringFormatArg(PluginLocation));
				const FString UnzipCommandLineArgs = FString::Format(TEXT("{0} \"{1}\" {2} \"{3}\""), UnzipFormattedArgs);
				TSharedPtr<FMonitoredProcess> UnzipProcess = MakeShareable(new FMonitoredProcess(TEXT("tar"), UnzipCommandLineArgs, true));
				NotifyLog(TEXT("Extracting..."));
				if (!UnzipProcess->Launch())
				{
					NotifyComplete(false);
					return;
				}
				while (UnzipProcess->Update())
				{
					FPlatformProcess::Sleep(0.01f);
				}
				const int UnzipRetCode = UnzipProcess->GetReturnCode();
				if (UnzipRetCode != 0)
				{
					NotifyComplete(false);
					return;
				}
				NotifyLog(TEXT("Extracting Complete!"));

				NotifyComplete(true);
			}
		}
	);
}

void UDownloadInnequinPluginAction::NotifyLog(const FString& Message)
{
	if (IsInGameThread())
	{
		LogCallback.ExecuteIfBound(Message);
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this, Message]()
			{
				NotifyLog(Message);
			}
		);
	}
}

void UDownloadInnequinPluginAction::NotifyComplete(bool bSuccess)
{
	if (IsInGameThread())
	{
		if (bSuccess)
		{
			if (auto* World = GEditor->GetEditorWorldContext().World())
			{
				auto* InworldEditorApi = World->GetSubsystem<UInworldEditorApiSubsystem>();
				InworldEditorApi->NotifyRestartRequired();
			}
			NotifyLog(TEXT("Restart Required!"));
		}
		else
		{
			NotifyLog(TEXT("Something went wrong! Please download file and install manually."));
		}
		DownloadComplete.Broadcast(bSuccess);
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this, bSuccess]()
			{
				NotifyComplete(bSuccess);
			}
		);
	}
}
