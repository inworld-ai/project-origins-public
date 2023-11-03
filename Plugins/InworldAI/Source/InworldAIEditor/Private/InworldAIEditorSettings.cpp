// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.


#include "InworldAIEditorSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundSubmix.h"
#include "InworldPlayerAudioCaptureComponent.h"
#include "InworldPlayerTargetingComponent.h"
#include "InworldCharacterPlaybackAudio.h"
#include "Components/AudioComponent.h"

UInworldAIEditorSettings::UInworldAIEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InworldPlayerComponent = UInworldPlayerComponent::StaticClass();

	OtherPlayerComponents = { UInworldPlayerAudioCaptureComponent::StaticClass(), UInworldPlayerTargetingComponent::StaticClass() };

	InworldCharacterComponent = UInworldCharacterComponent::StaticClass();
	CharacterPlaybacks = { UInworldCharacterPlaybackAudio::StaticClass() };
	OtherCharacterComponents = { UAudioComponent::StaticClass() };
}
