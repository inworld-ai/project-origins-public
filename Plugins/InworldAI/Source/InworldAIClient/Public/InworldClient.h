/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "proto/ProtoDisableWarning.h"

#include <string>

#include "InworldState.h"

#include <grpcpp/grpcpp.h>

#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"

#include "Http.h"

#if PLATFORM_WINDOWS
#include "Windows/PreWindowsApi.h"
#endif
#include "proto/world-engine.grpc.pb.h"
#if PLATFORM_WINDOWS
#include "Windows/PostWindowsApi.h"
#endif

#include "proto/ai/inworld/studio/v1alpha/tokens.grpc.pb.h"
#include "proto/ai/inworld/studio/v1alpha/tokens.pb.h"
#include "proto/packets.pb.h"

#if WITH_EDITOR
#include "proto/ai/inworld/studio/v1alpha/users.grpc.pb.h"
#include "proto/ai/inworld/studio/v1alpha/workspaces.grpc.pb.h"
#include "proto/ai/inworld/studio/v1alpha/scenes.grpc.pb.h"
#include "proto/ai/inworld/studio/v1alpha/characters.grpc.pb.h"
#include "proto/ai/inworld/studio/v1alpha/apikeys.grpc.pb.h"
#endif

#include "SentryTransaction.h"
#include "InworldStudioUserData.h"
#include "AudioSessionDumper.h"

namespace InworldEngine = ai::inworld::engine;
namespace InworldPackets = ai::inworld::packets;
namespace InworldV1alpha = ai::inworld::studio::v1alpha;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using FReaderWriter = ::grpc::ClientReaderWriter< InworldPackets::InworldPacket, InworldPackets::InworldPacket>;

namespace Inworld
{
	class FInworldPacket;
	class FTextEvent;
	class FDataEvent;
	class FCustomEvent;

	class FInworldRunnable : public FRunnable
	{
	public:
		bool IsDone() const { return bDone; };
		virtual void Stop() override { bDone = true; }

	protected:
		TAtomic<bool> bDone = false;
	};

    class FAsyncRoutine
    {
    public:
		~FAsyncRoutine() { Stop(); }

		void Start(FInworldRunnable* InRunnable, const TCHAR* ThreadName);
		void Stop()
        {
            if (Thread)
            {
                Thread->Kill(true);
                delete Thread;
                Thread = nullptr;
            }

            if (Runnable)
            {
                delete Runnable;
                Runnable = nullptr;
            }
        }

		bool IsValid() { return Runnable && Thread; }
        bool IsDone() { return Runnable && Runnable->IsDone(); }

        template<typename T>
        T* GetTask() { return static_cast<T*>(Runnable); }

    private:
        FRunnableThread* Thread = nullptr;
		FInworldRunnable* Runnable = nullptr;
    };

    class FRunnableMessaging : public FInworldRunnable
    {
	public:
        FRunnableMessaging(FReaderWriter& InReaderWriter, TAtomic<bool>& bInHasReaderWriterFinished, TQueue<TSharedPtr<Inworld::FInworldPacket>>& InPackets, TFunction<void(const TSharedPtr<Inworld::FInworldPacket> InPacket)> InProcessedCallback = nullptr, TFunction<void(const grpc::Status& Status)> InErrorCallback = nullptr)
			: ReaderWriter(InReaderWriter)
			, bHasReaderWriterFinished(bInHasReaderWriterFinished)
			, Packets(InPackets)
			, ProcessedCallback(InProcessedCallback)
			, ErrorCallback(InErrorCallback)
		{}

	protected:
		FReaderWriter& ReaderWriter;
		TAtomic<bool>& bHasReaderWriterFinished;

		TQueue<TSharedPtr<Inworld::FInworldPacket>>& Packets;
		TFunction<void(const TSharedPtr<Inworld::FInworldPacket> InPacket)> ProcessedCallback;
		TFunction<void(const grpc::Status& Status)> ErrorCallback;
    };

    class FRunnableRead : public FRunnableMessaging
    {
    public:
        FRunnableRead(FReaderWriter& InReaderWriter, TAtomic<bool>& bInHasReaderWriterFinished, TQueue<TSharedPtr<Inworld::FInworldPacket>>& InPackets, TFunction<void(const TSharedPtr<Inworld::FInworldPacket> InPacket)> InProcessedCallback = nullptr, TFunction<void(const grpc::Status& Status)> InErrorCallback = nullptr)
			: FRunnableMessaging(InReaderWriter, bInHasReaderWriterFinished, InPackets, InProcessedCallback, InErrorCallback)
		{}

        virtual uint32 Run() override;
	};

	class FRunnableWrite : public FRunnableMessaging
	{
	public:
        FRunnableWrite(FReaderWriter& InReaderWriter, TAtomic<bool>& bInHasReaderWriterFinished, TQueue<TSharedPtr<Inworld::FInworldPacket>>& InPackets, TFunction<void(const TSharedPtr<Inworld::FInworldPacket> InPacket)> InProcessedCallback = nullptr, TFunction<void(const grpc::Status& Status)> InErrorCallback = nullptr)
			: FRunnableMessaging(InReaderWriter, bInHasReaderWriterFinished, InPackets, InProcessedCallback, InErrorCallback)
		{}

		virtual uint32 Run() override;
	};

	class FRunnableAudioDumper : public FInworldRunnable
	{
	public:
		FRunnableAudioDumper(TQueue<std::string>& InAudioChuncks, const FString& InFileName)
			: AudioChuncks(InAudioChuncks)
			, FileName(InFileName)
		{}

		virtual uint32 Run() override;

	private:

		FAudioSessionDumper AudioDumper;
		TQueue<std::string>& AudioChuncks;
		FString FileName;
	};

	template<typename TService, class TResponse>
	class FRunnableRequest : public FInworldRunnable
	{
	public:
		FRunnableRequest(const FString& InServerUrl, TFunction<void(const grpc::Status& Status, const TResponse& Response)> InCallback = nullptr)
			: ServerUrl(InServerUrl)
			, Callback(InCallback)
		{}

		virtual grpc::Status RunProcess() = 0;

		virtual uint32 Run() override
		{
			Status = RunProcess();

			if (Callback)
			{
				Callback(Status, Response);
			}

			bDone = true;

			return 0;
		}

		virtual void Stop() override
		{
			if (Context)
			{
				Context->TryCancel();
			}

			FInworldRunnable::Stop();
		}

		grpc::Status& GetStatus() { return Status; }
		TResponse& GetResponse() { return Response; }
		std::unique_ptr<ClientContext>& GetContext() { return Context; }

	protected:
		struct FHeader
		{
			std::string Key;
			std::string Value;
		};

		std::unique_ptr<typename TService::Stub>& CreateStub()
		{
			Stub = TService::NewStub(grpc::CreateChannel(TCHAR_TO_UTF8(*ServerUrl), grpc::SslCredentials(grpc::SslCredentialsOptions())));
			return Stub;
		}

		std::unique_ptr<ClientContext>& UpdateContext(const TArray<FHeader>& Headers)
		{
			Context = std::make_unique<ClientContext>();
			for (auto& Header : Headers)
			{
				Context->AddMetadata(Header.Key, Header.Value);
			}
			return Context;
		}
		
		std::unique_ptr<typename TService::Stub> Stub;
		grpc::Status Status;
		TResponse Response;

	private:
		FString ServerUrl;
		std::unique_ptr<ClientContext> Context;

		TFunction<void(const grpc::Status& Status, const TResponse& Response)> Callback;
	};

	class FRunnableLoadScene : public FRunnableRequest<InworldEngine::WorldEngine, InworldEngine::LoadSceneResponse>
	{
	public:
        FRunnableLoadScene(const FString& InAuthUrl, const FString& InLoadSceneUrl, const FString& InSceneName, const FString& InApiKey, const FString& InApiSecret, const FString& InPlayerName, const FSentryTransactionHeader& InSentryHeader, TFunction<void(const grpc::Status& Status, const InworldEngine::LoadSceneResponse& Response)> InCallback = nullptr)
			: FRunnableRequest(InLoadSceneUrl, InCallback)
			, AuthUrl(InAuthUrl)
			, SceneName(InSceneName)
			, ApiKey(InApiKey)
			, ApiSecret(InApiSecret)
			, PlayerName(InPlayerName)
			, SentryHeader(InSentryHeader)
		{}

		virtual grpc::Status RunProcess() override;
		
        std::unique_ptr<FReaderWriter> Session();

	private:
		std::string GenerateHeader() const;
		std::string GenerateUserId() const;

		FString AuthUrl;
		FString LoadSceneUrl;
		FString SceneName;
		FString ApiKey;
		FString ApiSecret;
		FString PlayerName;

		FSentryTransactionHeader SentryHeader;

		InworldV1alpha::SessionAccessToken Token;
	};

	struct FAgentInfo
	{
		FString BrainName;
		FName AgentId;
		FString GivenName;
	};

	struct FClientOptions
	{
		FString AuthUrl;
		FString LoadSceneUrl;
		FString SceneName;
		FString ApiKey;
		FString ApiSecret;
		FString PlayerName;
		FString SentryDSN;
		FString SentryTransactionName;
		FString SentryTransactionOperation;
	};

	class INWORLDAICLIENT_API FClient
	{
	public:
		void SendPacket(TSharedPtr<Inworld::FInworldPacket> Packet);
		TSharedRef<FTextEvent> SendTextMessage(const FName& AgentId, const FString& Text);
		TSharedRef<FDataEvent> SendSoundMessage(const FName& AgentId, const std::string& Data);
		TSharedRef<FCustomEvent> SendCustomEvent(FName AgentId, const FString& Name);

		void CancelResponse(const FName& AgentId, const FName& InteractionId, const TArray<FName>& UtteranceIds);

		void StartAudioSession(const FName& AgentId);
		void StopAudioSession(const FName& AgentId);

		void InitClient(TFunction<void(EInworldConnectionState ConnectionState)> ConnectionStateCallback, TFunction<void(TSharedPtr<Inworld::FInworldPacket>)> PacketCallback);
		void StartClient(const FClientOptions& Options, TFunction<void(const TArray<Inworld::FAgentInfo>&)> LoadSceneCallback);
		void PauseClient();
		void ResumeClient();
		void StopClient();
		void DestroyClient();

		EInworldConnectionState GetConnectionState() const { return ConnectionState; }
		bool GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const;

		void NotifyVoiceStarted();
		void NotifyVoiceStopped();

	private:
		void OnSceneLoaded(const grpc::Status& Status, const InworldEngine::LoadSceneResponse& Response);

		void SetConnectionState(EInworldConnectionState State);

		void StartReaderWriter();
		void StopReaderWriter();
		void TryToStartReadTask();
		void TryToStartWriteTask();

		FSentryTransaction SentryTransaction;

		TFunction<void(const TArray<Inworld::FAgentInfo>&)> OnLoadSceneCallback;
		TFunction<void(EInworldConnectionState ConnectionState)> OnConnectionStateChangedCallback;
		TFunction<void(TSharedPtr<Inworld::FInworldPacket>)> OnPacketCallback;

		std::unique_ptr<FReaderWriter> ReaderWriter;
		TAtomic<bool> bHasReaderWriterFinished = false;

		FAsyncRoutine AsyncReadTask;
        FAsyncRoutine AsyncWriteTask;
        FAsyncRoutine AsyncLoadSceneTask; 
		FAsyncRoutine AsyncAudioDumper;

		TQueue<TSharedPtr<FInworldPacket>> IncomingPackets;
		TQueue<TSharedPtr<FInworldPacket>> OutgoingPackets;

		TAtomic<bool> bPendingIncomingPacketFlush = false;

		TQueue<std::string> AudioChunksToDump;

		EInworldConnectionState ConnectionState = EInworldConnectionState::Idle;
		FString ErrorMessage = FString();
		int32 ErrorCode = grpc::StatusCode::OK;
	};

	// TODO(Artem): move to another h file
#if WITH_EDITOR
	class FRunnableGenerateUserTokenRequest : public FRunnableRequest<InworldV1alpha::Users, InworldV1alpha::GenerateTokenUserResponse>
	{
	public:
		FRunnableGenerateUserTokenRequest(const std::string& InFirebaseToken, const FString& InServerUrl, TFunction<void(const grpc::Status& Status, const InworldV1alpha::GenerateTokenUserResponse& Response)> InCallback)
			: FRunnableRequest(InServerUrl, InCallback)
			, FirebaseToken(InFirebaseToken)
		{}

		virtual grpc::Status RunProcess() override;

	private:
		std::string FirebaseToken;
	};

	class FRunnableListWorkspacesRequest : public FRunnableRequest<InworldV1alpha::Workspaces, InworldV1alpha::ListWorkspacesResponse>
	{
	public:
		FRunnableListWorkspacesRequest(const std::string& InInworldToken, const FString& InServerUrl, TFunction<void(const grpc::Status& Status, const InworldV1alpha::ListWorkspacesResponse& Response)> InCallback)
			: FRunnableRequest(InServerUrl, InCallback)
			, InworldToken(InInworldToken)
		{}

		virtual grpc::Status RunProcess() override;

	private:
		std::string InworldToken;
	};

	class FRunnableListScenesRequest : public FRunnableRequest<InworldV1alpha::Scenes, InworldV1alpha::ListScenesResponse>
	{
	public:
		FRunnableListScenesRequest(const std::string& InInworldToken, const FString& InServerUrl, const std::string& InWorkspace, TFunction<void(const grpc::Status& Status, const InworldV1alpha::ListScenesResponse& Response)> InCallback)
			: FRunnableRequest(InServerUrl, InCallback)
			, InworldToken(InInworldToken)
			, Workspace(InWorkspace)
		{}

		virtual grpc::Status RunProcess() override;

	private:
		std::string InworldToken;
		std::string Workspace;
	};

	class FRunnableListCharactersRequest : public FRunnableRequest<InworldV1alpha::Characters, InworldV1alpha::ListCharactersResponse>
	{
	public:
		FRunnableListCharactersRequest(const std::string& InInworldToken, const FString& InServerUrl, const std::string& InWorkspace, TFunction<void(const grpc::Status& Status, const InworldV1alpha::ListCharactersResponse& Response)> InCallback)
			: FRunnableRequest(InServerUrl, InCallback)
			, InworldToken(InInworldToken)
			, Workspace(InWorkspace)
		{}

		virtual grpc::Status RunProcess() override;

	private:
		std::string InworldToken;
		std::string Workspace;
	};

	class FRunnableListApiKeysRequest : public FRunnableRequest<InworldV1alpha::ApiKeys, InworldV1alpha::ListApiKeysResponse>
	{
	public:
		FRunnableListApiKeysRequest(const std::string& InInworldToken, const FString& InServerUrl, const std::string& InWorkspace, TFunction<void(const grpc::Status& Status, const InworldV1alpha::ListApiKeysResponse& Response)> InCallback)
			: FRunnableRequest(InServerUrl, InCallback)
			, InworldToken(InInworldToken)
			, Workspace(InWorkspace)
		{}

		virtual grpc::Status RunProcess() override;

	private:
		std::string InworldToken;
		std::string Workspace;
	};

	class FHttpRequest
	{
	public:
		FHttpRequest(const FString& InURL, const FString& InVerb, const FString& InContent, TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> InCallback)
			: URL(InURL)
			, Verb(InVerb)
			, Content(InContent)
			, Callback(InCallback)
		{
			Process();
		}

		void Cancel();
		bool IsDone() const;

	private:
		void Process();
		void CallCallback(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

		FHttpRequestPtr Request;
		FString URL; 
		FString Verb;
		FString Content;
		TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> Callback;
		bool bCallbackCalled = false;
		bool bCanceled = false;
	};

	struct FEditorClientOptions
	{
		FString ServerUrl;
		FString ExchangeToken;
	};

	class INWORLDAICLIENT_API FEditorClient
	{
	public:
		void RequestUserData(const FEditorClientOptions& Options, TFunction<void(const FInworldStudioUserData& UserData, bool IsError)> InCallback);
		void CancelRequests();

		bool IsUserDataReady() const { return UserData.Workspaces.Num() != 0; }
		bool IsRequestInProgress() const { return Requests.Num() != 0 || HttpRequests.Num() != 0; }
		const FInworldStudioUserData& GetUserData() const { return UserData; }

		void Tick(float DeltaTime);

		void RequestReadyPlayerMeModelData(const FInworldStudioUserCharacterData& CharacterData, TFunction<void(const TArray<uint8>& Data)> InCallback);

		bool GetActiveApiKey(FInworldStudioUserWorkspaceData& WorkspaceData, FInworldStudioUserApiKeyData& ApiKeyData);

		const FString& GetError() const { return ErrorMessage; }

	private:
		void Request(FInworldRunnable* Runnable, const TCHAR* ThreadName);
		void CheckDoneRequests();

		void HttpRequest(const FString& InURL, const FString& InVerb, const FString& InContent, TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> InCallback);

		void OnFirebaseTokenResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

		void OnUserTokenReady(const InworldV1alpha::GenerateTokenUserResponse& Response);
		void OnWorkspacesReady(const InworldV1alpha::ListWorkspacesResponse& Response);
		void OnApiKeysReady(const InworldV1alpha::ListApiKeysResponse& Response, FInworldStudioUserWorkspaceData& WorkspaceData);
		void OnScenesReady(const InworldV1alpha::ListScenesResponse& Response, FInworldStudioUserWorkspaceData& WorkspaceData);
		void OnCharactersReady(const InworldV1alpha::ListCharactersResponse& Response, FInworldStudioUserWorkspaceData& WorkspaceData);
		
		void OnCharacterModelReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData);
		void OnCharacterImageReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData);
		void OnCharacterPortraitReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData);
		void OnCharacterPostureReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData);

		void Error(FString Message);
		void ClearError();

		FCriticalSection RequestsMutex;
		TArray<FAsyncRoutine> Requests;

		TArray<FHttpRequest> HttpRequests;

		FInworldStudioUserData UserData;

		std::string InworldToken;
		
		TFunction<void(const FInworldStudioUserData& UserData, bool IsError)> UserDataCallback;
		TFunction<void(const TArray<uint8>& Data)> RPMActorCreateCallback;
		FString ServerUrl;

		FString ErrorMessage;
	};
#endif
}
