
#pragma once

#include "NodeBase.h"
#include "NodeTI.h"
#include "Uncrushable/Widget_Manager.h"
#include "NodeSC.h"


bool ANodeBase::IsAlarm = false;
Politic ANodeBase::politic = Politic::NotForbidden;
int ANodeBase::sameSignChance = 85;
int ANodeBase::upSignChance = 10;
int ANodeBase::behaviorChance = 15;
int ANodeBase::healChance = 60;
int ANodeBase::killChance = 80;
TSubclassOf<APacket> ANodeBase::packetTemp = nullptr;

ANodeBase::ANodeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	RootComponent = Mesh;
}

void ANodeBase::BeginPlay()
{
	Super::BeginPlay();
	nodeState = NodeState::Working;
	sProtection = new Protection();
	sProtection->threatSigns = { Signature::Crash_1, Signature::RootKit_1, Signature::Spy_1 };
	ANodeBase::politic = Politic::NotForbidden;
}
void ANodeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANodeBase::AddLink(ALink* _link, ANodeBase* sourceNode, ANodeBase* targetNode)
{
	switch (sourceNode->nodeType)
	{
	case NodeType::TechnicalInfrastructure:
		switch (targetNode->nodeType)
		{
		case NodeType::TechnicalInfrastructure: break;
		default: targetNode->vlan = sourceNode->vlan; break;
		}
		break;
	default:
		switch (targetNode->nodeType)
		{
		case NodeType::TechnicalInfrastructure:
			sourceNode->vlan = targetNode->vlan; break;
		default:
			if (sourceNode->vlan == 0)
			{
				sourceNode->vlan = targetNode->vlan;
			}
			else
			{
				targetNode->vlan = sourceNode->vlan;
			}
			break;
		}
		break;

	}
	sourceNode->nodeLinks.push_back(new NodeLink{ _link, targetNode });
	targetNode->nodeLinks.push_back(new NodeLink{ _link, sourceNode });
}

TArray<FText> ANodeBase::GetKeyParameters() const
{
	// node type, id + vlan, workload, state
	TArray<FText> arr;
	arr.Add(FText::FromString(TEXT("Node") + GetTypeInfo().ToString()));
	arr.Add(FText::FromString(TEXT("ID: ") + FString::FromInt(id) + TEXT(" (") + FString::FromInt(vlan) + TEXT(")")));
	arr.Add(FText::FromString(TEXT("Workload: ") + FString::FromInt(workload)));
	arr.Add(FText::FromString(GetStateInfo()));
	return arr;
}
FString ANodeBase::GetInfo() const
{
	FString str = TEXT("id: ") + FString::FromInt(id) + "\n" + TEXT("vlan: ") + FString::FromInt(vlan) + "\n" + TEXT("workload: ") + FString::FromInt(workload);
	return str;
}
FText ANodeBase::GetTypeInfo() const
{
	switch (nodeType)
	{
	case NodeType::ExternalOutput: return FText::FromString(TEXT("EO"));
	case NodeType::TechnicalInfrastructure: return FText::FromString(TEXT("TI"));
	case NodeType::Security: return FText::FromString(TEXT("SC"));
	case NodeType::PersonalComputer: return FText::FromString(TEXT("PC"));
	case NodeType::DataStorage: return FText::FromString(TEXT("DS"));
	default: return FText::FromString(TEXT("WUT!?"));
	}
}
FString ANodeBase::GetStateInfo() const
{
	switch (nodeState)
	{
	case NodeState::Working: return TEXT("Working");
	case NodeState::Captured: return TEXT("Captured");
	case NodeState::Overloaded: return TEXT("Overloaded");
	case NodeState::Offline: return TEXT("Offline");
	default: return TEXT("WUT!?");
	}
}

inline void ANodeBase::AddWorkload(short quantity)
{
	workload += quantity;
	if (workload > 100)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "I'm (" + FString::FromInt(this->id) + ") dead");
		nodeState = NodeState::Overloaded;
		FTimerHandle full_unload_timer = FTimerHandle();
		GetWorldTimerManager().SetTimer(full_unload_timer, [this]
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::FromInt(id) + " unloaded");
			nodeState = NodeState::Working;
		}, 1.0f, false, 20.0f);
	}
}
void ANodeBase::AddWorkloadWithDelay(short _add_work = 0, float delay_time = 0.0f)
{
	AddWorkload(_add_work);
	FTimerHandle unloading_timer = FTimerHandle();
	GetWorldTimerManager().SetTimer(unloading_timer, [_add_work, this]
	{
		AddWorkload(-_add_work);
	}, 1.0f, false, delay_time);
}
//empty
void ANodeBase::GeneratePacket(int chance)
{
}

std::vector<ANodeBase*> ANodeBase::ComputeNodePath(const ANodeBase* sender, int _id, int counter)
{
	if (this->id != _id)
	{
		for (auto node : nodeLinks)
		{
			if (node->link->isAlive && node->targetNode != sender)
			{
				if (counter < 50)
				{
					std::vector<ANodeBase*> _vec = node->targetNode->ComputeNodePath(this, _id, counter + 1);
					if (!_vec.empty())
					{
						_vec.push_back(this);
						return _vec;
					}
				}
				else return {};
			}
		}
	}
	else
	{
		return { this };
	}
	return {};
}
int ANodeBase::FindRouter(int _vlan, int counter) const
{
	for (auto node : nodeLinks)
	{
		if (node->link->isAlive && node->targetNode->vlan == _vlan)
		{
			if (node->targetNode->nodeType == NodeType::TechnicalInfrastructure) return node->targetNode->id;
			else
			{
				if (counter > 20) return -1;
				else return node->targetNode->FindRouter(_vlan, (counter + 1));
			}
		}
	}
	return -1;
}
ANodeBase* ANodeBase::CheckNeighbour(int node_id) const
{
	for (auto nodeLink : nodeLinks)
	{
		if (nodeLink->targetNode->id == node_id) return nodeLink->targetNode;
	}
	return nullptr;
}
ANodeBase* ANodeBase::CheckNeighbour(NodeType _nodeType) const
{
	for (auto nodeLink : nodeLinks)
	{
		if (nodeLink->targetNode->nodeType == _nodeType)
		{
			return nodeLink->targetNode;
		}
	}
	return nullptr;
}
std::vector<ANodeBase*>* ANodeBase::DeterminePath(int node_id)
{
	if (nodeType == NodeType::TechnicalInfrastructure) return new std::vector<ANodeBase*> { this };
	std::vector<ANodeBase*>* nodes = nullptr;
	ANodeBase* fast_bolv = nullptr;
	switch (node_id)
	{
		default:
		fast_bolv = CheckNeighbour(node_id);
		break;
	case -1:
		fast_bolv = CheckNeighbour(NodeType::Security);
		break;
	case -2: return nullptr;
	}
	if (fast_bolv) return new std::vector<ANodeBase*>{ this, fast_bolv };

	std::vector<ANodeBase*> bolv;
	int router_id = FindRouter(vlan);
	if (router_id != -1)
	{
		bolv = ComputeNodePath(this, router_id);
		nodes = new std::vector<ANodeBase*>;
		for (auto it = bolv.rbegin(); it != bolv.rend(); it++)
		{
			(*nodes).push_back(*it);
		}
		return nodes;
	}
	else
	{
		return nullptr;
	}
}
//Physical dispatch
void ANodeBase::SendPacket(APacket* packet, std::vector<ANodeBase*>* vec, std::vector<ANodeBase*>::iterator it)
{
	if (nodeState == NodeState::Offline || nodeState == NodeState::Overloaded)
	{
		(*vec).clear();
		delete vec;
		packet->Destroy();
		return;
	}
	if ((*vec).size() == 1)
	{
		(*vec).clear();
		delete vec;
		AcceptPacket(packet);
		return;
	}
	
	ALink* link = nullptr;
	for (auto nodeLink : (*it)->nodeLinks)
	{
		if (nodeLink->targetNode == *(it + 1))
		{
			link = nodeLink->link;
			if ((!(*it)->IsHidden() || !(*(it + 1))->IsHidden()) && !link->IsHidden())
			{
				packet->SetActorHiddenInGame(false);
			}
			else
			{
				packet->SetActorHiddenInGame(true);
			}
			break;
		}
	}
	
	float time = packet->sPacketMove->ComputeNodePath(*it, *(it + 1), link);
	link->AddWorkloadWithDelay(packet->size, time);
	*it = nullptr;
	++it;
	FTimerHandle timer = FTimerHandle();
	GetWorldTimerManager().SetTimer(timer, [packet, vec, it, link]
	{
		(*it)->CheckPacket(packet, vec, it);
	}, 1.0f, false, time);
}
//Intermediate check (all nodes on the packet path)
void ANodeBase::CheckPacket(APacket* packet, std::vector<ANodeBase*>* vec, std::vector<ANodeBase*>::iterator it)
{
	if (nodeState == NodeState::Offline || nodeState == NodeState::Overloaded)
	{
		(*vec).clear();
		delete vec;
		packet->Destroy();
		return;
	}
	
	short add_work = 0;
	float delay_time = 0.0f;
	if (sProtection && sProtection->isOn)
	{
		/*
		* 0 - Deny
		* 1 - Allow
		* 2 - Additional verification 
		*/
		switch (sProtection->SourceTargetCheck(packet))
		{
		case 0:
			packet->Destroy();
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Bad politic");
			return;
		case 1: 
			delay_time += 0.2f;
			add_work += 2;
			break;
		case 2:
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Add_check");
			delay_time += 1.0f;
			add_work += 5;
			//Additional verification...
			break;
		}
	}
	if (sSpyInfo)
	{
		add_work += 5;
		delay_time += 1.0f;
		if (packet->packetType == PacketType::Informative)
		{
			if (packet->sInformation->key_info_count) sSpyInfo->stolen_key_info += packet->sInformation->key_info_count;
			if (!packet->sInformation->roots_for_id.empty())
				sSpyInfo->stolen_roots.insert(
					sSpyInfo->stolen_roots.end(), packet->sInformation->roots_for_id.begin(), packet->sInformation->roots_for_id.end());
		}
	}
	
	if (it != vec->end() - 1)
	{
		AddWorkloadWithDelay(add_work, delay_time);
		FTimerHandle timer = FTimerHandle();
		GetWorldTimerManager().SetTimer(timer, [packet, vec, it]
		{
			(*it)->SendPacket(packet, vec, it);
		}, 1.0f, false, delay_time);
	}
	else
	{
		(*vec).clear();
		delete vec;
		AcceptPacket(packet);
	}
}
void ANodeBase::AcceptPacket(APacket* packet)
{
	short add_work = 0;
	float processing_time = 0.0f; 
	if (sProtection && sProtection->isOn)
	{
		add_work += 5;
		processing_time += 0.5f;
		if (sProtection->SignatureCheck(packet))
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Detected sign!");
			add_work += 3;
			processing_time += 0.5f;
			SendAlarmPacket();
			goto finish_acceptance;
		}
	}
	
	switch (packet->packetType)
	{
		case PacketType::AttackSpam:
		{
			//greatly increases the load
			add_work += 100;
			processing_time += 3.0f;
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Spamed!");
			
		} break;
		case PacketType::Simple:
		{
			//just a packet, just loads
			add_work += 4;
			processing_time += 1.5f;
		} break;
		case PacketType::AttackSpy:
		{
			//infects the node with a spy who collects information and sends it to a hacker
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Spy attack succesful!");
			sSpyInfo = new SpyInfo{ FTimerHandle(), 0, {}, packet->sThreat->spy_id };
			AddWorkload(5);
			GetWorldTimerManager().SetTimer(sSpyInfo->spyTimer, [this]
			{
				if (sProtection && sProtection->isOn && sProtection->behaviorAnalizer && rand() % (100 + 1) < behaviorChance)
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Spy detected!");
					GetWorldTimerManager().ClearTimer(sSpyInfo->spyTimer);
					delete sSpyInfo;
					AddWorkload(-5);
					SendAlarmPacket();
					return;
				}
				std::vector<ANodeBase*>* nodes = DeterminePath(sSpyInfo->spy_id);
				if (nodes && !(*nodes).empty())
				{
					APacket* packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
					packet->InitPacket(PacketType::Simple, this->id, sSpyInfo->spy_id);
					packet->sInformation = new APacket::Information();
					packet->sInformation->for_spy_ref = this;
					packet->sInformation->key_info_count += sSpyInfo->stolen_key_info;
					sSpyInfo->stolen_key_info = 0;
					if (this->sInformation)
					{
						packet->sInformation->key_info_count += this->sInformation->key_info_count;
						packet->sInformation->roots_for_id.insert(
							packet->sInformation->roots_for_id.end(), this->sInformation->vec_roots.begin(), this->sInformation->vec_roots.end());
						this->sInformation->key_info_count = 0;
					}
					SendPacket(packet, nodes, (*nodes).begin());
				}
			}, 20.0f, true, 20.0f);
		} break;
		case PacketType::AttackCapture:
		{
			//puts the node under the control of a hacker, instantly with roots and after many time without
			if (packet->sThreat->have_root)
			{
				add_work += 10;
				processing_time += 2.0f;
				nodeState = NodeState::Captured;
				if (sProtection) sProtection->isOn = false;
				UWidget_Manager::self_ref->AddNodeInfo(this, false);
				UWidget_Manager::self_ref->roots.Remove(this->id);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Captured by roots!");
				goto finish_acceptance;
			}
			
			AddWorkload(15);
			float brutforse_time;
			short* counter = new short(0);
			FTimerHandle* brutforseTimer = new FTimerHandle();
			switch (nodeType)
			{
			case NodeType::PersonalComputer: brutforse_time = 100.0f; break;
			case NodeType::TechnicalInfrastructure: brutforse_time = 1000.0f; break;
			case NodeType::DataStorage: brutforse_time = 1000.0f; break;
			case NodeType::Security: brutforse_time = 2000.0f; break;
			default: brutforse_time = 0.0f; break;
			}
			GetWorldTimerManager().SetTimer(*brutforseTimer, [this, brutforseTimer, counter, brutforse_time]
			{
				(*counter)++;
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Left until the capture: " + FString::FromInt(brutforse_time / 10 * (*counter)));
				if (*counter < 10)
				{
					if (sProtection && rand() % 101 > 50)
					{
						GetWorldTimerManager().ClearTimer(*brutforseTimer);
						delete counter;
						delete brutforseTimer;
						AddWorkload(-15);
						SendAlarmPacket();
					}
				}
				else
				{
					delete counter;
					GetWorldTimerManager().ClearTimer(*brutforseTimer);
					delete brutforseTimer;
					AddWorkload(-15);
					AddWorkloadWithDelay(10, 2.0f);
					nodeState = NodeState::Captured;
					if (sProtection) sProtection->isOn = false;
					UWidget_Manager::self_ref->AddNodeInfo(this, false);
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Captured!");
				}
			}, brutforse_time / 10.0f, true, 0.0f);
		} break;
		case PacketType::AttackCrash:
		{
			//Shuts down the node completely
			nodeState = NodeState::Offline;
			FTimerHandle timer = FTimerHandle();
			GetWorldTimerManager().SetTimer(timer, [this]
			{
				nodeState = NodeState::Working;
			}, 1.0f, false, 30.0f);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Crushed!");
		} break;
		case PacketType::Informative:
		{
			//All effects in AcceptPacket of other nodes 
			add_work += 4;
			processing_time += 1.5f;
		} break;
		case PacketType::Helpful:
		{
			//Heal or kill node
			add_work += 5;
			processing_time += 3.0f;
			bool operationState = false;
			if (nodeState == NodeState::Captured || sSpyInfo)
			{
				switch (packet->sHelper->helpState)
				{
				case APacket::Helper::HelpState::Killer:
				{
					if (rand() % (101) < killChance)
					{

						add_work += 10;
						processing_time += 5;
						operationState = true; 
						if (sSpyInfo)
						{
							delete sSpyInfo;
							AddWorkload(-5);
						}
						nodeState = NodeState::Offline;
					}
				}
				break;
				case APacket::Helper::HelpState::Healer:
				{
					if (rand() % (101) < healChance)
					{
						add_work += 30;
						processing_time += 10;
						operationState = true;
						if (sSpyInfo)
						{
							delete sSpyInfo;
							AddWorkload(-5);
						}
						nodeState = NodeState::Working;
					}
				}
				break;
				default: break;
				}
			}
			else operationState = true;
			std::vector<ANodeBase*>* nodes = DeterminePath(packet->source_id);
			if (nodes && !(*nodes).empty())
			{
				APacket* _packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
				_packet->InitPacket(PacketType::Helpful, this->id, packet->source_id);
				if (operationState)
				{
					_packet->sHelper->helpState = APacket::Helper::HelpState::SuccessReport;
				}
				else 
				{
					_packet->sHelper->helpState = APacket::Helper::HelpState::DefeatReport; 
				}
				SendPacket(_packet, nodes, (*nodes).begin());
			}
		} break;
	}
finish_acceptance:
	AddWorkloadWithDelay(add_work, processing_time);
	packet->Destroy();
}

void ANodeBase::SendAlarmPacket()
{
	if (nodeType == NodeType::Security)
	{
		APacket* packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
		packet->InitPacket(PacketType::Helpful, this->id, -1);
		packet->sHelper->isAlarm = true;
		AcceptPacket(packet);
	}
	if (ANodeSC::id_counter != 30)
	{
		std::vector<ANodeBase*>* nodes = DeterminePath(-1);
		if (!(*nodes).empty())
		{
			APacket* packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
			packet->InitPacket(PacketType::Helpful, this->id, -1);
			packet->sHelper->isAlarm = true;
			SendPacket(packet, nodes, (*nodes).begin());
		}
	}
}

bool ANodeBase::Protection::SignatureCheck(const APacket* packet) const
{
	auto signCheck = [](Signature sign_1, Signature sign_2) -> bool
	{
		if (sign_1 == sign_2 && rand() % (100 + 1) < sameSignChance) return true;
		switch (sign_1)
		{
			case Signature::Crash_1: if (sign_2 == Signature::Crash_2 && rand() % (100 + 1) < upSignChance) return true;
			case Signature::RootKit_1: if (sign_2 == Signature::RootKit_2 && rand() % (100 + 1) < upSignChance) return true;
			case Signature::Spy_1: if (sign_2 == Signature::Spy_2 && rand() % (100 + 1) < upSignChance) return true;
			default: return false;
		}
	};
	if (packet->sThreat)
	{
		for (auto _sign : threatSigns)
		{
			if (signCheck(_sign, packet->sThreat->sign)) return true;
		}
	}
	return false;
}

int ANodeBase::Protection::SourceTargetCheck(const APacket* packet) const
{
	/*
	* 0 - Deny
	* 1 - Allow
	* 2 - Additional verification 
	*/
	int s_id = packet->source_id, t_id = packet->target_id;
	if (s_id < 5)//If source is EO
	{
		if (politic == Politic::OnlyAllowed) return 0;
		if (t_id < 70) return 0;//If target is not PC
		return 1;
	}
	if (t_id < 5)//If target is EO
	{
		if (politic == Politic::OnlyAllowed) return 0;
		if (s_id < 70) return 1;//If source is not PC 
		return 1;
	}
	return 1;
}

void ANodeBase::AddInformation(ANodeBase* node_ptr)
{
	if (!sInformation)
	{
		sInformation = new Information();
		sInformation->vec_net_id = { node_ptr };
	}
	else
	{
		if (sInformation->vec_net_id.empty()) sInformation->vec_net_id = { node_ptr };
		else
		{
			auto contain = [](std::vector<ANodeBase*> &vec, int node_id) -> bool
			{
				for (auto elem : vec)
				{
					if (elem->id == node_id) return true;
				}
				return false;
			};
			if (!contain(sInformation->vec_net_id, node_ptr->id)) sInformation->vec_net_id.push_back(node_ptr);
		}
	}
}
void ANodeBase::AddKeyInfo()
{
	if (!sInformation) sInformation = new Information();
	sInformation->key_info_count = 2;
}
void ANodeBase::AddRoots(int root_id)
{
	if (!sInformation)
	{
		sInformation = new Information();
		sInformation->vec_roots.push_back(root_id);
	}
	else
	{
		if (sInformation->vec_roots.empty()) sInformation->vec_roots.push_back(root_id);
		else
		{
			auto contain = [](std::vector<short>& vec, int node_id) -> bool
			{
				for (auto elem : vec)
				{
					if (elem == node_id) return true;
				}
				return false;
			};
			if (!contain(sInformation->vec_roots, root_id)) sInformation->vec_roots.push_back(root_id);
		}
	}
}
bool ANodeBase::ContainInfo()
{
	if (sInformation != nullptr) return true; else return false;
}

void ANodeBase::FillPacketTemp(TSubclassOf<APacket> temp)
{
	packetTemp = temp;
}
void ANodeBase::SetVLAN(int num)
{
	vlan = num;
}
