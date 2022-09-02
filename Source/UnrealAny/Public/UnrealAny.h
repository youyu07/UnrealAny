// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UnrealAny.generated.h"


USTRUCT(BlueprintType)
struct UNREALANY_API FAny
{
	GENERATED_BODY()

public:
	constexpr FAny() noexcept {}

    FAny(const FAny& Other) : Content(Other.Content ? Other.Content->Clone() : nullptr) {}

    FAny(FAny&& Other) noexcept : Content(std::move(Other.Content))
    {
        Other.Content = nullptr;
    }

    template<class ValueType>
    FAny(const ValueType& Value) : Content(new Holder<ValueType>(Value)) {}


    template<class ValueType, class T = typename std::decay<ValueType>::type, typename std::enable_if<(!std::is_same<T, FAny>::value), int>::type = 0>
    FAny(ValueType&& Value) noexcept : Content(new Holder<T>(std::forward<ValueType>(Value))) {}



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

    template<class ValueType, class T = typename std::decay<ValueType>::type, typename std::enable_if<(!std::is_same<T, FAny>::value), int>::type = 0>
    FAny& operator=(T&& Value)
    {
        FAny(std::move(Value)).Swap(*this);
        return *this;
    }


	bool IsValid() {
		return Content != nullptr;
	}

    const std::type_info& TypeInfo() const noexcept
    {
        return Content ? Content->Type() : typeid(void);
    }


    template<class ValueType>
    inline ValueType Get() const
    {
        return *(&(static_cast<Holder<ValueType>*>(Content)->Held));
    }

    template<class ValueType>
    inline void Set(const ValueType& Value)
    {
        Content = new Holder<ValueType>(Value);
    }

    template<class ValueType, class T = typename std::decay<ValueType>::type, typename std::enable_if<(!std::is_same<T, FAny>::value), int>::type = 0>
    inline void Set(ValueType&& Value)
    {
        Content = new Holder<T>(std::forward<ValueType>(Value));
    }

public:
    friend bool operator==(const FAny& A, const FAny& B)
    {
        return A.Content == B.Content;
    }

private:
    void Reset() noexcept
    {
        if (Content) { delete Content; Content = nullptr; }
    }

    void Swap(FAny& Other) noexcept
    {
        std::swap(Content, Other.Content);
    }

private:
    class Placeholder
    {
    public:
        virtual ~Placeholder() {}
        virtual const std::type_info& Type() const = 0;
        virtual Placeholder* Clone() const = 0;
    };

    template<typename ValueType>
    class Holder : public Placeholder
    {
    public:
        Holder(const ValueType& Value) : Held(Value) {}

        Holder(ValueType&& Value) : Held(std::move(Value)) {}

        virtual const std::type_info& Type() const override
        {
            return typeid(ValueType);
        }

        virtual Placeholder* Clone() const override
        {
            return new Holder(Held);
        }

        ValueType Held;
    };

    Placeholder* Content = nullptr;


public:
    bool Serialize(FArchive& Ar);
    //bool Serialize(FStructuredArchive::FSlot& Slot) { return Serialize(Slot.GetUnderlyingArchive()); }
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
        //WithStructuredSerializer = true,
        WithSerializer = true,
    };
};