
#pragma once

#include "NodeTI.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include <functional>
#include "NodeSC.h"

int ANodeTI::id_counter = 10;
int ANodeTI::vlan_counter = 1;

ANodeTI::ANodeTI() : ANodeBase()
{
}
void ANodeTI::BeginPlay()
{
	ANodeBase::BeginPlay();
	AddWorkload(20);
	nodeType = NodeType::TechnicalInfrastructure;
	routingTable = {};
	id = id_counter;
	id_counter++;
	vlan = vlan_counter;
	vlan_counter++;
	sInformation = new Information;
}
		
void ANodeTI::AcceptPacket(APacket* packet)
{
	if (packet->target_id != id)
	{
		if (packet->packetType == EPacketType::Helpful && packet->sHelper && packet->sHelper->isAlarm && nodeState != NodeState::Captured)
		{
			// Then look for the closest known security node
			std::vector<ANodeBase*> main{}, bolv{};
			for (int i = ANodeSC::id_counter - 1; i >= 30; i--)
			{
				if (CheckIDInTable(i))
				{
					ComputeNodePath(this, i, main);
					if (bolv.empty() || main.size() < bolv.size())
					{
						bolv = main;
					}
				}
			}
			if (!bolv.empty())
			{
				std::vector<ANodeBase*> nodes{};
				for (auto it = bolv.rbegin(); it != bolv.rend(); it++)
				{
					nodes.push_back(*it);
				}
				packet->target_id = (*bolv.begin())->id;
				packet->path = std::vector<AActor*>(nodes.begin(), nodes.end());
				SendPacket(packet, packet->path.begin());
			}
			goto add_work;
			
		}
		if (CheckIDInTable(packet->target_id))
		{
			std::vector<ANodeBase*> nodes{}, bolv{};
			ComputeNodePath(this, packet->target_id, bolv);
			for (auto it = bolv.rbegin(); it != bolv.rend(); it++)
			{
				nodes.push_back(*it);
			}
			if (!nodes.empty())
			{
				packet->path = std::vector<AActor*>(nodes.begin(), nodes.end());
				SendPacket(packet, packet->path.begin());
				goto add_work;
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Not find in Table");
			packet->Destroy();
		}
	add_work:
		AddWorkloadWithDelay(3, 0.2f);
	}
	else
	{
		ANodeBase::AcceptPacket(packet);
	}
	
}
bool ANodeTI::CheckIDInTable(int _id) const
{
	auto contains = [](const std::vector<int>& id_vec, int node_id) -> bool
	{
		for (const auto& elem : id_vec)
		{
			if (elem == node_id) 
				return true;
		}
		return false;
	};
	for (const auto& routes : routingTable)
	{
		if (contains(routes->id_numbers, _id)) 
			return true;
	}
	return false;
}

ANodeTI::Routing::Routing(int vlan_num)
{
	vlan = vlan_num;
	id_numbers = {};
}

void ANodeTI::CreateVLAN(ANodeTI* node)
{
	Routing* route = new Routing(node->vlan);
	routingTable.push_back(route);
	FillVLAN(route->id_numbers, route->vlan);
}
void ANodeTI::FillVLAN(std::vector<int>& id_vec, int vlan_num)
{
	auto contain = [](const std::vector<int>& id_vec, int node_id) -> bool
	{
		for (auto elem : id_vec)
		{
			if (elem == node_id) return true;
		}
		return false;
	};

	std::function<void(std::vector<int>& id_vec, ANodeBase* target)> f = 
		[&f, vlan_num, &contain](std::vector<int>& id_vec, ANodeBase* target) -> void
	{
		for (auto nodeLink : target->nodeLinks)
		{
			if (nodeLink->targetNode->vlan == vlan_num && !contain(id_vec, nodeLink->targetNode->id))
			{
				id_vec.push_back(nodeLink->targetNode->id);
				f(id_vec, nodeLink->targetNode);
			}
		}
	};

	f(id_vec, this);
	if (vlan == vlan_num && !contain(id_vec, id)) id_vec.push_back(id);
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, (TEXT("VLAN ") + FString::FromInt(vlan) + TEXT(" members: ") + FString::FromInt(id_vec.size())));

	TArray<AActor*> nodes_arr = TArray<AActor*>();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANodeBase::StaticClass(), nodes_arr);
	nodes_arr = nodes_arr.FilterByPredicate( [id_vec, contain](AActor* const &node_ptr)
	{
		return contain(id_vec, ((ANodeBase*)node_ptr)->id);
	});
	for (auto elem : nodes_arr)
	{
		AddInformation((ANodeBase*)elem);
	}
}
