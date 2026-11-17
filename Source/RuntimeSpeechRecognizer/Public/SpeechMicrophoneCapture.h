// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SpeechMicrophoneCapture.generated.h"

class USpeechRecognizer;
namespace Audio { class FAudioCapture; }

/**
 * Free microphone input for the speech recognizer, built on UE's own Audio Capture plugin
 * Captured audio is fed straight into the given speech recognizer, no extra plugin needed
 */
UCLASS(BlueprintType, Category = "Runtime Speech Recognizer")
class RUNTIMESPEECHRECOGNIZER_API USpeechMicrophoneCapture : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Creates a microphone capture object. Store it in a variable, otherwise it is garbage collected and capture stops
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Speech Recognizer|Microphone")
	static USpeechMicrophoneCapture* CreateMicrophoneCapture();

	/**
	 * Returns the names of the available input devices (microphones). The array index is the Device Index for Start Capture
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Speech Recognizer|Microphone")
	static TArray<FString> GetAvailableInputDevices();

	/**
	 * Starts capturing from a microphone and feeds the audio into the speech recognizer
	 * The recognizer should already be started (Start Speech Recognition)
	 * Calling it again while capturing switches to the new device
	 *
	 * @param SpeechRecognizer The recognizer to feed the audio into
	 * @param DeviceIndex Index from Get Available Input Devices, or -1 for the system default microphone
	 * @return True if capture started
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Speech Recognizer|Microphone")
	bool StartCapture(USpeechRecognizer* SpeechRecognizer, int32 DeviceIndex = -1);

	/**
	 * Same as Start Capture, but the microphone is picked by name from a dropdown on the node
	 * Falls back to the system default microphone if the name is not found (e.g. on a player's PC with other devices)
	 *
	 * @param SpeechRecognizer The recognizer to feed the audio into
	 * @param DeviceName The microphone name, or "Default" for the system default microphone
	 * @return True if capture started
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Speech Recognizer|Microphone")
	bool StartCaptureByName(USpeechRecognizer* SpeechRecognizer, UPARAM(meta = (GetOptions = "GetInputDeviceOptions")) FString DeviceName = TEXT("Default"));

	/** Options for the Start Capture By Name dropdown: "Default" followed by the available microphones */
	UFUNCTION(BlueprintCallable, Category = "Runtime Speech Recognizer|Microphone")
	static TArray<FString> GetInputDeviceOptions();

	/**
	 * Stops capturing. The speech recognizer keeps running, so capture can be started again quickly
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Speech Recognizer|Microphone")
	void StopCapture();

	/**
	 * Whether the microphone is currently capturing
	 */
	UFUNCTION(BlueprintPure, Category = "Runtime Speech Recognizer|Microphone")
	bool IsCapturing() const;

	virtual void BeginDestroy() override;

private:
	TSharedPtr<Audio::FAudioCapture> AudioCapture;
};
