#include "UnrealAnyCustomization.h"
#include "IPropertyUtilities.h"
#include "SPinTypeSelector.h"
#include "EdGraphSchema_K2.h"


#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"



#define DEFINE_PINTYPE(Category, SubCategory, SubObject) FEdGraphPinType(Category, SubCategory, SubObject, EPinContainerType::None, false, FEdGraphTerminalType())

static FEdGraphPinType OnGetPinInfo(TArray<FAny*> Anys, bool bMultiType)
{
	if (!bMultiType) {
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
		case NAME_Class: {
			auto Class = Anys[0]->Get<FAny::FAnyClass>().BaseClass.Get();
			return DEFINE_PINTYPE(UEdGraphSchema_K2::PC_Class, NAME_None, Class);
		}
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

static void InitAnyClass(const FEdGraphPinType& PinType, const TArray<FAny*>& Anys)
{
	for (auto& Any : Anys)
	{
		*Any = Cast<UClass>(PinType.PinSubCategoryObject.Get());
	}
}

static void PinInfoChanged(const FEdGraphPinType& PinType, TArray<FAny*> Anys, TSharedRef<IPropertyHandle> Handle, TSharedRef<IPropertyUtilities> Utilities)
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
		InitMap.Add(UEdGraphSchema_K2::PC_Class, InitAnyClass);
	}

	auto Category = PinType.PinCategory;
	if (Category == UEdGraphSchema_K2::PC_Byte && PinType.PinSubCategoryObject.IsValid()) {
		Category = UEdGraphSchema_K2::PC_Enum;
	}

	if (auto it = InitMap.Find(Category)) {
		(*it)(PinType, Anys);
	}

	Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
	Utilities->ForceRefresh();
}


TSharedRef<SPinTypeSelector> FUnrealAnyCustomization::CreatePinSelector(TArray<FAny*> Anys, TSharedRef<IPropertyHandle> InHandle, TSharedRef<IPropertyUtilities> Utilities, bool bMultiType)
{
	auto Schema = GetDefault<UEdGraphSchema_K2>();
	return SNew(SPinTypeSelector, FGetPinTypeTree::CreateUObject(Schema, &UEdGraphSchema_K2::GetVariableTypeTree))
		.TargetPinType_Static(OnGetPinInfo, Anys, bMultiType)
		.OnPinTypeChanged_Static(PinInfoChanged, Anys, InHandle, Utilities)
		.SelectorType(SPinTypeSelector::ESelectorType::Partial)
		.Schema(Schema);
}



#undef LOCTEXT_NAMESPACE
