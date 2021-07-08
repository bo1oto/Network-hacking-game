// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NodeBase.h"
#include "NodeEO.generated.h"

/**
 * 
 */
UCLASS()
class UNCRUSHABLE_API ANodeEO : public ANodeBase
{
	GENERATED_BODY()

public:
	ANodeEO();
	void BeginPlay() final;
	//void Tick(float DeltaTime) override final;

	static int id_counter;//EO - 0-5
private:
	/*void CheckPacket(APacket* packet, std::vector<ANodeBase*>* vec, std::vector<ANodeBase*>::iterator it, float time = 0.0f) override final;
	void GeneratePacket(int chance) override final;*/
	void AcceptPacket(APacket* packet) final;
	
};
