// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UnrealAnyCustomization.h"
#include "IPropertyUtilities.h"
#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"


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
		T* DataPtr = Anys[0]->GetPtr<T>();
		TSharedPtr<FStructOnScope> Scope = MakeShareable(new FStructOnScope(TBaseStructure<T>::Get(), reinterpret_cast<uint8*>(DataPtr)));
		Setup(InChildBuilder, Scope.ToSharedRef());
	}

	void Setup(IDetailChildrenBuilder& InChildBuilder, TSharedRef<FStructOnScope> Scope)
	{
		auto Name = Scope->GetStruct()->GetFName();
		auto Row = InChildBuilder.AddExternalStructureProperty(Scope, NAME_None, FAddPropertyParams());
		Row->DisplayName(FText::FromName(Name));
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
};