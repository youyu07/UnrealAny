// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealAny.h"


static FArchive& operator<<(FArchive& Ar, FAny::FAnyStruct& Any)
{
	UObject* Object = Any.Struct.Get(true);
	Ar << Object;

	if (Ar.IsLoading()) {
		Any.Struct = Cast<UScriptStruct>(Object);
	}

	Ar << Any.Value;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FAny::FAnyEnum& Any)
{
	UObject* Object = Any.Enum.Get(true);
	Ar << Object;
	if (Ar.IsLoading()) {
		Any.Enum = Cast<UEnum>(Object);
	}
	Ar << Any.Value;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FAny::FAnyObject& Any)
{
	UObject* Class = Any.Class.Get(true);
	UObject* Object = Any.Object.Get(true);
	Ar << Class;
	Ar << Object;
	if (Ar.IsLoading()) {
		Any.Class = Cast<UClass>(Class);
		Any.Object = Object;
	}
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FAny::FAnyClass& Any)
{
	UObject* BaseClass = Any.BaseClass.Get(true);
	UObject* Class = Any.Class.Get(true);
	Ar << BaseClass;
	Ar << Class;
	if (Ar.IsLoading()) {
		Any.BaseClass = Cast<UClass>(BaseClass);
		Any.Class = Cast<UClass>(Class);
	}
	return Ar;
}

namespace Any
{
	template<typename T>
	static void Loading(FAny& Any, FArchive& Ar)
	{
		T Value = T(); 
		Ar << Value; 
		Any = Value;
	}
	template<typename T>
	static void Saving(const FAny& Any, FArchive& Ar)
	{
		auto Value = Any.Get<T>();
		Ar << Value;
	}

	template<> void Loading<UClass*>(FAny& Any, FArchive& Ar)
	{
		UClass* Value = nullptr;
		Ar << Value;
		Any = Value;
	}
}

#define ADD_TYPE(Type, NameEnum)  \
{ \
	TypeMap.Add(NameEnum, MakeTuple(Any::Loading<Type>, Any::Saving<Type>)); \
}


static TMap<EName, TTuple<TFunction<void(FAny&, FArchive&)>, TFunction<void(const FAny&, FArchive&)>>> TypeMap;
static void InitializeTypeInfo()
{
	static bool IsInit = false;
	if (!IsInit) {
		IsInit = true;
		ADD_TYPE(bool, NAME_BoolProperty)
		ADD_TYPE(int32, NAME_Int32Property)
		ADD_TYPE(int64, NAME_Int64Property)
		ADD_TYPE(float, NAME_FloatProperty)
		ADD_TYPE(double, NAME_DoubleProperty)
		ADD_TYPE(FString, NAME_StrProperty)
		ADD_TYPE(FName, NAME_NameProperty)
		ADD_TYPE(FText, NAME_TextProperty)
		ADD_TYPE(FVector, NAME_Vector)
		ADD_TYPE(FRotator, NAME_Rotator)
		ADD_TYPE(FTransform, NAME_Transform)

		ADD_TYPE(FAny::FAnyStruct, NAME_StructProperty)
		ADD_TYPE(FAny::FAnyEnum, NAME_EnumProperty)
		ADD_TYPE(FAny::FAnyObject, NAME_ObjectProperty)
		ADD_TYPE(FAny::FAnyClass, NAME_Class)
	}
}
#undef ADD_TYPE


bool FAny::Serialize(FArchive& Ar)
{
	InitializeTypeInfo();

	Ar << TypeName;

	if (Ar.IsSaving()) {
		if (auto Ptr = TypeMap.Find(TypeName)) {
			Ptr->Get<1>()(*this, Ar);
		}
	}

	if (Ar.IsLoading()) {
		if (auto Ptr = TypeMap.Find(TypeName)) {
			Ptr->Get<0>()(*this, Ar);
		}
	}

	return true;
}

bool FAny::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	

	return false;
}
