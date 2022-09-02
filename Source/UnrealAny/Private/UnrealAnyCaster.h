// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealAny.h"
#include <typeinfo>
#include "UnrealAnyCaster.generated.h"


UCLASS()
class UAnyPropertyBase : public UObject
{
	GENERATED_BODY()

public:
	static UAnyPropertyBase* FindCaster(EClassCastFlags Flags);

	virtual FAny ToAny(FProperty* Property, uint8* ValueAddress)
	{
		return FAny();
	}

	virtual FAny ToAnyArray(FArrayProperty* Property, uint8* ValueAddress)
	{
		return FAny();
	}

	virtual void WriteProperty(const FAny& Any, FProperty* Property, uint8*& ValueAddress)
	{
	
	}

	virtual void WriteArrayProperty(const FAny& Any, FArrayProperty* Property, uint8*& ValueAddress)
	{

	}

	virtual EClassCastFlags GetClassCastFlags() const
	{
		return CASTCLASS_None;
	}
};



#define DEFINE_ANYCAS(PropertyType) \
	virtual FAny ToAny(FProperty* Property, uint8* ValueAddress) override; \
	virtual FAny ToAnyArray(FArrayProperty* Property, uint8* ValueAddress) override; \
	virtual void WriteProperty(const FAny& Any, FProperty* Property, uint8*& ValueAddress) override; \
	virtual void WriteArrayProperty(const FAny& Any, FArrayProperty* Property, uint8*& ValueAddress) override; \
	virtual EClassCastFlags GetClassCastFlags() const override \
	{ \
		return EClassCastFlags(PropertyType::StaticClassCastFlags()); \
	}

UCLASS()
class UAnyCasterBool : public UAnyPropertyBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS(FBoolProperty)
};

UCLASS()
class UAnyCasterInt : public UAnyPropertyBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS(FIntProperty)
};

UCLASS()
class UAnyCasterFloat : public UAnyPropertyBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS(FFloatProperty)
};

UCLASS()
class UAnyCasterDouble : public UAnyPropertyBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS(FDoubleProperty)
};

UCLASS()
class UAnyCasterString : public UAnyPropertyBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS(FStrProperty)
};


UCLASS()
class UAnyCasterName : public UAnyPropertyBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS(FNameProperty)
};


UCLASS()
class UAnyCasterText : public UAnyPropertyBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS(FTextProperty)
};


UCLASS()
class UAnyCasterClass : public UAnyPropertyBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS(FClassProperty)
};


UCLASS()
class UAnyCasterObject : public UAnyPropertyBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS(FObjectProperty)
};

UCLASS()
class UAnyCasterStruct : public UAnyPropertyBase
{
	GENERATED_BODY()
public:
	DEFINE_ANYCAS(FStructProperty)
};


UCLASS()
class UAnyCasterEnum : public UAnyPropertyBase
{
	GENERATED_BODY()
public:
	DEFINE_ANYCAS(FEnumProperty)
};

UCLASS()
class UAnyCasterByte : public UAnyPropertyBase
{
	GENERATED_BODY()
public:
	DEFINE_ANYCAS(FByteProperty)
};


UCLASS()
class UAnyStructBase : public UObject
{
	GENERATED_BODY()

public:
	static UAnyStructBase* FindCaster(UScriptStruct* Struct);

	virtual FAny ToAny(FStructProperty* Property, uint8* ValueAddress)
	{
		return FAny();
	}

	virtual FAny ToAnyArray(FArrayProperty* Property, uint8* ValueAddress)
	{
		return FAny();
	}

	virtual void WriteProperty(const FAny& Any, FStructProperty* Property, uint8*& ValueAddress)
	{

	}

	virtual void WriteArrayProperty(const FAny& Any, FArrayProperty* Property, uint8*& ValueAddress)
	{

	}

	virtual FName GetStructName() const {
		return NAME_None;
	}
};

#define DEFINE_ANYCAS_STRUCT(StructType, StructTypeName) \
	virtual FAny ToAny(FStructProperty* Property, uint8* ValueAddress) override; \
	virtual FAny ToAnyArray(FArrayProperty* Property, uint8* ValueAddress) override; \
	virtual void WriteProperty(const FAny& Any, FStructProperty* Property, uint8*& ValueAddress) override; \
	virtual void WriteArrayProperty(const FAny& Any, FArrayProperty* Property, uint8*& ValueAddress) override; \
	virtual FName GetStructName() const override { \
		return StructTypeName; \
	}


UCLASS()
class UAnyCasterVector : public UAnyStructBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS_STRUCT(FVector, NAME_Vector)
};



UCLASS()
class UAnyCasterRotator : public UAnyStructBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS_STRUCT(FRotator, NAME_Rotator)
};

UCLASS()
class UAnyCasterTransform : public UAnyStructBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS_STRUCT(FTransform, NAME_Transform)
};

UCLASS()
class UAnyCasterVector2D : public UAnyStructBase
{
	GENERATED_BODY()

	DEFINE_ANYCAS_STRUCT(FVector2D, NAME_Vector2D)
};


UCLASS()
class UAnyCasterArray : public UAnyPropertyBase
{
	GENERATED_BODY()

public:
	virtual FAny ToAny(FProperty* Property, uint8* ValueAddress) override;
	virtual void WriteProperty(const FAny& Any, FProperty* Property, uint8*& ValueAddress) override;

	virtual EClassCastFlags GetClassCastFlags() const
	{
		return EClassCastFlags(FArrayProperty::StaticClassCastFlags());
	}
};