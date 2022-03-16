#include "UnrealAny.h"
#include "Misc/AutomationTest.h"
#include "UnrealAnyFunctionLibrary.h"

//static const FGuid AnyVerGUID(0xB7B84C63, 0x4AD98D40, 0x078FB9A3, 0x7E4E145F);
//FCustomVersionRegistration GRegisterAnyCustomVersion(AnyVerGUID, FAny::VerType::LatestVersion, TEXT("AnyVer"));
//
//
//bool FAny::Serialize(FArchive& Ar)
//{
//	Ar.UsingCustomVersion(AnyVerGUID);
//	if (Ar.IsLoading()) {
//		FName TempTypeName;
//		Ar << TempTypeName;
//
//	}
//	else if (Ar.IsSaving()) {
//		FName TempTypeName = TypeFName();
//		Ar << TempTypeName;
//	}
//	
//	return true;
//}



IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnyTest, "Example.Any", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAnyTest::RunTest(const FString& Parameters)
{
	{
		FAny Any = nullptr;
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