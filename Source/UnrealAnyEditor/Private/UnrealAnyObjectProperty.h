// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UnrealAnyCustomization.h"


class FAnyObjectProperty : public FAnyProperty
{
public:
	FAnyObjectProperty(TSharedRef<class IPropertyHandle> InStructPropertyHandle) {
		Handle = InStructPropertyHandle;
	}

	virtual void Make(class IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys) override;

	FString OnGetObjectPath() const;
	void OnObjectChanged(const FAssetData&);

protected:
	TArray<FAny*> Anys;
};