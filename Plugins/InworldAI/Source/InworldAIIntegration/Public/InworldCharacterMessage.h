// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "InworldPackets.h"

#include "InworldCharacterMessage.generated.h"

struct FCharacterMessageUtterance;
struct FCharacterMessageSilence;
struct FCharacterMessageTrigger;
struct FCharacterMessageInteractionEnd;

class ICharacterMessageVisitor
{
public:
	virtual void Handle(const FCharacterMessageUtterance& Event) { }
	virtual void Interrupt(const FCharacterMessageUtterance& Event) { }

	virtual void Handle(const FCharacterMessageSilence& Event) { }
	virtual void Interrupt(const FCharacterMessageSilence& Event) { }

	virtual void Handle(const FCharacterMessageTrigger& Event) { }

	virtual void Handle(const FCharacterMessageInteractionEnd& Event) { }
};

USTRUCT(BlueprintType)
struct FCharacterMessage
{
	GENERATED_BODY()

	FCharacterMessage() {}
	virtual ~FCharacterMessage() = default;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString UtteranceId;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString InteractionId;

	virtual bool IsReady() const { return true; }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) PURE_VIRTUAL(FCharacterMessage::AcceptHandle)
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) PURE_VIRTUAL(FCharacterMessage::AcceptInterrupt)
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) PURE_VIRTUAL(FCharacterMessage::AcceptCancel)

	virtual FString ToDebugString() const PURE_VIRTUAL(FCharacterMessage::ToDebugString, return FString();)
};

USTRUCT(BlueprintType)
struct FCharacterUtteranceVisemeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Code;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	float Timestamp = 0.f;
};

USTRUCT(BlueprintType)
struct FCharacterMessageUtterance : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TArray<FCharacterUtteranceVisemeInfo> VisemeInfos;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bTextFinal = false;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bAudioFinal = false;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TArray<uint8> SoundData;

	virtual bool IsReady() const override { return bTextFinal && bAudioFinal; }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { Visitor.Interrupt(*this); }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { }

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Utterance. Text: %s"), *Text); }
};

USTRUCT(BlueprintType)
struct FCharacterMessagePlayerTalk : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bTextFinal = false;

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { }

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("PlayerTalk. Text: %s"), *Text); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageSilence : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	float Duration = 0.f;

	virtual bool IsReady() const override { return Duration != 0.f; }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { Visitor.Interrupt(*this); }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { }

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Silence. Duration: %f"), Duration); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageTrigger : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TMap<FString, FString> Params;

	virtual bool IsReady() const override { return !Name.IsEmpty(); }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Trigger. Name: %s"), *Name); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageInteractionEnd : public FCharacterMessage
{
	GENERATED_BODY()

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }

	virtual FString ToDebugString() const override { return TEXT("InteractionEnd"); }
};

struct FCharacterMessageQueue : public TSharedFromThis<FCharacterMessageQueue>
{
	FCharacterMessageQueue()
		: FCharacterMessageQueue(nullptr)
	{}
	FCharacterMessageQueue(class ICharacterMessageVisitor* InMessageVisitor)
		: MessageVisitor(InMessageVisitor)
	{
	}

	class ICharacterMessageVisitor* MessageVisitor;

	TSharedPtr<FCharacterMessage> CurrentMessage;

	struct FCharacterMessageQueueEntry
	{
		FCharacterMessageQueueEntry(TSharedPtr<FCharacterMessage> InMessage, float InTimestamp)
			: Message(InMessage)
			, Timestamp(InTimestamp)
		{}
		TSharedPtr<FCharacterMessage> Message;
		float Timestamp = 0.f;
	};

	TArray<FCharacterMessageQueueEntry> PendingMessageEntries;


	template<class T>
	void AddOrUpdateMessage(const FInworldPacket& Event, float Timestamp, TFunction<void(TSharedPtr<T> MessageToPopulate)> PopulateProperties = nullptr)
	{
		const FString& InteractionId = Event.PacketId.InteractionId;
		const FString& UtteranceId = Event.PacketId.UtteranceId;

		TSharedPtr<T> Message = nullptr;
		const auto Index = PendingMessageEntries.FindLastByPredicate( [&InteractionId, &UtteranceId](const auto& Q)
			{
				return Q.Message->InteractionId == InteractionId && Q.Message->UtteranceId == UtteranceId;
			}
		);
		if (Index != INDEX_NONE)
		{
			Message = StaticCastSharedPtr<T>(PendingMessageEntries[Index].Message);
		}

		if (!Message.IsValid() || Message->IsReady())
		{
			Message = MakeShared<T>();
			Message->InteractionId = InteractionId;
			Message->UtteranceId = UtteranceId;
			PendingMessageEntries.Emplace(Message, Timestamp);
		}
		if (PopulateProperties)
		{
			PopulateProperties(Message);
		}

		if (CanceledInteractions.Contains(InteractionId))
		{
			auto FilterPredicate = [InteractionId](const FCharacterMessageQueueEntry& MessageQueueEntry)
				{
					return MessageQueueEntry.Message->InteractionId == InteractionId;
				};

			PendingMessageEntries.RemoveAll(FilterPredicate);
		}

		if (Message->StaticStruct()->IsChildOf(FCharacterMessageInteractionEnd::StaticStruct()))
		{
			CanceledInteractions.Remove(InteractionId);
		}

		TryToProgress();
	}

	TArray<FString> CancelInteraction(const FString& InteractionId);
	void TryToProgress(bool bForce = false);
	TOptional<float> GetBlockingTimestamp() const;
	void Clear();

	TArray<FString> CanceledInteractions;

	int32 LockCount = 0;
	TSharedPtr<struct FCharacterMessageQueueLock> MakeLock();
};

struct FCharacterMessageQueueLock
{
	FCharacterMessageQueueLock(TSharedRef<FCharacterMessageQueue> InQueue);
	~FCharacterMessageQueueLock();

	TWeakPtr<FCharacterMessageQueue> QueuePtr;
	TWeakPtr<FCharacterMessage> MessagePtr;
};

USTRUCT(BlueprintType)
struct FInworldCharacterMessageQueueLockHandle
{
	GENERATED_BODY()

	TSharedPtr<FCharacterMessageQueueLock> Lock;
};
