// Georgy Treshchev 2024.

#include "Misc/AutomationTest.h"
#include "SpeechRecognizerStringUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpeechRecognizerCommandMatchTest, "RuntimeSpeechRecognizer.StringUtils.FindBestMatchingCommand",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FSpeechRecognizerCommandMatchTest::RunTest(const FString& Parameters)
{
	int32 Index;
	float Similarity;
	const TArray<FString> Commands = {TEXT("close door"), TEXT("Open door")};

	TestTrue(TEXT("Phrase inside a sentence"), USpeechRecognizerStringUtils::FindBestMatchingCommand(TEXT(" Hey, please open the door now."), Commands, Index, Similarity));
	TestEqual(TEXT("Picks 'open door'"), Index, 1);
	TestTrue(TEXT("High similarity"), Similarity >= 0.75f);

	TestTrue(TEXT("Tolerates a misrecognized word"), USpeechRecognizerStringUtils::FindBestMatchingCommand(TEXT("[MUSIC] close the dor!"), Commands, Index, Similarity, 0.7f));
	TestEqual(TEXT("Picks 'close door'"), Index, 0);

	TestFalse(TEXT("Blank audio tag is ignored"), USpeechRecognizerStringUtils::FindBestMatchingCommand(TEXT("[BLANK_AUDIO]"), Commands, Index, Similarity));
	TestEqual(TEXT("No index"), Index, -1);

	const TArray<FString> Spells = {TEXT("fireball"), TEXT("heal")};
	TestTrue(TEXT("Single word in a sentence"), USpeechRecognizerStringUtils::FindBestMatchingCommand(TEXT("Cast Fireball!"), Spells, Index, Similarity, 1.0f));
	TestEqual(TEXT("Picks 'fireball'"), Index, 0);
	TestTrue(TEXT("Split word still matches"), USpeechRecognizerStringUtils::FindBestMatchingCommand(TEXT("fire ball"), Spells, Index, Similarity, 1.0f));
	TestEqual(TEXT("Picks 'fireball' from split"), Index, 0);
	TestFalse(TEXT("Strict single word rejects a different word"), USpeechRecognizerStringUtils::FindBestMatchingCommand(TEXT("I feel great"), Spells, Index, Similarity, 0.85f));

	TestFalse(TEXT("Unrelated speech"), USpeechRecognizerStringUtils::FindBestMatchingCommand(TEXT("what a lovely weather today"), Commands, Index, Similarity));

	TestEqual(TEXT("Language code"), USpeechRecognizerStringUtils::GetLanguageCode(ESpeechRecognizerLanguage::En), FString(TEXT("en")));
	TestEqual(TEXT("Language full name"), USpeechRecognizerStringUtils::GetLanguageFullName(ESpeechRecognizerLanguage::De), FString(TEXT("German")));
	return true;
}

#endif
