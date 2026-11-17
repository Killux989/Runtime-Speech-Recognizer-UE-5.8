// Georgy Treshchev 2024.

#include "SpeechRecognizerStringUtils.h"

#include "SpeechRecognizerDefines.h"
#include "Math/UnrealMathUtility.h"
#include "Algo/LevenshteinDistance.h"
#include "SpeechRecognizerPrivate.h"

float USpeechRecognizerStringUtils::ComputeLevenshteinSimilarity(const FString& BaseString, const FString& CandidateString)
{
	// Convert strings to lowercase and trim whitespace
	const FString BaseStringTrimmed = BaseString.TrimStartAndEnd().ToLower();
	const FString CandidateStringTrimmed = CandidateString.TrimStartAndEnd().ToLower();

	// If candidate string is empty, return perfect match
	if (CandidateStringTrimmed.IsEmpty())
	{
		return 1.0f;
	}

	// Compute similarity using Levenshtein distance
	const float WorstCase = FMath::Max3(CandidateStringTrimmed.Len(), BaseStringTrimmed.Len(), 1);
	const float Similarity = 1.0f - (Algo::LevenshteinDistance(BaseStringTrimmed, CandidateStringTrimmed) / WorstCase);

	UE_LOG(LogRuntimeSpeechRecognizer, Verbose, TEXT("Levenshtein similarity between '%s' and '%s' is %f"), *BaseStringTrimmed, *CandidateStringTrimmed, Similarity);
	return Similarity;
}

TArray<FString> USpeechRecognizerStringUtils::NormalizeToWords(const FString& Text)
{
	FString Clean;
	Clean.Reserve(Text.Len());
	TCHAR SkipUntil = 0;
	for (const TCHAR Char : Text.ToLower())
	{
		if (SkipUntil)
		{
			if (Char == SkipUntil)
			{
				SkipUntil = 0;
			}
			continue;
		}
		if (Char == TEXT('[')) { SkipUntil = TEXT(']'); continue; }
		if (Char == TEXT('(')) { SkipUntil = TEXT(')'); continue; }
		if (Char == TEXT('*')) { SkipUntil = TEXT('*'); continue; }
		Clean.AppendChar(FChar::IsAlnum(Char) || Char == TEXT('\'') ? Char : TEXT(' '));
	}

	TArray<FString> Words;
	Clean.ParseIntoArrayWS(Words);
	return Words;
}

bool USpeechRecognizerStringUtils::FindBestMatchingCommand(const FString& RecognizedText, const TArray<FString>& Commands, int32& OutCommandIndex, float& OutSimilarity, float Threshold)
{
	OutCommandIndex = -1;
	OutSimilarity = 0.0f;

	const TArray<FString> TextWords = NormalizeToWords(RecognizedText);
	if (TextWords.Num() == 0)
	{
		return false;
	}

	int32 BestIndex = -1;
	float BestSimilarity = 0.0f;
	for (int32 CommandIndex = 0; CommandIndex < Commands.Num(); ++CommandIndex)
	{
		const TArray<FString> CommandWords = NormalizeToWords(Commands[CommandIndex]);
		if (CommandWords.Num() == 0)
		{
			continue;
		}
		const FString Command = FString::Join(CommandWords, TEXT(" "));
		const FString CommandNoSpaces = Command.Replace(TEXT(" "), TEXT(""));

		// Slide a window of the command's word count (and one more, compared without spaces, so "fire ball" still matches "fireball")
		for (int32 WindowSize = FMath::Min(CommandWords.Num(), TextWords.Num()); WindowSize <= FMath::Min(CommandWords.Num() + 1, TextWords.Num()); ++WindowSize)
		{
			for (int32 Start = 0; Start + WindowSize <= TextWords.Num(); ++Start)
			{
				const TArrayView<const FString> WindowWords = TArrayView<const FString>(TextWords).Slice(Start, WindowSize);
				const float Similarity = FMath::Max(
					ComputeLevenshteinSimilarity(FString::Join(WindowWords, TEXT(" ")), Command),
					ComputeLevenshteinSimilarity(FString::Join(WindowWords, TEXT("")), CommandNoSpaces));
				if (Similarity > BestSimilarity)
				{
					BestSimilarity = Similarity;
					BestIndex = CommandIndex;
				}
			}
		}

		// Multi-word commands: match each command word, in order, to its most similar text word, allowing filler words in between ("open the door")
		if (CommandWords.Num() > 1)
		{
			float SimilaritySum = 0.0f;
			int32 SearchFrom = 0;
			for (const FString& CommandWord : CommandWords)
			{
				float BestWordSimilarity = 0.0f;
				int32 BestWordIndex = INDEX_NONE;
				for (int32 TextIndex = SearchFrom; TextIndex < TextWords.Num(); ++TextIndex)
				{
					const float WordSimilarity = ComputeLevenshteinSimilarity(TextWords[TextIndex], CommandWord);
					if (WordSimilarity > BestWordSimilarity)
					{
						BestWordSimilarity = WordSimilarity;
						BestWordIndex = TextIndex;
					}
				}
				if (BestWordIndex == INDEX_NONE)
				{
					SimilaritySum = 0.0f;
					break;
				}
				SimilaritySum += BestWordSimilarity;
				SearchFrom = BestWordIndex + 1;
			}
			const float Similarity = SimilaritySum / CommandWords.Num();
			if (Similarity > BestSimilarity)
			{
				BestSimilarity = Similarity;
				BestIndex = CommandIndex;
			}
		}
	}

	OutSimilarity = BestSimilarity;
	if (BestIndex != INDEX_NONE && BestSimilarity >= Threshold)
	{
		OutCommandIndex = BestIndex;
		return true;
	}
	return false;
}

FString USpeechRecognizerStringUtils::GetLanguageCode(ESpeechRecognizerLanguage Language)
{
	return FString(UTF8_TO_TCHAR(EnumToString(Language)));
}

FString USpeechRecognizerStringUtils::GetLanguageFullName(ESpeechRecognizerLanguage Language)
{
	if (Language == ESpeechRecognizerLanguage::Auto)
	{
		return TEXT("Auto");
	}

	// Taken from whisper instead of the UENUM DisplayName, which is editor-only metadata
	const int32 LanguageId = whisper_lang_id(EnumToString(Language));
	const char* FullName = LanguageId >= 0 ? whisper_lang_str_full(LanguageId) : nullptr;
	if (!FullName)
	{
		return FString();
	}

	FString Result(UTF8_TO_TCHAR(FullName));
	if (Result.Len() > 0)
	{
		Result[0] = FChar::ToUpper(Result[0]);
	}
	return Result;
}
