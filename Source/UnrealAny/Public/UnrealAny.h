// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UnrealAny.generated.h"


USTRUCT(BlueprintType, meta=(HasNativeMake))
struct FAny
{
    GENERATED_BODY()
public:
    constexpr FAny() noexcept : Content(nullptr){}

    FAny(const FAny& Other) : Content(Other.Content ? Other.Content->Clone() : nullptr){}

    FAny(FAny&& Other) noexcept : Content(std::move(Other.Content))
    {
        Other.Content = nullptr;
    }

    template<class ValueType>
    FAny(const ValueType& Value) : Content(new Holder<ValueType>(Value)){}

    template<class ValueType, class T = typename std::decay<ValueType>::type, typename std::enable_if<(!std::is_same<T, FAny>::value), int>::type = 0>
    FAny(ValueType&& Value) noexcept : Content(new Holder<T>(std::forward<ValueType>(Value))){}

    ~FAny()
    {
        Reset();
    }

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

    void Reset() noexcept
    {
        delete Content; Content = nullptr;
    }

    void Swap(FAny& Other) noexcept
    {
        std::swap(Content, Other.Content);
    }

    bool HasValue() const noexcept
    {
        return Content != nullptr;
    }

    const std::type_info& Type() const noexcept
    {
        return HasValue() ? Content->Type() : typeid(void);
    }

    const FString TypeName() const noexcept
    {
        return FString(Type().name());
    }

    template<class ValueType>
    inline ValueType Cast() const
    {
        check(Type() == typeid(ValueType));
        const ValueType* result = &(static_cast<Holder<ValueType>*>(Content)->Held);
        return *result;
    }

private:
    class Placeholder
    {
    public:
        virtual ~Placeholder(){}
        virtual const std::type_info& Type() const = 0;
        virtual Placeholder* Clone() const = 0;
    };

    template<typename ValueType>
    class Holder : public Placeholder
    {
    public:
        Holder(ValueType const& Value) : Held(Value){}

        Holder(ValueType&& Value) : Held(std::move(Value)){}

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

    Placeholder* Content;
};