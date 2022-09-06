// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealAnyCaster.h"



UAnyPropertyBase* UAnyPropertyBase::FindCaster(EClassCastFlags Flags)
{
	static TMap<EClassCastFlags, UAnyPropertyBase*> CasterMap;
	static bool IsRegister = false;
	if (!IsRegister) {
		IsRegister = true;

		for (TObjectIterator<UAnyPropertyBase> it(RF_NoFlags); it; ++it)
		{
			CasterMap.Add(it->GetClassCastFlags(), *it);
		}
	}

	if (auto Ptr = CasterMap.Find(Flags)) {
		return *Ptr;
	}

	return UAnyPropertyBase::StaticClass()->GetDefaultObject<UAnyPropertyBase>();
}


UAnyStructBase* UAnyStructBase::FindCaster(UScriptStruct* Struct)
{
	static TMap<FName, UAnyStructBase*> CasterMap;
	static bool IsRegister = false;
	if (!IsRegister) {
		IsRegister = true;

		for (TObjectIterator<UAnyStructBase> it(RF_NoFlags); it; ++it)
		{
			CasterMap.Add(it->GetStructName(), *it);
		}
	}

	if (auto Ptr = CasterMap.Find(Struct->GetFName())) {
		return *Ptr;
	}

	return UAnyStructBase::StaticClass()->GetDefaultObject<UAnyStructBase>();
}


#define DEFINE_IMPL_TOANY(ClassName, PropertyType, ValueType) \
FAny ClassName::ToAny(FProperty* Property, uint8* ValueAddress) \
{ \
	auto ValueProperty = CastField<PropertyType>(Property); \
	ValueType Value = ValueProperty->GetPropertyValue(ValueAddress); \
	return MoveTemp(Value); \
}

#define DEFINE_IMPL_WRITEPROPERTY(ClassName, PropertyType, ValueType) \
void ClassName::WriteProperty(const FAny& Any, FProperty* Property, uint8*& ValueAddress) \
{ \
	CastField<PropertyType>(Property)->SetPropertyValue(ValueAddress, Any.Get<ValueType>()); \
}


#define DEFINE_IMPLE_ANYCAS(ClassName, PropertyType, ValueType) \
DEFINE_IMPL_TOANY(ClassName, PropertyType, ValueType) \
DEFINE_IMPL_WRITEPROPERTY(ClassName, PropertyType, ValueType) 



DEFINE_IMPLE_ANYCAS(UAnyCasterBool, FBoolProperty, bool)
DEFINE_IMPLE_ANYCAS(UAnyCasterInt, FIntProperty, int32)
DEFINE_IMPLE_ANYCAS(UAnyCasterFloat, FFloatProperty, float)
DEFINE_IMPLE_ANYCAS(UAnyCasterDouble, FDoubleProperty, double)
DEFINE_IMPLE_ANYCAS(UAnyCasterString, FStrProperty, FString)
DEFINE_IMPLE_ANYCAS(UAnyCasterName, FNameProperty, FName)
DEFINE_IMPLE_ANYCAS(UAnyCasterText, FTextProperty, FText)
//DEFINE_IMPLE_ANYCAS(UAnyCasterObject, FObjectProperty, UObject*)
DEFINE_IMPLE_ANYCAS(UAnyCasterByte, FByteProperty, uint8)



FAny UAnyCasterClass::ToAny(FProperty* Property, uint8* ValueAddress)
{
	auto ValueProperty = CastField<FClassProperty>(Property);
	return FAny(ValueProperty->MetaClass);
}
DEFINE_IMPL_WRITEPROPERTY(UAnyCasterClass, FClassProperty, UClass*)


#pragma region Struct
FAny UAnyCasterStruct::ToAny(FProperty* Property, uint8* ValueAddress)
{
	auto StructProperty = CastField<FStructProperty>(Property);
	return UAnyStructBase::FindCaster(StructProperty->Struct)->ToAny(StructProperty, ValueAddress);
}

void UAnyCasterStruct::WriteProperty(const FAny& Any, FProperty* Property, uint8*& ValueAddress)
{
	auto StructProperty = CastField<FStructProperty>(Property);
	UAnyStructBase::FindCaster(StructProperty->Struct)->WriteProperty(Any, StructProperty, ValueAddress);
}
#pragma endregion



#pragma region Enum
FAny UAnyCasterEnum::ToAny(FProperty* Property, uint8* ValueAddress)
{
	auto EnumProperty = CastField<FEnumProperty>(Property);
	return EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValueAddress);
}

void UAnyCasterEnum::WriteProperty(const FAny& Any, FProperty* Property, uint8*& ValueAddress)
{
	auto EnumProperty = CastField<FEnumProperty>(Property);
	auto Value = Any.Get<FAny::FAnyEnum>();
	EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ValueAddress, Value.Value);
}
#pragma endregion




#define DEFINE_IMPL_STRUCTTOANY(ClassName, ValueType) \
FAny ClassName::ToAny(FStructProperty* Property, uint8* ValueAddress) \
{ \
	ValueType Value; \
	Property->CopyValuesInternal(&Value, ValueAddress, 1); \
	return MoveTemp(Value); \
} \
void ClassName::WriteProperty(const FAny& Any, FStructProperty* Property, uint8*& ValueAddress) \
{ \
	auto Value = Any.Get<ValueType>(); \
	Property->CopyValuesInternal(ValueAddress, &Value, 1); \
}

DEFINE_IMPL_STRUCTTOANY(UAnyCasterVector, FVector)
DEFINE_IMPL_STRUCTTOANY(UAnyCasterRotator, FRotator)
DEFINE_IMPL_STRUCTTOANY(UAnyCasterTransform, FTransform)
