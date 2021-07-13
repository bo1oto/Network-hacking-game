
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Nodes/Nodes.h"
#include "Nodes/Employee.h"
#include "Widget_Manager.generated.h"

USTRUCT(BlueprintType)
struct FNodeInfo
{
	GENERATED_BODY()

public:
	FNodeInfo();
	FNodeInfo(int _node_id);
	FNodeInfo(ANodeBase* _node_ptr);

	UPROPERTY(BlueprintReadOnly)
	int node_id;
	UPROPERTY(BlueprintReadOnly)
	ANodeBase* node_ptr;
	UPROPERTY(BlueprintReadOnly)
	FString characteristic;
	bool isFullInfo;
};

UCLASS()
class UNCRUSHABLE_API UWidget_Manager : public UUserWidget
{
	GENERATED_BODY()
	
private:
	/* 10 - victory
	* Intercepted packet from DS to PC - 1
	* Information downloaded from DS (depending on the number of DS, but this is usually 10)
	* Information contained on PC/SC - 2
	*/
	short key_info_counter = 0;

public:
////////////////////////Hacker/////////////////////////////////
	UFUNCTION(BlueprintCallable)
	static void SetSelfRef(UWidget_Manager* _self_ref);
	static UWidget_Manager* self_ref;
	UFUNCTION(BlueprintCallable)
	static void StartGame();
	static bool isGameStart;

	static FString FillNodeCharacteristic(ANodeBase* node_ptr);
	UFUNCTION(BlueprintCallable)
	void AddNodeInfo(ANodeBase* node_ptr, bool asID);
	
	void AddKeyInfo(short quantity);

	UPROPERTY(BlueprintReadWrite)
	TArray<	FNodeInfo> known_nodes = {};


	UFUNCTION(BlueprintCallable)
	void InitSpamAttack(int target_node_id, ANodeBase* source_node);
	UFUNCTION(BlueprintCallable)
	void InitAttack(int target_node_id, ANodeBase* source_node, bool upThreat, int spoof_id, int attack_type);
	UFUNCTION(BlueprintCallable)
	void InitInformative(int target_node_id, ANodeBase* source_node, int spoof_id);
};
