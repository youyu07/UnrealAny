#include "UnrealAnyBasicProperty.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"


void FAnyBoolProperty::Make(IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys)
{
	Anys = InAnys;
	auto Widget = SNew(SCheckBox)
		.IsEnabled(!IsReadOnly())
		.IsChecked(this, &FAnyBoolProperty::OnGetCheckBoxState)
		.OnCheckStateChanged(this, &FAnyBoolProperty::OnValueChanged);

	auto NameWidget = SNew(STextBlock).Text(LOCTEXT("BoolKey", "Bool"));

	InChildBuilder.AddCustomRow(GetSearchText())
		.NameContent()[NameWidget]
		.ValueContent()[Widget];
}


ECheckBoxState FAnyBoolProperty::OnGetCheckBoxState() const
{
	TSet<ECheckBoxState> States;
	for (auto& Any : Anys)
	{
		States.Add(Any->Get<bool>() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	}

	return States.Num() > 1 ? ECheckBoxState::Undetermined : *States.begin();
}


void FAnyBoolProperty::OnValueChanged(ECheckBoxState State)
{
	for (auto& Any : Anys)
	{
		*Any = State == ECheckBoxState::Checked ? true : false;
	}

	NotifyPostChange();
}


template<> FText FAnyNumberProperty<BYTE>::GetNameText() const
{
	return LOCTEXT("ByteKey", "Byte");
}

template<> FText FAnyNumberProperty<int32>::GetNameText() const
{
	return LOCTEXT("Int32Key", "Int32");
}

template<> FText FAnyNumberProperty<int64>::GetNameText() const
{
	return LOCTEXT("Int64Key", "Int64");
}

template<> FText FAnyNumberProperty<float>::GetNameText() const
{
	return LOCTEXT("FloatKey", "Float");
}

template<> FText FAnyNumberProperty<double>::GetNameText() const
{
	return LOCTEXT("DoubleKey", "Double");
}



template<> FText FAnyTextProperty<FName>::GetNameText() const
{
	return LOCTEXT("NameKey", "Name");
}

template<> FText FAnyTextProperty<FText>::GetNameText() const
{
	return LOCTEXT("TextKey", "Text");
}

template<> FText FAnyTextProperty<FString>::GetNameText() const
{
	return LOCTEXT("StringKey", "String");
}


template<> FText FAnyTextProperty<FName>::OnGetValue() const
{
	TSet<FName> Values;
	for (auto& Any : Anys)
	{
		Values.Add(Any->Get<FName>());
	}
	if (Values.Num() > 1) return MultipleValuesText();
	else return FText::FromName(*Values.begin());
}

template<> FText FAnyTextProperty<FString>::OnGetValue() const
{
	TSet<FString> Values;
	for (auto& Any : Anys)
	{
		Values.Add(Any->Get<FString>());
	}
	if (Values.Num() > 1) return MultipleValuesText();
	else return FText::FromString(*Values.begin());
}

template<> FText FAnyTextProperty<FText>::OnGetValue() const
{
	TSet<FString> Values;
	for (auto& Any : Anys)
	{
		Values.Add(Any->Get<FText>().ToString());
	}
	if (Values.Num() > 1) return MultipleValuesText();
	else return Anys[0]->Get<FText>();
}


template<> void FAnyTextProperty<FName>::OnValueChanged(const FText& NewText, ETextCommit::Type)
{
	for (auto& Any : Anys)
	{
		*Any = FName(NewText.ToString());
	}
	NotifyPostChange();
}

template<> void FAnyTextProperty<FString>::OnValueChanged(const FText& NewText, ETextCommit::Type)
{
	for (auto& Any : Anys)
	{
		*Any = NewText.ToString();
	}
	NotifyPostChange();
}

template<> void FAnyTextProperty<FText>::OnValueChanged(const FText& NewText, ETextCommit::Type)
{
	for (auto& Any : Anys)
	{
		*Any = NewText;
	}
	NotifyPostChange();
}


#undef LOCTEXT_NAMESPACE