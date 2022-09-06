// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealAny.h"

namespace Any
{
	template<typename T>
	static void Loading(FAny& Any, FArchive& Ar)
	{
		T Value = T(); Ar << Value; Any = Value;
	}
	template<typename T>
	static void Saving(const FAny& Any, FArchive& Ar)
	{
		auto Value = Any.Get<T>();
		Ar << Value;
	}
}

#define ADD_TYPE(Type, NameEnum)  \
{ \
	FName TypeName = typeid(Type).name(); \
	TypeMap.Add(TypeName, MakeTuple(NameEnum, Any::Loading<Type>, Any::Saving<Type>)); \
	TypeNameMap.Add(NameEnum, TypeName); \
}


static TMap<FName, TTuple<EName, TFunction<void(FAny&, FArchive&)>, TFunction<void(const FAny&, FArchive&)>>> TypeMap;
static TMap<EName, FName> TypeNameMap;
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
	}
}
#undef ADD_TYPE


bool FAny::Serialize(FArchive& Ar)
{
	InitializeTypeInfo();

	Ar << TypeName;

	if (Ar.IsSaving()) {
		if (auto Ptr = TypeMap.Find(TypeInfo().name())) {
			/*EName Type = Ptr->Get<0>();
			Ar << Type;*/
			Ptr->Get<2>()(*this, Ar);
		}
		/*else {
			Ar << NAME_None;
		}*/
	}


	if (Ar.IsLoading()) {
		/*EName Type = NAME_None;
		Ar << Type;*/
		if (auto NamePtr = TypeNameMap.Find(TypeName)) {
			auto Info = TypeMap[*NamePtr];
			Info.Get<1>()(*this, Ar);
		}
	}

	return true;
}

bool FAny::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	

	return false;
}
