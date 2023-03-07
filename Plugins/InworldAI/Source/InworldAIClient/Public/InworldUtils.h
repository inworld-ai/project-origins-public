/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"

#include <string>
#include <vector>

class UOVRLipSyncFrameSequence;
class USoundWave;
class UWorld;

namespace Inworld
{
    namespace Utils
    {
        INWORLDAICLIENT_API USoundWave* StringToSoundWave(const std::string& String);
        INWORLDAICLIENT_API bool SoundWaveToString(USoundWave* SoundWave, std::string& String);

		INWORLDAICLIENT_API bool SoundWaveToVec(USoundWave* SoundWave, std::vector<int16>& data);
		INWORLDAICLIENT_API USoundWave* VecToSoundWave(const std::vector<int16>& data);

        INWORLDAICLIENT_API TArray<uint8> HmacSha256(const TArray<uint8>& Data, const TArray<uint8>& Key);
        INWORLDAICLIENT_API std::string ToHex(const TArray<uint8>& Data);

        INWORLDAICLIENT_API void Log(const FString& Text, bool bAddOnScreenLog = false, FColor DisplayColor = FColor::Black, float TimeToDisplay = 3.f);
        INWORLDAICLIENT_API void ErrorLog(const FString& Text, bool bAddOnScreenLog = false, float TimeToDisplay = 3.f);

        INWORLDAICLIENT_API void PrepareSslCreds();

        class FTextColors
        {
        public:
            static FColor Player() { return FColor::White; }
            static FColor Character() { return FColor::Magenta; }
            static FColor Error() { return FColor::Red; }
            static FColor Message() { return FColor::Green; }
            static FColor TextMessage() { return FColor::Blue; }
        };

        class INWORLDAICLIENT_API FWorldTimer
        {
        public:
            FWorldTimer(float InThreshold) : Threshold(InThreshold) {}
             
            void SetOneTime(UWorld* World, float Threshold);
            bool CheckPeriod(UWorld* World);
			bool IsExpired(UWorld* World) const { return World->GetTimeSeconds() > LastTime + Threshold; }
            float GetThreshold() const { return Threshold; }

        private:
            float Threshold = 0.f;
            float LastTime = 0.f;
        };
        
    }
}