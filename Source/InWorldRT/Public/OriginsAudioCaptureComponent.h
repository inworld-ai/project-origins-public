/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "AudioCapture.h"
#include "AudioCaptureDeviceInterface.h"
#include "Components/SynthComponent.h"
#include "OriginsAudioCaptureComponent.generated.h"

class INWORLDRT_API FOriginsAudioCaptureSynth
{
public:
	FOriginsAudioCaptureSynth();
	virtual ~FOriginsAudioCaptureSynth();

	// Gets the capture device info at the index
	bool GetCaptureDeviceInfo(Audio::FCaptureDeviceInfo& OutInfo, int Index);

	// Opens up a stream to the default capture device
	bool OpenStream(int Index);

	// Starts capturing audio
	bool StartCapturing();

	// Stops capturing audio
	void StopCapturing();

	// Immediately stop capturing audio
	void AbortCapturing();

	// Returned if the capture synth is closed
	bool IsStreamOpen() const;

	// Returns true if the capture synth is capturing audio
	bool IsCapturing() const;

	// Retrieves audio data from the capture synth.
	// This returns audio only if there was non-zero audio since this function was last called.
	bool GetAudioData(TArray<float>& OutAudioData);

	// Returns the number of samples enqueued in the capture synth
	int32 GetNumSamplesEnqueued();

	// Number of samples enqueued
	int32 NumSamplesEnqueued;

	// Information about the default capture device we're going to use
	Audio::FCaptureDeviceInfo CaptureInfo;

	// Audio capture object dealing with getting audio callbacks
	Audio::FAudioCapture AudioCapture;

	// Critical section to prevent reading and writing from the captured buffer at the same time
	FCriticalSection CaptureCriticalSection;

	// Buffer of audio capture data, yet to be copied to the output 
	TArray<float> AudioCaptureData;


	// If the object has been initialized
	bool bInitialized;

	// If we're capturing data
	bool bIsCapturing;
};

USTRUCT(BlueprintType)
struct INWORLDRT_API FOriginsAudioDevice
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Id;
};

UCLASS(ClassGroup = Synth, meta = (BlueprintSpawnableComponent))
class INWORLDRT_API UOriginsAudioCaptureComponent : public USynthComponent
{
	GENERATED_BODY()

protected:

	UOriginsAudioCaptureComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Audio|Devices")
	static void GetAudioDevices(TArray<FOriginsAudioDevice>& OutDevices);

	UFUNCTION(BlueprintCallable, Category = "Audio|Devices")
	bool SetAudioDevice(const FString& DeviceName);

	//~ Begin USynthComponent interface
	virtual bool Init(int32& SampleRate) override;
	virtual int32 OnGenerateAudio(float* OutAudio, int32 NumSamples) override;
	virtual void OnBeginGenerate() override;
	virtual void OnEndGenerate() override;
	//~ End USynthComponent interface

	//~ Begin UObject interface
	virtual void BeginDestroy();
	virtual bool IsReadyForFinishDestroy() override;
	virtual void FinishDestroy() override;
	//~ End UObject interface

public:
	/**
	*   Induced latency in audio frames to use to account for jitter between mic capture hardware and audio render hardware.
	 *	Increasing this number will increase latency but reduce potential for underruns.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Latency", meta = (ClampMin = "0", ClampMax = "1024"))
	int32 JitterLatencyFrames;

private:

	int32 GetAudioDeviceIndex();
	int32 AudioDeviceIndex;
	int32 GetValidAudioDeviceIndex();
	int32 ValidAudioDeviceIndex;

	FOriginsAudioCaptureSynth CaptureSynth;
	TArray<float> CaptureAudioData;
	int32 CapturedAudioDataSamples;

	FOriginsAudioDevice OriginsAudioDevice;

	bool bSuccessfullyInitialized;
	bool bIsCapturing;
	bool bIsStreamOpen;
	int32 CaptureChannels;
	int32 FramesSinceStarting;
	int32 ReadSampleIndex;
	FThreadSafeBool bIsDestroying;
	FThreadSafeBool bIsNotReadyForForFinishDestroy;
};