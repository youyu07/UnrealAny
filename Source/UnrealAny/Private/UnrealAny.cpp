#include "UnrealAny.h"
#include "Misc/AutomationTest.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnyTest, "Example.Any", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)


bool FAnyTest::RunTest(const FString& Parameters)
{
	{
		FAny Any = nullptr;
		Any.Reset();
		UTEST_EQUAL("Test Nullptr", Any.HasValue(), false);
	}
	{
		auto Any = FAny(FVector::ZeroVector);
		UTEST_EQUAL("Test Cast Vector", Any.Cast<FVector>(), FVector::ZeroVector);
	}
	{
		FAny Any = FRotator::ZeroRotator;
		UTEST_EQUAL("Test Cast Rotator", Any.Cast<FRotator>(), FRotator::ZeroRotator);
	}
	{
		FAny Any = 0;
		UTEST_EQUAL("Test Any Type Name", Any.TypeName(), FString("int"));
		UTEST_EQUAL("Test Any Type Info", Any.Type(), typeid(int32));
	}

	return true;
}