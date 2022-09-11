// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


struct CProvidesStaticEnum
{
    template<typename T>
    auto Requires(const T&) -> decltype(StaticEnum<T>());
};

struct CProvidesStaticBaseStruct
{
	template<typename T>
	auto Requires(const T&) -> decltype(TBaseStructure<T>().Get());
};


template<typename T>
struct TProvidesEnum
{
	static constexpr bool Value = TAnd<TModels<CProvidesStaticEnum, T>, TOr<TIsEnumClass<T>, TIsEnum<T>>>::Value;
};


template<typename T>
struct TProvidesObject
{
	static constexpr bool Value = TModels<CProvidesStaticClass, T>::Value;
};


template<typename T>
struct TProvidesStruct
{
	static constexpr bool Value = (TModels<CProvidesStaticStruct, T>::Value || TModels<CProvidesStaticBaseStruct, T>::Value);
};

template<typename T>
struct TGetType
{
	typedef T Type;
};

template<typename T>
struct TGetType<T*>
{
	typedef typename TGetType<T>::Type Type;
};