// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "NodePC.h"
#include "Uncrushable/Widget_Manager.h"
#include "Nodes.h"

int ANodePC::id_counter = 70;

ANodePC::ANodePC() : ANodeBase()
{
}

void ANodePC::BeginPlay()
{
	ANodeBase::BeginPlay();
	AddWorkload(30);
	nodeType = NodeType::PersonalComputer;
	id = id_counter;
	id_counter++;
}

void ANodePC::Tick(float DeltaTime)
{
	//min + (rand() % (max - min + 1))
	if (UWidget_Manager::isGameStart && nodeState != NodeState::Overloaded && nodeState != NodeState::Offline && !nodeLinks.empty())
	{
		GeneratePacket(std::rand() % (101));
	}
	ANodeBase::Tick(DeltaTime);

}
void ANodePC::AcceptPacket(APacket* packet)
{
	if (have_phys_connection && nodeState == NodeState::Captured 
		&& packet->packetType == PacketType::Simple && packet->sInformation && packet->sInformation->for_spy_ref)
	{
		UWidget_Manager::self_ref->AddNodeInfo((ANodeBase*)(packet->sInformation->for_spy_ref), false);
		ANodeBase::Information* fast_ptr = ((ANodeBase*)packet->sInformation->for_spy_ref)->sInformation;
		if (fast_ptr)
		{
			for (auto elem : fast_ptr->vec_net_id) UWidget_Manager::self_ref->AddNodeInfo(elem, true);

			if (fast_ptr->key_info_count) UWidget_Manager::self_ref->AddKeyInfo(fast_ptr->key_info_count);
		}
	}
	ANodeBase::AcceptPacket(packet);
}

void ANodePC::GeneratePacket(int chance)
{
	auto generate_PC_ID = [my_id = this->id]() -> int
	{
		int id = 70 + std::rand() % (ANodePC::id_counter - 70);
		if (id == my_id)
		{
			if (id + 1 != ANodePC::id_counter && id + 1 < 90) return id + 1;
			if (id - 1 != ANodePC::id_counter && id - 1 > ANodePC::id_counter) return id - 1;
			return -2;
		}
		return id;
	};
	/* PC chance:
	* 0-65: nothing
	* 65-90: simple for PC/EO
	* 90-95: informative for PC/DS (contain roots for PC/DS/TI)
	* 95-100: packet-request for DS, that return informative packet with key info
	*/
	if (chance < 65) return;
	else if (chance >= 65 && chance < 90)
	{
		if (ANodeEO::id_counter == 0) return;
		if (ANodePC::id_counter == 70) return;
		int _id = -2;
		switch (std::rand() % (2))
		{
		case 0: _id = generate_PC_ID(); break; //PC
		case 1: _id = std::rand() % (ANodeEO::id_counter); break; //EO
		}

		std::vector<ANodeBase*>* nodes = DeterminePath(_id);
		if (nodes && !(*nodes).empty())
		{
			APacket* packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
			packet->InitPacket(PacketType::Simple, this->id, _id);
			SendPacket(packet, nodes, (*nodes).begin());
		}
	}
	else if (chance >= 90 && chance < 95)
	{
		if (ANodeDS::id_counter == 50) return;
		if (ANodePC::id_counter == 70) return;
		int _id = -1;
		switch (std::rand() % (2))
		{
		case 0: _id = generate_PC_ID(); break; //PC
		case 1: _id = 50 + std::rand() % (ANodeDS::id_counter - 50); break; //DS
		}

		std::vector<ANodeBase*>* nodes = DeterminePath(_id);
		if (nodes && !(*nodes).empty())
		{
			APacket* packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
			packet->InitPacket(PacketType::Informative, this->id, _id);
			switch (std::rand() % (3))
			{
			case 0: packet->sInformation->roots_for_id = 70 + std::rand() % (ANodePC::id_counter - 70); break; //Root PC
			case 1: packet->sInformation->roots_for_id = 50 + std::rand() % (ANodeDS::id_counter - 50); break; //Root DS
			case 2: packet->sInformation->roots_for_id = 10 + std::rand() % (ANodeTI::id_counter - 10); break; //Root TI
			}
			SendPacket(packet, nodes, (*nodes).begin());
		}
	}
	else if (chance >= 95)
	{
		if (ANodeDS::id_counter == 50) return;
		int _id = 50 + std::rand() % (ANodeDS::id_counter - 50);

		std::vector<ANodeBase*>* nodes = DeterminePath(_id);
		if (nodes && !(*nodes).empty())
		{
			APacket* packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
			packet->InitPacket(PacketType::Informative, this->id, _id);
			packet->sInformation->isDSRequest = true;
			SendPacket(packet, nodes, (*nodes).begin());
		}
	}
}
