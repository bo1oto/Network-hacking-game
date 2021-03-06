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

void ANodePC::AcceptPacket(APacket* packet)
{
	if (have_phys_connection && nodeState == NodeState::Captured && packet->packetType == PacketType::Simple 
		&& packet->sInformation && packet->sInformation->for_spy_ref)
	{
		UWidget_Manager::self_ref->AddNodeInfo((ANodeBase*)(packet->sInformation->for_spy_ref), false);
		
		if (packet->sInformation->key_info_count) UWidget_Manager::self_ref->AddKeyInfo(packet->sInformation->key_info_count);
		if (!packet->sInformation->roots_for_id.empty())
		{
			for (auto root : packet->sInformation->roots_for_id)
			{
				if (!UWidget_Manager::self_ref->known_nodes.ContainsByPredicate([root](FNodeInfo nodeInfo)
				{
					return nodeInfo.node_id == root;
				}))
					UWidget_Manager::self_ref->roots.Add(root);
			}
		}
		ANodeBase::Information* fast_ptr = ((ANodeBase*)packet->sInformation->for_spy_ref)->sInformation;
		if (fast_ptr)
		{
			for (auto elem : fast_ptr->vec_net_id) UWidget_Manager::self_ref->AddNodeInfo(elem, true);
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
	* 0-60: nothing
	* 60-85: simple for PC/EO
	* 85-92.5: informative for PC/DS (contain roots for PC/DS/TI)
	* 92.5-100: packet-request for DS, that return informative packet with key info
	*/
	if (chance < 60) return;
	else if (chance >= 60 && chance < 85)
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
	else if (chance >= 85 && chance < 92.5)
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
			case 0: packet->sInformation->roots_for_id.push_back(70 + std::rand() % (ANodePC::id_counter - 70)); break; //Root PC
			case 1: packet->sInformation->roots_for_id.push_back(50 + std::rand() % (ANodeDS::id_counter - 50)); break; //Root DS
			case 2: packet->sInformation->roots_for_id.push_back(10 + std::rand() % (ANodeTI::id_counter - 10)); break; //Root TI
			}
			SendPacket(packet, nodes, (*nodes).begin());
		}
	}
	else if (chance >= 92.5)
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
