// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UnrealAnyCustomization.h"


class FAnyClassProperty : public FAnyProperty
{
public:
	FAnyClassProperty(TSharedRef<class IPropertyHandle> InStructPropertyHandle) {
		Handle = InStructPropertyHandle;
	}

	virtual void Make(class IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys) override;

	const UClass* OnGetClass() const;
	void OnClassChanged(const UClass*);

protected:
	TArray<FAny*> Anys;
};