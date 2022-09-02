#include "UnrealAny.h"
#include "Misc/AutomationTest.h"
#include "Engine/Texture2D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnyTest, "Example.Any", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAnyTest::RunTest(const FString& Parameters)
{
	auto Object = UTexture2D::CreateTransient(32, 32);

	{
		FAny Any = Object;
		UTEST_EQUAL("Test Object", Any.IsValid(), true);
	}
	{
		FAny Any = 0;
		UTEST_EQUAL("Test Value", Any.IsValid(), true);
	}

	return true;
}