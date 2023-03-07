/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Packets.h"
#include "proto/ProtoDisableWarning.h"

namespace Inworld {

    FString RandomUUID()
    {
        const FString Symbols = "0123456789abcdef";

        FString Result = "00000000-0000-0000-0000-0000000000";
        for (int32 i = 0; i < Result.Len(); i++) {
            if (Result[i] != '-')
            {
                Result[i] = Symbols[FMath::RandRange(0, Symbols.Len() - 1)];
            }
        }
        return Result;
    }

    InworldPakets::Actor FActor::ToProto() const
    {
        InworldPakets::Actor actor;
        actor.set_type(Type);
        actor.set_name(TCHAR_TO_UTF8(*Name.ToString()));
        return actor;
    }

    InworldPakets::Routing FRouting::ToProto() const
    {
        InworldPakets::Routing routing;
        *routing.mutable_source() = Source.ToProto();
        *routing.mutable_target() = Target.ToProto();
        return routing;
    }

    FRouting FRouting::Player2Agent(const FName& AgentId) {
        return FRouting(FActor(InworldPakets::Actor_Type_PLAYER, NAME_None), FActor(InworldPakets::Actor_Type_AGENT, AgentId));
    }

    InworldPakets::PacketId FPacketId::ToProto() const
    {
        InworldPakets::PacketId proto;
        proto.set_packet_id(TCHAR_TO_UTF8(*PacketId));
        proto.set_utterance_id(TCHAR_TO_UTF8(*UtteranceId.ToString()));
        proto.set_interaction_id(TCHAR_TO_UTF8(*InteractionId.ToString()));
        return proto;
    }

    InworldPakets::InworldPacket FInworldPacket::ToProto() const
    {
        InworldPakets::InworldPacket Proto;
        *Proto.mutable_packet_id() = PacketId.ToProto();
        *Proto.mutable_routing() = Routing.ToProto();
        *Proto.mutable_timestamp() = ::google::protobuf::util::TimeUtil::TimeTToTimestamp(Timestamp.ToUnixTimestamp());
        ToProtoInternal(Proto);
        return Proto;
    }

    void FTextEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const 
    {
        Proto.mutable_text()->set_text(TCHAR_TO_UTF8(*Text));
        Proto.mutable_text()->set_final(bFinal);
        Proto.mutable_text()->set_source_type(SourceType);
    }

    void FControlEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        Proto.mutable_control()->set_action(Action);
    }

    void FDataEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        Proto.mutable_data_chunk()->set_chunk(std::move(Chunk));
        Proto.mutable_data_chunk()->set_type(ai::inworld::packets::DataChunk_DataType_AUDIO);
    }

    FEmotionEvent::FEmotionEvent(const InworldPakets::InworldPacket& GrpcPacket) : FInworldPacket(GrpcPacket)
    {
        auto& Emotion = GrpcPacket.emotion();
        State.Joy = Emotion.joy();
        State.Fear = Emotion.fear();
        State.Trust = Emotion.trust();
        State.Surprise = Emotion.surprise();
        Behavior = GrpcPacket.emotion().behavior();
        Strength = GrpcPacket.emotion().strength();
    }

    void FEmotionEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        Proto.mutable_emotion()->set_joy(State.Joy);
        Proto.mutable_emotion()->set_fear(State.Fear);
        Proto.mutable_emotion()->set_trust(State.Trust);
        Proto.mutable_emotion()->set_surprise(State.Surprise);
        Proto.mutable_emotion()->set_behavior(Behavior);
    }

    void FCancelResponseEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        Proto.mutable_cancelresponses()->set_interaction_id(TCHAR_TO_UTF8(*InteractionId.ToString()));
        for (const auto& U : UtteranceIds)
        {
            Proto.mutable_cancelresponses()->add_utterance_id(TCHAR_TO_UTF8(*U.ToString()));
        }
    }

    void FSimpleGestureEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        Proto.mutable_gesture()->set_type(Gesture);
        Proto.mutable_gesture()->set_playback(Playback);
	}

	void FCustomGestureEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
	{
		
	}

	void FCustomEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
	{
        Proto.mutable_custom()->set_name(TCHAR_TO_UTF8(*Name));
	}

	void FSilenceEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
	{
    
	}

}
