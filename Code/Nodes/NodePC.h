// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NodeBase.h"
#include "NodePC.generated.h"

/**
 * 
 */
UCLASS()
class UNCRUSHABLE_API ANodePC : public ANodeBase
{
	GENERATED_BODY()
	
public:
	ANodePC();
	void BeginPlay() final;
	void Tick(float DeltaTime) final;
	UPROPERTY(BlueprintReadWrite, Category = "GameRules")
	bool have_phys_connection = false;

	static int id_counter;//PC - 70-89
private:

	void AcceptPacket(APacket* packet) final;
	void GeneratePacket(int chance) final;
};
