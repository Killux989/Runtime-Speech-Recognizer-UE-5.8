// Georgy Treshchev 2024.

#include "SpeechMicrophoneCapture.h"
#include "SpeechRecognizer.h"
#include "SpeechRecognizerDefines.h"
#include "AudioCaptureCore.h"
#include "Async/Async.h"

USpeechMicrophoneCapture* USpeechMicrophoneCapture::CreateMicrophoneCapture()
{
	return NewObject<USpeechMicrophoneCapture>();
}

TArray<FString> USpeechMicrophoneCapture::GetAvailableInputDevices()
{
	Audio::FAudioCapture Capture;
	TArray<Audio::FCaptureDeviceInfo> Devices;
	Capture.GetCaptureDevicesAvailable(Devices);

	TArray<FString> Names;
	for (const Audio::FCaptureDeviceInfo& Device : Devices)
	{
		Names.Add(Device.DeviceName);
	}
	return Names;
}

bool USpeechMicrophoneCapture::StartCapture(USpeechRecognizer* SpeechRecognizer, int32 DeviceIndex)
{
	if (!SpeechRecognizer)
	{
		UE_LOG(LogRuntimeSpeechRecognizer, Error, TEXT("Unable to start microphone capture: the speech recognizer is null"));
		return false;
	}

	StopCapture();
	AudioCapture = MakeShared<Audio::FAudioCapture>();

	Audio::FAudioCaptureDeviceParams Params;
	Params.DeviceIndex = DeviceIndex;

	// Called on the audio capture thread. Audio is copied and handed to the recognizer on the game thread, where the UObject is safe to use
	TWeakObjectPtr<USpeechRecognizer> WeakRecognizer = SpeechRecognizer;
	Audio::FOnCaptureFunction OnCapture = [WeakRecognizer](const float* Audio, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverflow)
	{
		TArray<float> PCMData(Audio, NumFrames * NumChannels);
		AsyncTask(ENamedThreads::GameThread, [WeakRecognizer, PCMData = MoveTemp(PCMData), NumChannels, SampleRate]() mutable
		{
			if (USpeechRecognizer* Recognizer = WeakRecognizer.Get())
			{
				if (!Recognizer->GetIsStopped() && !Recognizer->GetIsStopping())
				{
					Recognizer->ProcessAudioData(MoveTemp(PCMData), SampleRate, NumChannels, false);
				}
			}
		});
	};

	// The float-callback overload is the one exported from AudioCaptureCore
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const bool bOpened = AudioCapture->OpenCaptureStream(Params, MoveTemp(OnCapture), 1024);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	if (!bOpened || !AudioCapture->StartStream())
	{
		UE_LOG(LogRuntimeSpeechRecognizer, Error, TEXT("Unable to start microphone capture on device %d. Is the Audio Capture plugin enabled and a microphone connected?"), DeviceIndex);
		StopCapture();
		return false;
	}

	UE_LOG(LogRuntimeSpeechRecognizer, Log, TEXT("Microphone capture started on device %d (%d Hz)"), DeviceIndex, AudioCapture->GetSampleRate());
	return true;
}

TArray<FString> USpeechMicrophoneCapture::GetInputDeviceOptions()
{
	TArray<FString> Options = {TEXT("Default")};
	Options.Append(GetAvailableInputDevices());
	return Options;
}

bool USpeechMicrophoneCapture::StartCaptureByName(USpeechRecognizer* SpeechRecognizer, FString DeviceName)
{
	const int32 DeviceIndex = GetAvailableInputDevices().IndexOfByKey(DeviceName);
	if (DeviceIndex == INDEX_NONE && DeviceName != TEXT("Default"))
	{
		UE_LOG(LogRuntimeSpeechRecognizer, Warning, TEXT("Microphone '%s' not found, using the system default microphone"), *DeviceName);
	}
	return StartCapture(SpeechRecognizer, DeviceIndex);
}

void USpeechMicrophoneCapture::StopCapture()
{
	if (AudioCapture.IsValid())
	{
		AudioCapture->StopStream();
		AudioCapture->CloseStream();
		AudioCapture.Reset();
	}
}

bool USpeechMicrophoneCapture::IsCapturing() const
{
	return AudioCapture.IsValid() && AudioCapture->IsCapturing();
}

void USpeechMicrophoneCapture::BeginDestroy()
{
	StopCapture();
	Super::BeginDestroy();
}
