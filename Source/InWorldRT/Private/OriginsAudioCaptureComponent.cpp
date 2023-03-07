/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "OriginsAudioCaptureComponent.h"

static const unsigned int MaxBufferSize = 2 * 5 * 48000;

FOriginsAudioCaptureSynth::FOriginsAudioCaptureSynth()
	: NumSamplesEnqueued(0)
	, bInitialized(false)
	, bIsCapturing(false)
{
}

FOriginsAudioCaptureSynth::~FOriginsAudioCaptureSynth()
{
}

bool FOriginsAudioCaptureSynth::GetCaptureDeviceInfo(Audio::FCaptureDeviceInfo& OutInfo, int Index = INDEX_NONE)
{
	return AudioCapture.GetCaptureDeviceInfo(OutInfo, Index);
}

bool FOriginsAudioCaptureSynth::OpenStream(int DeviceIndex)
{
	bool bSuccess = true;
	if (!AudioCapture.IsStreamOpen())
	{
		Audio::FOnCaptureFunction OnCapture = [this](const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverFlow)
		{
			int32 NumSamples = NumChannels * NumFrames;

			FScopeLock Lock(&CaptureCriticalSection);

			if (bIsCapturing)
			{
				// Append the audio memory to the capture data buffer
				int32 Index = AudioCaptureData.AddUninitialized(NumSamples);
				float* AudioCaptureDataPtr = AudioCaptureData.GetData();

				//Avoid reading outside of buffer boundaries
				if (!(AudioCaptureData.Num() + NumSamples > MaxBufferSize))
				{
					FMemory::Memcpy(&AudioCaptureDataPtr[Index], AudioData, NumSamples * sizeof(float));
				}
			}
		};

		// Prepare the audio buffer memory for 2 seconds of stereo audio at 48k SR to reduce chance for allocation in callbacks
		AudioCaptureData.Reserve(2 * 2 * 48000);

		Audio::FAudioCaptureDeviceParams Params = Audio::FAudioCaptureDeviceParams();
		Params.DeviceIndex = DeviceIndex;

		// Start the stream here to avoid hitching the audio render thread. 
		if (AudioCapture.OpenCaptureStream(Params, MoveTemp(OnCapture), 1024))
		{
			AudioCapture.StartStream();
		}
		else
		{
			bSuccess = false;
		}
	}
	return bSuccess;
}

bool FOriginsAudioCaptureSynth::StartCapturing()
{
	FScopeLock Lock(&CaptureCriticalSection);

	AudioCaptureData.Reset();

	check(AudioCapture.IsStreamOpen());

	bIsCapturing = true;
	return true;
}

void FOriginsAudioCaptureSynth::StopCapturing()
{
	FScopeLock Lock(&CaptureCriticalSection);
	bIsCapturing = false;
}

void FOriginsAudioCaptureSynth::AbortCapturing()
{
	AudioCapture.AbortStream();
	AudioCapture.CloseStream();
}

bool FOriginsAudioCaptureSynth::IsStreamOpen() const
{
	return AudioCapture.IsStreamOpen();
}

bool FOriginsAudioCaptureSynth::IsCapturing() const
{
	return bIsCapturing;
}

int32 FOriginsAudioCaptureSynth::GetNumSamplesEnqueued()
{
	FScopeLock Lock(&CaptureCriticalSection);
	return AudioCaptureData.Num();
}

bool FOriginsAudioCaptureSynth::GetAudioData(TArray<float>& OutAudioData)
{
	FScopeLock Lock(&CaptureCriticalSection);

	int32 CaptureDataSamples = AudioCaptureData.Num();
	if (CaptureDataSamples > 0)
	{
		// Append the capture audio to the output buffer
		int32 OutIndex = OutAudioData.AddUninitialized(CaptureDataSamples);
		float* OutDataPtr = OutAudioData.GetData();

		//Check bounds of buffer
		if (!(OutIndex > MaxBufferSize))
		{
			FMemory::Memcpy(&OutDataPtr[OutIndex], AudioCaptureData.GetData(), CaptureDataSamples * sizeof(float));
		}
		else
		{
			return false;
		}

		// Reset the capture data buffer since we copied the audio out
		AudioCaptureData.Reset();
		return true;
	}
	return false;
}

UOriginsAudioCaptureComponent::UOriginsAudioCaptureComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bSuccessfullyInitialized = false;
	bIsCapturing = false;
	CapturedAudioDataSamples = 0;
	ReadSampleIndex = 0;
	bIsDestroying = false;
	bIsNotReadyForForFinishDestroy = false;
	bIsStreamOpen = false;
	CaptureAudioData.Reserve(2 * 48000 * 5);
}

void UOriginsAudioCaptureComponent::GetAudioDevices(TArray<FOriginsAudioDevice>& OutDevices)
{
	Audio::FAudioCapture AudioCapture;
	TArray<Audio::FCaptureDeviceInfo> OutAudioDevices;
	AudioCapture.GetCaptureDevicesAvailable(OutAudioDevices);
	for (const auto& AudioDevice : OutAudioDevices)
	{
		FOriginsAudioDevice& NewOriginsAudioDevice = OutDevices.AddDefaulted_GetRef();
		NewOriginsAudioDevice.Name = AudioDevice.DeviceName;
		NewOriginsAudioDevice.Id = AudioDevice.DeviceId;
	}
}

bool UOriginsAudioCaptureComponent::SetAudioDevice(const FString& DeviceId)
{
	Stop();

	if (CaptureSynth.IsStreamOpen())
	{
		CaptureSynth.AbortCapturing();
	}

	TArray<FOriginsAudioDevice> OutDeviceNames;
	GetAudioDevices(OutDeviceNames);

	FOriginsAudioDevice* FoundDevice = OutDeviceNames.FindByPredicate([DeviceId](const FOriginsAudioDevice& AudioDevice) -> bool
		{
			return AudioDevice.Id == DeviceId;
		});

	if (FoundDevice != nullptr)
	{
		OriginsAudioDevice = *FoundDevice;
		ValidAudioDeviceIndex = GetValidAudioDeviceIndex();
		AudioDeviceIndex = GetAudioDeviceIndex();
		return true;
	}

	return false;
}

int32 UOriginsAudioCaptureComponent::GetAudioDeviceIndex()
{
	Audio::FAudioCapture AudioCapture;
	TArray<Audio::FCaptureDeviceInfo> OutDevices;
	AudioCapture.GetCaptureDevicesAvailable(OutDevices);
	int32 NumValidDevices = OutDevices.Num();
	int32 NumFoundValidDevices = 0;

	int32 DeviceIndex = 0;
	Audio::FCaptureDeviceInfo DeviceInfo;
	while (NumFoundValidDevices != NumValidDevices)
	{
		AudioCapture.GetCaptureDeviceInfo(DeviceInfo, DeviceIndex);
		if (DeviceInfo.DeviceId == OriginsAudioDevice.Id)
		{
			return DeviceIndex;
		}
		if (DeviceInfo.DeviceId == OutDevices[NumFoundValidDevices].DeviceId)
		{
			NumFoundValidDevices++;
		}
		DeviceIndex++;
	}

	return INDEX_NONE;
}

int32 UOriginsAudioCaptureComponent::GetValidAudioDeviceIndex()
{
	Audio::FAudioCapture AudioCapture;
	TArray<Audio::FCaptureDeviceInfo> OutDevices;
	AudioCapture.GetCaptureDevicesAvailable(OutDevices);
	int32 NumValidDevices = OutDevices.Num();
	int32 NumFoundValidDevices = 0;

	int32 DeviceIndex = 0;
	Audio::FCaptureDeviceInfo DeviceInfo;
	while (NumFoundValidDevices != NumValidDevices)
	{
		AudioCapture.GetCaptureDeviceInfo(DeviceInfo, DeviceIndex);
		if (DeviceInfo.DeviceId == OriginsAudioDevice.Id)
		{
			return NumFoundValidDevices;
		}
		if (OutDevices.FindByPredicate([DeviceInfo](const Audio::FCaptureDeviceInfo& Device) -> bool
			{
				return Device.DeviceId == DeviceInfo.DeviceId;
			}
		))
		{
			NumFoundValidDevices++;
		}
		DeviceIndex++;
	}

	return INDEX_NONE;
}

bool UOriginsAudioCaptureComponent::Init(int32& SampleRate)
{
	Audio::FCaptureDeviceInfo DeviceInfo;
	CaptureSynth.GetCaptureDeviceInfo(DeviceInfo, AudioDeviceIndex);
	if (DeviceInfo.DeviceId == OriginsAudioDevice.Id)
	{
		SampleRate = DeviceInfo.PreferredSampleRate;
		NumChannels = DeviceInfo.InputChannels;

		// Only support mono and stereo mic inputs for now...
		if (NumChannels == 1 || NumChannels == 2)
		{
			// This may fail if capture synths aren't supported on a given platform or if something went wrong with the capture device
			bIsStreamOpen = CaptureSynth.OpenStream(ValidAudioDeviceIndex);
			return true;
		}
		else
		{
			UE_LOG(LogAudio, Warning, TEXT("Audio capture components only support mono and stereo mic input."));
		}
	}

	return false;
}

void UOriginsAudioCaptureComponent::BeginDestroy()
{
	Super::BeginDestroy();

	// Flag that we're beginning to be destroyed
	// This is so that if a mic capture is open, we shut it down on the render thread
	bIsDestroying = true;

	// Make sure stop is kicked off
	Stop();
}

bool UOriginsAudioCaptureComponent::IsReadyForFinishDestroy()
{
	return !bIsNotReadyForForFinishDestroy;
}

void UOriginsAudioCaptureComponent::FinishDestroy()
{
	if (CaptureSynth.IsStreamOpen())
	{
		CaptureSynth.AbortCapturing();
	}

	check(!CaptureSynth.IsStreamOpen());

	Super::FinishDestroy();
	bSuccessfullyInitialized = false;
	bIsCapturing = false;
	bIsDestroying = false;
	bIsStreamOpen = false;
}

void UOriginsAudioCaptureComponent::OnBeginGenerate()
{
	CapturedAudioDataSamples = 0;
	ReadSampleIndex = 0;
	CaptureAudioData.Reset();

	if (!bIsStreamOpen)
	{
		bIsStreamOpen = CaptureSynth.OpenStream(ValidAudioDeviceIndex);
	}

	if (bIsStreamOpen)
	{
		CaptureSynth.StartCapturing();
		check(CaptureSynth.IsCapturing());

		// Don't allow this component to be destroyed until the stream is closed again
		bIsNotReadyForForFinishDestroy = true;
		FramesSinceStarting = 0;
		ReadSampleIndex = 0;
	}

}

void UOriginsAudioCaptureComponent::OnEndGenerate()
{
	if (bIsStreamOpen)
	{
		CaptureSynth.StopCapturing();
		bIsStreamOpen = false;

		bIsNotReadyForForFinishDestroy = false;
	}
}

int32 UOriginsAudioCaptureComponent::OnGenerateAudio(float* OutAudio, int32 NumSamples)
{
	// Don't do anything if the stream isn't open
	if (!bIsStreamOpen || !CaptureSynth.IsStreamOpen() || !CaptureSynth.IsCapturing())
	{
		// Just return NumSamples, which uses zero'd buffer
		return NumSamples;
	}
	int32 OutputSamplesGenerated = 0;
	//In case of severe overflow, just drop the data
	if (CaptureAudioData.Num() > MaxBufferSize)
	{
		//Clear the CaptureSynth's data, too
		CaptureSynth.GetAudioData(CaptureAudioData);
		CaptureAudioData.Reset();
		return NumSamples;
	}
	if (CapturedAudioDataSamples > 0 || CaptureSynth.GetNumSamplesEnqueued() > 1024)
	{
		// Check if we need to read more audio data from capture synth
		if (ReadSampleIndex + NumSamples > CaptureAudioData.Num())
		{
			// but before we do, copy off the remainder of the capture audio data buffer if there's data in it
			int32 SamplesLeft = FMath::Max(0, CaptureAudioData.Num() - ReadSampleIndex);
			if (SamplesLeft > 0)
			{
				float* CaptureDataPtr = CaptureAudioData.GetData();
				if (!(ReadSampleIndex + NumSamples > MaxBufferSize - 1))
				{
					FMemory::Memcpy(OutAudio, &CaptureDataPtr[ReadSampleIndex], SamplesLeft * sizeof(float));
				}
				else
				{
					UE_LOG(LogAudio, Warning, TEXT("Attempt to write past end of buffer in OnGenerateAudio, when we got more data from the synth"));
					return NumSamples;
				}
				// Track samples generated
				OutputSamplesGenerated += SamplesLeft;
			}
			// Get another block of audio from the capture synth
			CaptureAudioData.Reset();
			CaptureSynth.GetAudioData(CaptureAudioData);
			// Reset the read sample index since we got a new buffer of audio data
			ReadSampleIndex = 0;
		}
		// note it's possible we didn't get any more audio in our last attempt to get it
		if (CaptureAudioData.Num() > 0)
		{
			// Compute samples to copy
			int32 NumSamplesToCopy = FMath::Min(NumSamples - OutputSamplesGenerated, CaptureAudioData.Num() - ReadSampleIndex);
			float* CaptureDataPtr = CaptureAudioData.GetData();
			if (!(ReadSampleIndex + NumSamplesToCopy > MaxBufferSize - 1))
			{
				FMemory::Memcpy(&OutAudio[OutputSamplesGenerated], &CaptureDataPtr[ReadSampleIndex], NumSamplesToCopy * sizeof(float));
			}
			else
			{
				UE_LOG(LogAudio, Warning, TEXT("Attempt to read past end of buffer in OnGenerateAudio, when we did not get more data from the synth"));
				return NumSamples;
			}
			ReadSampleIndex += NumSamplesToCopy;
			OutputSamplesGenerated += NumSamplesToCopy;
		}
		CapturedAudioDataSamples += OutputSamplesGenerated;
	}
	else
	{
		// Say we generated the full samples, this will result in silence
		OutputSamplesGenerated = NumSamples;
	}
	return OutputSamplesGenerated;
}