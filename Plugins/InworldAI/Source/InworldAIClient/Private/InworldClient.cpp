/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldClient.h"
#include "proto/ProtoDisableWarning.h"
#include "Packets.h"
#include "InworldUtils.h"

#include <grpcpp/grpcpp.h>

#include "SocketSubsystem.h"
#include "IPAddress.h"

#include "JsonObjectConverter.h"
#include "Interfaces/IPluginManager.h"

static TAutoConsoleVariable<bool> CVarEnableMicSoundDump(
	TEXT("Inworld.Debug.EnableSoundDump"),
	false,
	TEXT(""));

static TAutoConsoleVariable<FString> CVarSoundDumpPath(
	TEXT("Inworld.Debug.SoundDumpPath"),
    TEXT("C:/Tmp/AudioDump.wav"),
	TEXT(""));

TSharedRef<Inworld::FTextEvent> Inworld::FClient::SendTextMessage(const FName& AgentId, const FString & Text)
{
    auto Packet = MakeShared<Inworld::FTextEvent>(Text, Inworld::FRouting::Player2Agent(AgentId));
    SendPacket(Packet);
    return Packet;
}

TSharedRef<Inworld::FDataEvent> Inworld::FClient::SendSoundMessage(const FName& AgentId, const std::string& Data)
{
    auto Packet = MakeShared<Inworld::FDataEvent>(Data, Inworld::FRouting::Player2Agent(AgentId));
    SendPacket(Packet);

    if (CVarEnableMicSoundDump.GetValueOnGameThread())
    {
        AudioChunksToDump.Enqueue(Data);
    }
    return Packet;
}

TSharedRef<Inworld::FCustomEvent> Inworld::FClient::SendCustomEvent(FName AgentId, const FString& Name)
{
	auto Packet = MakeShared<Inworld::FCustomEvent>(Name, Inworld::FRouting::Player2Agent(AgentId));
	SendPacket(Packet);
    return Packet;
}

void Inworld::FClient::CancelResponse(const FName& AgentId, const FName& InteractionId, const TArray<FName>& UtteranceIds)
{
    auto Packet = MakeShared<Inworld::FCancelResponseEvent>(InteractionId, UtteranceIds, Inworld::FRouting(Inworld::FActor(), Inworld::FActor(ai::inworld::packets::Actor_Type_AGENT, AgentId)));
    SendPacket(Packet);
}

void Inworld::FClient::StartAudioSession(const FName& AgentId)
{
    auto DataPacket = MakeShared<Inworld::FControlEvent>(ai::inworld::packets::ControlEvent_Action_AUDIO_SESSION_START, Inworld::FRouting::Player2Agent(AgentId));
    SendPacket(DataPacket);
}

void Inworld::FClient::StopAudioSession(const FName& AgentId)
{
    auto DataPacket = MakeShared<Inworld::FControlEvent>(ai::inworld::packets::ControlEvent_Action_AUDIO_SESSION_END, Inworld::FRouting::Player2Agent(AgentId));
    SendPacket(DataPacket);
}

void Inworld::FClient::SendPacket(TSharedPtr<Inworld::FInworldPacket> Packet)
{
    OutgoingPackets.Enqueue(Packet);

    TryToStartWriteTask();
}

void Inworld::FClient::InitClient(TFunction<void(EInworldConnectionState ConnectionState)> ConnectionStateCallback, TFunction<void(TSharedPtr<Inworld::FInworldPacket>)> PacketCallback)
{
    Inworld::Utils::PrepareSslCreds();

    OnConnectionStateChangedCallback = ConnectionStateCallback;
    OnPacketCallback = PacketCallback;

    if (CVarEnableMicSoundDump.GetValueOnGameThread())
    {
        AsyncAudioDumper.Start(new FRunnableAudioDumper(AudioChunksToDump, CVarSoundDumpPath.GetValueOnGameThread()), TEXT("InworldAudioDumper"));
    }

    SetConnectionState(EInworldConnectionState::Idle);
}

void Inworld::FClient::StartClient(const FClientOptions& Options, TFunction<void(const TArray<Inworld::FAgentInfo>&)> LoadSceneCallback)
{
    if (ConnectionState != EInworldConnectionState::Idle && ConnectionState != EInworldConnectionState::Failed)
    {
        return;
    }

    OnLoadSceneCallback = LoadSceneCallback;

    SetConnectionState(EInworldConnectionState::Connecting);

    SentryTransaction.Init(Options.SentryDSN, Options.SentryTransactionName, Options.SentryTransactionOperation);

    AsyncLoadSceneTask.Start(new FRunnableLoadScene(Options.AuthUrl, Options.LoadSceneUrl, Options.SceneName, Options.ApiKey, Options.ApiSecret, Options.PlayerName, SentryTransaction.GetHeader(),
        [&](const grpc::Status& Status, const InworldEngine::LoadSceneResponse& Response) {
            AsyncTask(ENamedThreads::GameThread, [&]() {
                OnSceneLoaded(Status, Response);
            });
    }), TEXT("InworldLoadScene"));
}

void Inworld::FClient::PauseClient()
{
    if (ConnectionState != EInworldConnectionState::Connected)
    {
        return;
    }

    StopReaderWriter();

    SetConnectionState(EInworldConnectionState::Paused);
}

void Inworld::FClient::ResumeClient()
{
    if (ConnectionState != EInworldConnectionState::Disconnected && ConnectionState != EInworldConnectionState::Paused)
    {
        return;
    }

    SetConnectionState(EInworldConnectionState::Reconnecting);

    StartReaderWriter();
}

void Inworld::FClient::StopClient()
{
    if (ConnectionState == EInworldConnectionState::Idle)
    {
        return;
    }

    SentryTransaction.Destroy();
    StopReaderWriter();
    AsyncLoadSceneTask.Stop();
    SetConnectionState(EInworldConnectionState::Idle);
}

void Inworld::FClient::DestroyClient()
{
    SentryTransaction.Destroy();
    StopReaderWriter();
    AsyncLoadSceneTask.Stop();
    AsyncAudioDumper.Stop();
}

bool Inworld::FClient::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const
{
    OutErrorMessage = ErrorMessage;
    OutErrorCode = ErrorCode;
    return ErrorCode != grpc::StatusCode::OK;
}

void Inworld::FClient::NotifyVoiceStarted()
{
    SentryTransaction.Start("ClientVoice", "Voice started");
}

void Inworld::FClient::NotifyVoiceStopped()
{
    SentryTransaction.Stop();
}

void Inworld::FClient::SetConnectionState(EInworldConnectionState State)
{
    if (ConnectionState == State)
    {
        return;
    }

    ConnectionState = State;

    if (ConnectionState == EInworldConnectionState::Connected || ConnectionState == EInworldConnectionState::Idle)
    {
        ErrorMessage = FString();
        ErrorCode = grpc::StatusCode::OK;
    }

    if (ensure(OnConnectionStateChangedCallback))
    {
        OnConnectionStateChangedCallback(ConnectionState);
    }
}

void Inworld::FClient::OnSceneLoaded(const grpc::Status& Status, const InworldEngine::LoadSceneResponse& Response)
{
    if (!ensure(OnLoadSceneCallback))
    {
        return;
    }

    if (!Status.ok())
    {
        ErrorMessage = FString(Status.error_message().c_str());
        ErrorCode = Status.error_code();
        Inworld::Utils::ErrorLog(FString::Printf(TEXT("Load scene FALURE! %s, Code: %d"), *ErrorMessage, ErrorCode), true);
        SetConnectionState(EInworldConnectionState::Failed);
        return;
    }

    TArray<Inworld::FAgentInfo> AgentInfos;
    for (int32 i = 0; i < Response.agents_size(); i++)
    {
        auto& Info = AgentInfos.Emplace_GetRef();
        Info.BrainName = UTF8_TO_TCHAR(Response.agents(i).brain_name().c_str());
        Info.AgentId = UTF8_TO_TCHAR(Response.agents(i).agent_id().c_str());
        Info.GivenName = UTF8_TO_TCHAR(Response.agents(i).given_name().c_str());

        Inworld::Utils::Log(FString::Printf(TEXT("Character registered: %s, Id: %s, GivenName: %s"), *Info.BrainName, *Info.AgentId.ToString(), *Info.GivenName), true, Inworld::Utils::FTextColors::Message());
    }

    auto& Info = AgentInfos.Emplace_GetRef();
    Info.BrainName = "__DUMMY__";
    Info.AgentId = "__DUMMY__";
	Info.GivenName = "__DUMMY__";

	OnLoadSceneCallback(AgentInfos);
	OnLoadSceneCallback = nullptr;

    SetConnectionState(EInworldConnectionState::Connected);
    StartReaderWriter();

    Inworld::Utils::Log(TEXT("Create world SUCCESS!"), true, Inworld::Utils::FTextColors::Message());
}

void Inworld::FClient::StartReaderWriter()
{
    const bool bHasPendingWriteTask = AsyncWriteTask.IsValid() && !AsyncWriteTask.IsDone();
    const bool bHasPendingReadTask = AsyncReadTask.IsValid() && !AsyncReadTask.IsDone();
    if (!bHasPendingWriteTask && !bHasPendingReadTask)
    {
        ErrorMessage = FString();
        ErrorCode = grpc::StatusCode::OK;
        ReaderWriter = AsyncLoadSceneTask.GetTask<FRunnableLoadScene>()->Session();
        bHasReaderWriterFinished = false;
        TryToStartReadTask();
        TryToStartWriteTask();
    }
}

void Inworld::FClient::StopReaderWriter()
{
    bHasReaderWriterFinished = true;
    auto* Task = AsyncLoadSceneTask.GetTask<FRunnableLoadScene>();
    if (Task)
    {
        auto& Context = Task->GetContext();
        if (Context)
        {
            Context->TryCancel();
        }
    }
    AsyncReadTask.Stop();
    AsyncWriteTask.Stop();
    ReaderWriter.reset();
}

void Inworld::FClient::TryToStartReadTask()
{
    if (!ReaderWriter)
    {
        return;
    }

    const bool bHasPendingReadTask = AsyncReadTask.IsValid() && !AsyncReadTask.IsDone();
    if (!bHasPendingReadTask)
    {
        AsyncReadTask.Start(new FRunnableRead(*ReaderWriter.get(), bHasReaderWriterFinished, IncomingPackets,
            [this](const TSharedPtr<Inworld::FInworldPacket> InPacket)
            {
                if (!bPendingIncomingPacketFlush)
                {
                    bPendingIncomingPacketFlush = true;
                    AsyncTask(ENamedThreads::GameThread,
                    [this]()
                    {
                        TSharedPtr<Inworld::FInworldPacket> Packet;
                        while (IncomingPackets.Dequeue(Packet))
                        {
                            if (Packet)
                            {
                                OnPacketCallback(Packet);
                            }
                        }
                        bPendingIncomingPacketFlush = false;
                    });
                }
                if (ConnectionState != EInworldConnectionState::Connected)
                {
                    AsyncTask(ENamedThreads::GameThread,
                    [this]()
                    {
                        SetConnectionState(EInworldConnectionState::Connected);
                    });
                }
            },
            [this](const grpc::Status& Status)
            {
                ErrorMessage = FString(Status.error_message().c_str());
                ErrorCode = Status.error_code();
                Inworld::Utils::ErrorLog(FString::Printf(TEXT("Message READ failed: %s. Code: %d"), *ErrorMessage, ErrorCode), true);
                AsyncTask(ENamedThreads::GameThread,
                [this]()
                {
                    SetConnectionState(EInworldConnectionState::Disconnected);
                });
            }), TEXT("InworldRead"));
    }
}

void Inworld::FClient::TryToStartWriteTask()
{
    if (!ReaderWriter)
    {
        return;
    }

    const bool bHasPendingWriteTask = AsyncWriteTask.IsValid() && !AsyncWriteTask.IsDone();
    if (!bHasPendingWriteTask)
    {
        const bool bHasOutgoingPackets = !OutgoingPackets.IsEmpty();
        if (bHasOutgoingPackets)
        {
            AsyncWriteTask.Start(new FRunnableWrite(*ReaderWriter.get(), bHasReaderWriterFinished, OutgoingPackets,
                [this](const TSharedPtr<Inworld::FInworldPacket> InPacket)
                {
                    if (ConnectionState != EInworldConnectionState::Connected)
                    {
                        AsyncTask(ENamedThreads::GameThread, [this]() {
                            SetConnectionState(EInworldConnectionState::Connected);
                        });
                    }
                },
                [this](const grpc::Status& Status)
                {
                    ErrorMessage = FString(Status.error_message().c_str());
                    ErrorCode = Status.error_code();
                    Inworld::Utils::ErrorLog(FString::Printf(TEXT("Message WRITE failed: %s. Code: %d"), *ErrorMessage, ErrorCode), true);
                    AsyncTask(ENamedThreads::GameThread, [this]() {
                        SetConnectionState(EInworldConnectionState::Disconnected);
                    });
                }), TEXT("InworldWrite"));
        }
    }
}

uint32 Inworld::FRunnableRead::Run()
{
    while (!bHasReaderWriterFinished)
    {
        InworldPackets::InworldPacket IncomingPacket;
        if (!ReaderWriter.Read(&IncomingPacket))
        {
            if (!bHasReaderWriterFinished)
            {
                bHasReaderWriterFinished = true;
                ErrorCallback(ReaderWriter.Finish());
            }

            bDone = true;

            return 0;
        }

        TSharedPtr<Inworld::FInworldPacket> Packet;
        // Text event
        if (IncomingPacket.has_text())
        {
            Packet = MakeShared<Inworld::FTextEvent>(IncomingPacket);
        }
        else if (IncomingPacket.has_data_chunk())
        {
            // Audio response with Uncompressed 16-bit signed little-endian samples (Linear PCM) data.
            if (IncomingPacket.data_chunk().type() == ai::inworld::packets::DataChunk_DataType_AUDIO)
            {
                Packet = MakeShared<Inworld::FDataEvent>(IncomingPacket);
            }
        }
        else if (IncomingPacket.has_control())
        {
            Packet = MakeShared<Inworld::FControlEvent>(IncomingPacket);
        }
        else if (IncomingPacket.has_emotion())
        {
            Packet = MakeShared<Inworld::FEmotionEvent>(IncomingPacket);
        }
        else if (IncomingPacket.has_gesture())
        {
            Packet = MakeShared<Inworld::FSimpleGestureEvent>(IncomingPacket);
        }
        else if (IncomingPacket.has_custom())
        {
            Packet = MakeShared<Inworld::FCustomEvent>(IncomingPacket);
        }

        Packets.Enqueue(Packet);

        ProcessedCallback(Packet);
    }

    bDone = true;

    return 0;
}

uint32 Inworld::FRunnableWrite::Run()
{
    TSharedPtr<Inworld::FInworldPacket> Packet;
    while (!bHasReaderWriterFinished && Packets.Peek(Packet))
    {
        InworldPackets::InworldPacket Event = Packet->ToProto();
        if (!ReaderWriter.Write(Event))
        {
            if (!bHasReaderWriterFinished)
            {
                bHasReaderWriterFinished = true;
                ErrorCallback(ReaderWriter.Finish());
            }

            bDone = true;

            return 0;
        }

        Packets.Dequeue(Packet);

        ProcessedCallback(Packet);
    }

    bDone = true;

    return 0;
}

grpc::Status Inworld::FRunnableLoadScene::RunProcess()
{
    InworldV1alpha::GenerateSessionTokenRequest AuthRequest;
    AuthRequest.set_key(TCHAR_TO_UTF8(*ApiKey));

    auto& AuthCtx = UpdateContext({ {"authorization", GenerateHeader() } });

    auto AuthStub = InworldV1alpha::Tokens::NewStub(grpc::CreateChannel(TCHAR_TO_UTF8(*AuthUrl), grpc::SslCredentials(grpc::SslCredentialsOptions())));
    
    Status = AuthStub->GenerateSessionToken(AuthCtx.get(), AuthRequest, &Token);
    if (!Status.ok())
    {
		return Status;
    }
	
    InworldEngine::LoadSceneRequest LoadSceneRequest;
	LoadSceneRequest.set_name(TCHAR_TO_UTF8(*SceneName));

	auto* Capabilities = LoadSceneRequest.mutable_capabilities();
	Capabilities->set_animations(false);
	Capabilities->set_text(true);
	Capabilities->set_audio(true);
	Capabilities->set_emotions(true);
	Capabilities->set_gestures(true);
	Capabilities->set_interruptions(true);
	Capabilities->set_triggers(true);
    Capabilities->set_emotion_streaming(true);
    Capabilities->set_silence_events(true);

	auto* User = LoadSceneRequest.mutable_user();
	User->set_id(GenerateUserId());
	User->set_name(TCHAR_TO_UTF8(*PlayerName));

    auto* Client = LoadSceneRequest.mutable_client();
    Client->set_id("unreal");
    TSharedPtr<IPlugin> InworldAIPlugin = IPluginManager::Get().FindPlugin("InworldAI");
    if (ensure(InworldAIPlugin.IsValid()))
    {
        Client->set_version(TCHAR_TO_UTF8(*InworldAIPlugin.Get()->GetDescriptor().VersionName));
    }

	auto& Ctx = UpdateContext({
		{ "authorization", std::string("Bearer ") + Token.token() },
		{ "session-id", Token.session_id() }
		});

	return CreateStub()->LoadScene(Ctx.get(), LoadSceneRequest, &Response);
}

std::unique_ptr<FReaderWriter> Inworld::FRunnableLoadScene::Session()
{
	auto& Ctx = UpdateContext({
			{ "authorization", std::string("Bearer ") + Token.token() },
			{ "session-id", Token.session_id() }
		});
    if (SentryHeader.Key.size() && SentryHeader.Value.size())
    {
        Ctx->AddMetadata(SentryHeader.Key, SentryHeader.Value);
    }
	return Stub->Session(Ctx.get());
}

std::string Inworld::FRunnableLoadScene::GenerateHeader() const
{
	const FString CurrentTime = FDateTime::UtcNow().ToString().Replace(TEXT("."), TEXT("")).Replace(TEXT("-"), TEXT(""));

	const FString Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
	FString Nonce = "00000000000";
	for (int32 i = 0; i < Nonce.Len(); i++)
	{
		Nonce[i] = Chars[FMath::RandRange(0, Chars.Len() - 1)];
	}

	TArray<std::string> CryptoArgs = {
		TCHAR_TO_UTF8(*CurrentTime),
        TCHAR_TO_UTF8(*AuthUrl),
		"ai.inworld.studio.v1alpha.Tokens/GenerateSessionToken",
		TCHAR_TO_UTF8(*Nonce),
		"iw1_request"
	};

	const std::string IniKey = std::string("IW1") + TCHAR_TO_UTF8(*ApiSecret);
    TArray<uint8> Key;
    Key.SetNumZeroed(IniKey.size());
    FMemory::Memcpy(Key.GetData(), IniKey.data(), IniKey.size());
    
    for (const auto& Arg : CryptoArgs)
    {
		TArray<uint8> Data;
        Data.SetNumZeroed(Arg.size());
		FMemory::Memcpy(Data.GetData(), Arg.data(), Arg.size());
        Key = Inworld::Utils::HmacSha256(Data, Key);
    }

    const std::string Signature = Inworld::Utils::ToHex(Key);

	const std::string ApiKeyStr = TCHAR_TO_UTF8(*ApiKey);
	return std::string("IW1-HMAC-SHA256 ApiKey=") + ApiKeyStr + ",DateTime=" + CryptoArgs[0] +
        ",Nonce=" + CryptoArgs[3] + ",Signature=" + Signature;
}

std::string Inworld::FRunnableLoadScene::GenerateUserId() const
{
    bool CanBindAll;
	TSharedRef<FInternetAddr> LocalIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, CanBindAll);
    if (!LocalIp->IsValid())
    {
        Inworld::Utils::ErrorLog("Couldn't generate user id, local ip isn't valid", true);
        return "";
    }

    const std::string LocalIpStr = TCHAR_TO_UTF8(*LocalIp->ToString(false));

	TArray<uint8> Data;
    Data.SetNumZeroed(LocalIpStr.size());
	FMemory::Memcpy(Data.GetData(), LocalIpStr.data(), LocalIpStr.size());

    Data = Inworld::Utils::HmacSha256(Data, Data);

    return Utils::ToHex(Data);
}

void Inworld::FAsyncRoutine::Start(FInworldRunnable* InRunnable, const TCHAR* ThreadName)
{
    Stop();
    Runnable = InRunnable;
    Thread = FRunnableThread::Create(InRunnable, ThreadName);
}

#if WITH_EDITOR
grpc::Status Inworld::FRunnableGenerateUserTokenRequest::RunProcess()
{
    InworldV1alpha::GenerateTokenUserRequest Request;
    Request.set_type(InworldV1alpha::AuthType::AUTH_TYPE_FIREBASE);
    Request.set_token(FirebaseToken);

    auto& Ctx = UpdateContext({
        {"x-authorization-bearer-type", "firebase"},
        {"authorization", std::string("Bearer ") + FirebaseToken }
    });

    return CreateStub()->GenerateTokenUser(Ctx.get(), Request, &Response);
}

grpc::Status Inworld::FRunnableListWorkspacesRequest::RunProcess()
{
	InworldV1alpha::ListWorkspacesRequest Request;

	auto& Ctx = UpdateContext({
		{"x-authorization-bearer-type", "inworld"},
		{"authorization", std::string("Bearer ") + InworldToken }
		});

    return CreateStub()->ListWorkspaces(Ctx.get(), Request, &Response);
}

grpc::Status Inworld::FRunnableListScenesRequest::RunProcess()
{
	InworldV1alpha::ListScenesRequest Request;
    Request.set_parent(Workspace);

	auto& Ctx = UpdateContext({
		{"x-authorization-bearer-type", "inworld"},
		{"authorization", std::string("Bearer ") + InworldToken }
		});

    return CreateStub()->ListScenes(Ctx.get(), Request, &Response);
}

grpc::Status Inworld::FRunnableListCharactersRequest::RunProcess()
{
	InworldV1alpha::ListCharactersRequest Request;
	Request.set_parent(Workspace);
    Request.set_view(InworldV1alpha::CharacterView::CHARACTER_VIEW_DEFAULT);

	auto& Ctx = UpdateContext({
		{"x-authorization-bearer-type", "inworld"},
		{"authorization", std::string("Bearer ") + InworldToken }
		});

    return CreateStub()->ListCharacters(Ctx.get(), Request, &Response);
}

grpc::Status Inworld::FRunnableListApiKeysRequest::RunProcess()
{
	InworldV1alpha::ListApiKeysRequest Request;
	Request.set_parent(Workspace);

	auto& Ctx = UpdateContext({
		{"x-authorization-bearer-type", "inworld"},
		{"authorization", std::string("Bearer ") + InworldToken }
		});

    return CreateStub()->ListApiKeys(Ctx.get(), Request, &Response);
}

void Inworld::FEditorClient::RequestUserData(const FEditorClientOptions& Options, TFunction<void(const FInworldStudioUserData& UserData, bool IsError)> InCallback)
{
	Inworld::Utils::PrepareSslCreds();

	ClearError();

    FString IdToken;
    FString RefreshToken;
    if (!Options.ExchangeToken.Split(":", &IdToken, &RefreshToken))
    {
        Error(FString::Printf(TEXT("FEditorClient::RequestUserData FALURE! Invalid Refresh Token.")));
        return;
    }

    UserData.Workspaces.Empty();

	UserDataCallback = InCallback;
    ServerUrl = Options.ServerUrl;

    FRefreshTokenRequestData RequestData;
    RequestData.grant_type = "refresh_token";
    RequestData.refresh_token = RefreshToken;
    FString JsonString;
    FJsonObjectConverter::UStructToJsonObjectString(RequestData, JsonString);

    HttpRequest("https://securetoken.googleapis.com/v1/token?key=AIzaSyAPVBLVid0xPwjuU4Gmn_6_GyqxBq-SwQs", "POST", JsonString,
        [this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) 
        { 
            OnFirebaseTokenResponse(Request, Response, bSuccess); 
        });
}

void Inworld::FEditorClient::CancelRequests()
{
	for (auto& Request : HttpRequests)
	{
		Request.Cancel();
	}

	HttpRequests.Empty();

    {
        FScopeLock Lock(&RequestsMutex);

        for (auto& Request : Requests)
        {
            Request.Stop();
        }

        Requests.Empty();
    }

    UserDataCallback = nullptr;
}

void Inworld::FEditorClient::CheckDoneRequests()
{
	if (Requests.Num() == 0 && HttpRequests.Num() == 0)
	{
		if (UserDataCallback)
		{
			UserDataCallback(UserData, !ErrorMessage.IsEmpty());
			UserDataCallback = nullptr;
		}
        return;
	}

    {
        FScopeLock Lock(&RequestsMutex);

        for (int32 i = 0; i < Requests.Num();)
        {
            auto& Request = Requests[i];
            if (Request.IsDone())
            {
                Requests.RemoveAt(i, 1, false);
            }
            else
            {
                i++;
            }
        }
	}

	for (int32 i = 0; i < HttpRequests.Num();)
	{
		auto& Request = HttpRequests[i];
		if (Request.IsDone())
		{
            HttpRequests.RemoveAt(i, 1, false);
		}
		else
		{
			i++;
		}
	}
}

void Inworld::FEditorClient::HttpRequest(const FString& InURL, const FString& InVerb, const FString& InContent, TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> InCallback)
{
    HttpRequests.Emplace(InURL, InVerb, InContent, InCallback);
}

void Inworld::FEditorClient::OnFirebaseTokenResponse(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bSuccess)
{
    if (!bSuccess)
    {
		Error(FString::Printf(TEXT("FEditorClient::OnFirebaseTokenResponse FALURE! Code: %d"), InResponse ? InResponse->GetResponseCode() : -1));
		return;
    }

    FRefreshTokenResponseData ResponseData;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(InResponse->GetContentAsString(), &ResponseData))
    {
        Error(FString::Printf(TEXT("FEditorClient::OnFirebaseTokenResponse FALURE! Invalid Response Data.")));
		return;
    }

	Request(new FRunnableGenerateUserTokenRequest(TCHAR_TO_UTF8(*ResponseData.id_token), ServerUrl, [this](const grpc::Status& Status, const InworldV1alpha::GenerateTokenUserResponse& Response)
		{
			if (!Status.ok())
			{
                Error(FString::Printf(TEXT("FRunnableGenerateUserTokenRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
				return;
			}

			OnUserTokenReady(Response);
		}), TEXT("FRunnableGenerateUserTokenRequest"));
}

void Inworld::FEditorClient::Tick(float DeltaTime)
{
    if (!ErrorMessage.IsEmpty())
    {
        CancelRequests();
    }

	CheckDoneRequests();
}

void Inworld::FEditorClient::RequestReadyPlayerMeModelData(const FInworldStudioUserCharacterData& CharacterData, TFunction<void(const TArray<uint8>& Data)> InCallback)
{
    RPMActorCreateCallback = InCallback;

    static const FString UriArgsStr = "?pose=A&meshLod=0&textureAtlas=none&textureSizeLimit=1024&morphTargets=ARKit,Oculus%20Visemes&useHands=true";
    HttpRequest(FString::Printf(TEXT("%s%s"), *CharacterData.RpmModelUri, *UriArgsStr), "GET", FString(), [this, &CharacterData](FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bSuccess)
        {
            if (!bSuccess || InResponse->GetContent().Num() == 0)
			{
                Error(FString::Printf(TEXT("FEditorClient::RequestReadyPlayerMeModelData request FALURE!, Code: %d"), InResponse ? InResponse->GetResponseCode() : -1));
                return;
            }

            if (!ensure(RPMActorCreateCallback))
            {
                return;
            }

            RPMActorCreateCallback(InResponse->GetContent());
            });
}

bool Inworld::FEditorClient::GetActiveApiKey(FInworldStudioUserWorkspaceData& InWorkspaceData, FInworldStudioUserApiKeyData& InApiKeyData)
{
    for (auto& ApiKeyData : InWorkspaceData.ApiKeys)
    {
        if (ApiKeyData.IsActive)
        {
            InApiKeyData = ApiKeyData;
            return true;
        }
    }
    return false;
}

void Inworld::FEditorClient::Request(FInworldRunnable* Runnable, const TCHAR* ThreadName)
{
    FScopeLock Lock(&RequestsMutex);

    auto& AsyncTask = Requests.Emplace_GetRef();
    AsyncTask.Start(Runnable, ThreadName);
}

void Inworld::FEditorClient::OnUserTokenReady(const InworldV1alpha::GenerateTokenUserResponse& Response)
{
    InworldToken = Response.token();

	Request(new FRunnableListWorkspacesRequest(InworldToken, ServerUrl, [this](const grpc::Status& Status, const InworldV1alpha::ListWorkspacesResponse& Response)
		{
			if (!Status.ok())
			{
				Error(FString::Printf(TEXT("FRunnableListWorkspacesRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
				return;
			}

            OnWorkspacesReady(Response);
		}), TEXT("FRunnableListWorkspacesRequest"));
}

static FString CreateShortName(const FString& Name)
{
	int32 Idx;
    FString ShortName = Name;
	if (ShortName.FindLastChar('/', Idx))
	{
        ShortName.RightChopInline(Idx + 1);
	}
    return MoveTemp(ShortName);
}

void Inworld::FEditorClient::OnWorkspacesReady(const InworldV1alpha::ListWorkspacesResponse& Response)
{
    UserData.Workspaces.Reserve(Response.workspaces_size());

    for (int32 i = 0; i < Response.workspaces_size(); i++)
    {
        const auto& GrpcWorkspace = Response.workspaces(i);
        auto& Workspace = UserData.Workspaces.Emplace_GetRef();
        Workspace.Name = UTF8_TO_TCHAR(GrpcWorkspace.name().data());
        Workspace.ShortName = CreateShortName(Workspace.Name);

		Request(new FRunnableListScenesRequest(InworldToken, ServerUrl, GrpcWorkspace.name(), [this, &Workspace](const grpc::Status& Status, const InworldV1alpha::ListScenesResponse& Response)
			{
				if (!Status.ok())
				{
					Error(FString::Printf(TEXT("FRunnableListScenesRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
					return;
				}

                OnScenesReady(Response, Workspace);
			}), TEXT("FRunnableListScenesRequest"));

		Request(new FRunnableListCharactersRequest(InworldToken, ServerUrl, GrpcWorkspace.name(), [this, &Workspace](const grpc::Status& Status, const InworldV1alpha::ListCharactersResponse& Response)
			{
				if (!Status.ok())
				{
                    Error(FString::Printf(TEXT("FRunnableListCharactersRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
					return;
				}

				OnCharactersReady(Response, Workspace);
			}), TEXT("FRunnableListCharactersRequest"));

		Request(new FRunnableListApiKeysRequest(InworldToken, ServerUrl, GrpcWorkspace.name(), [this, &Workspace](const grpc::Status& Status, const InworldV1alpha::ListApiKeysResponse& Response)
			{
				if (!Status.ok())
				{
                    Error(FString::Printf(TEXT("FRunnableListCharactersRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
					return;
				}

                OnApiKeysReady(Response, Workspace);
			}), TEXT("FRunnableListApiKeysRequest"));
    }
}

void Inworld::FEditorClient::OnApiKeysReady(const InworldV1alpha::ListApiKeysResponse& Response, FInworldStudioUserWorkspaceData& Workspace)
{
    Workspace.ApiKeys.Reserve(Response.api_keys_size());

    for (int32 i = 0; i < Response.api_keys_size(); i++)
    {
        const auto& GrpcApiKey = Response.api_keys(i);
        auto& ApiKey = Workspace.ApiKeys.Emplace_GetRef();
        ApiKey.Name = UTF8_TO_TCHAR(GrpcApiKey.name().data());
        ApiKey.Key = UTF8_TO_TCHAR(GrpcApiKey.key().data());
        ApiKey.Secret = UTF8_TO_TCHAR(GrpcApiKey.secret().data());
        ApiKey.IsActive = GrpcApiKey.state() == InworldV1alpha::ApiKey_State_ACTIVE;
    }
}

void Inworld::FEditorClient::OnScenesReady(const InworldV1alpha::ListScenesResponse& Response, FInworldStudioUserWorkspaceData& Workspace)
{
	Workspace.Scenes.Reserve(Response.scenes_size());

	for (int32 i = 0; i < Response.scenes_size(); i++)
	{
		const auto& GrpcScene = Response.scenes(i);
		auto& Scene = Workspace.Scenes.Emplace_GetRef();
		Scene.Name = UTF8_TO_TCHAR(GrpcScene.name().data());
        Scene.ShortName = CreateShortName(Scene.Name);
        Scene.Characters.Reserve(GrpcScene.character_references_size());
        for (int32 j = 0; j < GrpcScene.character_references_size(); j++)
        {
            Scene.Characters.Emplace(UTF8_TO_TCHAR(GrpcScene.character_references(j).character().data()));
        }
	}
}

void Inworld::FEditorClient::OnCharactersReady(const InworldV1alpha::ListCharactersResponse& Response, FInworldStudioUserWorkspaceData& Workspace)
{
	Workspace.Characters.Reserve(Response.characters_size());

	for (int32 i = 0; i < Response.characters_size(); i++)
	{
		const auto& GrpcCharacter = Response.characters(i);
		auto& Character = Workspace.Characters.Emplace_GetRef();
		Character.Name = UTF8_TO_TCHAR(GrpcCharacter.name().data());
        Character.ShortName = CreateShortName(Character.Name);
        Character.RpmModelUri = UTF8_TO_TCHAR(GrpcCharacter.default_character_assets().rpm_model_uri().data());
		Character.RpmImageUri = UTF8_TO_TCHAR(GrpcCharacter.default_character_assets().rpm_image_uri().data());
        Character.RpmPortraitUri = UTF8_TO_TCHAR(GrpcCharacter.default_character_assets().rpm_image_uri_portrait().data());
        Character.RpmPostureUri = UTF8_TO_TCHAR(GrpcCharacter.default_character_assets().rpm_image_uri_posture().data());
        Character.bMale = GrpcCharacter.default_character_description().pronoun() == InworldV1alpha::Character_CharacterDescription_Pronoun_PRONOUN_MALE;
	}
}

void Inworld::FEditorClient::OnCharacterModelReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData)
{

}

void Inworld::FEditorClient::OnCharacterImageReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData)
{

}

void Inworld::FEditorClient::OnCharacterPortraitReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData)
{

}

void Inworld::FEditorClient::OnCharacterPostureReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData)
{

}

void Inworld::FEditorClient::Error(FString Message)
{
    Inworld::Utils::ErrorLog(Message);
    ErrorMessage = Message;
}

void Inworld::FEditorClient::ClearError()
{
	ErrorMessage.Empty();
}

void Inworld::FHttpRequest::Process()
{
    if (Request.IsValid())
    {
        Request->CancelRequest();
    }
    Request.Reset();
	Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(URL);
	Request->SetVerb(Verb);
	if (!Content.IsEmpty())
	{
		Request->SetContentAsString(Content);
		Request->SetHeader("Content-Type", "application/json");
	}
	Request->OnProcessRequestComplete().BindRaw(this, &FHttpRequest::CallCallback);
	Request->SetTimeout(5.f);
	Request->ProcessRequest();
}

void Inworld::FHttpRequest::Cancel()
{
	bCanceled = true;
	if (Request.IsValid())
	{
		Request->CancelRequest();
	}
}

bool Inworld::FHttpRequest::IsDone() const
{
	if (bCanceled || bCallbackCalled)
	{
		return true;
	}

	return false;
}

void Inworld::FHttpRequest::CallCallback(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bSuccess)
{
	if (!bCanceled && ensure(Callback))
	{
		Callback(RequestPtr, ResponsePtr, bSuccess);
		bCallbackCalled = true;
	}
}

#endif

uint32 Inworld::FRunnableAudioDumper::Run()
{
    AudioDumper.OnSessionStart(TCHAR_TO_UTF8(*FileName));

    while (!bDone)
    {
        FPlatformProcess::Sleep(0.1f);

		std::string Chunk;
		while (AudioChuncks.Dequeue(Chunk))
		{
			AudioDumper.OnMessage(Chunk);
		}
    }

    AudioDumper.OnSessionStop();

    return 0;
}
