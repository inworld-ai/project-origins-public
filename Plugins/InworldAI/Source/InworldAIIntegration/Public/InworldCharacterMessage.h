/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include <string>

namespace Inworld
{
	struct FCharacterMessageUtterance;
	struct FCharacterMessageSilence;
	struct FCharacterMessageTrigger;
	struct FCharacterMessageInteractionEnd;

	class FCharacterMessageVisitor
	{
	public:
		virtual void Visit(const FCharacterMessageUtterance& Event) { }
		virtual void Visit(const FCharacterMessageSilence& Event) { }
		virtual void Visit(const FCharacterMessageTrigger& Event) { }
		virtual void Visit(const FCharacterMessageInteractionEnd& Event) { }
	};

	struct FCharacterMessage
	{
		FName UtteranceId;
		FName InteractionId;

		virtual bool IsValid() const { return true; }
		virtual bool IsSkippable() const { return true; }
		virtual ~FCharacterMessage() = default;

		virtual void Accept(FCharacterMessageVisitor& Visitor) = 0;
	};

	struct FCharacterMessageUtterance : public FCharacterMessage
	{
		FString Text;
		std::string AudioData;
		FString CustomGesture;
		bool bTextFinal = false;

		virtual bool IsValid() const override { return (!Text.IsEmpty() && !AudioData.empty()) || !CustomGesture.IsEmpty(); }

		virtual void Accept(FCharacterMessageVisitor& Visitor) override { Visitor.Visit(*this); }
	};

	struct FCharacterMessageSilence : public FCharacterMessage
	{
		float Duration = 0.f;

		virtual bool IsValid() const override { return Duration != 0.f; }

		virtual void Accept(FCharacterMessageVisitor& Visitor) override { Visitor.Visit(*this); }
	};

	struct FCharacterMessageTrigger : public FCharacterMessage
	{
		FString Name;

		virtual bool IsValid() const override { return !Name.IsEmpty(); }

		virtual bool IsSkippable() const override { return false; }

		virtual void Accept(FCharacterMessageVisitor& Visitor) override { Visitor.Visit(*this); }
	};

	struct FCharacterMessageInteractionEnd : public FCharacterMessage
	{
		virtual bool IsSkippable() const override { return false; }

		virtual void Accept(FCharacterMessageVisitor& Visitor) override { Visitor.Visit(*this); }
	};
}
