#include "UnrealAny.h"
#include "Misc/AutomationTest.h"
#include "Engine/Texture2D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnyTest, "Example.Any", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAnyTest::RunTest(const FString& Parameters)
{
	{
		FAny Any = true;
		UTEST_EQUAL("Test Value", Any.Get<bool>(), true);
	}
	{
		FAny Any = 1;
		UTEST_EQUAL("Test Value", Any.Get<int32>() == 1, true);
	}

	{
		FAny Any = FVector2D::ZeroVector;
		UTEST_EQUAL("Test Value", Any.Get<FVector2D>() == FVector2D::ZeroVector, true);
	}

	{
		FAny Any = FIntVector();
		UTEST_EQUAL("Test Value", Any.Get<FIntVector>() == FIntVector(), true);
	}

	{
		auto Now = FDateTime::Now();
		FAny Any = Now;
		UTEST_EQUAL("Test Value", Any.Get<FDateTime>() == Now, true);
	}

	{
		FAny Any = FQuat();
		UTEST_EQUAL("Test Value", Any.Get<FQuat>() == FQuat(), true);
	}

	{
		FAny Any = FPlane();
		UTEST_EQUAL("Test Value", Any.Get<FPlane>() == FPlane(), true);
	}

	{
		FAny Any = ECollisionChannel::ECC_Visibility;
		UTEST_EQUAL("Test Value", Any.Get<ECollisionChannel>() == ECollisionChannel::ECC_Visibility, true);
	}

	{
		FAny Any = EOrientation::Orient_Vertical;
		UTEST_EQUAL("Test Value", Any.Get<EOrientation>() == EOrientation::Orient_Vertical, true);
	}

	{
		auto Tex = UTexture2D::CreateTransient(100, 100);
		FAny Any = Tex;
		UTEST_EQUAL("Test Value", Any.Get<UTexture2D*>() == Tex, true);
	}

	return true;
}