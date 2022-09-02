#include "UnrealAnyCustomization.h"
#include "DetailWidgetRow.h"
#include "UnrealAnyBlueprintLibrary.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "Slate.h"
#include <type_traits>

#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"

template<typename T>
static void OnActionSetType(TSharedRef<IPropertyHandle> Handle, TArray<FAny*> Anys, TSharedPtr<IPropertyUtilities> Utilities)
{
	Handle->NotifyPreChange();
	for (auto& it : Anys)
	{
		it->Set(T());
	}
	Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
	Utilities->ForceRefresh();
}


template<typename T>
static FUIAction CreateMenuAction(TSharedRef<IPropertyHandle> Handle, TArray<FAny*> Anys, TSharedPtr<IPropertyUtilities> Utilities)
{
	return FUIAction(
		FExecuteAction::CreateStatic(&OnActionSetType<T>, Handle, Anys, Utilities));
}

template<typename T>
class SAnyWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAnyWidget<T>)
	{}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& Args, TSharedRef<IPropertyHandle> PropertyHandle, const TArray<FAny*>& InAnys)
	{
		Handle = PropertyHandle;
		Anys = InAnys;
		ChildSlot[
			SNew(SNumericEntryBox<T>).AllowSpin(true)
				.MinValue(TAttribute<TOptional<T>>())
				.MaxValue(TAttribute<TOptional<T>>())
				.MinSliderValue(TAttribute<TOptional<T>>())
				.MaxSliderValue(TAttribute<TOptional<T>>())
				.Value(this, &SAnyWidget<T>::TempOnGetValue)
				.OnValueChanged(this, &SAnyWidget<T>::TempOnValueChanged)
		];
	}

private:
	void TempOnValueChanged(T Value)
	{
		Handle->NotifyPreChange();
		for (auto& it : Anys)
		{
			it->Set(Value);
		}
		Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}

	TOptional<T> TempOnGetValue() const
	{
		return Anys[0]->Get<T>();
	}

private:
	TSharedPtr<IPropertyHandle> Handle;
	TArray<FAny*> Anys;
};

template<typename T>
TSharedRef<SWidget> CreateAnyWidget(TSharedRef<IPropertyHandle> PropertyHandle, const TArray<FAny*>& Anys)
{
	if constexpr(std::is_same_v<void, T>)
		return SNullWidget::NullWidget;
	else 
		return SNew(SAnyWidget<T>, PropertyHandle, Anys);
}

static TFunction<TSharedRef<SWidget>(TSharedRef<IPropertyHandle>, const TArray<FAny*>&)> GetCreateAnyWidgetFunction(const FName& TypeName)
{
	static TMap<FName, TFunction<TSharedRef<SWidget>(TSharedRef<IPropertyHandle>, const TArray<FAny*>&)>> Map;
	bool bIsInitialized = false;
	if (!bIsInitialized) {
		Map.Add(FName(typeid(void).name()), CreateAnyWidget<void>);
		Map.Add(FName(typeid(bool).name()), CreateAnyWidget<bool>);
		Map.Add(FName(typeid(int32).name()), CreateAnyWidget<int32>);
		Map.Add(FName(typeid(int64).name()), CreateAnyWidget<int64>);
		Map.Add(FName(typeid(float).name()), CreateAnyWidget<float>);
		Map.Add(FName(typeid(double).name()), CreateAnyWidget<double>);

		Map.Add(FName(typeid(FVector).name()), CreateAnyWidget<FVector>);
	}

	if (Map.Contains(TypeName)) {
		return Map[TypeName];
	}
	return Map["void"];
}


#define ADD_MENU(Type, TypeStr) HeaderRow.AddCustomContextMenuAction(CreateMenuAction<Type>(InStructPropertyHandle, Anys, Utilities), LOCTEXT(#TypeStr##"TypeKey", #TypeStr))


TSharedRef<IPropertyTypeCustomization> FUnrealAnyCustomization::MakeInstance()
{
	return MakeShareable(new FUnrealAnyCustomization);
}

void FUnrealAnyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyUtilities> Utilities = StructCustomizationUtils.GetPropertyUtilities();

	TArray<FAny*> Anys;
	InStructPropertyHandle->EnumerateRawData([&](void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/) -> bool {
		Anys.Add(reinterpret_cast<FAny*>(RawData));
		return true;
	});

	InStructPropertyHandle->SetOnPropertyResetToDefault(FSimpleDelegate::CreateLambda([](TSharedRef<IPropertyHandle> Handle, const TArray<FAny*>& TempAnys, TSharedPtr<IPropertyUtilities> TempUtilities) {
		Handle->NotifyPreChange();
		auto Class = Handle->GetOuterBaseClass();
		auto Name = Handle->GetProperty()->GetFName();
		auto Defualt = Class->FindPropertyByName(Name)->ContainerPtrToValuePtr<FAny>(Class->GetDefaultObject());
		for (auto& Any : TempAnys)
		{
			*Any = *Defualt;
		}
		Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
		TempUtilities->ForceRefresh();
	}, InStructPropertyHandle, Anys, Utilities));


	if (Anys.Num() > 0) {
		
		ADD_MENU(bool, Bool);
		ADD_MENU(BYTE, Byte);
		ADD_MENU(int32, Int32);
		ADD_MENU(int64, Int64);
		ADD_MENU(float, Float);
		ADD_MENU(double, Double);
		ADD_MENU(FString, String);
		ADD_MENU(FName, Name);
		ADD_MENU(FText, Text);
		ADD_MENU(UClass*, Class);
		ADD_MENU(UObject*, Object);
		ADD_MENU(FVector, Vector);
		ADD_MENU(FRotator, Rotator);
		ADD_MENU(FTransform, Transform);
		ADD_MENU(FVector2D, Vector2D);
		ADD_MENU(FQuat, Quat);

		HeaderRow.NameContent()[InStructPropertyHandle->CreatePropertyNameWidget()];

		bool MultiTyps = false;

		TSet<FName> Types;
		for (auto& it : Anys)
		{
			Types.Add(it->TypeInfo().name());
		}

		if (Types.Num() == 1) {
			auto Type = *Types.begin();
			auto Widget = GetCreateAnyWidgetFunction(Type)(InStructPropertyHandle, Anys);
			HeaderRow.ValueContent().HAlign(HAlign_Fill)[Widget];
		}
	}
}


void FUnrealAnyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	
}


template<>
class SAnyWidget<bool> : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAnyWidget<bool>)
	{}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& Args, TSharedRef<IPropertyHandle> PropertyHandle, const TArray<FAny*>& InAnys)
	{
		Handle = PropertyHandle;
		Anys = InAnys;
		ChildSlot.HAlign(HAlign_Left)[
			SNew(SCheckBox).IsChecked(this, &SAnyWidget<bool>::TempOnGetValue)
				.OnCheckStateChanged(this, &SAnyWidget<bool>::TempOnValueChanged)
		];
	}

private:
	void TempOnValueChanged(ECheckBoxState Value)
	{
		Handle->NotifyPreChange();
		for (auto& it : Anys)
		{
			it->Set(Value == ECheckBoxState::Checked);
		}
		Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}

	ECheckBoxState TempOnGetValue() const
	{
		return Anys[0]->Get<bool>() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

private:
	TSharedPtr<IPropertyHandle> Handle;
	TArray<FAny*> Anys;
};


template<>
class SAnyWidget<FVector> : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAnyWidget<FVector>)
	{}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& Args, TSharedRef<IPropertyHandle> PropertyHandle, const TArray<FAny*>& InAnys)
	{
		Handle = PropertyHandle;
		Anys = InAnys;
		ChildSlot[
			SNew(SVectorInputBox).AllowSpin(true)
				.Vector(this, &SAnyWidget<FVector>::TempOnGetValue)
				.OnVectorCommitted(this, &SAnyWidget<FVector>::TempOnValueChanged)
		];
	}

private:
	void TempOnValueChanged(FVector3f Value, ETextCommit::Type CommitType)
	{
		Handle->NotifyPreChange();
		for (auto& it : Anys)
		{
			it->Set(FVector(Value));
		}
		Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}

	TOptional<FVector3f> TempOnGetValue() const
	{
		return FVector3f(Anys[0]->Get<FVector>());
	}

private:
	TSharedPtr<IPropertyHandle> Handle;
	TArray<FAny*> Anys;
};



#undef LOCTEXT_NAMESPACE
