#include "UnrealAnyCustomization.h"
#include "DetailWidgetRow.h"
#include "UnrealAnyBlueprintLibrary.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "Slate.h"
#include <type_traits>


#include "SPinTypeSelector.h"
#include "EdGraphSchema_K2.h"


#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"


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
			*it = Value;
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

static TFunction<TSharedRef<SWidget>(TSharedRef<IPropertyHandle>, const TArray<FAny*>&)> GetCreateAnyWidgetFunction(const EName& TypeName)
{
	static TMap<EName, TFunction<TSharedRef<SWidget>(TSharedRef<IPropertyHandle>, const TArray<FAny*>&)>> Map;
	bool bIsInitialized = false;
	if (!bIsInitialized) {
		Map.Add(NAME_None, CreateAnyWidget<void>);
		Map.Add(NAME_BoolProperty, CreateAnyWidget<bool>);
		Map.Add(NAME_Int32Property, CreateAnyWidget<int32>);
		Map.Add(NAME_Int64Property, CreateAnyWidget<int64>);
		Map.Add(NAME_FloatProperty, CreateAnyWidget<float>);
		Map.Add(NAME_Double, CreateAnyWidget<double>);

		Map.Add(NAME_Vector, CreateAnyWidget<FVector>);
	}

	if (Map.Contains(TypeName)) {
		return Map[TypeName];
	}
	return Map[NAME_None];
}


TSharedRef<IPropertyTypeCustomization> FUnrealAnyCustomization::MakeInstance()
{
	return MakeShareable(new FUnrealAnyCustomization);
}


#define DEFINE_PINTYPE(Category, SubCategory, SubObject) FEdGraphPinType(Category, SubCategory, SubObject, EPinContainerType::None, false, FEdGraphTerminalType())

static FEdGraphPinType OnGetPinInfo(TArray<FAny*> Anys, bool MultiType)
{
	if (!MultiType) {
		auto Type = Anys[0]->Type();
		switch (Type)
		{
		case NAME_BoolProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr);
		case NAME_ByteProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr);
		case NAME_Int32Property: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr);
		case NAME_Int64Property: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr);
		case NAME_FloatProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, nullptr);
		case NAME_DoubleProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double, nullptr);
		case NAME_StrProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_String, NAME_None, nullptr);
		case NAME_NameProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr);
		case NAME_TextProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr);

		case NAME_Vector: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FVector>::Get());
		case NAME_Rotator: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FRotator>::Get());
		case NAME_Transform: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FTransform>::Get());

		case NAME_StructProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Struct, NAME_None, Anys[0]->Get<FAny::FAnyStruct>().Struct.Get());
		case NAME_EnumProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Enum, NAME_None, Anys[0]->Get<FAny::FAnyEnum>().Enum.Get());
		case NAME_ObjectProperty: return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Object, NAME_None, Anys[0]->Get<FAny::FAnyObject>().Class.Get());
		}
	}

	return FEdGraphPinType();
}

template<typename T>
static void InitAnyBasic(const FEdGraphPinType& PinType, const TArray<FAny*>& Anys)
{
	for (auto& Any : Anys)
	{
		*Any = T();
	}
}

static void InitAnyReal(const FEdGraphPinType& PinType, const TArray<FAny*>& Anys)
{
	for (auto& Any : Anys)
	{
		if (PinType.PinSubCategory == UEdGraphSchema_K2::PC_Float) {
			*Any = 0.0f;
		}
		else if (PinType.PinSubCategory == UEdGraphSchema_K2::PC_Double) {
			*Any = 0.0;
		}
	}
}

static void InitAnyStruct(const FEdGraphPinType& PinType, const TArray<FAny*>& Anys)
{
	if (auto EName = PinType.PinSubCategoryObject->GetFName().ToEName()) {
		switch (*EName)
		{
		case NAME_Vector: InitAnyBasic<FVector>(PinType, Anys); return;
		case NAME_Rotator: InitAnyBasic<FRotator>(PinType, Anys); return;
		case NAME_Transform: InitAnyBasic<FTransform>(PinType, Anys); return;
		case NAME_IntPoint: InitAnyBasic<FIntPoint>(PinType, Anys); return;
		}
	}

	FAny::FAnyStruct AnyStruct{ Cast<UScriptStruct>(PinType.PinSubCategoryObject) };
	AnyStruct.Value.SetNumZeroed(AnyStruct.Struct->GetStructureSize());
	for (auto& Any : Anys)
	{
		*Any = AnyStruct;
	}
}

static void InitAnyEnum(const FEdGraphPinType& PinType, const TArray<FAny*>& Anys)
{
	FAny::FAnyEnum AnyEnum{ Cast<UEnum>(PinType.PinSubCategoryObject) };
	for (auto& Any : Anys)
	{
		*Any = AnyEnum;
	}
}

static void InitAnyObject(const FEdGraphPinType& PinType, const TArray<FAny*>& Anys)
{
	FAny::FAnyObject AnyObject{ Cast<UClass>(PinType.PinSubCategoryObject) };
	for (auto& Any : Anys)
	{
		*Any = AnyObject;
	}
}

static void PinInfoChanged(const FEdGraphPinType& PinType, TArray<FAny*> Anys, TSharedRef<IPropertyHandle> Handle, TSharedPtr<IPropertyUtilities> Utilities)
{
	static TMap<FName, TFunction<void(const FEdGraphPinType& PinType, const TArray<FAny*>&)>> InitMap;
	static bool IsInitialized = false;
	if (!IsInitialized) {
		IsInitialized = true;
		InitMap.Add(UEdGraphSchema_K2::PC_Boolean, InitAnyBasic<bool>);
		InitMap.Add(UEdGraphSchema_K2::PC_Byte, InitAnyBasic<BYTE>);
		InitMap.Add(UEdGraphSchema_K2::PC_Int, InitAnyBasic<int32>);
		InitMap.Add(UEdGraphSchema_K2::PC_Int64, InitAnyBasic<int64>);
		InitMap.Add(UEdGraphSchema_K2::PC_Real, InitAnyReal);
		InitMap.Add(UEdGraphSchema_K2::PC_String, InitAnyBasic<FString>);
		InitMap.Add(UEdGraphSchema_K2::PC_Name, InitAnyBasic<FName>);
		InitMap.Add(UEdGraphSchema_K2::PC_Text, InitAnyBasic<FText>);

		InitMap.Add(UEdGraphSchema_K2::PC_Struct, InitAnyStruct);
		InitMap.Add(UEdGraphSchema_K2::PC_Enum, InitAnyEnum);
		InitMap.Add(UEdGraphSchema_K2::PC_Object, InitAnyObject);
	}

	Handle->NotifyPreChange();

	if (auto it = InitMap.Find(PinType.PinCategory)) {
		(*it)(PinType, Anys);
	}

	Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
	Utilities->ForceRefresh();
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

	TSet<EName> Types;
	for (auto& it : Anys)
	{
		Types.Add(it->Type());
	}

	if (Anys.Num() > 0) {
		
		auto Schema = GetDefault<UEdGraphSchema_K2>();
		auto TypeSelector = SNew(SPinTypeSelector, FGetPinTypeTree::CreateUObject(Schema, &UEdGraphSchema_K2::GetVariableTypeTree))
			.TargetPinType_Static(OnGetPinInfo, Anys, Types.Num() > 1)
			.OnPinTypeChanged_Static(PinInfoChanged, Anys, InStructPropertyHandle, Utilities)
			.SelectorType(SPinTypeSelector::ESelectorType::Partial)
			.Schema(Schema);

		HeaderRow.NameContent().HAlign(HAlign_Fill)[
			SNew(SHorizontalBox) + SHorizontalBox::Slot()[
				InStructPropertyHandle->CreatePropertyNameWidget()
			] + SHorizontalBox::Slot().HAlign(HAlign_Right)[
				TypeSelector
			]
		];

		if (Types.Num() == 1) {
			auto Type = *Types.begin();
			auto Widget = GetCreateAnyWidgetFunction(Type)(InStructPropertyHandle, Anys);
			HeaderRow.ValueContent().HAlign(HAlign_Fill)[Widget];

			if (Type == NAME_StructProperty) {
				InStructPropertyHandle->AddChildStructure(MakeShareable(new FStructOnScope(Anys[0]->Get<FAny::FAnyStruct>().Struct.Get())));
			}
		}
	}
}


void FUnrealAnyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TArray<FAny*> Anys;
	InStructPropertyHandle->EnumerateRawData([&](void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/) -> bool {
		Anys.Add(reinterpret_cast<FAny*>(RawData));
		return true;
	});

	TSet<EName> Types;
	for (auto& it : Anys)
	{
		Types.Add(it->Type());
	}

	if (Types.Num() == 1 && (*Types.begin()) == NAME_StructProperty) {
		TSharedPtr<FStructOnScope> Scope = MakeShareable(new FStructOnScope(Anys[0]->Get<FAny::FAnyStruct>().Struct.Get()));
		auto Name = Scope->GetStruct()->GetFName();
		ChildBuilder.AddExternalStructureProperty(Scope.ToSharedRef(), NAME_None, FAddPropertyParams().AllowChildren(false))->DisplayName(FText::FromName(Name));
	}

	/*uint32 SelfChildNumber = 0;
	InStructPropertyHandle->GetNumChildren(SelfChildNumber);

	if (SelfChildNumber > 0) {
		if (auto Self = InStructPropertyHandle->GetChildHandle(0)) {
			uint32 ChildNumber = 0;
			Self->GetNumChildren(ChildNumber);

			for (uint32 i = 0; i < ChildNumber; i++)
			{
				ChildBuilder.AddProperty(Self->GetChildHandle(i).ToSharedRef());
			}
		}
	}*/
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
			*it = Value == ECheckBoxState::Checked;
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
			SNew(SNumericVectorInputBox<double>).AllowSpin(true)
				.bColorAxisLabels(true)
				.X(this, &SAnyWidget<FVector>::TempOnGetValue, EAxis::X)
				.Y(this, &SAnyWidget<FVector>::TempOnGetValue, EAxis::Y)
				.Z(this, &SAnyWidget<FVector>::TempOnGetValue, EAxis::Z)
				.OnXChanged(this, &SAnyWidget<FVector>::TempOnValueChanged, EAxis::X)
				.OnYChanged(this, &SAnyWidget<FVector>::TempOnValueChanged, EAxis::Y)
				.OnZChanged(this, &SAnyWidget<FVector>::TempOnValueChanged, EAxis::Z)
				.OnXCommitted(this, &SAnyWidget<FVector>::TempOnValueCommit, EAxis::X)
				.OnYCommitted(this, &SAnyWidget<FVector>::TempOnValueCommit, EAxis::Y)
				.OnZCommitted(this, &SAnyWidget<FVector>::TempOnValueCommit, EAxis::Z)
		];
	}

private:
	void TempOnValueChanged(double AxisValue, EAxis::Type Axis)
	{
		Handle->NotifyPreChange();
		for (auto& it : Anys)
		{
			auto Vector = it->Get<FVector>();
			Vector.SetComponentForAxis(Axis, AxisValue);
			*it = Vector;
		}
		Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}

	void TempOnValueCommit(double AxisValue, ETextCommit::Type CommitType, EAxis::Type Axis)
	{
		TempOnValueChanged(AxisValue, Axis);
	}

	TOptional<double> TempOnGetValue(EAxis::Type Axis) const
	{
		TSet<double> Set;
		for (auto& it : Anys)
		{
			Set.Add(it->Get<FVector>().GetComponentForAxis(Axis));
		}

		if (Set.Num() > 1) {
			return TOptional<double>();
		}
		else {
			return *Set.begin();
		}
	}

private:
	TSharedPtr<IPropertyHandle> Handle;
	TArray<FAny*> Anys;
};



#undef LOCTEXT_NAMESPACE
