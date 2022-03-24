// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

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
	FEdGraphPinType OnGetPinInfo(struct FAny* Any) const;
	void PinInfoChanged(const FEdGraphPinType& PinType, struct FAny* Any);
	void OnValueChanged(struct FAny* Any);
	void InitializeStructValue(struct FAny* Any, struct FUnrealAnyEditorStructure* Struct);

	TSharedPtr<FStructOnScope> StructOnScope;
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;

	struct FAny* GetAnyPtr() const;
};

