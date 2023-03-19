// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


namespace Any::Private
{
	struct CProvidesStaticEnum
	{
		template<typename T>
		auto Requires(const T&) -> decltype(StaticEnum<T>());
	};

	struct CProvidesStaticStruct
	{
		template<typename T>
		auto Requires(const T&) -> decltype(StaticStruct<T>().Get());
	};

	struct CProvidesStaticBaseStruct
	{
		template<typename T>
		auto Requires(const T&) -> decltype(TBaseStructure<T>().Get());
	};

	struct CProvidesStaticClass
	{
		template<typename T>
		auto Requires(const T&) -> decltype(StaticClass<T>());
	};

	template<typename T>
	struct TIsStaticStruct
	{
		enum { Value = TModels<CProvidesStaticStruct, T>::Value };
	};

	template<typename T>
	struct TIsStaticBaseStruct
	{
		enum { Value = TModels<CProvidesStaticBaseStruct, T>::Value };
	};


	template<typename T>
	struct TIsProvidesEnum
	{
		enum { Value = TAnd<TModels<CProvidesStaticEnum, T>, TOr<TIsEnumClass<T>, TIsEnum<T>>>::Value };
	};

	template<typename T>
	struct TIsProvidesObject
	{
		enum { Value = TModels<CProvidesStaticClass, T>::Value };
	};

	template<typename T>
	struct TIsProvidesStruct
	{
		enum { Value = TOr<TModels<CProvidesStaticClass, T>, TModels<CProvidesStaticBaseStruct, T>>::Value };
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
}