#include "UnrealAny.h"

#if WITH_EDITOR
#include "EdGraphSchema_K2.h"
#endif

#define IMPL_ANYHOLDER_TYPE(TypeCpp, Name) \
template class UNREALANY_API FAny::Holder<TypeCpp>; \
template<> const EName FAny::Holder<TypeCpp>::Type() { return Name; }

#define IMPL_ANYHOLDER_PINTYPE(TypeCpp, Category, SubCategory, SubCategoryObject) \
template<> const FEdGraphPinType FAny::Holder<TypeCpp>::PinType() const { \
	return FEdGraphPinType(Category, SubCategory, SubCategoryObject, EPinContainerType::None, false, FEdGraphTerminalType()); \
}


//IMPL_ANYHOLDER_TYPE(bool, NAME_BoolProperty)
//IMPL_ANYHOLDER_TYPE(int32, NAME_Int32Property)
//IMPL_ANYHOLDER_TYPE(int64, NAME_Int64Property)
//IMPL_ANYHOLDER_TYPE(float, NAME_FloatProperty)
//IMPL_ANYHOLDER_TYPE(double, NAME_DoubleProperty)
//IMPL_ANYHOLDER_TYPE(FString, NAME_StrProperty)
//IMPL_ANYHOLDER_TYPE(FName, NAME_NameProperty)
//IMPL_ANYHOLDER_TYPE(FText, NAME_TextProperty)
//
//IMPL_ANYHOLDER_TYPE(FVector, NAME_Vector)
//IMPL_ANYHOLDER_TYPE(FRotator, NAME_Rotator)
//IMPL_ANYHOLDER_TYPE(FTransform, NAME_Transform)
//
//IMPL_ANYHOLDER_TYPE(BYTE, NAME_ByteProperty)
//IMPL_ANYHOLDER_TYPE(UClass*, NAME_Class)
//
//
//IMPL_ANYHOLDER_TYPE(FIntPoint, NAME_IntPoint)


#if WITH_EDITOR

//IMPL_ANYHOLDER_PINTYPE(bool, UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr)
//IMPL_ANYHOLDER_PINTYPE(int32, UEdGraphSchema_K2::PC_Int, NAME_None, nullptr)
//IMPL_ANYHOLDER_PINTYPE(int64, UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr)
//IMPL_ANYHOLDER_PINTYPE(float, UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, nullptr)
//IMPL_ANYHOLDER_PINTYPE(double, UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double, nullptr)
//IMPL_ANYHOLDER_PINTYPE(FString, UEdGraphSchema_K2::PC_String, NAME_None, nullptr)
//IMPL_ANYHOLDER_PINTYPE(FName, UEdGraphSchema_K2::PC_Name, NAME_None, nullptr)
//IMPL_ANYHOLDER_PINTYPE(FText, UEdGraphSchema_K2::PC_Text, NAME_None, nullptr)
//
//IMPL_ANYHOLDER_PINTYPE(FVector, UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FVector>().Get())
//IMPL_ANYHOLDER_PINTYPE(FRotator, UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FRotator>().Get())
//IMPL_ANYHOLDER_PINTYPE(FTransform, UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FTransform>().Get())
//
//IMPL_ANYHOLDER_PINTYPE(BYTE, UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr)
//
//
//
//IMPL_ANYHOLDER_PINTYPE(FIntPoint, UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FIntPoint>().Get())
//
//
//
//template<> const FEdGraphPinType FAny::Holder<UClass*>::PinType() const {
//	return FEdGraphPinType(UEdGraphSchema_K2::PC_Class, NAME_None, Held, EPinContainerType::None, false, FEdGraphTerminalType());
//}


#endif




//#undef IMPL_ANYHOLDER_TYPE
//#undef IMPL_ANYHOLDER_PINTYPE