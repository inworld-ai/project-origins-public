/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "SentryTransaction.h"

#ifdef INWORLD_SENTRY
#include "sentry.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "Interfaces/IPluginManager.h"

static void CopyHeader(const char* Key, const char* Value, void* UserData)
{
	Inworld::FSentryTransactionHeader* Header = (Inworld::FSentryTransactionHeader*)UserData;
	Header->Key = std::string(Key);
	Header->Value = std::string(Value);
}

void Inworld::FSentryTransaction::Init(const FString& DSN, const FString& Name, const FString& Operation)
{
	if (!ensureMsgf(!DSN.IsEmpty() && !Name.IsEmpty() && !Operation.IsEmpty(), TEXT("Customize SentryDSN, SentryTransactionName, SentryTransactionOperation in Engine.ini")))
	{
		return;
	}

	const FString SentryPluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();
	const FString HandlerPath = FPaths::Combine(SentryPluginDir, TEXT("Binaries"), TEXT("Win64"), TEXT("crashpad_handler.exe"));

	sentry_options_t* options = sentry_options_new();
	sentry_options_set_dsn(options, TCHAR_TO_ANSI(*DSN));
	sentry_options_set_handler_path(options, TCHAR_TO_ANSI(*HandlerPath));
	sentry_options_set_traces_sample_rate(options, 1.0);

	sentry_init(options);

	ensure(!Ctx && !Transaction);
	Ctx = sentry_transaction_context_new(TCHAR_TO_ANSI(*Name), TCHAR_TO_ANSI(*Operation));
	Transaction = sentry_transaction_start(Ctx, sentry_value_t());

	sentry_transaction_iter_headers(Transaction, CopyHeader, (void*)&Header);
}

void Inworld::FSentryTransaction::Destroy()
{
	Stop();
	if (Transaction)
	{
		sentry_transaction_finish(Transaction);
		Transaction = nullptr;
	}
}

void Inworld::FSentryTransaction::Start(const FString& Operation, const FString& Description)
{
	ensure(!Span);
	Span = sentry_transaction_start_child(Transaction, TCHAR_TO_ANSI(*Operation), TCHAR_TO_ANSI(*Description));
}

void Inworld::FSentryTransaction::Stop()
{
	if (Span)
	{
		sentry_span_finish(Span);
		Span = nullptr;
	}
}

#else

void Inworld::FSentryTransaction::Init(const FString& DSN, const FString& Name, const FString& Operation) { }
void Inworld::FSentryTransaction::Destroy() { }
void Inworld::FSentryTransaction::Start(const FString& Operation, const FString& Description) { }
void Inworld::FSentryTransaction::Stop() { }

#endif
