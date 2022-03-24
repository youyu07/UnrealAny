#include "UnrealAny.h"
#include "Misc/AutomationTest.h"
#include "UnrealAnyFunctionLibrary.h"

#define REGISTER_ANY_TYPE(Type, Number) AnyTypes.Add(typeid(Type).name(), Number)


const TMap<FName, uint32>& FAny::GetAnyTypes()
{
	static TMap<FName, uint32> AnyTypes;
	if (AnyTypes.Num() == 0) {
		REGISTER_ANY_TYPE(bool,				0);
		REGISTER_ANY_TYPE(int32,			1);
		REGISTER_ANY_TYPE(float,			2);
		REGISTER_ANY_TYPE(double,			3);
		REGISTER_ANY_TYPE(FString,			4);
		REGISTER_ANY_TYPE(FName,			5);
		REGISTER_ANY_TYPE(FText,			6);
		REGISTER_ANY_TYPE(FVector,			7);
		REGISTER_ANY_TYPE(FVector2D,		8);
		REGISTER_ANY_TYPE(FRotator,			9);
		REGISTER_ANY_TYPE(FTransform,		10);
		REGISTER_ANY_TYPE(FLinearColor,		11);
		REGISTER_ANY_TYPE(UClass*,			12);
		REGISTER_ANY_TYPE(UObject*,			13);
	}
	return AnyTypes;
}


static const FGuid AnyVerGUID(0xB7B84C63, 0x4AD98D40, 0x078FB9A3, 0x7E4E145F);
FCustomVersionRegistration GRegisterAnyCustomVersion(AnyVerGUID, FAny::EVersionType::LatestVersion, TEXT("AnyVersion"));

#define IMPL_LOAD_ANY(Type) {Type Value; Ar << Value; *this = Value; break;}
#define IMPL_SAVE_ANY(Type) {auto Value = CastUnchecked<Type>(); Ar << Value; break; }

#define IMPL_SWITCH_ANY_TYPE(Macro) \
switch (*Type) \
{ \
	case 0: Macro(bool); \
	case 1: Macro(int32); \
	case 2: Macro(float); \
	case 3: Macro(double); \
	case 4: Macro(FString); \
	case 5: Macro(FName); \
	case 6: Macro(FText); \
	case 7: Macro(FVector); \
	case 8: Macro(FVector2D); \
	case 9: Macro(FRotator); \
	case 10: Macro(FTransform); \
	case 11: Macro(FLinearColor); \
	case 12: Macro(UClass*); \
	case 13: Macro(UObject*); \
}

bool FAny::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(AnyVerGUID);
	if (Ar.IsLoading()) {
		FName TempTypeName;
		Ar << TempTypeName;
		if (auto Type = GetAnyTypes().Find(TempTypeName)) {
			IMPL_SWITCH_ANY_TYPE(IMPL_LOAD_ANY)
		}
	}
	else if (Ar.IsSaving()) {
		FName TempTypeName = TypeFName();
		Ar << TempTypeName;
		if (auto Type = GetAnyTypes().Find(TempTypeName)) {
			IMPL_SWITCH_ANY_TYPE(IMPL_SAVE_ANY)
		}
	}
	
	return true;
}



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