// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealAnyEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "UnrealAnyCustomization.h"

#define LOCTEXT_NAMESPACE "FUnrealAnyEditor"

void FUnrealAnyEditor::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("Any", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUnrealAnyCustomization::MakeInstance));
}

void FUnrealAnyEditor::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealAnyEditor, UnrealAnyEditor);