// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

void UItemSlot::SetItemInfo(FItemInfo info)
{
	mItemInfo = info;
}

bool UItemSlot::SetQuantity(int32 num)
{
	//if (num == 0 || (num > mItemInfo.MaxStack) && (mItemInfo.ItemType != EItemType::Equipment))
	//	return false;

	mItemInfo.Quantity = num;
	if (num == 0)
	{
		FItemInfo defaultInfo;
		mItemInfo = defaultInfo;
	}
	return true;
}



UItemSlot* UItemSlot::SplitItem(int32 num)
{
	if (mItemInfo.MaxStack == 1 || num == 0)
	{
		return nullptr;
	}

	if (mItemInfo.Quantity > num)
	{
		FItemInfo newInfo = mItemInfo;
		newInfo.Quantity = num;
		mItemInfo.Quantity -= num;
		auto newItem = NewObject<UItemSlot>();
		newItem->SetItemInfo(newInfo);

		return newItem;
	}
	else if (mItemInfo.Quantity == num)
	{
		return this;
	}


	return nullptr;
}

bool UItemSlot::CanSplit(int32 amount)
{
	if (mItemInfo.Quantity < amount || mItemInfo.MaxStack == 1 || amount == 0 || IsEmpty())
	{
		return false;
	}
	return true;
}

FItemInfo UItemSlot::SplitQuantity(int32 amount)
{
	if (amount > mItemInfo.Quantity)
		return FItemInfo();
	FItemInfo outValue = mItemInfo;
	if (amount == mItemInfo.Quantity)
	{
		ClearItemInfo();
	}
	else
	{
		mItemInfo.Quantity -= amount;
		outValue.Quantity = amount;
	}
	return outValue;
}

bool UItemSlot::MergeItem(UItemSlot* other)
{
	if (!other || other == this || other->IsEmpty())
	{
		return false;
	}

	// Empty slot
	if (IsEmpty())
	{
		FItemInfo NewInfo = other->GetItemInfo();
		int32 AmountToTake = FMath::Min(NewInfo.Quantity, NewInfo.MaxStack);
		NewInfo.Quantity = AmountToTake;
		SetItemInfo(NewInfo);
		other->SetQuantity(other->GetQuantity() - AmountToTake);
		return other->IsEmpty();
	}

	// Different item or already full
	if (mItemInfo.Quantity >= mItemInfo.MaxStack || other->GetItemID() != GetItemID())
	{
		return false;
	}

	int32 SpaceLeft = mItemInfo.MaxStack - mItemInfo.Quantity;
	int32 AmountToTake = FMath::Min(SpaceLeft, other->GetQuantity());
	mItemInfo.Quantity += AmountToTake;
	other->SetQuantity(other->GetQuantity() - AmountToTake);
	return other->IsEmpty(); 
}

void UItemSlot::SwapItemInfo(UItemSlot* other)
{
	FItemInfo tempInfo = mItemInfo;
	mItemInfo = other->GetItemInfo();
	other->SetItemInfo(tempInfo);
}
