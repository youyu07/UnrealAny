// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealAnyBlueprintLibrary.h"


template<typename PropertyType, typename ValueType>
static FAny ToAny(FProperty* Property, uint8* Address)
{
	auto ValueProperty = CastField<PropertyType>(Property);
	ValueType Value = ValueProperty->GetPropertyValue(Address);
	return MoveTemp(Value);
}

static FAny ByteToAny(FProperty* Property, uint8* Address)
{
	if (auto EnumProperty = CastField<FByteProperty>(Property)) {
		if (EnumProperty->IsEnum()) {
			return FAny::FAnyEnum{EnumProperty->Enum, int64(EnumProperty->GetPropertyValue(Address))};
		}
		else {
			return EnumProperty->GetPropertyValue(Address);
		}
	}
	return FAny();
}

static FAny EnumToAny(FProperty* Property, uint8* Address)
{
	if (auto EnumProperty = CastField<FEnumProperty>(Property)) {
		auto Value = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(Address);
		return FAny::FAnyEnum{ EnumProperty->GetEnum(), Value };
	}
	return FAny();
}

static FAny StructToAny(FProperty* Property, uint8* Address)
{
	if (auto StructProperty = CastField<FStructProperty>(Property)) {
		TArray<uint8> Data;
		Data.SetNumZeroed(StructProperty->GetSize());
		StructProperty->CopyValuesInternal(Data.GetData(), Address, 1);

		auto Type = StructProperty->Struct->GetFName();
		if (auto TypeEName = Type.ToEName()) {
			switch (*TypeEName) {
			case NAME_Vector: return *reinterpret_cast<FVector*>(Data.GetData());
			case NAME_Rotator: return *reinterpret_cast<FRotator*>(Data.GetData());
			case NAME_Transform: return *reinterpret_cast<FTransform*>(Data.GetData());
			}
		}
		return FAny::FAnyStruct{ StructProperty->Struct, Data };
	}
	return FAny();
}

static FAny ClassToAny(FProperty* Property, uint8* Address)
{
	if (auto ClassProperty = CastField<FClassProperty>(Property)) {
		auto Object = ClassProperty->GetPropertyValue(Address);
		return FAny::FAnyClass{ ClassProperty->MetaClass, Cast<UClass>(Object) };
	}
	return FAny();
}

static FAny ObjectToAny(FProperty* Property, uint8* Address)
{
	if (auto ObjectProperty = CastField<FObjectProperty>(Property)) {
		return FAny::FAnyObject{ ObjectProperty->PropertyClass, ObjectProperty->GetPropertyValue(Address) };
	}
	return FAny();
}



FAny Generic_ToAny(FProperty* Property, uint8* Address, bool& bOutSuccess)
{
	static TMap<uint64, TFunction<FAny(FProperty*, uint8*)>> FuncMap;
	if (FuncMap.Num() == 0) {
		FuncMap.Add(FBoolProperty::StaticClassCastFlags(), &ToAny<FBoolProperty, bool>);
		FuncMap.Add(FIntProperty::StaticClassCastFlags(), &ToAny<FIntProperty, int32>);
		FuncMap.Add(FInt64Property::StaticClassCastFlags(), &ToAny<FInt64Property, int64>);
		FuncMap.Add(FFloatProperty::StaticClassCastFlags(), &ToAny<FFloatProperty, float>);
		FuncMap.Add(FDoubleProperty::StaticClassCastFlags(), &ToAny<FDoubleProperty, double>);
		FuncMap.Add(FNameProperty::StaticClassCastFlags(), &ToAny<FNameProperty, FName>);
		FuncMap.Add(FStrProperty::StaticClassCastFlags(), &ToAny<FStrProperty, FString>);
		FuncMap.Add(FTextProperty::StaticClassCastFlags(), &ToAny<FTextProperty, FText>);

		FuncMap.Add(FByteProperty::StaticClassCastFlags(), &ByteToAny);
		FuncMap.Add(FEnumProperty::StaticClassCastFlags(), &EnumToAny);
		FuncMap.Add(FClassProperty::StaticClassCastFlags(), &ClassToAny);
		FuncMap.Add(FObjectProperty::StaticClassCastFlags(), &ObjectToAny);
		FuncMap.Add(FStructProperty::StaticClassCastFlags(), &StructToAny);
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


FAny UUnrealAnyBlueprintLibrary::ToAny(const int32 Value)
{
	check(0);
	return FAny();
}


DEFINE_FUNCTION(UUnrealAnyBlueprintLibrary::execToAny)
{
	Stack.StepCompiledIn<FProperty>(NULL);
	auto Property = Stack.MostRecentProperty;
	uint8* Addr = Stack.MostRecentPropertyAddress;

	bool Success = true;

	P_FINISH;
	P_NATIVE_BEGIN;
	*(FAny*)RESULT_PARAM = Generic_ToAny(Property, Addr, Success);
	P_NATIVE_END;

#if DO_BLUEPRINT_GUARD
	if (!Success) {
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::FatalError,
			NSLOCTEXT("UnrealAny", "ToAny", "Unsupport Type!")
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

static void AnyToStruct(const FAny& Any, FProperty* Property, uint8* Address)
{
	if (auto StructProperty = CastField<FStructProperty>(Property)) {
		const void* ValuePtr = nullptr;
		switch (Any.Type())
		{
		case NAME_Vector: ValuePtr = Any.GetPtr<FVector>(); break;
		case NAME_Rotator: ValuePtr = Any.GetPtr<FRotator>(); break;
		case NAME_Transform: ValuePtr = Any.GetPtr<FTransform>(); break;
		default: ValuePtr = Any.GetPtr<FAny::FAnyStruct>()->Value.GetData(); break;
		}
		if(ValuePtr)StructProperty->CopyValuesInternal(Address, ValuePtr, 1);
	}
}

static void AnyToEnum(const FAny& Any, FProperty* Property, uint8* Address)
{
	if (auto EnumProperty = CastField<FEnumProperty>(Property)) {
		const auto Value = Any.GetPtr<FAny::FAnyEnum>()->Value;
		EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Address, Value);
	}
}

static void AnyToByte(const FAny& Any, FProperty* Property, uint8* Address)
{
	if (auto EnumProperty = CastField<FByteProperty>(Property)) {
		if (EnumProperty->IsEnum()) {
			EnumProperty->SetPropertyValue(Address, Any.Get<FAny::FAnyEnum>().Value);
		}
		else {
			EnumProperty->SetPropertyValue(Address, Any.Get<BYTE>());
		}
	}
}

static void AnyToClass(const FAny& Any, FProperty* Property, uint8* Address)
{
	if (auto ClassProperty = CastField<FClassProperty>(Property)) {
		ClassProperty->SetPropertyValue(Address, Any.Get<FAny::FAnyClass>().Class.Get());
	}
}

static void AnyToObject(const FAny& Any, FProperty* Property, uint8* Address)
{
	if (auto ObjectProperty = CastField<FObjectProperty>(Property)) {
		ObjectProperty->SetPropertyValue(Address, Any.Get<FAny::FAnyObject>().Object.Get());
	}
}

void Generic_AnyTo(const FAny& Any, FProperty* Property, uint8* Address, bool& bOutSuccess)
{
	static TMap<uint64, TFunction<void(const FAny&, FProperty*, uint8*)>> FuncMap;
	if (FuncMap.Num() == 0) {
		FuncMap.Add(FBoolProperty::StaticClassCastFlags(), &AnyTo<FBoolProperty, bool>);
		FuncMap.Add(FIntProperty::StaticClassCastFlags(), &AnyTo<FIntProperty, int32>);
		FuncMap.Add(FInt64Property::StaticClassCastFlags(), &AnyTo<FInt64Property, int64>);
		FuncMap.Add(FFloatProperty::StaticClassCastFlags(), &AnyTo<FFloatProperty, float>);
		FuncMap.Add(FDoubleProperty::StaticClassCastFlags(), &AnyTo<FDoubleProperty, double>);
		FuncMap.Add(FNameProperty::StaticClassCastFlags(), &AnyTo<FNameProperty, FName>);
		FuncMap.Add(FStrProperty::StaticClassCastFlags(), &AnyTo<FStrProperty, FString>);
		FuncMap.Add(FTextProperty::StaticClassCastFlags(), &AnyTo<FTextProperty, FText>);

		FuncMap.Add(FStructProperty::StaticClassCastFlags(), &AnyToStruct);
		FuncMap.Add(FEnumProperty::StaticClassCastFlags(), &AnyToEnum);
		FuncMap.Add(FByteProperty::StaticClassCastFlags(), &AnyToByte);
		FuncMap.Add(FClassProperty::StaticClassCastFlags(), &AnyToClass);
		FuncMap.Add(FObjectProperty::StaticClassCastFlags(), &AnyToObject);
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
		case NAME_ByteProperty: {
			auto Enum = CastField<FByteProperty>(Property);
			if (Enum->IsEnum()) {
				bOutSuccess = Enum->Enum == Any.Get<FAny::FAnyEnum>().Enum;
			}
			break;
		}
		case NAME_ObjectProperty: {
			auto Object = CastField<FObjectProperty>(Property);
			auto AnyObject = Any.Get<FAny::FAnyObject>();
			bOutSuccess = AnyType == NAME_ObjectProperty && AnyObject.Class->GetDefaultObject()->IsA(Object->PropertyClass);
			break;
		}
		case NAME_Class: {
			auto Class = CastField<FClassProperty>(Property);
			bOutSuccess = AnyType == NAME_Class && (Class->MetaClass == Any.Get<FAny::FAnyClass>().Class);
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


void UUnrealAnyBlueprintLibrary::AnyTo(const FAny& Any, int32& Value)
{
	check(0);
}

DEFINE_FUNCTION(UUnrealAnyBlueprintLibrary::execAnyTo)
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
			NSLOCTEXT("UnrealAny", "AnyTo", "Unsupport Type!")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
#endif
}