#include "UnrealAnyCustomization.h"
#include "DetailWidgetRow.h"
#include "UnrealAnyFunctionLibrary.h"
#include "IDetailChildrenBuilder.h"
#include "SPinTypeSelector.h"
#include "EdGraphSchema_K2.h"
#include "IPropertyUtilities.h"

#include "UnrealAnyEditorStructure.h"


#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"

TSharedRef<IPropertyTypeCustomization> FUnrealAnyCustomization::MakeInstance()
{
	return MakeShareable(new FUnrealAnyCustomization);
}

void FUnrealAnyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyHandle = InStructPropertyHandle;
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();
	void* ValueAddress = nullptr;
	if (InStructPropertyHandle->GetValueData(ValueAddress)) {
		auto AnyPtr = (FAny*)ValueAddress;

		HeaderRow.NameContent()[
			InStructPropertyHandle->CreatePropertyNameWidget()
		];

		auto Struct = new TStructOnScope<FUnrealAnyEditorStructure>();
		Struct->InitializeAs<FUnrealAnyEditorStructure>(FUnrealAnyEditorStructure());
		StructOnScope = MakeShareable(Struct);
		InitializeStructValue(AnyPtr, Struct->Get());

		auto Children = InStructPropertyHandle->AddChildStructure(StructOnScope.ToSharedRef());

		auto Schema = GetDefault<UEdGraphSchema_K2>();

		auto TypeSelector = SNew(SPinTypeSelector, FGetPinTypeTree::CreateUObject(Schema, &UEdGraphSchema_K2::GetVariableTypeTree))
			.TargetPinType(this, &FUnrealAnyCustomization::OnGetPinInfo, AnyPtr)
			.OnPinTypeChanged(this, &FUnrealAnyCustomization::PinInfoChanged, AnyPtr)
			.SelectorType(SPinTypeSelector::ESelectorType::Partial)
			.Schema(Schema);

		HeaderRow.ValueContent().VAlign(VAlign_Center).HAlign(HAlign_Fill)[TypeSelector];
	}
}


void FUnrealAnyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (auto Self = InStructPropertyHandle->GetChildHandle(0)) {
		auto AnyPtr = GetAnyPtr();
		if (!AnyPtr) return;
		if (auto Index = FAny::GetAnyTypes().Find(AnyPtr->TypeFName())) {
			if (auto Child = Self->GetChildHandle(*Index)) {
				Child->SetOnPropertyValueChanged(FSimpleDelegate::CreateRaw(this, &FUnrealAnyCustomization::OnValueChanged, AnyPtr));
				Child->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateRaw(this, &FUnrealAnyCustomization::OnValueChanged, AnyPtr));
				ChildBuilder.AddProperty(Child.ToSharedRef());
			}
		}
	}
}

FAny* FUnrealAnyCustomization::GetAnyPtr() const
{
	void* ValueAddress = nullptr;
	if (PropertyHandle->GetValueData(ValueAddress)) {
		return (FAny*)ValueAddress;
	}
	return nullptr;
}

FEdGraphPinType FUnrealAnyCustomization::OnGetPinInfo(FAny* Any) const
{
	if (auto Type = FAny::GetAnyTypes().Find(Any->TypeFName())) {
		switch (*Type)
		{
		case 0: return FEdGraphPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
		case 1: return FEdGraphPinType(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
		case 2: return FEdGraphPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
		case 3: return FEdGraphPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
		case 4: return FEdGraphPinType(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
		case 5: return FEdGraphPinType(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
		case 6: return FEdGraphPinType(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
		case 7: return FEdGraphPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FVector>().Get(), EPinContainerType::None, false, FEdGraphTerminalType());
		case 8: return FEdGraphPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FVector2D>().Get(), EPinContainerType::None, false, FEdGraphTerminalType());
		case 9: return FEdGraphPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FRotator>().Get(), EPinContainerType::None, false, FEdGraphTerminalType());
		case 10: return FEdGraphPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FTransform>().Get(), EPinContainerType::None, false, FEdGraphTerminalType());
		case 11: return FEdGraphPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FLinearColor>().Get(), EPinContainerType::None, false, FEdGraphTerminalType());
		case 12: return FEdGraphPinType(UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass(), EPinContainerType::None, false, FEdGraphTerminalType());
		case 13: return FEdGraphPinType(UEdGraphSchema_K2::PC_Object, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
		}
	}
	return FEdGraphPinType();
}

void FUnrealAnyCustomization::PinInfoChanged(const FEdGraphPinType& PinType, FAny* Any)
{
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Boolean) {
		*Any = false;
	}
	else if (PinType.PinCategory == UEdGraphSchema_K2::PC_Int) {
		*Any = 0;
	}
	else if (PinType.PinCategory == UEdGraphSchema_K2::PC_Real) {
		if (PinType.PinSubCategory == UEdGraphSchema_K2::PC_Double) {
			*Any = 0.0;
		}
		else if(PinType.PinSubCategory == UEdGraphSchema_K2::PC_Float) {
			*Any = 0.0f;
		}
	}
	else if (PinType.PinCategory == UEdGraphSchema_K2::PC_String) {
		*Any = FString();
	}
	else if (PinType.PinCategory == UEdGraphSchema_K2::PC_Name) {
		*Any = FName();
	}
	else if (PinType.PinCategory == UEdGraphSchema_K2::PC_Text) {
		*Any = FText();
	}
	else if (PinType.PinCategory == UEdGraphSchema_K2::PC_Struct) {
		if (PinType.PinSubCategoryObject == TBaseStructure<FVector>().Get()) {
			*Any = FVector::ZeroVector;
		}
		else if (PinType.PinSubCategoryObject == TBaseStructure<FVector2D>().Get()) {
			*Any = FVector2D::ZeroVector;
		}
		else if (PinType.PinSubCategoryObject == TBaseStructure<FRotator>().Get()) {
			*Any = FRotator::ZeroRotator;
		}
		else if (PinType.PinSubCategoryObject == TBaseStructure<FTransform>().Get()) {
			*Any = FTransform::Identity;
		}
		else if (PinType.PinSubCategoryObject == TBaseStructure<FLinearColor>().Get()) {
			*Any = FLinearColor::Transparent;
		}
		else {
			return;
		}
	}
	else if (PinType.PinCategory == UEdGraphSchema_K2::PC_Class) {
		*Any = (UClass*)nullptr;
	}
	else if (PinType.PinCategory == UEdGraphSchema_K2::PC_Object) {
		*Any = (UObject*)nullptr;
	}
	else {
		return;
	}
	PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	PropertyUtilities->ForceRefresh();
}

void FUnrealAnyCustomization::OnValueChanged(FAny* Any)
{
	auto Struct = (FUnrealAnyEditorStructure*)StructOnScope->GetStructMemory();

	if (auto Type = FAny::GetAnyTypes().Find(Any->TypeFName())) {
		switch (*Type)
		{
		case 0: {*Any = Struct->Boolean; break; }
		case 1: {*Any = Struct->Int; break; }
		case 2: {*Any = Struct->Float; break; }
		case 3: {*Any = Struct->Double; break; }
		case 4: {*Any = Struct->String; break; }
		case 5: {*Any = Struct->Name; break; }
		case 6: {*Any = Struct->Text; break; }
		case 7: {*Any = Struct->Vector; break; }
		case 8: {*Any = Struct->Vector2D; break; }
		case 9: {*Any = Struct->Rotator; break; }
		case 10: {*Any = Struct->Transform; break; }
		case 11: {*Any = Struct->LinearColor; break; }
		case 12: {*Any = Struct->Class; break; }
		case 13: {*Any = Struct->Object; break; }
		}
	}
	PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
}


void FUnrealAnyCustomization::InitializeStructValue(FAny* Any, FUnrealAnyEditorStructure* Struct)
{
	if (auto Type = FAny::GetAnyTypes().Find(Any->TypeFName())) {
		switch (*Type)
		{
		case 0: {Struct->Boolean = Any->CastUnchecked<bool>(); break; }
		case 1: {Struct->Int = Any->CastUnchecked<int32>(); break; }
		case 2: {Struct->Float = Any->CastUnchecked<float>(); break; }
		case 3: {Struct->Double = Any->CastUnchecked<double>(); break; }
		case 4: {Struct->String = Any->CastUnchecked<FString>(); break; }
		case 5: {Struct->Name = Any->CastUnchecked<FName>(); break; }
		case 6: {Struct->Text = Any->CastUnchecked<FText>(); break; }
		case 7: {Struct->Vector = Any->CastUnchecked<FVector>(); break; }
		case 8: {Struct->Vector2D = Any->CastUnchecked<FVector2D>(); break; }
		case 9: {Struct->Rotator = Any->CastUnchecked<FRotator>(); break; }
		case 10: {Struct->Transform = Any->CastUnchecked<FTransform>(); break; }
		case 11: {Struct->LinearColor = Any->CastUnchecked<FLinearColor>(); break; }
		case 12: {Struct->Class = Any->CastUnchecked<UClass*>(); break; }
		case 13: {Struct->Object = Any->CastUnchecked<UObject*>(); break; }
		}
	}
}


#undef LOCTEXT_NAMESPACE
