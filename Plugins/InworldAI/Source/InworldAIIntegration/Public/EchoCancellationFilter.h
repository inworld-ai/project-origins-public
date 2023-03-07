/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include <vector>
#include <string>

class FEchoCancellationFilter
{
public:
	FEchoCancellationFilter();
	~FEchoCancellationFilter();

	std::vector<int16> FilterAudio(const std::vector<int16>& InAudio, const std::vector<int16>& OutAudio);

private:
	void* AecHandle;
};
