// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UnrealAnyEditorStructure.generated.h"


USTRUCT()
struct FUnrealAnyEditorStructure
{
	GENERATED_BODY()

	//0
	UPROPERTY(EditAnywhere)
	bool Boolean;
	//1
	UPROPERTY(EditAnywhere)
	int32 Int;
	//2
	UPROPERTY(EditAnywhere)
	float Float;
	//3
	UPROPERTY(EditAnywhere)
	double Double;
	//4
	UPROPERTY(EditAnywhere)
	FString String;
	//5
	UPROPERTY(EditAnywhere)
	FName Name;
	//6
	UPROPERTY(EditAnywhere)
	FText Text;
	//7
	UPROPERTY(EditAnywhere)
	FVector Vector;
	//8
	UPROPERTY(EditAnywhere)
	FVector2D Vector2D;
	//9
	UPROPERTY(EditAnywhere)
	FRotator Rotator;
	//10
	UPROPERTY(EditAnywhere)
	FTransform Transform;
	//11
	UPROPERTY(EditAnywhere)
	FLinearColor LinearColor;
	//12
	UPROPERTY(EditAnywhere)
	UClass* Class;
	//13
	UPROPERTY(EditAnywhere)
	UObject* Object;
};
