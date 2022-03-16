#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealAny.h"
#include "UnrealAnyFunctionLibrary.generated.h"



UCLASS()
class UNREALANY_API UUnrealAnyFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, meta = (BlueprintAutocast, CompactNodeTitle = "Type"), Category = Any)
	static FName GetAnyType(const FAny& Any);

public:
	UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Value"), Category = Any)
	static FAny MakeAny(const int32 Value);

	UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Value"), Category = Any)
	static void CastAny(const FAny& Any, int32& Value);

private:
	DECLARE_FUNCTION(execMakeAny);
	DECLARE_FUNCTION(execCastAny);
};
