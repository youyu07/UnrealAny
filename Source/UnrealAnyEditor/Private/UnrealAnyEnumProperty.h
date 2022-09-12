// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UnrealAnyCustomization.h"


class FAnyEnumProperty : public FAnyProperty
{
public:
	FAnyEnumProperty(TSharedRef<class IPropertyHandle> InStructPropertyHandle) {
		Handle = InStructPropertyHandle;
	}

	virtual void Make(class IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys) override;

	TArray<int32> OnGetEnum() const;
	void OnEnumChanged(int32, ESelectInfo::Type);

protected:
	TArray<FAny*> Anys;
};