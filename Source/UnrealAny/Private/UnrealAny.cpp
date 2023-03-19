// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealAny.h"


#define MAX_BUFFER_SIZE 1024 * 64


class FAnyReader : public FMemoryReader
{
public:
	FAnyReader(const TArray<uint8>& InBytes) : FMemoryReader(InBytes)
	{
		SetIsLoading(true);
		SetWantBinaryPropertySerialization(true);
		ArIgnoreClassRef = false;
		ArIgnoreArchetypeRef = false;
	}

	virtual FArchive& operator<<(UObject*& Value) override
	{
		FSoftObjectPtr Ptr(Value);
		*this << Ptr;
		Value = Ptr.LoadSynchronous();
		return *this;
	}

	virtual FArchive& operator<<(struct FObjectPtr& Value) override
	{
		FSoftObjectPtr Ptr(Value);
		*this << Ptr;
		Value = Ptr.LoadSynchronous();
		return *this;
	}

	virtual FArchive& operator<<(FLazyObjectPtr& Value) override
	{
		FArchive& Ar = *this;
		FUniqueObjectGuid ID;
		Ar << ID;

		Value = ID;
		return Ar;
	}

	virtual FArchive& operator<<(FSoftObjectPath& Value) override
	{
		Value.SerializePath(*this);
		return *this;
	}

	virtual FArchive& operator<<(FSoftObjectPtr& Value) override
	{
		Value.ResetWeakPtr();
		return *this << Value.GetUniqueID();
	}

	virtual FArchive& operator<<(FWeakObjectPtr& Value) override
	{
		return FArchiveUObject::SerializeWeakObjectPtr(*this, Value);
	}
};

class FAnyWriter : public FMemoryWriter
{
public:
	FAnyWriter(TArray<uint8>& InBytes) : FMemoryWriter(InBytes)
	{
		SetWantBinaryPropertySerialization(true);
		ArIgnoreClassRef = false;
		ArIgnoreArchetypeRef = false;
	}

	virtual FArchive& operator<<(UObject*& Value) override
	{
		auto Ptr = FSoftObjectPtr(Value);
		return *this << Ptr;
	}

	virtual FArchive& operator<<(struct FObjectPtr& Value) override
	{
		auto Ptr = FSoftObjectPtr(Value);
		return *this << Ptr;
	}

	virtual FArchive& operator<<(FLazyObjectPtr& Value) override
	{
		FUniqueObjectGuid ID = Value.GetUniqueID();
		return *this << ID;
	}

	virtual FArchive& operator<<(FSoftObjectPath& Value) override
	{
		Value.SerializePath(*this);
		return *this;
	}

	virtual FArchive& operator<<(FSoftObjectPtr& Value) override
	{
		return *this << Value.GetUniqueID();
	}

	virtual FArchive& operator<<(FWeakObjectPtr& Value) override
	{
		FArchiveUObject::SerializeWeakObjectPtr(*this, Value);
		return *this;
	}
};


//static void SerializeStruct(UScriptStruct* Struct, FArchive& Ar, TArray<uint8>& Buffer)
//{
//	FBinaryArchiveFormatter Formatter(Ar);
//	for (auto Property = Struct->PropertyLink; Property; Property = Property->PropertyLinkNext)
//	{
//		FStructuredArchive Archive(Formatter);
//		FStructuredArchive::FSlot Slot = Archive.Open();
//		Property->SerializeItem(Slot, Buffer.GetData() + Property->GetOffset_ForInternal());
//		Archive.Close();
//	}
//}

static FArchive& operator<<(FArchive& Ar, FAny::FAnyStruct& Any)
{
	UObject* Object = Any.Struct.Get(true);
	Ar << Object;

	if (Ar.IsLoading()) {
		Any.Struct = Cast<UScriptStruct>(Object);

		TArray<uint8> Buffer;
		Ar << Buffer;
		FAnyReader Archive(Buffer);

		Any.Value.SetNumZeroed(Any.Struct->GetStructureSize());
		Any.Struct->SerializeItem(Archive, Any.Value.GetData(), nullptr);
	}

	if (Ar.IsSaving()) {
		TArray<uint8> Buffer;
		Buffer.SetNumZeroed(MAX_BUFFER_SIZE);
		FAnyWriter Archive(Buffer);
		
		Any.Struct->SerializeItem(Archive, Any.Value.GetData(), nullptr);

		Buffer.SetNum(Archive.Tell());
		Ar << Buffer;
	}
	
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
	InitializeTypeInfo();
	Ar << TypeName;

	if (TypeName == NAME_StructProperty) {
		if (Ar.IsLoading()) {
			FAny::FAnyStruct StructValue;
			UObject* Object = StructValue.Struct.Get(true);
			Ar << Object;
			StructValue.Struct = Cast<UScriptStruct>(Object);

			Ar << StructValue.Value;
			FNetBitReader Reader(Map, StructValue.Value.GetData(), StructValue.Value.Num() * 8);

			TArray<uint8> Buffer;
			Buffer.SetNumZeroed(StructValue.Struct->GetStructureSize());

			FBinaryArchiveFormatter Formatter(Reader);
			for (auto Property = StructValue.Struct->PropertyLink; Property; Property = Property->PropertyLinkNext)
			{
				FStructuredArchive Archive(Formatter);
				FStructuredArchive::FSlot Slot = Archive.Open();
				Property->SerializeItem(Slot, Buffer.GetData() + Property->GetOffset_ForInternal());
				Archive.Close();
			}

			StructValue.Value = Buffer;
			*this = StructValue;
		}

		if (Ar.IsSaving()) {
			FAny::FAnyStruct StructValue = Get<FAny::FAnyStruct>();
			UObject* Object = StructValue.Struct.Get(true);
			Ar << Object;

			FNetBitWriter Writer(Map, MAX_BUFFER_SIZE);
			FBinaryArchiveFormatter Formatter(Writer);
			
			for (auto Property = StructValue.Struct->PropertyLink; Property; Property = Property->PropertyLinkNext)
			{
				FStructuredArchive Archive(Formatter);
				FStructuredArchive::FSlot Slot = Archive.Open();
				Property->SerializeItem(Slot, StructValue.Value.GetData() + Property->GetOffset_ForInternal());
				Archive.Close();
			}

			StructValue.Value = TArray<uint8>(Writer.GetData(), Writer.GetNumBytes());

			Ar << StructValue.Value;
		}
	}
	else {
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
	}

	return true;
}
