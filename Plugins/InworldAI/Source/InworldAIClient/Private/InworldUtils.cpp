/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldUtils.h"
#include "proto/ProtoDisableWarning.h"
#include "Logging/LogMacros.h"
#include "InworldAIClientModule.h"
#include "SslCredentials.h"
#include "Audio.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundGroups.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"

#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "openssl/hmac.h"
THIRD_PARTY_INCLUDES_END
#undef UI

USoundWave* Inworld::Utils::StringToSoundWave(const std::string& String)
{
    uint8* Data = (uint8*)String.data();
    const uint32 Num = String.size();

    FWaveModInfo WaveInfo;
    if (!WaveInfo.ReadWaveInfo(Data, Num))
    {
        return nullptr;
    }

    USoundWave* SoundWave = NewObject<USoundWave>(USoundWave::StaticClass());
    if (!ensure(SoundWave))
    {
        return nullptr;
	}

    SoundWave->Duration = *WaveInfo.pWaveDataSize / (*WaveInfo.pChannels * (*WaveInfo.pBitsPerSample / 8.f) * *WaveInfo.pSamplesPerSec);

    SoundWave->SetSampleRate(*WaveInfo.pSamplesPerSec);
    SoundWave->NumChannels = *WaveInfo.pChannels;
    SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;
    SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Voice;
    SoundWave->RawPCMData = (uint8*)FMemory::Malloc(WaveInfo.SampleDataSize);
    FMemory::Memcpy(SoundWave->RawPCMData, WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);

    return SoundWave;
}

bool Inworld::Utils::SoundWaveToString(USoundWave* SoundWave, std::string& String)
{
    //TODO: support other formats
    constexpr int32 SampleRate = 48000;
    constexpr int32 NumChannels = 2;
    if (!ensure(
        SoundWave->GetSampleRateForCurrentPlatform() == SampleRate &&
        SoundWave->NumChannels == NumChannels
    ))
    {
        return false;
    }

    const uint8* WaveData = SoundWave->RawPCMData;
    const int32 WaveDataSize = SoundWave->RawPCMDataSize;
    constexpr int32 MinSize = 0.01f * SampleRate * NumChannels * sizeof(uint16); // 10ms
    if (!ensure(WaveData && WaveDataSize > MinSize))
    {
        return false;
    }

    String.resize(WaveDataSize);
    uint8* StrData = (uint8*)String.data();
    FMemory::Memcpy(StrData, WaveData, WaveDataSize);

    uint16* pDst = (uint16*)String.data();
    uint16* pSrc = pDst + 5;
    uint16* pEnd = (uint16*)(StrData + String.size());

    // downsample to 16000 per sec and cut the second channel
    while (true)
    {
        FMemory::Memcpy(pDst, pSrc, sizeof(uint16));
        pDst += 1;
        pSrc += 6;
        if (pSrc >= pEnd)
        {
            break;
        }
    }

    const uint32 NewSampleDataSize = (uint8*)pDst - StrData - sizeof(uint16);
    String.resize(NewSampleDataSize);

    return true;
}

INWORLDAICLIENT_API bool Inworld::Utils::SoundWaveToVec(USoundWave* SoundWave, std::vector<int16>& data)
{
	constexpr int32 SampleRate = 48000;
	constexpr int32 DownsampleRate = SampleRate / 16000;
	constexpr int32 NumChannels = 2;
	if (!ensure(
		SoundWave->GetSampleRateForCurrentPlatform() == SampleRate &&
		SoundWave->NumChannels == NumChannels
	))
	{
		return false;
	}

	const int16* WaveData = (const int16*)SoundWave->RawPCMData;
	const int32 WaveDataSize = SoundWave->RawPCMDataSize;
	constexpr int32 MinSize = 0.01f * SampleRate * NumChannels * sizeof(uint16); // 10ms
	if (!ensure(WaveData && WaveDataSize > MinSize))
	{
		return false;
	}

	const auto nSamples = WaveDataSize / (NumChannels * sizeof(uint16_t) * DownsampleRate);
	data.resize(nSamples);
	int j = 0;
	for (int i = 0; i < nSamples; i++)
	{
		data[i] = WaveData[j];
		j += NumChannels * DownsampleRate;
	}

	return true;
}

INWORLDAICLIENT_API USoundWave* Inworld::Utils::VecToSoundWave(const std::vector<int16>& data)
{
	const int16* srcPtr = data.data();
	const auto nSamples = data.size();

	USoundWave* SoundWave = NewObject<USoundWave>(USoundWave::StaticClass());
	if (!ensure(SoundWave))
	{
		return nullptr;
	}

	constexpr int32 SamplesPerSec = 16000;
	SoundWave->Duration = nSamples / SamplesPerSec;

	SoundWave->SetSampleRate(SamplesPerSec);
	SoundWave->NumChannels = 1;
	SoundWave->RawPCMDataSize = nSamples * sizeof(int16);
	SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Voice;

	SoundWave->RawPCMData = (uint8*)FMemory::Malloc(SoundWave->RawPCMDataSize);
	FMemory::Memcpy(SoundWave->RawPCMData, srcPtr, SoundWave->RawPCMDataSize);
	return SoundWave;
}

TArray<uint8> Inworld::Utils::HmacSha256(const TArray<uint8>& Data, const TArray<uint8>& Key)
{
    TArray<uint8> Res;
    Res.SetNumZeroed(32);
	uint32 OutputLen = 0;
	HMAC(EVP_sha256(), Key.GetData(), Key.Num(), Data.GetData(), Data.Num(), Res.GetData(), &OutputLen);
    return Res;
}

std::string Inworld::Utils::ToHex(const TArray<uint8>& Data)
{
    std::string Res(Data.Num() * 2, '0');
	for (int32 i = 0; i < Data.Num(); i++)
	{
		FCStringAnsi::Sprintf((char*)(Res.data()) + (i * 2), "%02x", Data[i]);
	}

	return Res;
}

void Inworld::Utils::Log(const FString& Text, bool bAddOnScreenLog, FColor DisplayColor, float TimeToDisplay)
{
    UE_LOG(LogInworld, Log, TEXT("%s"), *Text);

#if INWORLD_ONSCREEN_LOG
    if (bAddOnScreenLog && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, DisplayColor, Text, false);
    }
#endif
}

void Inworld::Utils::ErrorLog(const FString& Text, bool bAddOnScreenLog, float TimeToDisplay)
{
    UE_LOG(LogInworld, Error, TEXT("%s"), *Text);

#if INWORLD_ONSCREEN_LOG_ERROR
    if (bAddOnScreenLog && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, FTextColors::Error(), Text, false);
    }
#endif
}

void Inworld::Utils::PrepareSslCreds()
{
    const FString RootsFileDir = FPaths::ProjectDir() + "/roots.pem";
    if (!FPaths::FileExists(RootsFileDir))
    {
        FFileHelper::SaveStringArrayToFile(SslRootsFileContents, *RootsFileDir);
    }
    
	FPlatformMisc::SetEnvironmentVar(TEXT("GRPC_DEFAULT_SSL_ROOTS_FILE_PATH"), *FPaths::ConvertRelativePathToFull(RootsFileDir));
}

void Inworld::Utils::FWorldTimer::SetOneTime(UWorld* World, float InThreshold)
{
    Threshold = InThreshold;
    LastTime = World->GetTimeSeconds();
}

bool Inworld::Utils::FWorldTimer::CheckPeriod(UWorld* World)
{
    if (IsExpired(World))
    {
        LastTime = World->GetTimeSeconds();
        return true;
    }
    return false;
}
