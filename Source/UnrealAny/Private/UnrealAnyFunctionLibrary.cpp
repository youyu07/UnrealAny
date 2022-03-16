#include "UnrealAnyFunctionLibrary.h"
#include "BlueprintAnyArrayMaker.h"

FName UUnrealAnyFunctionLibrary::GetAnyType(const FAny& Any)
{
	return Any.TypeFName();
}


FAny UUnrealAnyFunctionLibrary::MakeAny(int32 Value)
{
	check(0);
	return FAny();
}

void UUnrealAnyFunctionLibrary::CastAny(const FAny& Any, int32& Value)
{
	check(0)
}


template<typename T, typename PropertyType> static T GetPropertyValue(FProperty* Property, uint8* Address)
{
	return CastField<PropertyType>(Property)->GetPropertyValue(Address);
}

template<typename T> static T GetStructPropertyValue(UScriptStruct* Struct, uint8* Address)
{
	T Value = T();
	Struct->CopyScriptStruct(&Value, Address);
	return MoveTemp(Value);
}

template<typename PropertyType> static UObject* GetObjectPropertyValue(FProperty* Property, uint8* Address)
{
	return CastField<PropertyType>(Property)->GetObjectPropertyValue(Address);
}

struct FBlueprintAnyMap : public FBlueprintAnyBase
{
	FProperty* KeyProp;
	FProperty* ValueProp;
	FScriptMapLayout MapLayout;

	TScriptSet<FDefaultSetAllocator> Pairs;
	TArray<uint8> Data;

	FBlueprintAnyMap(FMapProperty* Property, uint8* Address)
		: KeyProp(Property->KeyProp) 
		, ValueProp(Property->ValueProp)
		, MapLayout(Property->MapLayout)
	{
		Data.SetNumZeroed(Property->GetSize());
		auto ScriptMap = Property->GetPropertyValuePtr(Address);

		
	};


	virtual const void* GetData() const override
	{
		return Data.GetData();
	}
};


static FAny Generic_Any(uint8* Address, FProperty* Property)
{
	auto Id = Property->GetClass()->GetId();
	switch (Id)
	{
	case CASTCLASS_FInt8Property:		return GetPropertyValue<int8, FInt8Property>(Property, Address);
	case CASTCLASS_FByteProperty:		return GetPropertyValue<uint8, FByteProperty>(Property, Address);
	case CASTCLASS_FIntProperty:		return GetPropertyValue<int32, FIntProperty>(Property, Address);
	case CASTCLASS_FFloatProperty:		return GetPropertyValue<float, FFloatProperty>(Property, Address);
	case CASTCLASS_FUInt64Property:		return GetPropertyValue<uint64, FUInt64Property>(Property, Address);
	case CASTCLASS_FUInt32Property:		return GetPropertyValue<uint32, FUInt32Property>(Property, Address);
	case CASTCLASS_FInterfaceProperty:	return CastField<FInterfaceProperty>(Property)->GetPropertyValue_InContainer(Address);
	case CASTCLASS_FClassProperty:		return Cast<UClass>(GetObjectPropertyValue<FClassProperty>(Property, Address));
	case CASTCLASS_FNameProperty:		return GetPropertyValue<FName, FNameProperty>(Property, Address);
	case CASTCLASS_FStrProperty:		return GetPropertyValue<FString, FStrProperty>(Property, Address);
	case CASTCLASS_FObjectProperty:		return GetObjectPropertyValue<FObjectProperty>(Property, Address);
	case CASTCLASS_FBoolProperty:		return GetPropertyValue<bool, FBoolProperty>(Property, Address);
	case CASTCLASS_FUInt16Property:		return GetPropertyValue<uint16, FUInt16Property>(Property, Address);
	case CASTCLASS_FInt64Property:		return GetPropertyValue<int64, FInt64Property>(Property, Address);
	case CASTCLASS_FWeakObjectProperty:	return MakeWeakObjectPtr(GetObjectPropertyValue<FObjectProperty>(Property, Address));
	case CASTCLASS_FLazyObjectProperty:	return TLazyObjectPtr<UObject>(GetObjectPropertyValue<FObjectProperty>(Property, Address));
	case CASTCLASS_FSoftObjectProperty:	return TSoftObjectPtr<UObject>(GetObjectPropertyValue<FObjectProperty>(Property, Address));
	case CASTCLASS_FTextProperty:		return GetPropertyValue<FText, FTextProperty>(Property, Address);
	case CASTCLASS_FInt16Property:		return GetPropertyValue<int16, FInt16Property>(Property, Address);
	case CASTCLASS_FDoubleProperty:		return GetPropertyValue<double, FDoubleProperty>(Property, Address);
	case CASTCLASS_FSoftClassProperty:	return TSoftClassPtr<UClass>(Cast<UClass>(GetObjectPropertyValue<FClassProperty>(Property, Address)));
	case CASTCLASS_FStructProperty:
	{
		auto Struct = CastField<FStructProperty>(Property)->Struct;
		if (auto Name = Struct->GetFName().ToEName()) {
			switch (*Name)
			{
			case NAME_BoxSphereBounds:		return GetStructPropertyValue<FBoxSphereBounds>(Struct, Address);
			case NAME_Sphere:				return GetStructPropertyValue<FSphere>(Struct, Address);
			case NAME_Box:					return GetStructPropertyValue<FBox>(Struct, Address);
			case NAME_Vector2D:				return GetStructPropertyValue<FVector2D>(Struct, Address);
			case NAME_IntRect:				return GetStructPropertyValue<FIntRect>(Struct, Address);
			case NAME_IntPoint:				return GetStructPropertyValue<FIntPoint>(Struct, Address);
			case NAME_Vector4:				return GetStructPropertyValue<FVector4>(Struct, Address);
			case NAME_Vector:				return GetStructPropertyValue<FVector>(Struct, Address);
			case NAME_Rotator:				return GetStructPropertyValue<FRotator>(Struct, Address);
			case NAME_Color:				return GetStructPropertyValue<FColor>(Struct, Address);
			case NAME_Plane:				return GetStructPropertyValue<FPlane>(Struct, Address);
			case NAME_Matrix:				return GetStructPropertyValue<FMatrix>(Struct, Address);
			case NAME_LinearColor:			return GetStructPropertyValue<FLinearColor>(Struct, Address);
			case NAME_Quat:					return GetStructPropertyValue<FQuat>(Struct, Address);
			case NAME_Transform:			return GetStructPropertyValue<FTransform>(Struct, Address);
			case NAME_Timer:				return GetStructPropertyValue<FTimer>(Struct, Address);
			}
		}
		if (Struct->GetFName() == "Any") {
			return GetStructPropertyValue<FAny>(Struct, Address);
		}
		auto BlueprintStruct = FBlueprintAnyStruct(Struct);
		Struct->CopyScriptStruct(BlueprintStruct.Data.GetData(), Address);
		return BlueprintStruct;
	}
	case CASTCLASS_FArrayProperty: return FBlueprintAnyArrayMaker(CastField<FArrayProperty>(Property), Address).GetAny();
	case CASTCLASS_FMapProperty:
	{
		break;
	}
	case CASTCLASS_FSetProperty:
		break;
	case CASTCLASS_FEnumProperty:
	{
		auto EnumProperty = CastField<FEnumProperty>(Property);
		auto Value = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(Address);
		return FBlueprintAnyEnum(EnumProperty->GetEnum(), Value);
	}
	}

	check(0);
	return FAny();
}


#define WRITE_PROPERTY(Type) {auto Value = Any.Cast<Type>(); Property->CopySingleValue(Address, &Value); return; }

static void Generic_CastAny(const FAny& Any, uint8*& Address, FProperty* Property)
{
	auto Id = Property->GetClass()->GetId();
	switch (Id)
	{
	case CASTCLASS_FInt8Property:		WRITE_PROPERTY(int8);
	case CASTCLASS_FByteProperty:		WRITE_PROPERTY(uint8);
	case CASTCLASS_FIntProperty:		WRITE_PROPERTY(int32);
	case CASTCLASS_FFloatProperty:		WRITE_PROPERTY(float);
	case CASTCLASS_FUInt64Property:		WRITE_PROPERTY(uint64);
	case CASTCLASS_FUInt32Property:		WRITE_PROPERTY(uint32);
	case CASTCLASS_FInterfaceProperty:	WRITE_PROPERTY(FScriptInterface);
	case CASTCLASS_FClassProperty:		WRITE_PROPERTY(UClass*);
	case CASTCLASS_FNameProperty:		WRITE_PROPERTY(FName);
	case CASTCLASS_FStrProperty:		WRITE_PROPERTY(FString);
	case CASTCLASS_FObjectProperty:		WRITE_PROPERTY(UObject*);
	case CASTCLASS_FBoolProperty:		WRITE_PROPERTY(bool);
	case CASTCLASS_FUInt16Property:		WRITE_PROPERTY(uint16);
	case CASTCLASS_FInt64Property:		WRITE_PROPERTY(int64);
	case CASTCLASS_FWeakObjectProperty:	WRITE_PROPERTY(TWeakObjectPtr<UObject>);
	case CASTCLASS_FLazyObjectProperty:	WRITE_PROPERTY(TLazyObjectPtr<UObject>);
	case CASTCLASS_FSoftObjectProperty:	WRITE_PROPERTY(TSoftObjectPtr<UObject>);
	case CASTCLASS_FTextProperty:		WRITE_PROPERTY(FText);
	case CASTCLASS_FInt16Property:		WRITE_PROPERTY(int16);
	case CASTCLASS_FDoubleProperty:		WRITE_PROPERTY(double);
	case CASTCLASS_FSoftClassProperty:	WRITE_PROPERTY(TSoftClassPtr<UClass>);
	case CASTCLASS_FStructProperty:
	{
		auto Struct = CastField<FStructProperty>(Property)->Struct;
		if (auto Name = Struct->GetFName().ToEName()) {
			switch (*Name)
			{
			case NAME_BoxSphereBounds:		WRITE_PROPERTY(FBoxSphereBounds);
			case NAME_Sphere:				WRITE_PROPERTY(FSphere);
			case NAME_Box:					WRITE_PROPERTY(FBox);
			case NAME_Vector2D:				WRITE_PROPERTY(FVector2D);
			case NAME_IntRect:				WRITE_PROPERTY(FIntRect);
			case NAME_IntPoint:				WRITE_PROPERTY(FIntPoint);
			case NAME_Vector4:				WRITE_PROPERTY(FVector4);
			case NAME_Vector:				WRITE_PROPERTY(FVector);
			case NAME_Rotator:				WRITE_PROPERTY(FRotator);
			case NAME_Color:				WRITE_PROPERTY(FColor);
			case NAME_Plane:				WRITE_PROPERTY(FPlane);
			case NAME_Matrix:				WRITE_PROPERTY(FMatrix);
			case NAME_LinearColor:			WRITE_PROPERTY(FLinearColor);
			case NAME_Quat:					WRITE_PROPERTY(FQuat);
			case NAME_Transform:			WRITE_PROPERTY(FTransform);
			case NAME_Timer:				WRITE_PROPERTY(FTimer);
			}
		}
		auto Value = Any.Cast<FBlueprintAnyStruct>();
		check(Value.Struct == Struct);
		Property->CopySingleValue(Address, Value.Data.GetData());
		return;
	}
	case CASTCLASS_FArrayProperty:			{FBlueprintAnyArrayMaker(CastField<FArrayProperty>(Property), Address).CastTo(Any); return; }
	case CASTCLASS_FMapProperty:
		break;
	case CASTCLASS_FSetProperty:
		break;
	case CASTCLASS_FEnumProperty:
	{
		auto Value = Any.Cast<FBlueprintAnyEnum>();
		auto EnumProperty = CastField<FEnumProperty>(Property);
		EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Address, Value.Value);
		return;
	}
	}
	check(0);
}

DEFINE_FUNCTION(UUnrealAnyFunctionLibrary::execMakeAny)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FProperty>(nullptr);
	auto Property = Stack.MostRecentProperty;

	P_FINISH;
	P_NATIVE_BEGIN;
	auto Any = Generic_Any(Stack.MostRecentPropertyAddress, Property);
	*(FAny*)Z_Param__Result = Any;
	P_NATIVE_END;
}


DEFINE_FUNCTION(UUnrealAnyFunctionLibrary::execCastAny)
{
	P_GET_STRUCT_REF(FAny, Any);
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FProperty>(NULL);
	P_FINISH;

	P_NATIVE_BEGIN;
	Generic_CastAny(Any, Stack.MostRecentPropertyAddress, Stack.MostRecentProperty);
	P_NATIVE_END;
}