// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealAnyBlueprintLibrary.h"
#include "UnrealAnyCaster.h"


template<typename PropertyType, typename ValueType>
static FAny ToAny(FProperty* Property, uint8* Address)
{
	auto ValueProperty = CastField<PropertyType>(Property);
	ValueType Value = ValueProperty->GetPropertyValue(Address);
	return MoveTemp(Value);
}



FAny Generic_CastToAny(FProperty* Property, uint8* Address, bool& bOutSuccess)
{
	static TMap<uint64, TFunction<FAny(FProperty*, uint8*)>> FuncMap;
	if (FuncMap.Num() == 0) {
		FuncMap.Add(FBoolProperty::StaticClassCastFlags(), &ToAny<FBoolProperty, bool>);
		FuncMap.Add(FByteProperty::StaticClassCastFlags(), &ToAny<FByteProperty, BYTE>);
		FuncMap.Add(FIntProperty::StaticClassCastFlags(), &ToAny<FIntProperty, int32>);
		FuncMap.Add(FInt64Property::StaticClassCastFlags(), &ToAny<FInt64Property, int64>);
		FuncMap.Add(FFloatProperty::StaticClassCastFlags(), &ToAny<FFloatProperty, float>);
		FuncMap.Add(FDoubleProperty::StaticClassCastFlags(), &ToAny<FDoubleProperty, double>);
		FuncMap.Add(FNameProperty::StaticClassCastFlags(), &ToAny<FNameProperty, FName>);
		FuncMap.Add(FStrProperty::StaticClassCastFlags(), &ToAny<FStrProperty, FString>);
		FuncMap.Add(FTextProperty::StaticClassCastFlags(), &ToAny<FTextProperty, FText>);
	}

	if (auto Func = FuncMap.Find(Property->GetCastFlags())) {
		bOutSuccess = true;
		return (*Func)(Property, Address);
	}
	else {
		bOutSuccess = false;
	}
	return FAny();
}



DEFINE_FUNCTION(UUnrealAnyBlueprintLibrary::execCastToAny)
{
	Stack.StepCompiledIn<FProperty>(NULL);
	auto Property = Stack.MostRecentProperty;
	uint8* Addr = Stack.MostRecentPropertyAddress;

	bool Success = true;

	P_FINISH;
	P_NATIVE_BEGIN;
	*(FAny*)RESULT_PARAM = Generic_CastToAny(Property, Addr, Success);
	P_NATIVE_END;

#if DO_BLUEPRINT_GUARD
	if (!Success) {
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::FatalError,
			NSLOCTEXT("UnrealAny", "CastToAny", "Unsupport Type!")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
#endif
}


template<typename PropertyType, typename ValueType>
static void AnyTo(const FAny& Any, FProperty* Property, uint8* Address)
{
	CastField<PropertyType>(Property)->SetPropertyValue(Address, Any.Get<ValueType>());
}

void Generic_AnyTo(const FAny& Any, FProperty* Property, uint8* Address, bool& bOutSuccess)
{
	static TMap<uint64, TFunction<void(const FAny&, FProperty*, uint8*)>> FuncMap;
	if (FuncMap.Num() == 0) {
		FuncMap.Add(FBoolProperty::StaticClassCastFlags(), &AnyTo<FBoolProperty, bool>);
		FuncMap.Add(FByteProperty::StaticClassCastFlags(), &AnyTo<FByteProperty, BYTE>);
		FuncMap.Add(FIntProperty::StaticClassCastFlags(), &AnyTo<FIntProperty, int32>);
		FuncMap.Add(FInt64Property::StaticClassCastFlags(), &AnyTo<FInt64Property, int64>);
		FuncMap.Add(FFloatProperty::StaticClassCastFlags(), &AnyTo<FFloatProperty, float>);
		FuncMap.Add(FDoubleProperty::StaticClassCastFlags(), &AnyTo<FDoubleProperty, double>);
		FuncMap.Add(FNameProperty::StaticClassCastFlags(), &AnyTo<FNameProperty, FName>);
		FuncMap.Add(FStrProperty::StaticClassCastFlags(), &AnyTo<FStrProperty, FString>);
		FuncMap.Add(FTextProperty::StaticClassCastFlags(), &AnyTo<FTextProperty, FText>);
	}

#if DO_BLUEPRINT_GUARD
	auto AnyType = Any.Type();
	if (auto Type = Property->GetClass()->GetFName().ToEName()) {
		bOutSuccess = AnyType == *Type;
		switch (*Type)
		{
		case NAME_StructProperty:
		{
			auto Struct = CastField<FStructProperty>(Property);
			switch (AnyType)
			{
			case NAME_Vector: bOutSuccess = Struct->Struct == TBaseStructure<FVector>::Get(); break;
			case NAME_Rotator: bOutSuccess = Struct->Struct == TBaseStructure<FRotator>::Get(); break;
			case NAME_Transform: bOutSuccess = Struct->Struct == TBaseStructure<FTransform>::Get(); break;
			default: bOutSuccess = AnyType == NAME_StructProperty && (Struct->Struct == Any.Get<FAny::FAnyStruct>().Struct); break;
			}
			break;
		}
		case NAME_EnumProperty: {
			auto Enum = CastField<FEnumProperty>(Property);
			bOutSuccess = AnyType == NAME_EnumProperty && (Enum->GetEnum() == Any.Get<FAny::FAnyEnum>().Enum);
			break;
		}
		case NAME_ObjectProperty: {
			auto Object = CastField<FObjectProperty>(Property);
			bOutSuccess = AnyType == NAME_EnumProperty && (Object->PropertyClass == Any.Get<FAny::FAnyObject>().Class);
			break;
		}
		case NAME_Class: {
			auto Class = CastField<FClassProperty>(Property);
			bOutSuccess = AnyType == NAME_Class && (Class->MetaClass == Any.Get<FAny::FAnyClass>().Class);
			break;
		}
		default: {
			bOutSuccess = AnyType == *Type;
			break;
		}
		}
	}
	if (!bOutSuccess) return;
#endif


	if (auto Func = FuncMap.Find(Property->GetCastFlags())) {
		(*Func)(Any, Property, Address);
		bOutSuccess = true;
	}
	else {
		bOutSuccess = false;
	}
}


DEFINE_FUNCTION(UUnrealAnyBlueprintLibrary::execAnyCast)
{
	P_GET_STRUCT_REF(FAny, Any);

	Stack.StepCompiledIn<FProperty>(NULL);
	auto Property = Stack.MostRecentProperty;
	uint8* Addr = Stack.MostRecentPropertyAddress;

	bool Success = true;

	P_FINISH;
	P_NATIVE_BEGIN;
	Generic_AnyTo(Any, Property, Addr, Success);
	P_NATIVE_END;

#if DO_BLUEPRINT_GUARD
	if (!Success) {
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::FatalError,
			NSLOCTEXT("UnrealAny", "AnyCast", "Unsupport Type!")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
#endif
}