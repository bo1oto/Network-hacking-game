
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Nodes/Nodes.h"

#include "Widget_Manager.generated.h"


USTRUCT(BlueprintType)
struct FNodeInfo
{
	GENERATED_BODY()

	// for in-engine creations
	FNodeInfo();
public:
	FNodeInfo(int _node_id);
	FNodeInfo(ANodeBase* _node_ptr);

	UPROPERTY(BlueprintReadOnly)
	int ID;
	UPROPERTY(BlueprintReadOnly)
	ANodeBase* NodePtr;
	UPROPERTY(BlueprintReadOnly)
	FString Characteristic;
	bool bIsFullInfo;
};

UCLASS()
class UNCRUSHABLE_API UWidget_Manager : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static void SetSelfRef(UWidget_Manager* _self_ref) noexcept;
	UFUNCTION(BlueprintCallable)
	static void AddToAllNodes(int id, ANodeBase* NodePtr) noexcept;

	static FString FillNodeCharacteristic(const ANodeBase& NodePtr);

	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION(BlueprintCallable)
	void AddNodeInfo(int node_id, bool bAsID);

	void AddKeyInfo(int quantity);
	UFUNCTION(BlueprintCallable)
	void InitSpamAttack(int target_id, ANodeBase* source_node, int spoof_id);

	UFUNCTION(BlueprintCallable)
	void InitAttack(int target_id, ANodeBase* source_node, bool upThreat, int spoof_id, int attack_type);

	UFUNCTION(BlueprintCallable)
	void InitInformative(int target_id, ANodeBase* source_node, int spoof_id);

public:
	static UWidget_Manager* self_ref;
	static bool bIsGameStart;
	static TMap<int, ANodeBase*> all_nodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float NetworkActivityInterval = 2.0f;
	/* 20 - victory
	* Intercepted packet from DS to PC - 1
	* Information downloaded from DS (depending on the number of DS, but this is usually 10)
	* Information contained on PC/SC - 2
	*/
	UPROPERTY(BlueprintReadOnly, meta = (BindingWidget))
	int nKeyInfo = 0;
	//Roots are deleted as soon as they have been used + Roots are not important for the captured node
	UPROPERTY(BlueprintReadWrite, meta = (BindingWidget))
	TSet<int> NodeRoots;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FNodeInfo> known_nodes = {};

private:
	std::vector<FTimerHandle>* generation_timers;
};
