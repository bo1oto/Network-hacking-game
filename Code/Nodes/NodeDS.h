// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NodeBase.h"
#include "NodeDS.generated.h"

/**
 * 
 */
UCLASS()
class UNCRUSHABLE_API ANodeDS : public ANodeBase
{
	GENERATED_BODY()

public:
	ANodeDS();
	void BeginPlay() final;

	void AcceptPacket(APacket* packet) final;

	static int id_counter;//DS - 50-69
private:

	//Ќе €вл€етс€ промежуточным агентом
	//—ервер не генерирует пакеты, а лишь отвечает на запросы при помощи AcceptPacket()
	//AcceptPacket тоже чисто виртуальный вроде как
};
