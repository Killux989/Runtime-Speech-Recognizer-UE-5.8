// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpeechRecognizerTypes.h"
#include "SpeechRecognizerStringUtils.generated.h"

/**
 * Utility class for string-related operations used by the speech recognizer.
 */
UCLASS()
class RUNTIMESPEECHRECOGNIZER_API USpeechRecognizerStringUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Computes the Levenshtein similarity between two strings
	 * This method is useful for recognizing spoken commands
	 * 
	 * @param BaseString The base string to compare against
	 * @param CandidateString The candidate string to compare
	 * @return The similarity between 0 and 1, where 1 is a perfect match
	 */
	UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer|String")
	static float ComputeLevenshteinSimilarity(const FString& BaseString, const FString& CandidateString);

	/**
	 * Finds the voice command that best matches any part of the recognized text, e.g. "open door" inside "hey, please open the door now"
	 * The text is lowercased, Whisper tags such as [BLANK_AUDIO] or (music) and punctuation are removed, then every
	 * window of words with the same length as the command is fuzzy-compared using Levenshtein similarity
	 *
	 * @param RecognizedText The text from On Recognized Text Segment
	 * @param Commands The commands (keywords or short phrases) to look for
	 * @param Threshold The minimum similarity (0-1) to accept a match. 0.7-0.8 tolerates small recognition mistakes
	 * @param OutCommandIndex Index of the best matching command in Commands, or -1 if none reached the threshold
	 * @param OutSimilarity Similarity of the best match (0-1)
	 * @return True if a command reached the threshold
	 */
	UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer|String")
	static bool FindBestMatchingCommand(const FString& RecognizedText, const TArray<FString>& Commands, int32& OutCommandIndex, float& OutSimilarity, float Threshold = 0.75f);

	/**
	 * Returns the Whisper language code, e.g. En -> "en"
	 */
	UFUNCTION(BlueprintPure, Category = "SpeechRecognizer|Language")
	static FString GetLanguageCode(ESpeechRecognizerLanguage Language);

	/**
	 * Returns the full language name, e.g. En -> "English". Works in packaged builds
	 */
	UFUNCTION(BlueprintPure, Category = "SpeechRecognizer|Language")
	static FString GetLanguageFullName(ESpeechRecognizerLanguage Language);

	/** Lowercases the text, strips Whisper tags ([..], (..), *..*) and punctuation, and splits it into words */
	static TArray<FString> NormalizeToWords(const FString& Text);
};
