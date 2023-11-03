/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Log.h"

#ifdef INWORLD_LOG

		#ifdef INWORLD_LOG_SPD
			#include "spdlog/spdlog.h"
		#endif

		#ifdef ANDROID
			#include <android/log.h>
		#endif

#endif

std::string Inworld::g_SessionId = "Unknown";

void Inworld::LogSetSessionId(const std::string Id)
{
    g_SessionId = Id;
}

void Inworld::LogClearSessionId()
{
    g_SessionId = "Unknown";
}

#ifdef INWORLD_LOG_CALLBACK
std::function<void(const char * message, int severity)> Inworld::g_LoggerCallback = nullptr;

void Inworld::LogSetLoggerCallback(LoggerCallBack callBack)
{
    g_LoggerCallback = callBack;
}

void Inworld::LogClearLoggerCallback() { g_LoggerCallback = nullptr; }
#endif

void Inworld::Log(const std::string& message)
{
#ifdef INWORLD_LOG
	#if  defined(INWORLD_LOG_CALLBACK)
		if (g_LoggerCallback) g_LoggerCallback(message.c_str(), 0);
	#elif defined(ANDROID)
		__android_log_print(ANDROID_LOG_INFO, "InworldNDK", "%s", message.c_str());
	#elif defined(INWORLD_LOG_SPD)
		spdlog::info(message);
	#else
		std::cout << message << std::endl;
	#endif
#endif
}

void Inworld::LogWarning(const std::string& message)
{
#ifdef INWORLD_LOG
	#if defined(INWORLD_LOG_CALLBACK)
		if (g_LoggerCallback) g_LoggerCallback(message.c_str(), 1);
	#elif defined(ANDROID)
		__android_log_print(ANDROID_LOG_WARN, "InworldNDK", "%s", message.c_str());
  #elif defined(INWORLD_LOG_SPD)
    spdlog::warn(message);
  #else
    std::cout << message << std::endl;
	#endif
#endif
}

void Inworld::LogError(const std::string& message)
{
#ifdef INWORLD_LOG
	const std::string error = VFormat("%s (SessionId: %s)", ARG_STR(message), ARG_STR(g_SessionId));
	#if defined(INWORLD_LOG_CALLBACK)
		if (g_LoggerCallback) g_LoggerCallback(message.c_str(), 2);
  #elif defined(ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "InworldNDK", "%s", error.c_str());
  #elif defined(INWORLD_LOG_SPD)
		spdlog::error(error);
  #else
    std::cout << error << std::endl;
	#endif
#endif
}
