#include "UnrealAnyEnumProperty.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "SEnumCombo.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "UnrealAnyCustomization"


class SAnyEnum : public SComboButton
{
public:
	DECLARE_DELEGATE_TwoParams(FOnEnumSelectionChanged, int32, ESelectInfo::Type);

public:
	SLATE_BEGIN_ARGS(SAnyEnum)
		: _CurrentValue()
		, _ContentPadding(FMargin(4.0, 2.0))
		, _ButtonStyle(nullptr)
	{}

	SLATE_ATTRIBUTE(TArray<int32>, CurrentValue)
	SLATE_ATTRIBUTE(FMargin, ContentPadding)
	SLATE_ATTRIBUTE(FSlateFontInfo, Font)
	SLATE_EVENT(FOnEnumSelectionChanged, OnEnumSelectionChanged)
	SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const UEnum* InEnum)
	{
		static const FName UseEnumValuesAsMaskValuesInEditorName(TEXT("UseEnumValuesAsMaskValuesInEditor"));

		Enum = InEnum;
		CurrentValue = InArgs._CurrentValue;
		check(CurrentValue.IsBound());
		OnEnumSelectionChangedDelegate = InArgs._OnEnumSelectionChanged;
		Font = InArgs._Font;
		bUpdatingSelectionInternally = false;
		bIsBitflagsEnum = Enum->HasMetaData(TEXT("Bitflags"));

		if (bIsBitflagsEnum)
		{
			const bool bUseEnumValuesAsMaskValues = Enum->GetBoolMetaData(UseEnumValuesAsMaskValuesInEditorName);
			const int32 BitmaskBitCount = sizeof(int32) << 3;

			for (int32 i = 0; i < Enum->NumEnums() - 1; i++)
			{
				// Note: SEnumComboBox API prior to bitflags only supports 32 bit values, truncating the value here to keep the old API.
				int32 Value = Enum->GetValueByIndex(i);
				const bool bIsHidden = Enum->HasMetaData(TEXT("Hidden"), i);
				if (Value >= 0 && !bIsHidden)
				{
					if (bUseEnumValuesAsMaskValues)
					{
						if (Value >= MAX_int32 || !FMath::IsPowerOfTwo(Value))
						{
							continue;
						}
					}
					else
					{
						if (Value >= BitmaskBitCount)
						{
							continue;
						}
						Value = 1 << Value;
					}

					FText DisplayName = Enum->GetDisplayNameTextByIndex(i);
					FText TooltipText = Enum->GetToolTipTextByIndex(i);
					if (TooltipText.IsEmpty())
					{
						TooltipText = FText::Format(LOCTEXT("BitmaskDefaultFlagToolTipText", "Toggle {0} on/off"), DisplayName);
					}

					VisibleEnums.Emplace(i, Value, DisplayName, TooltipText);
				}
			}
		}
		else
		{
			for (int32 i = 0; i < Enum->NumEnums() - 1; i++)
			{
				if (Enum->HasMetaData(TEXT("Hidden"), i) == false)
				{
					VisibleEnums.Emplace(i, Enum->GetValueByIndex(i), Enum->GetDisplayNameTextByIndex(i), Enum->GetToolTipTextByIndex(i));
				}
			}
		}

		SComboButton::Construct(SComboButton::FArguments()
			.ButtonStyle(InArgs._ButtonStyle)
			.ContentPadding(InArgs._ContentPadding)
			.OnGetMenuContent(this, &SAnyEnum::OnGetMenuContent)
			.ButtonContent()
			[
				SNew(STextBlock)
				.Font(Font)
				.Text(this, &SAnyEnum::GetCurrentValueText)
			]);
	}

private:
	FText GetCurrentValueText() const
	{
		auto Array = CurrentValue.Get();
		{
			TSet<int32> Temp;
			for (int32 i = 0; i < Array.Num(); i++)
			{
				Temp.Add(Array[i]);
			}
			if (Temp.Num() > 1) {
				return LOCTEXT("MultipleValues", "Multiple Values");
			}
		}

		if (bIsBitflagsEnum)
		{
			if (CurrentValue.IsSet())
			{
				const int32 BitmaskValue = Array[0];
				if (BitmaskValue != 0)
				{
					TArray<FText> SetFlags;
					SetFlags.Reserve(VisibleEnums.Num());

					for (const FEnumInfo& FlagInfo : VisibleEnums)
					{
						if ((BitmaskValue & FlagInfo.Value) != 0)
						{
							SetFlags.Add(FlagInfo.DisplayName);
						}
					}
					if (SetFlags.Num() > 3)
					{
						SetFlags.SetNum(3);
						SetFlags.Add(FText::FromString("..."));
					}

					return FText::Join(FText::FromString(" | "), SetFlags);
				}
				return LOCTEXT("BitmaskButtonContentNoFlagsSet", "(No Flags Set)");
			}
			return FText::GetEmpty();
		}

		const int32 ValueNameIndex = Enum->GetIndexByValue(Array[0]);
		return Enum->GetDisplayNameTextByIndex(ValueNameIndex);
	}

	TSharedRef<SWidget> OnGetMenuContent()
	{
		const bool bCloseAfterSelection = !bIsBitflagsEnum;
		FMenuBuilder MenuBuilder(bCloseAfterSelection, nullptr);

		for (const FEnumInfo& FlagInfo : VisibleEnums)
		{
			MenuBuilder.AddMenuEntry(
				FlagInfo.DisplayName,
				FlagInfo.TooltipText,
				FSlateIcon(),
				FUIAction
				(
					FExecuteAction::CreateLambda([this, FlagInfo]()
					{
						if (bIsBitflagsEnum)
						{
							// Toggle value
							const int32 Value = CurrentValue.Get()[0] ^ FlagInfo.Value;
							OnEnumSelectionChangedDelegate.ExecuteIfBound(Value, ESelectInfo::Direct);
						}
						else
						{
							// Set value
							OnEnumSelectionChangedDelegate.ExecuteIfBound(FlagInfo.Value, ESelectInfo::OnMouseClick);
						}
					}),
					FCanExecuteAction(),
					FGetActionCheckState::CreateLambda([this, FlagInfo]() -> ECheckBoxState
						{
							TSet<ECheckBoxState> Values;
							auto Array = CurrentValue.Get();
							for (int32 i = 0; i < Array.Num(); i++)
							{
								Values.Add((Array[0] & FlagInfo.Value) != 0 ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
							}
							return Values.Num() > 1 ? ECheckBoxState::Undetermined : *Values.begin();
						})
					),
				NAME_None,
				bIsBitflagsEnum ? EUserInterfaceActionType::Check : EUserInterfaceActionType::None);
		}

		return MenuBuilder.MakeWidget();
	}

private:

	struct FEnumInfo
	{
		FEnumInfo() = default;
		FEnumInfo(const int32 InIndex, const int32 InValue, const FText InDisplayName, const FText InTooltipText)
			: Index(InIndex), Value(InValue), DisplayName(InDisplayName), TooltipText(InTooltipText)
		{}

		int32 Index = 0;
		int32 Value = 0;
		FText DisplayName;
		FText TooltipText;
	};

	const UEnum* Enum;
	TArray<FAny*> Anys;

	TAttribute<TArray<int32>> CurrentValue;

	TAttribute<FSlateFontInfo> Font;

	TArray<FEnumInfo> VisibleEnums;

	bool bUpdatingSelectionInternally;

	bool bIsBitflagsEnum;

	FOnEnumSelectionChanged OnEnumSelectionChangedDelegate;
};


void FAnyEnumProperty::Make(IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys)
{
	Anys = InAnys;

	TSet<UEnum*> Enums;
	for (auto& Any : Anys)
	{
		Enums.Add(Any->Get<FAny::FAnyEnum>().Enum.Get());
	}

	if (Enums.Num() == 1) {

		auto Enum = Anys[0]->GetPtr<FAny::FAnyEnum>();

		auto EnumWidget = SNew(SAnyEnum, Enum->Enum.Get())
			.IsEnabled(!IsReadOnly())
			.CurrentValue(this, &FAnyEnumProperty::OnGetEnum)
			.OnEnumSelectionChanged(this, &FAnyEnumProperty::OnEnumChanged);

		auto NameWidget = SNew(STextBlock).Text(Enum->Enum->GetDisplayNameText());

		InChildBuilder.AddCustomRow(GetSearchText())
			.NameContent()[NameWidget]
			.ValueContent()[EnumWidget];
	}
}


TArray<int32> FAnyEnumProperty::OnGetEnum() const
{
	TArray<int32> Array;
	for (auto& Any : Anys)
	{
		Array.Add(Any->GetPtr<FAny::FAnyEnum>()->Value);
	}

	return MoveTemp(Array);
}


void FAnyEnumProperty::OnEnumChanged(int32 InValue, ESelectInfo::Type InType)
{
	if (!IsReadOnly()) {
		for (auto& Any : Anys)
		{
			Any->GetPtr<FAny::FAnyEnum>()->Value = InValue;
		}

		NotifyPostChange();
	}
}


#undef LOCTEXT_NAMESPACE