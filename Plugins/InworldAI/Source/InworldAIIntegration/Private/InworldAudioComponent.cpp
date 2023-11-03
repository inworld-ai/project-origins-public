// Fill out your copyright notice in the Description page of Project Settings.


#include "InworldAudioComponent.h"

void UInworldAudioComponent::Play(float StartTime /* = 0.f*/)
{
	if (Sound != nullptr)
	{
		Sound->SourceEffectChain = EffectSourcePresetChain;
	}
	Super::Play(StartTime);
}