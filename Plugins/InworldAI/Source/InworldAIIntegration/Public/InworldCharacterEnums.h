/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EInworldCharacterGesturePlayback : uint8
{
	UNSPECIFIED = 0,
	INTERACTION = 1,
	INTERACTION_END = 2,
	UTTERANCE = 3,
};

UENUM(BlueprintType)
enum class EInworldCharacterEmotionalState : uint8
{
	Idle,
	Joy,
	Sadness,
	Fear,
	Anger,
	Disgust,
	Trust,
	Anticipation,
	Surprise,
};

UENUM(BlueprintType)
enum class EInworldCharacterEmotionalBehavior : uint8
{
	NEUTRAL = 0,
	DISGUST = 1,
	CONTEMPT = 2,
	BELLIGERENCE = 3,
	DOMINEERING = 4,
	CRITICISM = 5,
	ANGER = 6,
	TENSION = 7,
	TENSE_HUMOR = 8,
	DEFENSIVENESS = 9,
	WHINING = 10,
	SADNESS = 11,
	STONEWALLING = 12,
	INTEREST = 13,
	VALIDATION = 14,
	AFFECTION = 15,
	HUMOR = 16,
	SURPRISE = 17,
	JOY = 18,
};

UENUM(BlueprintType)
enum class EInworldCharacterEmotionStrength : uint8
{
	UNSPECIFIED = 0,
	WEAK = 1,
	STRONG = 2,
};