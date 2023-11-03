// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

THIRD_PARTY_INCLUDES_START
#include "Packets.h"
THIRD_PARTY_INCLUDES_END

#include "InworldPackets.h"

class InworldPacketTranslator : public Inworld::PacketVisitor
{
public:
	virtual ~InworldPacketTranslator() = default;

	virtual void Visit(const Inworld::TextEvent& Event) override { MakePacket<Inworld::TextEvent, FInworldTextEvent>(Event); }
	virtual void Visit(const Inworld::DataEvent& Event) override { MakePacket<Inworld::DataEvent, FInworldDataEvent>(Event); }
	virtual void Visit(const Inworld::AudioDataEvent& Event) override { MakePacket<Inworld::AudioDataEvent, FInworldAudioDataEvent>(Event); }
	virtual void Visit(const Inworld::SilenceEvent& Event) override { MakePacket<Inworld::SilenceEvent, FInworldSilenceEvent>(Event); }
	virtual void Visit(const Inworld::ControlEvent& Event) override { MakePacket<Inworld::ControlEvent, FInworldControlEvent>(Event); }
	virtual void Visit(const Inworld::EmotionEvent& Event) override { MakePacket<Inworld::EmotionEvent, FInworldEmotionEvent>(Event); };
	virtual void Visit(const Inworld::CustomEvent& Event) override { MakePacket<Inworld::CustomEvent, FInworldCustomEvent>(Event); };
	virtual void Visit(const Inworld::ChangeSceneEvent& Event) override { MakePacket<Inworld::ChangeSceneEvent, FInworldChangeSceneEvent>(Event); };

	TSharedPtr<FInworldPacket> GetPacket() { return Packet; }

protected:
	TSharedPtr<FInworldPacket> Packet;

	static void TranslateInworldActor(const Inworld::Actor& Original, FInworldActor& New);
	static void TranslateInworldRouting(const Inworld::Routing& Original, FInworldRouting& New);
	static void TranslateInworldPacketId(const Inworld::PacketId& Original, FInworldPacketId& New);
	static void TranslateInworldPacket(const Inworld::Packet& Original, FInworldPacket& New);

	template<typename TOrig, typename TNew>
	static void TranslateEvent(const TOrig& Original, TNew& New);

	template<typename TOrig, typename TNew>
	void MakePacket(const TOrig& Event)
	{
		TSharedPtr<TNew> NewPacket = TSharedPtr<TNew>(new TNew());
		TranslateEvent<TOrig, TNew>(Event, *NewPacket.Get());
		Packet = NewPacket;
	}
};
