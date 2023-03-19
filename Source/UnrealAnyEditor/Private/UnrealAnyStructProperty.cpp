#include "UnrealAnyStructProperty.h"


template<>
void FAnyStructProperty<FAny::FAnyStruct>::Make(IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys)
{
	Anys = InAnys;
	TSet<UScriptStruct*> Structs;
	for (auto& Any : Anys)
	{
		auto Struct = Any->Get<FAny::FAnyStruct>(); 
		Structs.Add(Struct.Struct.Get());
		Data.Insert(Struct.Value, Data.Num());
	}

	if (Anys.Num() == 1) {
		auto Struct = *Structs.begin();

		TSharedPtr<FStructOnScope> Scope = MakeShareable(new FStructOnScope(Struct, Data.GetData()));
		Setup(InChildBuilder, Scope);
	}
}

template<>
void FAnyStructProperty<FAny::FAnyStruct>::OnValueChanged()
{
	ChildHandle->EnumerateRawData([&](void* RawData, const int32 Index, const int32 NumDatas) -> bool {
		auto Src = Anys[Index]->GetPtr<FAny::FAnyStruct>();
		Src->Struct->CopyScriptStruct(Src->Value.GetData(), RawData);

		return true;
	});
	NotifyPostChange();
}
