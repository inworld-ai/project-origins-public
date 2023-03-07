/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"
#include "InworldClient.h"
#include "Subsystems/WorldSubsystem.h"
#include "InworldState.h"
#include "InworldComponentInterface.h"
#include "InworldApi.generated.h"

namespace Inworld
{
	class ICharacterComponent;
	class IPlayerComponent;
}
class USoundWave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionStateChanged, EInworldConnectionState, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomTrigger, FString, Name);

UCLASS(BlueprintType, Config = Engine)
class INWORLDAIINTEGRATION_API UInworldApiSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    UInworldApiSubsystem();

    /**
     * Start InworldAI session
     * call after all Player/Character components have been registered
     * @param SceneName : full inworld studio scene name
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void StartSession(const FString& SceneName, const FString& PlayerName, const FString& ApiKey, const FString& ApiSecret, const FString& AuthUrlOverride, const FString& TargetUrlOverride);

    /**
     * Pause InworldAI session
     * call after StartSession has been called to pause
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void PauseSession();

    /**
     * Resume InworldAI session
     * call after PauseSession has been called to resume
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void ResumeSession();

    /**
     * Stop InworldAI session
     * call after StartSession has been called to stop
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void StopSession();

    /**
     * Register Character component
     * call before StartSession
     */
    void RegisterCharacterComponent(Inworld::ICharacterComponent* Component);
    void UnregisterCharacterComponent(Inworld::ICharacterComponent* Component);

    /**
     * Register Player component
     * call before StartSession
     */
	void RegisterPlayerComponent(Inworld::IPlayerComponent* Component);
	void UnregisterPlayerComponent();

    /** Send text to agent */
	UFUNCTION(BlueprintCallable, Category = "Messages")
    void SendTextMessage(const FName& AgentId, const FString& Text);

    /** Send custom event to agent */
	UFUNCTION(BlueprintCallable, Category = "Messages")
	void SendCustomEvent(FName AgentId, const FString& Name);

    /** Send custom event to player's current target agent */
	UFUNCTION(BlueprintCallable, Category = "Messages")
	void SendCustomEventForTargetAgent(const FString& Name);

    /**
     * Send audio to agent
     * start audio session before sending audio
     * stop audio session after all audio chunks have been sent
     * chunks should be ~100ms
     */
    UFUNCTION(BlueprintCallable, Category = "Messages")
	void SendAudioMessage(const FName& AgentId, USoundWave* SoundWave);
	
    void SendAudioDataMessage(const FName& AgentId, const std::string& Data);
    
    /**
     * Start audio session with agent
     * call before sending audio messages
     */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StartAudioSession(const FName& AgentId);

    /**
     * Stop audio session with agent
     * call after all audio messages have been sent
     */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAudioSession(const FName& AgentId);

    /** Needed for performance measure */
    void NotifyVoiceStarted();
    void NotifyVoiceStopped();

    /** Get current connection state */
    UFUNCTION(BlueprintCallable, Category = "Connection")
	EInworldConnectionState GetConnectionState() const { return Client.GetConnectionState(); }

    /** Get connection error*/
    UFUNCTION(BlueprintPure, Category = "Connection")
    bool GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const { return Client.GetConnectionError(OutErrorMessage, OutErrorCode); }
    
    /** Get all registered character components */
	const TArray<Inworld::ICharacterComponent*>& GetCharacterComponents() const { return CharacterComponentRegistry; }

    /** Cancel agents response in case agent has been interrupted by player */
    UFUNCTION(BlueprintCallable, Category = "Messages")
    void CancelResponse(const FName& AgentId, const FName& InteractionId, const TArray<FName>& UtteranceIds);

    /** 
    * Call on Inworld::FCustomEvent coming to agent
    * custom events meant to be triggered on interaction end (see InworldCharacterComponent)
    */
    void NotifyCustomTrigger(const FString& Name) { OnCustomTrigger.Broadcast(Name); }

    /** Subsystem interface */
    virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
    virtual void Deinitialize();

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers")
    FOnConnectionStateChanged OnConnectionStateChanged;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers")
    FCustomTrigger OnCustomTrigger;

private:
    UPROPERTY(EditAnywhere, config, Category = "Connection")
    FString SentryDSN;

	UPROPERTY(EditAnywhere, config, Category = "Connection")
	FString SentryTransactionName;

	UPROPERTY(EditAnywhere, config, Category = "Connection")
	FString SentryTransactionOperation;

    UPROPERTY(EditAnywhere, config, Category = "Connection")
    float RetryConnectionIntervalTime = 0.25f;

    UPROPERTY(EditAnywhere, config, Category = "Connection")
    float MaxRetryConnectionTime = 5.0f;

    float CurrentRetryConnectionTime = 1.0f;

    FTimerHandle RetryConnectionTimerHandle;

    TMap<FName, Inworld::ICharacterComponent*> CharacterComponentByAgentId;
    TArray<Inworld::ICharacterComponent*> CharacterComponentRegistry;
    TMap<FString, Inworld::FAgentInfo> AgentInfoByBrain;

    Inworld::IPlayerComponent* PlayerComponent;

    Inworld::FClient Client;

    bool bCharactersInitialized = false;
};