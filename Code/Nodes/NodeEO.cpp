// Fill out your copyright notice in the Description page of Project Settings.

#include "Uncrushable/Widget_Manager.h"
#include "NodeEO.h"

int ANodeEO::id_counter = 0;

ANodeEO::ANodeEO() : ANodeBase()
{
}

void ANodeEO::BeginPlay()
{
	ANodeBase::BeginPlay();
	nodeType = NodeType::ExternalOutput;
	nodeState = NodeState::Captured;
	id = id_counter;
	id_counter++;
}

void ANodeEO::AcceptPacket(APacket* packet)
{
	if (nodeState == NodeState::Captured && packet->packetType == PacketType::Informative && packet->sInformation->for_spy_ref)
	{
		UWidget_Manager::self_ref->AddNodeInfo((ANodeBase*)(packet->sInformation->for_spy_ref), false);
		ANodeBase::Information* fast_ptr = ((ANodeBase*)packet->sInformation->for_spy_ref)->sInformation;
		if (fast_ptr)
		{
			for (auto elem : fast_ptr->vec_net_id)
			{
				UWidget_Manager::self_ref->AddNodeInfo(elem, true);
			}

			if (fast_ptr->key_info_count) UWidget_Manager::self_ref->AddKeyInfo(fast_ptr->key_info_count);
		}
	}
	ANodeBase::AcceptPacket(packet);
}

/*void ANodeEO::Tick(float DeltaTime)
{
	ANodeBase::Tick(DeltaTime);
	//min + (rand() % (max - min + 1))
	if (nodeState != NodeState::Overloaded && nodeState != NodeState::Offline && !nodeLinks.empty())
	{
		GeneratePacket(std::rand() % (101));
		int node_id = 1 + (std::rand() % (id_counter - 1));
		if (this->id != node_id)
			InitSpamAttack(simpleTemp, node_id);
	}

}

//Тут можно генерировать обычные и спам пакеты, но пока я не готов к этому :)
void ANodeEO::GeneratePacket(int chance)
{
	/* PC генерирует:
	* 0-65: ничего
	* 65-90: обычный
	* 90-95: иформативный
	* 95-100: вредоносный
	* Тут надо шансы подвергать поправке на персонал узла и прочее (если будет)
	*/