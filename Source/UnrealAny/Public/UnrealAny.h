// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "UnrealAny.generated.h"


struct CProvidesStaticEnum
{
    template<typename T>
    auto Requires(const T&) -> decltype(StaticEnum<T>());
};


USTRUCT(BlueprintType)
struct UNREALANY_API FAny
{
	GENERATED_BODY()

public:
    struct FAnyStruct
    {
        TWeakObjectPtr<UScriptStruct> Struct = nullptr;
        TArray<uint8> Value;
    };

    struct FAnyEnum
    {
        TWeakObjectPtr<UEnum> Enum = nullptr;
        int64 Value = 0;
    };

    struct FAnyObject
    {
        TWeakObjectPtr<UClass> Class = nullptr;
        TWeakObjectPtr<UObject> Object = nullptr;
    };


public:
	constexpr FAny() noexcept {}

    FAny(const FAny& Other) 
        : TypeName(Other.TypeName)
        , Content(Other.Content ? Other.Content->Clone() : nullptr) 
    {
    }

    FAny(FAny&& Other) noexcept 
        : TypeName(Other.TypeName)
        , Content(std::move(Other.Content))
    {
        Other.Content = nullptr;
    }

    template<typename T>
    FORCEINLINE FAny(T Value)
    {
        if constexpr (TAnd<TModels<CProvidesStaticEnum, T>, TOr<TIsEnumClass<T>, TIsEnum<T>>>::Value) {
            TypeName = NAME_EnumProperty;
            auto Enum = FAnyEnum{ StaticEnum<T>(), static_cast<int64>(Value) };
            Content = new Holder(Enum);
        }
        else if constexpr (TIsClass<T>::Value) {
            TypeName = NAME_StructProperty;
            auto Struct = FAnyStruct{ TBaseStructure<T>().Get() };
            if constexpr (TProvidesStaticStruct<T>::Value) {
                Struct.Struct = T::StaticStruct();
            }
            else {
                Struct.Struct = TBaseStructure<T>().Get();
            }
            FMemoryWriter Writer(Struct.Value, true);
            Writer << Value;

            Content = new Holder(Struct);
        }
        else {
            static_assert(TIsPODType<T>::value, "UNSUPPORT TYPE");
        }
    }

    template<typename T, typename = TModels<CProvidesStaticClass, T>>
    FORCEINLINE FAny(T* Value)
    {
        TypeName = NAME_ObjectProperty;
        auto Object = FAnyObject{ Value->GetClass(), Value };
        Content = new Holder(Object);
    }

    ~FAny() { Reset(); }


    FAny& operator=(const FAny& Other)
    {
        FAny(Other).Swap(*this);
        return *this;
    }

    FAny& operator=(FAny&& Other) noexcept
    {
        FAny(std::move(Other)).Swap(*this);
        return *this;
    }

    template< class ValueType >
    FAny& operator=(const ValueType& Value)
    {
        FAny(Value).Swap(*this);
        return *this;
    }

    /*template<class ValueType, class T = typename std::decay<ValueType>::type, typename std::enable_if<(!std::is_same<T, FAny>::value), int>::type = 0>
    FAny& operator=(T&& Value)
    {
        FAny(std::move(Value)).Swap(*this);
        return *this;
    }*/


	bool IsValid() {
		return Content != nullptr;
	}

    const std::type_info& TypeInfo() const noexcept
    {
        return Content ? Content->TypeInfo() : typeid(void);
    }

    const EName Type() const {
        return TypeName;
    }

    template<class ValueType>
    inline ValueType Get() const
    {
        return *(&(static_cast<Holder<ValueType>*>(Content)->Held));
    }

public:
    friend bool operator==(const FAny& A, const FAny& B)
    {
        return A.TypeName == B.TypeName && A.Content == B.Content;
    }

private:
    void Reset() noexcept
    {
        TypeName = NAME_None;
        if (Content) { delete Content; Content = nullptr; }
    }

    void Swap(FAny& Other) noexcept
    {
        std::swap(TypeName, Other.TypeName);
        std::swap(Content, Other.Content);
    }

private:
    class Placeholder
    {
    public:
        virtual ~Placeholder() {}
        virtual const std::type_info& TypeInfo() const = 0;
#if WITH_EDITOR
        virtual const FEdGraphPinType PinType() const {return FEdGraphPinType(); }
#endif
        virtual Placeholder* Clone() const = 0;
    };

    template<typename ValueType>
    class Holder : public Placeholder
    {
    public:
        Holder(const ValueType& Value) : Held(Value) {}

        Holder(ValueType&& Value) : Held(std::move(Value)) {}

        virtual const std::type_info& TypeInfo() const override
        {
            return typeid(ValueType);
        }

        virtual Placeholder* Clone() const override
        {
            return new Holder(Held);
        }

        ValueType Held;
    };

    EName TypeName = NAME_None;
    Placeholder* Content = nullptr;

public:
    bool Serialize(FArchive& Ar);
    bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

    friend CORE_API FArchive& operator<<(FArchive& Ar, FAny& Any)
    {
        return Ar << Any;
    }
};


template<> struct TStructOpsTypeTraits<FAny> : public TStructOpsTypeTraitsBase2<FAny>
{
    enum
    {
        WithIdenticalViaEquality = true,
        WithNoInitConstructor = true,
        WithZeroConstructor = true,
        WithNetSerializer = true,
        WithSerializer = true,
    };
};


#define IMPL_ANY(InTypeCpp, InTypeName) \
template<> FORCEINLINE FAny::FAny(InTypeCpp Value) : TypeName(InTypeName), Content(new Holder<InTypeCpp>(Value)) {}


#define IMPL_ANY_CUSTOM_STRUCT(InTypeName) \
template<> FORCEINLINE FAny::FAny(F##InTypeName Value) : TypeName(NAME_StructProperty) \
{ \
    auto Struct = FAnyStruct{ FindObjectChecked<UScriptStruct>(ANY_PACKAGE, L#InTypeName) }; \
    FMemoryWriter Writer(Struct.Value, true); \
    Writer << Value; \
    Content = new Holder(Struct); \
}


template<> FORCEINLINE FAny::FAny(EForceInit) 
    : TypeName(NAME_None)
    , Content(nullptr) 
{}

template<> FORCEINLINE FAny::FAny(UClass* Value)
    : TypeName(NAME_Class)
    , Content(new Holder(Value))
{}

IMPL_ANY(bool, NAME_BoolProperty)
IMPL_ANY(BYTE, NAME_ByteProperty)
IMPL_ANY(int32, NAME_Int32Property)
IMPL_ANY(int64, NAME_Int64Property)
IMPL_ANY(float, NAME_FloatProperty)
IMPL_ANY(double, NAME_DoubleProperty)
IMPL_ANY(FString, NAME_StrProperty)
IMPL_ANY(FName, NAME_NameProperty)
IMPL_ANY(FText, NAME_TextProperty)
IMPL_ANY(FVector, NAME_Vector)
IMPL_ANY(FRotator, NAME_Rotator)
IMPL_ANY(FTransform, NAME_Transform)


IMPL_ANY(FAny::FAnyStruct, NAME_StructProperty)
IMPL_ANY(FAny::FAnyEnum, NAME_EnumProperty)
IMPL_ANY(FAny::FAnyObject, NAME_ObjectProperty)


IMPL_ANY_CUSTOM_STRUCT(IntPoint)
IMPL_ANY_CUSTOM_STRUCT(IntVector)


#undef IMPL_ANY
#undef IMPL_ANY_CUSTOM_STRUCT