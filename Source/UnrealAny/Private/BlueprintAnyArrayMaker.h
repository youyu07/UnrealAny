#pragma once

#include "CoreMinimal.h"
#include "BlueprintAnyTypes.h"

#define IMPL_MAKE_ANY_ARRAY(Type) \
TArray<Type> Arr; \
for (int32 i = 0; i < Array->Num(); i++) \
{ \
	Arr.Add(*((Type*)Array->GetData() + i)); \
} \
return Arr;

#define IMPL_CAST_ANY_ARRAY(Type) \
auto Array = Any.Cast<TArray<Type>>(); \
Helper.Resize(Array.Num()); \
FMemory::Memcpy(Helper.GetRawPtr(0), Array.GetData(), sizeof(Type) * Array.Num()); \
return;

struct FBlueprintAnyArrayMaker
{
	FArrayProperty* Property;
	uint8*& Address;

	FBlueprintAnyArrayMaker(FArrayProperty* InProperty, uint8*& InAddress) : Property(InProperty), Address(InAddress) {}
	FAny GetAny()
	{
		auto Id = Property->Inner->GetClass()->GetId();
		auto Array = Property->GetPropertyValuePtr(Address);
		switch (Id)
		{
		case CASTCLASS_FInt8Property:		{ IMPL_MAKE_ANY_ARRAY(int8) }
		case CASTCLASS_FByteProperty:		{ IMPL_MAKE_ANY_ARRAY(uint8) }
		case CASTCLASS_FIntProperty:		{ IMPL_MAKE_ANY_ARRAY(int32) }
		case CASTCLASS_FFloatProperty:		{ IMPL_MAKE_ANY_ARRAY(float) }
		case CASTCLASS_FUInt64Property:		{ IMPL_MAKE_ANY_ARRAY(uint64) }
		case CASTCLASS_FUInt32Property:		{ IMPL_MAKE_ANY_ARRAY(uint32) }
		case CASTCLASS_FInterfaceProperty:	{ IMPL_MAKE_ANY_ARRAY(FScriptInterface) }
		case CASTCLASS_FClassProperty:		{ IMPL_MAKE_ANY_ARRAY(UClass*) }
		case CASTCLASS_FNameProperty:		{ IMPL_MAKE_ANY_ARRAY(FName) }
		case CASTCLASS_FStrProperty:		{ IMPL_MAKE_ANY_ARRAY(FString) }
		case CASTCLASS_FObjectProperty:		{ IMPL_MAKE_ANY_ARRAY(UObject*) }
		case CASTCLASS_FBoolProperty:		{ IMPL_MAKE_ANY_ARRAY(bool) }
		case CASTCLASS_FUInt16Property:		{ IMPL_MAKE_ANY_ARRAY(uint16) }
		case CASTCLASS_FInt64Property:		{ IMPL_MAKE_ANY_ARRAY(int64) }
		case CASTCLASS_FWeakObjectProperty: { IMPL_MAKE_ANY_ARRAY(TWeakObjectPtr<UObject>) }
		case CASTCLASS_FLazyObjectProperty: { IMPL_MAKE_ANY_ARRAY(TLazyObjectPtr<UObject>) }
		case CASTCLASS_FSoftObjectProperty: { IMPL_MAKE_ANY_ARRAY(TSoftObjectPtr<UObject>) }
		case CASTCLASS_FTextProperty:		{ IMPL_MAKE_ANY_ARRAY(FText) }
		case CASTCLASS_FInt16Property:		{ IMPL_MAKE_ANY_ARRAY(int16) }
		case CASTCLASS_FDoubleProperty:		{ IMPL_MAKE_ANY_ARRAY(double) }
		case CASTCLASS_FSoftClassProperty:	{ IMPL_MAKE_ANY_ARRAY(TSoftClassPtr<UClass>) }
		case CASTCLASS_FStructProperty:
		{
			auto Struct = CastField<FStructProperty>(Property->Inner)->Struct;
			if (auto Name = Struct->GetFName().ToEName()) {
				switch (*Name)
				{
				case NAME_BoxSphereBounds:	{ IMPL_MAKE_ANY_ARRAY(FBoxSphereBounds) }
				case NAME_Sphere:			{ IMPL_MAKE_ANY_ARRAY(FSphere) }
				case NAME_Box:				{ IMPL_MAKE_ANY_ARRAY(FBox) }
				case NAME_Vector2D:			{ IMPL_MAKE_ANY_ARRAY(FVector2D) }
				case NAME_IntRect:			{ IMPL_MAKE_ANY_ARRAY(FIntRect) }
				case NAME_IntPoint:			{ IMPL_MAKE_ANY_ARRAY(FIntPoint) }
				case NAME_Vector4:			{ IMPL_MAKE_ANY_ARRAY(FVector4) }
				case NAME_Vector:			{ IMPL_MAKE_ANY_ARRAY(FVector) }
				case NAME_Rotator:			{ IMPL_MAKE_ANY_ARRAY(FRotator) }
				case NAME_Color:			{ IMPL_MAKE_ANY_ARRAY(FColor) }
				case NAME_Plane:			{ IMPL_MAKE_ANY_ARRAY(FPlane) }
				case NAME_Matrix:			{ IMPL_MAKE_ANY_ARRAY(FMatrix) }
				case NAME_LinearColor:		{ IMPL_MAKE_ANY_ARRAY(FLinearColor) }
				case NAME_Quat:				{ IMPL_MAKE_ANY_ARRAY(FQuat) }
				case NAME_Transform:		{ IMPL_MAKE_ANY_ARRAY(FTransform) }
				case NAME_Timer:			{ IMPL_MAKE_ANY_ARRAY(FTimer) }
				}
			}
			if (Struct->GetFName() == "Any") { IMPL_MAKE_ANY_ARRAY(FAny) }
			TArray<FBlueprintAnyStruct> Arr;
			for (int32 i = 0; i < Array->Num(); i++)
			{
				auto BlueprintStruct = FBlueprintAnyStruct(Struct);
				Struct->CopyScriptStruct(BlueprintStruct.Data.GetData(), (uint8*)Array->GetData() + Struct->PropertiesSize * i);
				Arr.Add(BlueprintStruct);
			}
			return Arr;
		}
		case CASTCLASS_FEnumProperty:
		{
			auto EnumProperty = CastField<FEnumProperty>(Property->Inner);
			TArray<FBlueprintAnyEnum> Arr;
			for (int32 i = 0; i < Array->Num(); i++)
			{
				auto ValuePtr = (uint8*)Array->GetData() + EnumProperty->ElementSize * i;
				auto BlueprintEnum = FBlueprintAnyEnum(EnumProperty->GetEnum(), *(int64*)ValuePtr);
				Arr.Add(BlueprintEnum);
			}
			return Arr;
		}
		}

		check(0);
		return FAny();
	}

	void CastTo(const FAny& Any)
	{
		auto Helper = FScriptArrayHelper(Property, Address);
		auto Id = Property->Inner->GetClass()->GetId();
		switch (Id)
		{
		case CASTCLASS_FInt8Property:		{ IMPL_CAST_ANY_ARRAY(int8) }
		case CASTCLASS_FByteProperty:		{ IMPL_CAST_ANY_ARRAY(uint8) }
		case CASTCLASS_FIntProperty:		{ IMPL_CAST_ANY_ARRAY(int32) }
		case CASTCLASS_FFloatProperty:		{ IMPL_CAST_ANY_ARRAY(float) }
		case CASTCLASS_FUInt64Property:		{ IMPL_CAST_ANY_ARRAY(uint64) }
		case CASTCLASS_FUInt32Property:		{ IMPL_CAST_ANY_ARRAY(uint32) }
		case CASTCLASS_FInterfaceProperty:	{ IMPL_CAST_ANY_ARRAY(FScriptInterface) }
		case CASTCLASS_FClassProperty:		{ IMPL_CAST_ANY_ARRAY(UClass*) }
		case CASTCLASS_FNameProperty:		{ IMPL_CAST_ANY_ARRAY(FName) }
		case CASTCLASS_FStrProperty:		{ IMPL_CAST_ANY_ARRAY(FString) }
		case CASTCLASS_FObjectProperty:		{ IMPL_CAST_ANY_ARRAY(UObject*) }
		case CASTCLASS_FBoolProperty:		{ IMPL_CAST_ANY_ARRAY(bool) }
		case CASTCLASS_FUInt16Property:		{ IMPL_CAST_ANY_ARRAY(uint16) }
		case CASTCLASS_FInt64Property:		{ IMPL_CAST_ANY_ARRAY(int64) }
		case CASTCLASS_FWeakObjectProperty: { IMPL_CAST_ANY_ARRAY(TWeakObjectPtr<UObject>) }
		case CASTCLASS_FLazyObjectProperty: { IMPL_CAST_ANY_ARRAY(TLazyObjectPtr<UObject>) }
		case CASTCLASS_FSoftObjectProperty: { IMPL_CAST_ANY_ARRAY(TSoftObjectPtr<UObject>) }
		case CASTCLASS_FTextProperty:		{ IMPL_CAST_ANY_ARRAY(FText) }
		case CASTCLASS_FInt16Property:		{ IMPL_CAST_ANY_ARRAY(int16) }
		case CASTCLASS_FDoubleProperty:		{ IMPL_CAST_ANY_ARRAY(double) }
		case CASTCLASS_FSoftClassProperty:	{ IMPL_CAST_ANY_ARRAY(TSoftClassPtr<UClass>) }
		case CASTCLASS_FStructProperty:
		{
			auto Struct = CastField<FStructProperty>(Property->Inner)->Struct;
			if (auto Name = Struct->GetFName().ToEName()) {
				switch (*Name)
				{
				case NAME_BoxSphereBounds:	{ IMPL_CAST_ANY_ARRAY(FBoxSphereBounds) }
				case NAME_Sphere:			{ IMPL_CAST_ANY_ARRAY(FSphere) }
				case NAME_Box:				{ IMPL_CAST_ANY_ARRAY(FBox) }
				case NAME_Vector2D:			{ IMPL_CAST_ANY_ARRAY(FVector2D) }
				case NAME_IntRect:			{ IMPL_CAST_ANY_ARRAY(FIntRect) }
				case NAME_IntPoint:			{ IMPL_CAST_ANY_ARRAY(FIntPoint) }
				case NAME_Vector4:			{ IMPL_CAST_ANY_ARRAY(FVector4) }
				case NAME_Vector:			{ IMPL_CAST_ANY_ARRAY(FVector) }
				case NAME_Rotator:			{ IMPL_CAST_ANY_ARRAY(FRotator) }
				case NAME_Color:			{ IMPL_CAST_ANY_ARRAY(FColor) }
				case NAME_Plane:			{ IMPL_CAST_ANY_ARRAY(FPlane) }
				case NAME_Matrix:			{ IMPL_CAST_ANY_ARRAY(FMatrix) }
				case NAME_LinearColor:		{ IMPL_CAST_ANY_ARRAY(FLinearColor) }
				case NAME_Quat:				{ IMPL_CAST_ANY_ARRAY(FQuat) }
				case NAME_Transform:		{ IMPL_CAST_ANY_ARRAY(FTransform) }
				case NAME_Timer:			{ IMPL_CAST_ANY_ARRAY(FTimer) }
				}
			}
			if (Struct->GetFName() == "Any") { IMPL_CAST_ANY_ARRAY(FAny) }

			auto Array = Any.Cast<TArray<FBlueprintAnyStruct>>();
			if (Array.Num() > 0) {
				check(Struct == Array[0].Struct);
				Helper.Resize(Array.Num());
				for (int32 i = 0; i < Array.Num(); i++)
				{
					FMemory::Memcpy(Helper.GetRawPtr(i), Array[i].GetData(), Struct->PropertiesSize);
				}
			}
			return;
		}
		case CASTCLASS_FEnumProperty:
		{
			auto Array = Any.Cast<TArray<FBlueprintAnyEnum>>();
			auto Enum = CastField<FEnumProperty>(Property->Inner)->GetEnum();
			if (Array.Num() > 0) {
				check(Enum == Array[0].Enum);
				Helper.Resize(Array.Num());
				for (int32 i = 0; i < Array.Num(); i++)
				{
					FMemory::Memcpy(Helper.GetRawPtr(i), Array[i].GetData(), sizeof(int64));
				}
			}
			return;
		}
		}
		check(0);
	}
};