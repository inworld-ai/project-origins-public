/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#ifdef _WIN32
#define INWORLD_EXPORT __declspec(dllexport)
#else
#define INWORLD_EXPORT __attribute__((visibility("default")))
#endif
