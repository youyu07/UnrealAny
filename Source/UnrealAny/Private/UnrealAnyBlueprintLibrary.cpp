// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealAnyBlueprintLibrary.h"
#include "UnrealAnyCaster.h"


DEFINE_FUNCTION(UUnrealAnyBlueprintLibrary::execCastToAny)
{
	Stack.StepCompiledIn<FProperty>(NULL);
	auto Property = Stack.MostRecentProperty;
	uint8* Addr = Stack.MostRecentPropertyAddress;

	P_FINISH;
	P_NATIVE_BEGIN;
	EClassCastFlags Flags = EClassCastFlags(Property->GetCastFlags());
	*(FAny*)RESULT_PARAM = UAnyPropertyBase::FindCaster(Flags)->ToAny(Property, Addr);
	P_NATIVE_END;
}


DEFINE_FUNCTION(UUnrealAnyBlueprintLibrary::execAnyCast)
{
	P_GET_STRUCT_REF(FAny, Any);

	Stack.StepCompiledIn<FProperty>(NULL);
	auto Property = Stack.MostRecentProperty;
	uint8* Addr = Stack.MostRecentPropertyAddress;

	P_FINISH;
	P_NATIVE_BEGIN;
	EClassCastFlags Flags = EClassCastFlags(Property->GetCastFlags());
	UAnyPropertyBase::FindCaster(Flags)->WriteProperty(Any, Property, Addr);
	P_NATIVE_END;
}