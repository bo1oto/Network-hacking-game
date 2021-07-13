// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NodeBase.h"
#include "Uncrushable/Widget_Manager.h"
#include "NodeTI.h"
#include "NodeSC.h"


bool ANodeBase::IsAlarm = false;
Politic ANodeBase::politic = Politic::NotForbidden;//вообще, политика должна определяться, исходя из уровня
int ANodeBase::sameSignChance = 85;
int ANodeBase::upSignChance = 10;
int ANodeBase::behaviorChance = 15;
int ANodeBase::healChance = 60;
int ANodeBase::killChance = 80;
TSubclassOf<APacket> ANodeBase::packetTemp = nullptr;

ANodeBase::ANodeBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
	//min + (rand() % (max - min + 1))

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

FString ANodeBase::GetInfo()
{
	FString str = TEXT("id: ") + FString::FromInt(id) + "\n" + TEXT("vlan: ") + FString::FromInt(vlan) + "\n" + TEXT("workload: ") + FString::FromInt(workload);
	//Тут можно сделать перебор среди штат, и выделить тех, у кого есть доступ к узлу
	/*std::vector<UEmployee*>::const_iterator it = staff.begin();
	while (it != staff.end())
	{
		str += (((*it)->GetShortInfo()).c_str());
		++it; 
	}*/
	return str;
}
FText ANodeBase::GetTypeInfo()
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
inline void ANodeBase::AddWorkloadWithDelay(short _add_work = 0, float delay_time = 0.0f)
{
	AddWorkload(_add_work);
	FTimerHandle unloading_timer = FTimerHandle();
	GetWorldTimerManager().SetTimer(unloading_timer, [_add_work, this]
	{
		AddWorkload(-_add_work);
	}, 1.0f, false, delay_time);
}

//Потом, если будет продолжение разработки, то реализую тут алгоритм Дийкстры попросту, с учётом скорости линков, размера и прочего
std::vector<ANodeBase*> ANodeBase::ComputeNodePath(ANodeBase* sender, int _id, int counter)
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
int ANodeBase::FindRouter(int _vlan, int counter)
{
	//Если TI находится в более чем двух соединениях, то метод зацикливается
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
ANodeBase* ANodeBase::CheckNeighbour(int node_id)
{
	for (auto nodeLink : nodeLinks)
	{
		if (nodeLink->targetNode->id == node_id) return nodeLink->targetNode;
	}
	return nullptr;
}
ANodeBase* ANodeBase::CheckNeighbour(NodeType _nodeType)
{
	for (auto nodeLink : nodeLinks)
	{
		if (nodeLink->targetNode->nodeType == _nodeType)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Node (by type) is neighbour");
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
	//Проверка соседних узлов
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
	if (fast_bolv)
	{
		nodes = new std::vector<ANodeBase*>{ this, fast_bolv };
		return nodes;
	}

	std::vector<ANodeBase*> bolv;
	int router_id = FindRouter(vlan);
	if (router_id != -1)
	{
		bolv = ComputeNodePath(this, router_id);
		nodes = new std::vector<ANodeBase*>;
		for (std::vector<ANodeBase*>::reverse_iterator it = bolv.rbegin(); it != bolv.rend(); it++)
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

//Физическое перемещение пакета
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
	float time = packet->sPacketMove->ComputeNodePath(*it, *(it + 1));
	if (!(*it)->IsHidden() || !(*(it + 1))->IsHidden())
	{
		for (auto nodeLink : (*it)->nodeLinks)
		{
			if (nodeLink->targetNode == *(it + 1))
			{
				if (nodeLink->link->IsHidden())
				{
					packet->SetActorHiddenInGame(true);
				}
				else
				{
					packet->SetActorHiddenInGame(false);
				}
			}
		}
	}
	else 
	{ 
		packet->SetActorHiddenInGame(true); 
	}
	*it = nullptr;
	it++;
	FTimerHandle timer = FTimerHandle();
	GetWorldTimerManager().SetTimer(timer, [packet, vec, it]
	{
		(*it)->CheckPacket(packet, vec, it);
	}, 1.0f, false, time);
}
//Промежуточная проверка пакета
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
		* 0 - отклонить
		* 1 - пропустить
		* 2 - //Дополнительная проверка
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
			//какие-то указания к доп проверке (по политике)
			break;
		}
	}
	if (sSpyInfo)
	{
		if (packet->packetType == PacketType::Informative && packet->sInformation->key_info_count)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Stolen!");
			this->sSpyInfo->stolen_key_info += packet->sInformation->key_info_count;
			add_work += 5;
			delay_time += 1.0f;
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
//Обработка пакета узлом назначения
void ANodeBase::AcceptPacket(APacket* packet)
{
	short add_work = 0;
	float processing_time = 0.0f; 
	//По идее не нужна, так как CheckPacket отсеет таких, а он всегда перед AcceptPacket
	//if (nodeState == NodeState::Overloaded || nodeState == NodeState::Offline) goto finish_acceptance;
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
			//Сильно повышает нагрузку узла
			add_work += 50;
			processing_time += 3.0f;
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Spamed!");
			
		} break;
		case PacketType::Simple:
		{
			//просто пакет просто повышает нагрузку
			add_work += 4;
			processing_time += 1.5f;
		} break;
		case PacketType::AttackSpy:
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Spy here!");
			sSpyInfo = new SpyInfo{ FTimerHandle(), 0, packet->sThreat->spy_id };
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
						this->sInformation->key_info_count = 0;
					}
					SendPacket(packet, nodes, (*nodes).begin());
				}
			}, 20.0f, true, 20.0f);
		} break;
		case PacketType::AttackCapture:
		{
			add_work += 6;
			processing_time += 1.5f;
			nodeState = NodeState::Captured;
			if (sProtection) sProtection->isOn = false;
			UWidget_Manager::self_ref->AddNodeInfo(this, false);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Captured!");
			//пакет, содержащий в себе вредоносный код, захватывающий узел (при прочих условиях)
		} break;
		case PacketType::AttackCrash:
		{
			//Просто переводит узел в режим оффлайн
			//Наверно тут же должен запускаться таймер восстановления узла в строй (у каждого своё время и свои потери вроде как)
			//С другой стороны, тут поможет только физическое вмешательство
			//Но пока так
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
			//...
			add_work += 4;
			processing_time += 1.5f;
		} break;
		case PacketType::Helpful:
		{
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
						FTimerHandle timer = FTimerHandle();
						GetWorldTimerManager().SetTimer(timer, [this]
						{
							nodeState = NodeState::Offline;
						}, 0.0f, false, 2.0f);
						GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "EZ Kill!");
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
						FTimerHandle timer = FTimerHandle();
						GetWorldTimerManager().SetTimer(timer, [this]
						{
							nodeState = NodeState::Working;
						}, 1.0f, false, 11.0f);
						GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "EZ Heal!");
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
//Находим ближайший узел безопасности и отправляем ему пакет с тревогой
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

bool ANodeBase::Protection::SignatureCheck(APacket* packet)
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

int ANodeBase::Protection::SourceTargetCheck(APacket* packet)
{
	/* Какие у нас отношения тут присутствуют?
	*	1. Пакеты из EO (0-5) при политики OnlyAllowed блокируются, при NotForbidden только определённые?
	*	2. Пакеты из EO в DS и обратно блокируются всегда (Может быть, если это EO, ведущее к другому филиалу например, но это на потом)
	*	3. Пакеты из SC и TI в EO и обратно подвержены доп проверке?
	*	4. Пакеты из TI в DS подвержены доп проверке
	*	5. Пакеты из TI как источника это в принципе странно
	* 
	*/
	/* 
	* 0 - отклонить
	* 1 - пропустить
	* 2 - //Дополнительная проверка (в зависимоти от политики)
	*/
	int s_id = packet->source_id, t_id = packet->target_id;
	if (s_id < 5)//Если это источник EO
	{
		if (politic == Politic::OnlyAllowed) return 0;//Тут поправка на другие филиалы
		if (t_id < 70) return 0;//Если таргет это не PC (тут не уверен, надо подумать)
		//Тут возможно условие, что если запрещён этот id
		return 1;
	}
	if (t_id < 5)//Если таргет это EO
	{
		if (politic == Politic::OnlyAllowed) return 0;//Тут поправка на другие филиалы
		if (s_id < 70) return 1;//Если источник это не PC (тут не уверен, надо подумать)
		//Тут возможно условие, что если запрещён этот id
		return 1;
	}
	return 1;
}

void ANodeBase::GeneratePacket(int chance)
{
}

void ANodeBase::FillPacketTemp(TSubclassOf<APacket> temp)
{
	packetTemp = temp;
}
void ANodeBase::SetVLAN(int num)
{
	vlan = num;
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
bool ANodeBase::ContainInfo()
{
	if (sInformation != nullptr) return true; else return false;
}

