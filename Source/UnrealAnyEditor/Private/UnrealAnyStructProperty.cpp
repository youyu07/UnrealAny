#include "UnrealAnyStructProperty.h"


template<>
void FAnyStructProperty<FAny::FAnyStruct>::Make(IDetailChildrenBuilder& InChildBuilder, TArray<FAny*> InAnys)
{
	Anys = InAnys;
	auto Struct = Anys[0]->GetPtr<FAny::FAnyStruct>();
	TSharedPtr<FStructOnScope> Scope = MakeShareable(new FStructOnScope(Struct->Struct.Get(), Struct->Value.GetData()));
	Setup(InChildBuilder, Scope.ToSharedRef());
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
