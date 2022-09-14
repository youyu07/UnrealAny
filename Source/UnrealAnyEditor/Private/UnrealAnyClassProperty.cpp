#include "UnrealAnyClassProperty.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"


void FAnyClassProperty::Make(IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys)
{
	Anys = InAnys;

	if (Anys.Num() == 1) {
		auto Class = Anys[0]->GetPtr<FAny::FAnyClass>();

		auto Widget = SNew(SClassPropertyEntryBox)
			.IsEnabled(!IsReadOnly())
			.MetaClass(Class->BaseClass.Get())
			.SelectedClass(this, &FAnyClassProperty::OnGetClass)
			.OnSetClass(this, &FAnyClassProperty::OnClassChanged);

		auto NameText = Class->BaseClass->GetDisplayNameText();
		auto NameWidget = SNew(STextBlock).Text(NameText);

		InChildBuilder.AddCustomRow(GetSearchText())
			.NameContent()[NameWidget]
			.ValueContent()[Widget];
	}
}

const UClass* FAnyClassProperty::OnGetClass() const
{
	return Anys[0]->Get<FAny::FAnyClass>().Class.Get();
}


void FAnyClassProperty::OnClassChanged(const UClass* InClass)
{
	for (auto& Any : Anys)
	{
		Any->GetPtr<FAny::FAnyClass>()->Class = const_cast<UClass*>(InClass);
	}

	NotifyPostChange();
}


#undef LOCTEXT_NAMESPACE