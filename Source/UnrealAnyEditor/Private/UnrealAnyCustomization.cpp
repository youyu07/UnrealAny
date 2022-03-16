#include "UnrealAnyCustomization.h"
#include "DetailWidgetRow.h"
#include "UnrealAnyFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"

TSharedRef<IPropertyTypeCustomization> FUnrealAnyCustomization::MakeInstance()
{
	return MakeShareable(new FUnrealAnyCustomization);
}


void FUnrealAnyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	void* ValueAddress = nullptr;
	if (InStructPropertyHandle->GetValueData(ValueAddress)) {
		auto AnyPtr = (FAny*)ValueAddress;

		HeaderRow.NameContent()[
			InStructPropertyHandle->CreatePropertyNameWidget()
		];

		HeaderRow.ValueContent().VAlign(VAlign_Center).HAlign(HAlign_Fill)[
			SNew(STextBlock).Text(FText::FromName(UUnrealAnyFunctionLibrary::GetAnyType(*AnyPtr)))
		];
	}
}

#undef LOCTEXT_NAMESPACE
