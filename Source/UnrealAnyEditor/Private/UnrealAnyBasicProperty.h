// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UnrealAnyCustomization.h"
#include "Slate.h"

class FAnyBoolProperty : public FAnyProperty
{
public:
	FAnyBoolProperty(TSharedRef<class IPropertyHandle> InStructPropertyHandle) {
		Handle = InStructPropertyHandle;
	}

	virtual void Make(class IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys) override;

	ECheckBoxState OnGetCheckBoxState() const;
	void OnValueChanged(ECheckBoxState);

protected:
	TArray<FAny*> Anys;
};


template<typename T>
class FAnyNumberProperty : public FAnyProperty
{
public:
	FAnyNumberProperty(TSharedRef<class IPropertyHandle> InStructPropertyHandle) {
		Handle = InStructPropertyHandle;
	}

	virtual void Make(class IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys) override
	{
		Anys = InAnys;
		auto Widget = SNew(SNumericEntryBox<T>)
			.AllowSpin(true)
			.MinValue(TOptional<T>())
			.MaxValue(TOptional<T>())
			.MinSliderValue(TOptional<T>())
			.MaxSliderValue(TOptional<T>())
			.Value(this, &FAnyNumberProperty<T>::OnGetValue)
			.UndeterminedString(MultipleValuesText())
			.OnValueCommitted(this, &FAnyNumberProperty<T>::OnValueChanged);

		auto NameWidget = SNew(STextBlock).Text(GetNameText());

		InChildBuilder.AddCustomRow(GetSearchText())
			.NameContent()[NameWidget]
			.ValueContent()[Widget];
	}


	FText GetNameText() const;

	TOptional<T> OnGetValue() const
	{
		TSet<T> Values;
		for (auto& Any : Anys)
		{
			Values.Add(Any->Get<T>());
		}

		if (Values.Num() == 1) {
			return *Values.begin();
		}
		else {
			return TOptional<T>();
		}
	}

	void OnValueChanged(T Value, ETextCommit::Type)
	{
		for (auto& Any : Anys)
		{
			*Any = Value;
		}
		NotifyPostChange();
	}

protected:
	TArray<FAny*> Anys;
};


template<typename T>
class FAnyTextProperty : public FAnyProperty
{
public:
	FAnyTextProperty(TSharedRef<class IPropertyHandle> InStructPropertyHandle) {
		Handle = InStructPropertyHandle;
	}

	virtual void Make(IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys) override
	{
		Anys = InAnys;

		bool ReadOnly = false;

		auto Widget = SNew(SMultiLineEditableTextBox)
			.Text(this, &FAnyTextProperty::OnGetValue)
			.SelectAllTextWhenFocused(false)
			.ClearKeyboardFocusOnCommit(false)
			.OnTextCommitted(this, &FAnyTextProperty::OnValueChanged)
			.SelectAllTextOnCommit(false)
			.IsReadOnly(ReadOnly)
			.AutoWrapText(true)
			.ModiferKeyForNewLine(EModifierKey::Shift);

		auto NameWidget = SNew(STextBlock).Text(GetNameText());

		InChildBuilder.AddCustomRow(GetSearchText())
			.NameContent()[NameWidget]
			.ValueContent()[Widget];
	}

	FText GetNameText() const;

	FText OnGetValue() const;

	void OnValueChanged(const FText& NewText, ETextCommit::Type);

protected:
	TArray<FAny*> Anys;
};