#include "UnrealAny.h"
#include "Misc/AutomationTest.h"
#include "Engine/Texture2D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnyTest, "Example.Any", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAnyTest::RunTest(const FString& Parameters)
{
	{
		FAny Any = true;
		UTEST_EQUAL("Test Any Bool", Any.Get<bool>(), true);
	}
	{
		FAny Any = 1;
		UTEST_EQUAL("Test Any Int", Any.Get<int32>() == 1, true);
	}

	{
		FAny Any = FVector2D::ZeroVector;
		UTEST_EQUAL("Test Any Vector2D", Any.Get<FVector2D>() == FVector2D::ZeroVector, true);
	}

	{
		FAny Any = FIntVector::ZeroValue;
		UTEST_EQUAL("Test Any IntVector", Any.Get<FIntVector>() == FIntVector::ZeroValue, true);
	}

	{
		auto Now = FDateTime::Now();
		FAny Any = Now;
		UTEST_EQUAL("Test Any DataTime", Any.Get<FDateTime>() == Now, true);
	}

	{
		FAny Any = FQuat::Identity;
		UTEST_EQUAL("Test Any Quat", Any.Get<FQuat>() == FQuat::Identity, true);
	}

	{
		FAny Any = ECollisionChannel::ECC_Visibility;
		UTEST_EQUAL("Test Any Enum CollisionChannel", Any.Get<ECollisionChannel>() == ECollisionChannel::ECC_Visibility, true);
	}

	{
		FAny Any = EOrientation::Orient_Vertical;
		UTEST_EQUAL("Test Any Enum Orientation", Any.Get<EOrientation>() == EOrientation::Orient_Vertical, true);
	}

	{
		auto Tex = UTexture2D::CreateTransient(100, 100);
		FAny Any = Tex;
		UTEST_EQUAL("Test Any Object Texture2D", Any.Get<UTexture2D*>() == Tex, true);
	}

	{
		auto Cls = UStaticMesh::StaticClass();
		FAny Any = Cls;
		UTEST_EQUAL("Test Any Class StaticMesh", Any.Get<UClass*>() == Cls, true);
	}

	/*{
		auto Margin = FMargin();
		FAny Any = Margin;
		UTEST_EQUAL("Test Any Margin", Any.Get<FMargin>() == Margin, true);
	}*/

	return true;
}