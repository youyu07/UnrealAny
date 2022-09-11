#include "UnrealAnyEnumProperty.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "SEnumCombo.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"


void FAnyEnumProperty::Make(IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys)
{
	Anys = InAnys;
	auto Enum = Anys[0]->GetPtr<FAny::FAnyEnum>();

	auto EnumWidget = SNew(SEnumComboBox, Enum->Enum.Get())
		.CurrentValue(this, &FAnyEnumProperty::OnGetEnum)
		.OnEnumSelectionChanged(this, &FAnyEnumProperty::OnEnumChanged);

	auto NameWidget = SNew(STextBlock).Text(Enum->Enum->GetDisplayNameText());

	InChildBuilder.AddCustomRow(GetSearchText())
		.NameContent()[NameWidget]
		.ValueContent()[EnumWidget];
}


int32 FAnyEnumProperty::OnGetEnum() const
{
	return Anys[0]->GetPtr<FAny::FAnyEnum>()->Value;
}


void FAnyEnumProperty::OnEnumChanged(int32 InValue, ESelectInfo::Type InType)
{
	for (auto& Any : Anys)
	{
		Any->GetPtr<FAny::FAnyEnum>()->Value = InValue;
	}

	NotifyPostChange();
}


#undef LOCTEXT_NAMESPACE