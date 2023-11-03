/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include <time.h>
#include <string>
#include "Define.h"

namespace Inworld
{
	struct INWORLD_EXPORT SessionInfo
	{
		std::string SessionId;
		std::string Token;
		std::string SessionSavedState;
		int64_t ExpirationTime;

		bool IsValid() const
		{
			return !Token.empty() && !SessionId.empty() && ExpirationTime > std::time(0);
		}
	};

	struct INWORLD_EXPORT AgentInfo
	{
		std::string BrainName;
		std::string AgentId;
		std::string GivenName;
	};

}
