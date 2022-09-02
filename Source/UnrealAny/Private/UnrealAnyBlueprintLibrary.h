// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealAny.h"
#include "UnrealAnyBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UUnrealAnyBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, CustomThunk, meta = (CompactNodeTitle = "->", CustomStructureParam = "Value"))
	static FAny CastToAny(const int32 Value);

	UFUNCTION(BlueprintPure, CustomThunk, meta = (CompactNodeTitle = "->", CustomStructureParam = "Value"))
	static void AnyCast(const FAny& Any, int32& Value);

private:
	DECLARE_FUNCTION(execCastToAny);
	DECLARE_FUNCTION(execAnyCast);
};
