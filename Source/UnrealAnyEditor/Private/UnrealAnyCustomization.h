// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "UnrealAny.h"


class FAnyProperty : public TSharedFromThis<FAnyProperty>
{
public:
	virtual ~FAnyProperty() {};

	virtual void Make(class IDetailChildrenBuilder&, TArray<FAny*>) = 0;

	FText GetSearchText() const;

	void NotifyPostChange();

	FText MultipleValuesText() const;
protected:
	TSharedPtr<class IPropertyHandle> Handle;
};


/** Customization for the message tag struct */
class FUnrealAnyCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** Overridden to show an edit button to launch the message tag editor */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	
	/** Overridden to do nothing */
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	TSharedRef<class SPinTypeSelector> CreatePinSelector(TArray<struct FAny*>, TSharedRef<IPropertyHandle>, TSharedRef<IPropertyUtilities>, bool);


	TSharedPtr<FAnyProperty> AnyProperty;
};

