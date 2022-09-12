// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UnrealAnyCustomization.h"
#include "IPropertyUtilities.h"
#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"
#include "IStructureDetailsView.h"


template<typename T>
class FAnyStructProperty : public FAnyProperty
{
public:
	FAnyStructProperty(TSharedRef<IPropertyHandle> InStructPropertyHandle) {
		Handle = InStructPropertyHandle;
	}

	virtual void Make(IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys) override
	{
		Anys = InAnys;
		for (int32 i = 0; i < Anys.Num(); i++)
		{
			auto Value = Anys[i]->Get<T>();
			Data.Insert(reinterpret_cast<uint8*>(&Value), sizeof(T), i * sizeof(T));
		}

		if (Anys.Num() == 1) {
			TSharedPtr<FStructOnScope> Scope = MakeShareable(new FStructOnScope(TBaseStructure<T>::Get(), Data.GetData()));
			Setup(InChildBuilder, Scope);
		}
	}

	void Setup(IDetailChildrenBuilder& InChildBuilder, TSharedPtr<FStructOnScope> Scope)
	{
		auto Name = Scope->GetStruct()->GetFName();
		auto Row = InChildBuilder.AddExternalStructureProperty(Scope.ToSharedRef(), NAME_None, FAddPropertyParams());
		Row->DisplayName(FText::FromName(Name));
		Row->EditCondition(!IsReadOnly(), FOnBooleanValueChanged());

		ChildHandle = Row->GetPropertyHandle();
		ChildHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FAnyStructProperty<T>::OnValueChanged));
	}

	void OnValueChanged()
	{
		ChildHandle->EnumerateRawData([&](void* RawData, const int32 Index, const int32 /*NumDatas*/) -> bool {
			*Anys[Index] = *reinterpret_cast<T*>(RawData);
			return true;
			});
		NotifyPostChange();
	}

protected:
	TSharedPtr<IPropertyHandle> ChildHandle;
	TArray<FAny*> Anys;

	TArray<uint8> Data;
};