#pragma once

#include "CoreMinimal.h"


class FBlueprintAnyBase
{
public:
	virtual ~FBlueprintAnyBase() {}
	virtual const void* GetData() const = 0;
};


struct FBlueprintAnyStruct : public FBlueprintAnyBase
{
	UScriptStruct* Struct;
	TArray<uint8> Data;

	FBlueprintAnyStruct() {}
	FBlueprintAnyStruct(UScriptStruct* InStruct) :Struct(InStruct) { Data.SetNumZeroed(InStruct->GetStructureSize()); }

	virtual const void* GetData() const override
	{
		return Data.GetData();
	}
};

struct FBlueprintAnyEnum : public FBlueprintAnyBase
{
	UEnum* Enum;
	int64 Value;

	FBlueprintAnyEnum() {}
	FBlueprintAnyEnum(UEnum* InEnum, int64 InValue) :Enum(InEnum), Value(InValue) {  }

	virtual const void* GetData() const override
	{
		return &Value;
	}
};
