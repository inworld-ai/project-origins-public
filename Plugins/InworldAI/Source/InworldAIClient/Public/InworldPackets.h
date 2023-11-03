// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "InworldEnums.h"
#include "InworldTypes.h"

#include "Serialization/MemoryArchive.h"

#include "InworldPackets.generated.h"

USTRUCT()
struct INWORLDAICLIENT_API FInworldActor
{
	GENERATED_BODY()

	FInworldActor() = default;
	FInworldActor(const FInworldActor& Other) = default;
	FInworldActor(const EInworldActorType& InType, const FString& InName)
		: Type(InType)
		, Name(InName)
	{}

	void Serialize(FMemoryArchive& Ar);

	void AppendDebugString(FString& Str) const;

	UPROPERTY()
	EInworldActorType Type =  EInworldActorType::UNKNOWN;
	UPROPERTY()
	FString Name;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldRouting
{
	GENERATED_BODY()
	
	FInworldRouting() = default;
	FInworldRouting(const FInworldRouting& Other) = default;
	FInworldRouting(const FInworldActor& InSource, const FInworldActor& InTarget)
		: Source(InSource)
		, Target(InTarget)
	{}

	void Serialize(FMemoryArchive& Ar);

	void AppendDebugString(FString& Str) const;

	UPROPERTY()
	FInworldActor Source;
	UPROPERTY()
	FInworldActor Target;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldPacketId
{
	GENERATED_BODY()

	FInworldPacketId() = default;
	FInworldPacketId(const FInworldPacketId& Other) = default;
	FInworldPacketId(const FString& InUID, const FString& InUtteranceId, const FString& InInteractionId)
		: UID(InUID)
		, UtteranceId(InUtteranceId)
		, InteractionId(InInteractionId)
	{}

	void Serialize(FMemoryArchive & Ar);

	void AppendDebugString(FString& Str) const;

	UPROPERTY()
	FString UID;
	UPROPERTY()
	FString UtteranceId;
	UPROPERTY()
	FString InteractionId;
};

struct FInworldTextEvent;
struct FInworldDataEvent;
struct FInworldAudioDataEvent;
struct FInworldSilenceEvent;
struct FInworldControlEvent;
struct FInworldEmotionEvent;
struct FInworldCancelResponseEvent;
struct FInworldSimpleGestureEvent;
struct FInworldCustomGestureEvent;
struct FInworldCustomEvent;
struct FInworldChangeSceneEvent;

class InworldPacketVisitor
{
public:
	virtual void Visit(const FInworldTextEvent& Event) {  }
	virtual void Visit(const FInworldDataEvent& Event) {  }
	virtual void Visit(const FInworldAudioDataEvent& Event) {  }
	virtual void Visit(const FInworldSilenceEvent& Event) {  }
	virtual void Visit(const FInworldControlEvent& Event) {  }
	virtual void Visit(const FInworldEmotionEvent& Event) {  }
	virtual void Visit(const FInworldCancelResponseEvent& Event) {  }
	virtual void Visit(const FInworldSimpleGestureEvent& Event) {  }
	virtual void Visit(const FInworldCustomGestureEvent& Event) {  }
	virtual void Visit(const FInworldCustomEvent& Event) {  }
	virtual void Visit(const FInworldChangeSceneEvent& Event) {  }
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldPacket
{
	GENERATED_BODY()

	FInworldPacket() = default;
	virtual ~FInworldPacket() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) {}

	virtual void Serialize(FMemoryArchive& Ar);

	FString ToDebugString() const;

	UPROPERTY()
	FInworldPacketId PacketId;
	UPROPERTY()
	FInworldRouting Routing;

protected:
	virtual void AppendDebugString(FString& Str) const PURE_VIRTUAL(FInworldPacket::AppendDebugString);
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldTextEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldTextEvent() = default;
	virtual ~FInworldTextEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	FString Text;
	UPROPERTY()
	bool Final = false;

protected:
	virtual void AppendDebugString(FString& Str) const override;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldDataEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldDataEvent() = default;
	virtual ~FInworldDataEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }

	virtual void Serialize(FMemoryArchive& Ar) override;

	TArray<uint8> Chunk;

protected:
	virtual void AppendDebugString(FString& Str) const;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldVisemeInfo
{
	GENERATED_BODY()

	FInworldVisemeInfo() = default;

	void Serialize(FMemoryArchive& Ar);

	FString Code;
	float Timestamp = 0.f;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldAudioDataEvent : public FInworldDataEvent
{
	GENERATED_BODY()

	FInworldAudioDataEvent() = default;
	virtual ~FInworldAudioDataEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	static void ConvertToReplicatableEvents(const FInworldAudioDataEvent& Event, TArray<FInworldAudioDataEvent>& RepEvents);

	virtual void Serialize(FMemoryArchive& Ar) override;

	TArray<FInworldVisemeInfo> VisemeInfos;
	bool bFinal = true;

protected:
	virtual void AppendDebugString(FString& Str) const;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldSilenceEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldSilenceEvent() = default;
	virtual ~FInworldSilenceEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	float Duration = 0.f;

protected:
	virtual void AppendDebugString(FString& Str) const;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldControlEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldControlEvent() = default;
	virtual ~FInworldControlEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	EInworldControlEventAction Action = EInworldControlEventAction::UNKNOWN;

protected:
	virtual void AppendDebugString(FString& Str) const;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldEmotionEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldEmotionEvent() = default;
	virtual ~FInworldEmotionEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }
	
	UPROPERTY()
	EInworldCharacterEmotionalBehavior Behavior = EInworldCharacterEmotionalBehavior::NEUTRAL;
	UPROPERTY()
	EInworldCharacterEmotionStrength Strength = EInworldCharacterEmotionStrength::NORMAL;

protected:
	virtual void AppendDebugString(FString& Str) const;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldCustomEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldCustomEvent() = default;
	virtual ~FInworldCustomEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }
		
	UPROPERTY()
	FString Name;

	UPROPERTY(NotReplicated)
	TMap<FString, FString> Params;
	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		TArray<FString> ParamKeys;
		TArray<FString> ParamValues;

		if (Ar.IsLoading())
		{
			Ar << ParamKeys;
			Ar << ParamValues;
			for (auto It = ParamKeys.CreateConstIterator(); It; ++It)
			{
				Params.Add(ParamKeys[It.GetIndex()], ParamValues[It.GetIndex()]);
			}
		}
		else
		{
			Params.GenerateKeyArray(ParamKeys);
			Params.GenerateValueArray(ParamValues);
			Ar << ParamKeys;
			Ar << ParamValues;
		}

		bOutSuccess = true;
		return true;
	}
	
protected:
	virtual void AppendDebugString(FString& Str) const;
};

template<>
struct TStructOpsTypeTraits<FInworldCustomEvent> : public TStructOpsTypeTraitsBase2<FInworldCustomEvent>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldChangeSceneEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldChangeSceneEvent() = default;
	virtual ~FInworldChangeSceneEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	TArray<FInworldAgentInfo> AgentInfos;

protected:
	virtual void AppendDebugString(FString& Str) const;
};
