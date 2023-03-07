/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include <string>

struct sentry_transaction_context_s;
struct sentry_transaction_s;
struct sentry_span_s;

namespace Inworld
{
	struct FSentryTransactionHeader
	{
		std::string Key;
		std::string Value;
	};

	class FSentryTransaction
	{
	public:

		void Init(const FString& DSN, const FString& Name, const FString& Operation);
		void Destroy();

		void Start(const FString& Operation, const FString& Description);
		void Stop();

		const FSentryTransactionHeader& GetHeader() const { return Header; }

	private:
		
		FSentryTransactionHeader Header;

		sentry_transaction_context_s* Ctx = nullptr;
		sentry_transaction_s* Transaction = nullptr;
		sentry_span_s* Span = nullptr;

	};
}