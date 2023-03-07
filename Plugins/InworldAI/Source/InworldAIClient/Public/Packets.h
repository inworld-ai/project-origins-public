/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "proto/ProtoDisableWarning.h"
#include "proto/packets.pb.h"
#include <google/protobuf/util/time_util.h>

namespace InworldPakets = ai::inworld::packets;

namespace Inworld {

	INWORLDAICLIENT_API FString RandomUUID();

	// Represents agent or player.
	struct INWORLDAICLIENT_API FActor
	{
		FActor() = default;
		FActor(const InworldPakets::Actor& Actor) 
			: Type(Actor.type())
			, Name(Actor.name().c_str())
		{}
		FActor(const InworldPakets::Actor_Type InType, const FName& InName) 
			: Type(InType)
			, Name(InName) 
		{}

        InworldPakets::Actor ToProto() const;
        
		// Is Actor player or agent.
        InworldPakets::Actor_Type Type;
        // agent id if this is agent.
        FName Name;
	};

	// Source and target for packet.
	struct INWORLDAICLIENT_API FRouting
	{
		FRouting() = default;
		FRouting(const InworldPakets::Routing& Routing) 
			: Source(Routing.source())
			, Target(Routing.target()) 
		{}
		FRouting(const FActor& InSource, const FActor& InTarget) 
			: Source(InSource)
			, Target(InTarget) 
		{}

		static FRouting Player2Agent(const FName& AgentId);

        InworldPakets::Routing ToProto() const;
        
		FActor Source;
        FActor Target;
	};

	struct INWORLDAICLIENT_API FPacketId {
		// Constructs with all random parameters.
        FPacketId() 
			: FPacketId(RandomUUID(), FName(RandomUUID()), FName(RandomUUID())) 
		{}
        FPacketId(const InworldPakets::PacketId& PacketId) 
			: FPacketId(PacketId.packet_id().c_str(), PacketId.utterance_id().c_str(), PacketId.interaction_id().c_str()) 
		{}
		FPacketId(const FString& InPacketId, const FName& InUtteranceId, const FName& InInteractionId) 
			: PacketId(InPacketId)
			, UtteranceId(InUtteranceId)
			, InteractionId(InInteractionId) 
		{}

        InworldPakets::PacketId ToProto() const;
        
		// Always unique for given packet.
        FString PacketId;
        // Text and audio can have same utterance ids, which means they represent same utterance.
        FName UtteranceId;
        // Interaction start when player triggers it and finished when agent answers to player.
        FName InteractionId;
	};

    class FTextEvent;
    class FDataEvent;
	class FSilenceEvent;
    class FControlEvent;
    class FEmotionEvent;
    class FCancelResponseEvent;
    class FSimpleGestureEvent;
    class FCustomGestureEvent;
	class FCustomEvent;

    class INWORLDAICLIENT_API FPacketVisitor
    {
    public:
        virtual void Visit(const FTextEvent& Event) { ensure(false); }
        virtual void Visit(const FDataEvent& Event) { ensure(false); }
		virtual void Visit(const FSilenceEvent& Event) { ensure(false); }
        virtual void Visit(const FControlEvent& Event) { ensure(false); }
        virtual void Visit(const FEmotionEvent& Event) { ensure(false); }
        virtual void Visit(const FCancelResponseEvent& Event) { ensure(false); }
        virtual void Visit(const FSimpleGestureEvent& Event) { ensure(false); }
        virtual void Visit(const FCustomGestureEvent& Event) { ensure(false); }
		virtual void Visit(const FCustomEvent& Event) { ensure(false); }
    };

	struct FEmotionalState;

	// Base class for all Inworld protocol packets
	class INWORLDAICLIENT_API FInworldPacket
    {
	public:
        FInworldPacket() = default;
		FInworldPacket(const InworldPakets::InworldPacket& GrpcPacket) 
			: PacketId(GrpcPacket.packet_id())
			, Routing(GrpcPacket.routing())
			, Timestamp(google::protobuf::util::TimeUtil::TimestampToTimeT(GrpcPacket.timestamp()))
		{}
        FInworldPacket(const FRouting& InRouting) 
			: Routing(InRouting) 
		{}
		virtual ~FInworldPacket() = default;

		virtual void Accept(FPacketVisitor& Visitor) = 0;

		InworldPakets::InworldPacket ToProto() const;

    protected:
        virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const = 0;
        
	public:
        FPacketId PacketId;
        FRouting Routing;
        // UTC time when packet was generated.
        FDateTime Timestamp = FDateTime::Now();
	};

	class INWORLDAICLIENT_API FTextEvent : public FInworldPacket
	{
	public:
		FTextEvent() = default;
        FTextEvent(const InworldPakets::InworldPacket& GrpcPacket)
            : FInworldPacket(GrpcPacket)
            , Text(GrpcPacket.text().text().c_str())
            , bFinal(GrpcPacket.text().final())
            , SourceType(GrpcPacket.text().source_type())
        {}
        FTextEvent(const FString& InText, const FRouting& InRouting)
            : FInworldPacket(InRouting)
            , Text(InText)
            , bFinal(true)
            , SourceType(InworldPakets::TextEvent_SourceType_TYPED_IN)
        {}

		virtual void Accept(FPacketVisitor& Visitor) override { Visitor.Visit(*this); }

		const FString& GetText() const { return Text; }

        bool IsFinal() const { return bFinal; }

	protected:
        virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const override;

	private:
		FString Text;
		bool bFinal;
		InworldPakets::TextEvent_SourceType SourceType;
	};

	class INWORLDAICLIENT_API FDataEvent : public FInworldPacket
    {
	public:
		FDataEvent() = default;
		FDataEvent(const InworldPakets::InworldPacket& GrpcPacket)
			: FInworldPacket(GrpcPacket)
			, Chunk(GrpcPacket.data_chunk().chunk())
		{}
		FDataEvent(const std::string& Data, const FRouting& Routing)
			: FInworldPacket(Routing)
			, Chunk(Data)
		{}

		virtual void Accept(FPacketVisitor& Visitor) override { Visitor.Visit(*this); }

        const std::string& GetDataChunk() const { return Chunk; }

    protected:
        virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const override;

	private:
		// protobuf stores bytes data as string, to save copy time we can use same data type.
		std::string Chunk;
	};

	class INWORLDAICLIENT_API FSilenceEvent : public FInworldPacket
	{
	public:
		FSilenceEvent() = default;
		FSilenceEvent(const InworldPakets::InworldPacket& GrpcPacket)
			: FInworldPacket(GrpcPacket)
			, Duration(GrpcPacket.data_chunk().duration_ms() * 0.001f)
		{}
		FSilenceEvent(float Duration, const FRouting& Routing)
			: FInworldPacket(Routing)
			, Duration(Duration)
		{}

		virtual void Accept(FPacketVisitor& Visitor) override { Visitor.Visit(*this); }

		float GetDuration() const { return Duration; }

	protected:
		virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const override;

	private:
		float Duration;
	};
    
	class INWORLDAICLIENT_API FControlEvent : public FInworldPacket
    {
    public:
		FControlEvent() = default;
		FControlEvent(const InworldPakets::InworldPacket& GrpcPacket)
			: FInworldPacket(GrpcPacket)
			, Action(GrpcPacket.control().action())
		{}
        FControlEvent(InworldPakets::ControlEvent_Action InAction, const FRouting& Routing)
			: FInworldPacket(Routing)
			, Action(InAction)
		{}

		virtual void Accept(FPacketVisitor& Visitor) override { Visitor.Visit(*this); }

        InworldPakets::ControlEvent_Action GetControlAction() const { return Action; }

    protected:
        virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const override;

	private:
		InworldPakets::ControlEvent_Action Action;
    };

    struct INWORLDAICLIENT_API FEmotionalState
    {
        float Joy = 0.f;
        float Fear = 0.f;
        float Trust = 0.f;
        float Surprise = 0.f;
    };

    class FEmotionEvent : public FInworldPacket
    {
    public:
		FEmotionEvent() = default;
		FEmotionEvent(const InworldPakets::InworldPacket& GrpcPacket);

        virtual void Accept(FPacketVisitor& Visitor) override { Visitor.Visit(*this); }

        const FEmotionalState& GetEmotionalState() const { return State; }
		InworldPakets::EmotionEvent_SpaffCode GetEmotionalBehavior() const { return Behavior; }
		InworldPakets::EmotionEvent_Strength GetStrength() const { return Strength; }

    protected:
        virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const override;

	private:
		FEmotionalState State;
		InworldPakets::EmotionEvent_SpaffCode Behavior;
		InworldPakets::EmotionEvent_Strength Strength;
    };

    class INWORLDAICLIENT_API FCancelResponseEvent : public FInworldPacket
    {
    public:
		FCancelResponseEvent() = default;
        FCancelResponseEvent(const FName& InInteractionId, const TArray<FName>& InUtteranceIds, const FRouting& Routing) 
			: FInworldPacket(Routing)
			, InteractionId(InInteractionId)
            , UtteranceIds(InUtteranceIds)
		{}

        virtual void Accept(FPacketVisitor& Visitor) override { Visitor.Visit(*this); }

    protected:
        virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const override;

    private:
		FName InteractionId;
		TArray<FName> UtteranceIds;
    };

    class INWORLDAICLIENT_API FSimpleGestureEvent : public FInworldPacket
    {
    public:
		FSimpleGestureEvent() = default;
		FSimpleGestureEvent(const InworldPakets::InworldPacket& GrpcPacket)
            : FInworldPacket(GrpcPacket)
            , Gesture(GrpcPacket.gesture().type())
			, Playback(GrpcPacket.gesture().playback())
        {}
		FSimpleGestureEvent(InworldPakets::GestureEvent_Type InGesture, InworldPakets::Playback InPlayback, const FRouting& Routing)
            : FInworldPacket(Routing)
            , Gesture(InGesture)
			, Playback(InPlayback)
        {}

        virtual void Accept(FPacketVisitor& Visitor) override { Visitor.Visit(*this); }

        InworldPakets::GestureEvent_Type GetSimpleGesture() const { return Gesture; }
		InworldPakets::Playback GetPlayback() const { return Playback; }

    protected:
        virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const override;

    private:
        InworldPakets::GestureEvent_Type Gesture;
		InworldPakets::Playback Playback;
    };

    class INWORLDAICLIENT_API FCustomGestureEvent : public FInworldPacket
    {
    public:
		FCustomGestureEvent() = default;
		FCustomGestureEvent(const InworldPakets::InworldPacket& GrpcPacket)
            : FInworldPacket(GrpcPacket)
            , GestureName(UTF8_TO_TCHAR(GrpcPacket.custom().name().data()))
			, Playback(GrpcPacket.custom().playback())
        {}
		FCustomGestureEvent(const FString& InGesture, const FRouting& Routing)
            : FInworldPacket(Routing)
            , GestureName(InGesture)
        {}

        virtual void Accept(FPacketVisitor& Visitor) override { Visitor.Visit(*this); }

        const FString& GetCustomGesture() const { return GestureName; }

    protected:
        virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const override;

    private:
		FString GestureName;
		InworldPakets::Playback Playback;
	};

	class INWORLDAICLIENT_API FCustomEvent : public FInworldPacket
	{
	public:
		FCustomEvent() = default;
		FCustomEvent(const InworldPakets::InworldPacket& GrpcPacket)
			: FInworldPacket(GrpcPacket)
			, Name(UTF8_TO_TCHAR(GrpcPacket.custom().name().data()))
		{}
		FCustomEvent(const FString& InName, const FRouting& Routing)
			: FInworldPacket(Routing)
			, Name(InName)
		{}

		virtual void Accept(FPacketVisitor& Visitor) override { Visitor.Visit(*this); }

		const FString& GetName() const { return Name; }

	protected:
		virtual void ToProtoInternal(InworldPakets::InworldPacket& Proto) const override;

	private:
		FString Name;
	};

}
#pragma warning(pop)
