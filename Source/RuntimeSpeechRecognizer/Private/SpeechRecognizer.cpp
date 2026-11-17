// Georgy Treshchev 2024.

#include "SpeechRecognizer.h"
#include "SpeechRecognizerDefines.h"
#include "SpeechRecognizerThread.h"
#include "Async/Async.h"

namespace
{
	// Recognition runs on worker threads; Blueprint events (spawning actors, VFX, UI) must fire on the game thread
	template <typename FuncType>
	void RunOnGameThread(USpeechRecognizer* Recognizer, FuncType&& Func)
	{
		if (IsInGameThread())
		{
			Func(Recognizer);
			return;
		}
		AsyncTask(ENamedThreads::GameThread, [WeakRecognizer = TWeakObjectPtr<USpeechRecognizer>(Recognizer), Func = Forward<FuncType>(Func)]() mutable
		{
			if (USpeechRecognizer* StrongRecognizer = WeakRecognizer.Get())
			{
				Func(StrongRecognizer);
			}
		});
	}
}

USpeechRecognizer::USpeechRecognizer()
{
	Thread = MakeShared<FSpeechRecognizerThread>();
	ensure(Thread.IsValid());

	Thread->OnRecognitionFinished.AddWeakLambda(this, [this]()
	{
		RunOnGameThread(this, [](USpeechRecognizer* Self)
		{
			Self->OnRecognitionFinished.Broadcast();
			Self->OnRecognitionFinishedNative.Broadcast();
		});
	});

	Thread->OnRecognitionError.AddWeakLambda(this, [this](const FString& ShortErrorMessage, const FString& LongErrorMessage)
	{
		RunOnGameThread(this, [ShortErrorMessage, LongErrorMessage](USpeechRecognizer* Self)
		{
			Self->OnRecognitionError.Broadcast(ShortErrorMessage, LongErrorMessage);
			Self->OnRecognitionErrorNative.Broadcast(ShortErrorMessage, LongErrorMessage);
		});
	});

	Thread->OnRecognizedTextSegment.AddWeakLambda(this, [this](const FString& RecognizedWords)
	{
		RunOnGameThread(this, [RecognizedWords](USpeechRecognizer* Self)
		{
			Self->OnRecognizedTextSegment.Broadcast(RecognizedWords);
			Self->OnRecognizedTextSegmentNative.Broadcast(RecognizedWords);
		});
	});

	Thread->OnRecognitionProgress.AddWeakLambda(this, [this](int32 Progress)
	{
		RunOnGameThread(this, [Progress](USpeechRecognizer* Self)
		{
			Self->OnRecognitionProgress.Broadcast(Progress);
			Self->OnRecognitionProgressNative.Broadcast(Progress);
		});
	});

	Thread->OnRecognitionStopped.AddWeakLambda(this, [this]()
	{
		RunOnGameThread(this, [](USpeechRecognizer* Self)
		{
			Self->OnRecognitionStopped.Broadcast();
			Self->OnRecognitionStoppedNative.Broadcast();
		});
	});
}

USpeechRecognizer* USpeechRecognizer::CreateSpeechRecognizer()
{
	return NewObject<USpeechRecognizer>();
}

void USpeechRecognizer::StartSpeechRecognition(const FOnSpeechRecognitionStartedDynamic& OnStarted)
{
	StartSpeechRecognition(FOnSpeechRecognitionStartedStatic::CreateWeakLambda(this, [OnStarted](bool bSucceeded) { OnStarted.ExecuteIfBound(bSucceeded); }));
}

void USpeechRecognizer::StartSpeechRecognition(const FOnSpeechRecognitionStartedStatic& OnStarted)
{
	if (!Thread->GetIsStopped())
	{
		UE_LOG(LogRuntimeSpeechRecognizer, Error, TEXT("Failed to start speech recognition: Speech recognition is already running"));
		OnStarted.ExecuteIfBound(false);
		return;
	}
	if (Thread->GetIsStopping())
	{
		UE_LOG(LogRuntimeSpeechRecognizer, Error, TEXT("Failed to start speech recognition: Speech recognition is stopping"));
		OnStarted.ExecuteIfBound(false);
		return;
	}
	Thread->StartThread().Next([WeakThis = TWeakObjectPtr<USpeechRecognizer>(this), OnStarted](bool bSucceeded)
	{
		if (USpeechRecognizer* StrongThis = WeakThis.Get())
		{
			RunOnGameThread(StrongThis, [OnStarted, bSucceeded](USpeechRecognizer*) { OnStarted.ExecuteIfBound(bSucceeded); });
		}
	});
}

void USpeechRecognizer::StopSpeechRecognition()
{
	Thread->StopThread();
}

void USpeechRecognizer::ProcessAudioData(TArray<float> PCMData, float SampleRate, int32 NumOfChannels, bool bLast)
{
	ProcessAudioData(Audio::FAlignedFloatBuffer(MoveTemp(PCMData)), SampleRate, NumOfChannels, bLast);
}

void USpeechRecognizer::ProcessAudioData(Audio::FAlignedFloatBuffer PCMData, float SampleRate, int32 NumOfChannels, bool bLast)
{
	Thread->ProcessPCMData(MoveTemp(PCMData), SampleRate, NumOfChannels, bLast);
}

void USpeechRecognizer::ForceProcessPendingAudioData()
{
	Thread->ForceProcessPendingAudioData();
}

void USpeechRecognizer::ClearAudioData(bool bClearPendingAudioData, bool bClearAudioQueue)
{
	Thread->ClearAudioData(bClearPendingAudioData, bClearAudioQueue);
}

bool USpeechRecognizer::GetIsStopped() const
{
	return Thread->GetIsStopped();
}

bool USpeechRecognizer::GetIsStopping() const
{
	return Thread->GetIsStopping();
}

bool USpeechRecognizer::GetIsFinished() const
{
	return Thread->GetIsFinished();
}

bool USpeechRecognizer::SetRecognitionParameters(const FSpeechRecognitionParameters& Parameters)
{
	return Thread->SetRecognitionParameters(Parameters);
}

FSpeechRecognitionParameters USpeechRecognizer::GetNonStreamingDefaults()
{
	return FSpeechRecognizerThread::GetNonStreamingDefaults();
}

FSpeechRecognitionParameters USpeechRecognizer::GetStreamingDefaults()
{
	return FSpeechRecognizerThread::GetStreamingDefaults();
}

FSpeechRecognitionParameters USpeechRecognizer::GetRecognitionParameters() const
{
	return Thread->GetRecognitionParameters();
}

bool USpeechRecognizer::SetNonStreamingDefaults()
{
	return Thread->SetNonStreamingDefaults();
}

bool USpeechRecognizer::SetStreamingDefaults()
{
	return Thread->SetStreamingDefaults();
}

bool USpeechRecognizer::SetNumOfThreads(int32 Value)
{
	return Thread->SetNumOfThreads(Value);
}

bool USpeechRecognizer::SetLanguage(ESpeechRecognizerLanguage Language)
{
	return Thread->SetLanguage(Language);
}

bool USpeechRecognizer::SetTranslateToEnglish(bool bTranslate)
{
	return Thread->SetTranslateToEnglish(bTranslate);
}

bool USpeechRecognizer::SetStepSize(int32 Value)
{
	return Thread->SetStepSize(Value);
}

bool USpeechRecognizer::SetNoContext(bool bNoContext)
{
	return Thread->SetNoContext(bNoContext);
}

bool USpeechRecognizer::SetSingleSegment(bool bSingleSegment)
{
	return Thread->SetSingleSegment(bSingleSegment);
}

bool USpeechRecognizer::SetMaxTokens(int32 Value)
{
	return Thread->SetMaxTokens(Value);
}

bool USpeechRecognizer::SetSpeedUp(bool bSpeedUp)
{
	return Thread->SetSpeedUp(bSpeedUp);
}

bool USpeechRecognizer::SetAudioContextSize(int32 Value)
{
	return Thread->SetAudioContextSize(Value);
}

bool USpeechRecognizer::SetTemperatureToIncrease(float Value)
{
	return Thread->SetTemperatureToIncrease(Value);
}

bool USpeechRecognizer::SetEntropyThreshold(float Value)
{
	return Thread->SetEntropyThreshold(Value);
}

bool USpeechRecognizer::SetSuppressBlank(bool Value)
{
	return Thread->SetSuppressBlank(Value);
}

bool USpeechRecognizer::SetSuppressNonSpeechTokens(bool Value)
{
	return Thread->SetSuppressNonSpeechTokens(Value);
}

bool USpeechRecognizer::SetBeamSize(int32 Value)
{
	return Thread->SetBeamSize(Value);
}

bool USpeechRecognizer::SetInitialPrompt(const FString& Value)
{
	return Thread->SetInitialPrompt(Value);
}

bool USpeechRecognizer::SetGPUAcceleration(bool bUseGPU)
{
	return Thread->SetGPUAcceleration(bUseGPU);
}

bool USpeechRecognizer::SetGPUDeviceID(int32 Value)
{
	return Thread->SetGPUDeviceID(Value);
}

ESpeechRecognizerLanguage USpeechRecognizer::GetDetectedLanguage() const
{
	return Thread->GetDetectedLanguage();
}
