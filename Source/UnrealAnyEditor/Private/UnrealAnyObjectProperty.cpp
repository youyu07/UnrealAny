#include "UnrealAnyObjectProperty.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"


void FAnyObjectProperty::Make(IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys)
{
	Anys = InAnys;
	auto Object = Anys[0]->GetPtr<FAny::FAnyObject>();

	auto EnumWidget = SNew(SObjectPropertyEntryBox)
		.AllowedClass(Object->Class.Get())
		.ObjectPath(this, &FAnyObjectProperty::OnGetObjectPath)
		.OnObjectChanged(this, &FAnyObjectProperty::OnObjectChanged);


	auto NameText = Object->Class->GetDisplayNameText();
	auto NameWidget = SNew(STextBlock).Text(NameText);

	InChildBuilder.AddCustomRow(GetSearchText())
		.NameContent()[NameWidget]
		.ValueContent()[EnumWidget];
}


FString FAnyObjectProperty::OnGetObjectPath() const
{
	return Anys[0]->Get<FAny::FAnyObject>().Object->GetPathName();
}

void FAnyObjectProperty::OnObjectChanged(const FAssetData& AssetData)
{
	for (auto& Any : Anys)
	{
		Any->GetPtr<FAny::FAnyObject>()->Object = AssetData.GetAsset();
	}

	NotifyPostChange();
}

#undef LOCTEXT_NAMESPACE