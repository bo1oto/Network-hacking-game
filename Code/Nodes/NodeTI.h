// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NodeBase.h"
#include "NodeTI.generated.h"

/**
 * 
 */
UCLASS()
class UNCRUSHABLE_API ANodeTI : public ANodeBase
{
	GENERATED_BODY()

public:
	ANodeTI();
	void BeginPlay() final;

	bool CheckIDInTable(int _id);

	static int id_counter;//TI - 10-29
private:
	static int vlan_counter;

	void AcceptPacket(APacket* packet) final;


	//Бред
	//Маршрутизатор также как и другие ноды знает своих соседей
	//Но его фишка в том, что он знает в каком vlan находятся другие ноды (ну или какие ноды у vlan соседнего маршрутизатора)
	//Соответственно мне нужно хранить записи в виде: vlan, id[]
	//Тогда vlan выдаётся обычным узлам при подключении к TI, а если подключение идёт из обычного к обычному, то тому присваивается vlan другого
	//Также при подключении к TI обычного узла, его соединения также обновляют vlan
	struct Routing final
	{
		Routing(int vlan_num, std::vector<int> vec = {});
	public:
		int vlan;
		std::vector<int> id_numbers;
	};
	//Нулевой - это собственный vlan
	std::vector<Routing*> routingTable;
	
	//Нужен метод, который переберёт все узлы в границах этого маршрутизатора
	//Но мне это пока не нужно, я могу по сути просто рукой назначать vlan, буду рукой вводить
	
	//я в блупринтах нахожу все ноды этого типа, и потом для каждого вызываю метод с каждым
	UFUNCTION(BlueprintCallable)
	void CreateVLAN(ANodeTI* node);
	void FillVLAN(std::vector<int>* id_vec, int vlan_num);
};
