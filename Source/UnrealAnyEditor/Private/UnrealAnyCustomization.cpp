#include "UnrealAnyCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "IDetailPropertyRow.h"
#include "Slate.h"


#include "SPinTypeSelector.h"
#include "UnrealAnyBasicProperty.h"
#include "UnrealAnyStructProperty.h"
#include "UnrealAnyEnumProperty.h"
#include "UnrealAnyClassProperty.h"
#include "UnrealAnyObjectProperty.h"


#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"


FText FAnyProperty::GetSearchText() const
{
	return LOCTEXT("AnyKey", "Any");
}

void FAnyProperty::NotifyPostChange()
{
	Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
}

FText FAnyProperty::MultipleValuesText() const
{
	return LOCTEXT("MultipleValues", "Multiple Values");
}


TSharedRef<IPropertyTypeCustomization> FUnrealAnyCustomization::MakeInstance()
{
	return MakeShareable(new FUnrealAnyCustomization);
}



void FUnrealAnyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyUtilities> Utilities = StructCustomizationUtils.GetPropertyUtilities();

	TArray<FAny*> Anys;
	InStructPropertyHandle->EnumerateRawData([&](void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/) -> bool {
		Anys.Add(reinterpret_cast<FAny*>(RawData));
		return true;
	});

	InStructPropertyHandle->SetOnPropertyResetToDefault(FSimpleDelegate::CreateLambda([](TSharedRef<IPropertyHandle> Handle, const TArray<FAny*>& TempAnys, TSharedPtr<IPropertyUtilities> TempUtilities) {
		Handle->NotifyPreChange();
		auto Class = Handle->GetOuterBaseClass();
		auto Name = Handle->GetProperty()->GetFName();
		auto Defualt = Class->FindPropertyByName(Name)->ContainerPtrToValuePtr<FAny>(Class->GetDefaultObject());
		for (auto& Any : TempAnys)
		{
			*Any = *Defualt;
		}
		Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
		TempUtilities->ForceRefresh();
	}, InStructPropertyHandle, Anys, Utilities));

	TSet<EName> Types;
	for (auto& it : Anys)
	{
		Types.Add(it->Type());
	}

	if (Anys.Num() > 0) {
		
		auto Schema = GetDefault<UEdGraphSchema_K2>();
		auto TypeSelector = CreatePinSelector(Anys, InStructPropertyHandle, Utilities.ToSharedRef(), Types.Num() > 1);

		HeaderRow.NameContent()[
			InStructPropertyHandle->CreatePropertyNameWidget()
		];

		HeaderRow.ValueContent()[TypeSelector];

		if (Types.Num() == 1) {
			auto Type = *Types.begin();

			switch (Type)
			{
			case NAME_StructProperty:
			{
				InStructPropertyHandle->AddChildStructure(MakeShareable(new FStructOnScope(Anys[0]->Get<FAny::FAnyStruct>().Struct.Get())));
				break;
			}
			case NAME_Vector:
			{
				InStructPropertyHandle->AddChildStructure(MakeShareable(new FStructOnScope(TBaseStructure<FVector>::Get())));
				break;
			}
			case NAME_Rotator:
			{
				InStructPropertyHandle->AddChildStructure(MakeShareable(new FStructOnScope(TBaseStructure<FRotator>::Get())));
				break;
			}
			case NAME_Transform:
			{
				InStructPropertyHandle->AddChildStructure(MakeShareable(new FStructOnScope(TBaseStructure<FTransform>::Get())));
				break;
			}
			default:
			{
				InStructPropertyHandle->AddChildStructure(MakeShareable(new FStructOnScope()));
				break;
			}
			}
		}
	}
}



#define CREATE_PROPERTY(TypeName, Type) \
case TypeName: \
{ \
	AnyProperty = MakeShareable(new Type(InStructPropertyHandle)); \
	AnyProperty->Make(ChildBuilder, Anys); \
	break; \
}


void FUnrealAnyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TArray<FAny*> Anys;
	InStructPropertyHandle->EnumerateRawData([&](void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/) -> bool {
		Anys.Add(reinterpret_cast<FAny*>(RawData));
		return true;
	});

	TSet<EName> Types;
	for (auto& it : Anys)
	{
		Types.Add(it->Type());
	}

	if (Types.Num() == 1) {
		auto Type = *Types.begin();
		switch (Type)
		{
		CREATE_PROPERTY(NAME_BoolProperty, FAnyBoolProperty)
		CREATE_PROPERTY(NAME_ByteProperty, FAnyNumberProperty<BYTE>)
		CREATE_PROPERTY(NAME_Int32Property, FAnyNumberProperty<int32>)
		CREATE_PROPERTY(NAME_Int64Property, FAnyNumberProperty<int64>)
		CREATE_PROPERTY(NAME_FloatProperty, FAnyNumberProperty<float>)
		CREATE_PROPERTY(NAME_DoubleProperty, FAnyNumberProperty<double>)
		CREATE_PROPERTY(NAME_NameProperty, FAnyTextProperty<FName>)
		CREATE_PROPERTY(NAME_StrProperty, FAnyTextProperty<FString>)
		CREATE_PROPERTY(NAME_TextProperty, FAnyTextProperty<FText>)

		CREATE_PROPERTY(NAME_StructProperty, FAnyStructProperty<FAny::FAnyStruct>)
		CREATE_PROPERTY(NAME_Vector, FAnyStructProperty<FVector>)
		CREATE_PROPERTY(NAME_Rotator, FAnyStructProperty<FRotator>)
		CREATE_PROPERTY(NAME_Transform, FAnyStructProperty<FTransform>)
		CREATE_PROPERTY(NAME_EnumProperty, FAnyEnumProperty)
		CREATE_PROPERTY(NAME_Class, FAnyClassProperty)
		CREATE_PROPERTY(NAME_ObjectProperty, FAnyObjectProperty)
		}
	}
}


#undef LOCTEXT_NAMESPACE