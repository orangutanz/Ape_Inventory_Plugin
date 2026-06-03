#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

UENUM(BlueprintType)
enum class EInventoryAccessLevel : uint8
{
	NoAccess  = 0	UMETA(DisplayName = "No Access"),
	ViewOnly  = 1	UMETA(DisplayName = "View Only"),
	Unlimited = 2	UMETA(DisplayName = "Unlimited")
};

USTRUCT(BlueprintType)
struct FInventoryAccessResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EInventoryAccessLevel AccessLevel = EInventoryAccessLevel::NoAccess;

	UPROPERTY(BlueprintReadWrite)
	FString Reason = "";
};

class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropInventoryItem, FItemInfo, itemInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUseInventoryItem, FItemInfo, itemInfo, int, inventoryIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, FItemInfo, itemInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, FItemInfo, itemInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSellItem, bool, fromEquipped, int, posIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuyItem, FName, itemID, int, Quantity);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInvalidClientAccess, UInventoryComponent*, targetComponent, FInventoryAccessResult, Reason);

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom))
class APE_INVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
public:

	//  ================ Server Only ================ //

	/* Server creates Inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	void InitializeInventory();

	/* Server calls OnDropInventoryItem if srinking and not big enough */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	void ResizeInventory();

	/* Server trys unequip slots or call OnDropInventoryItem when inventory's full */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	void RedefineEquipments();
	
	/* Server clears all info, no dropping */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	void DeinitializeInventory();

	/* Server returns true when fully added, else ItemSlot has remaining info */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	bool AddItem(UItemSlot* item);

	/* Server removes item completely */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	bool RemoveItemByName(FName ItemID, int32 Amount = 1);

	/* Server removes item completely */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	bool RemoveItemByIndex(int32 index, int32 Amount = 1);

	/* Server removes equipment completely, use UnequipEquip to keep item instead */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	bool RemoveEquipment(int32 index = -1, FName definition = "");

	/* Server clears inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	void ClearInventory();

	/* Server send item info to client about certain inventory */
	UFUNCTION()
	void SendInventoryInfo(UInventoryComponent* aboutInventory, const TArray<FItemInfo>& info);
	UFUNCTION(Client, Reliable)
	void CLIENT_RecieveInventoryInfo(UInventoryComponent* aboutInventory, const TArray<FItemInfo>& info);

	// ================ For Client ================ //

	/* Client requests view on an inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void GetViewInventory(UInventoryComponent* viewingInventory, bool toggle);
	UFUNCTION(Server, Reliable)
	void SERVER_ViewInventoryToggle(UInventoryComponent* viewingInventory, bool toggle);

	/* Client takes from an inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void TakeFromInventory(UInventoryComponent* takeFromInventory, const int32 itemIndex);

	/* Client takes ALL from an inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void TakeAllFrom(UInventoryComponent* takeFromInventory);

	/* Client puts into an inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void PutItemToInventory(UInventoryComponent* toInventory, const int32 itemIndex);

	/* Client puts ALL into an inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void TransferAllTo(UInventoryComponent* transferToInventory);

	UFUNCTION(Server, Reliable)
	void SERVER_MoveItemBetweenInventory(UInventoryComponent* targetInventory, int32 itemIndex, bool isTaking);
	UFUNCTION(Server, Reliable)
	void SERVER_TransferItems(UInventoryComponent* targetInventory, bool isTaking);

	/* Client swaps within inventory or with other inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void SwapItemByIndex(UInventoryComponent* fromInventory, UInventoryComponent* toInventory, const int32 fromA, const int32 toB);
	UFUNCTION(Server, Reliable)
	void SERVER_SwapItemByIndex(UInventoryComponent* fromInventory, UInventoryComponent* toInventory, const int32 fromA, const int32 toB);

	/* Client sorts items by name. Change sort logc in C++ */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void SortItems();
	UFUNCTION(Server, Reliable)
	void SERVER_SortItems();

	/* Client drops item */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void DropItemAtIndex(const int32 index, bool fromEquipment = false);
	UFUNCTION(Server, Reliable)
	void SERVER_DropItemAtIndex(const int32 index, bool fromEquipment);

	/* Client drops ALL items */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void DropAllItems();
	UFUNCTION(Server, Reliable)
	void SERVER_DropAllItems();

	/* Set toInventory to null to drop after split */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void SplitItem(const int32 fromIndex, const int32 toIndex, const int32 splitAmount, UInventoryComponent* fromInventroy, UInventoryComponent* toInventory);
	UFUNCTION(Server, Reliable)
	void SERVER_SplitItem(const int32 fromIndex, const int32 toIndex, const int32 splitAmount, UInventoryComponent* fromInventroy, UInventoryComponent* toInventory);

	// Use item by index or name
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void UseItem(UInventoryComponent* fromInventory, const int inventoryIndex = -1, FName itemName = "");
	UFUNCTION(Server, Reliable)
	void SERVER_UseInventoryItem(UInventoryComponent* fromInventory, const int32 inventoryIndex);

	// ================ Equipment ================ //

	/* Client equips item from self or other inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void EquipItem(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_EquipItem(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex);

	/* Client self unequips*/
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void UnequipItem(const int32 equipmentIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_UnequipItem(const int32 equipmentIndex);

	/* Client equip/unequip to target inventory */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void SwapEquipmentWithInventory(UInventoryComponent* targetInventory, const int32 inventoryIndex, const int32 equipmentIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_SwapEquipmentWithInventory(UInventoryComponent* targetInventory, const int32 inventoryIndex, const int32 equipmentIndex);

	/* Client swap equipment index */
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void SwapEquipmentPosition(const int32 fromIndex, const int32 toIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_SwapEquipmentPosition(const int32 fromIndex, const int32 toIndex);


	// ================ Local Functions ================ //
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	int FindItemQuantity(FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void CallEquipmentUpdate();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void CallInventoryUpdate();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	bool IsInventoryEmpty();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	UItemSlot* GetUtilitySlot();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	int32 GetInitialInventorySize() { return InitialInventorySize; }

	// ================ Optional Implementation ================ //

	/* Validate clients access level? Like looting/viewing chest they don't own */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ape_Inventory|Validation")
	FInventoryAccessResult  GetAccessLevel(UInventoryComponent* TargetInventory);

	UFUNCTION()
	bool ValidateAccess(UInventoryComponent* TargetInventory, EInventoryAccessLevel RequiredLevel);

private:
	// Internal functions
	void UpdateAllInfos();

	void UpdateInventoryInfos();

	void UpdateEquipmentInfos();

	// Notify
	UFUNCTION()
	void OnRep_InventoryUpdate();

	UFUNCTION()
	void OnRep_EquipmentUpdate();

	UFUNCTION(Client, Reliable)
	void CLIENT_NotifyItemAdded(FItemInfo itemInfo);

	UFUNCTION(Client, Reliable)
	void CLIENT_NotifyItemRemoved(FItemInfo itemInfo);

	UFUNCTION()
	void NotifyItemAdded(FItemInfo itemInfo) { OnItemAdded.Broadcast(itemInfo); } // Server Delegate

	UFUNCTION()
	void NotifyItemRemoved(FItemInfo itemInfo) { OnItemRemoved.Broadcast(itemInfo); } // Server Delegate

public:
	//  ================ Definition ================ //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Inventory|Server")
	int32 InventorySize = 12;

	int32 InitialInventorySize = InventorySize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Inventory", Replicated)
	TArray<FName> EquipmentDefinitions;

	//  ================ Inventory ================ //

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ape_Inventory", ReplicatedUsing = OnRep_InventoryUpdate)
	TArray<FItemInfo> InventoryInfos; //Infos for clients

	UPROPERTY() //Items on server
	TArray<UItemSlot*> Inventory;

	//  ================ Equipment ================ //

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ape_Inventory", ReplicatedUsing = OnRep_EquipmentUpdate)
	TArray<FItemInfo> EquipmentInfos; //Infos for clients

	UPROPERTY() //Items on server
	TArray<UItemSlot*> Equipments;

	//  ================ References ================ //

	UPROPERTY()
	TArray<TWeakObjectPtr<UInventoryComponent>> ViewingComponents;

	UPROPERTY()
	UItemSlot* UtilitySlot;

	//  ================ Delegates ================ //

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory")
	FOnEquipmentUpdated OnEquipmentUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory|Client")
	FOnItemAdded OnItemAdded; // Call on owning client

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory|Client")
	FOnItemRemoved OnItemRemoved; // Call on owning client

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory|Server")
	FOnDropInventoryItem OnDropInventoryItem; // Call on server

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory|Server")
	FOnUseInventoryItem OnUseInventoryItem; // Call on server

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory|Server")
	FOnInvalidClientAccess OnInvalidClientAccess;

private:
	bool bInistialized = false;
};
