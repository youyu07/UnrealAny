// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealAnyModule.h"
#include "UnrealAny.h"

#define LOCTEXT_NAMESPACE "FUnrealAnyModule"



class FUnrealAnyModule : public IUnrealAnyModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		
	}
	virtual void ShutdownModule() override
	{

	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealAnyModule, UnrealAny)

