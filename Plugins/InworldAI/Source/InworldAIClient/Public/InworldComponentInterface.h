#pragma once

#include "CoreMinimal.h"
#include "InworldState.h"

namespace Inworld
{
	class ICharacterComponent
	{
	public:
		virtual void HandleConnectionStateChanged(EInworldConnectionState State) = 0;
		virtual void SetAgentId(const FName& InAgentId) = 0;
		virtual const FName& GetAgentId() const = 0;
		virtual void SetGivenName(const FString& InGivenName) = 0;
		virtual const FString& GetGivenName() const = 0;
		virtual const FString& GetBrainName() const = 0;
		virtual void HandlePacket(TSharedPtr<FInworldPacket> Packet) = 0;
		virtual AActor* GetComponentOwner() const = 0;
	};

	class IPlayerComponent
	{
	public:
		virtual void HandleConnectionStateChanged(EInworldConnectionState State) = 0;
		virtual ICharacterComponent* GetTargetCharacter() = 0;
	};
}
