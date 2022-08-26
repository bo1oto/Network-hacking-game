
#pragma once
#include "NodeSC.h"
#include "Nodes.h"

int ANodeSC::id_counter = 30;

ANodeSC::ANodeSC() : ANodeBase()
{
}

void ANodeSC::BeginPlay()
{
	ANodeBase::BeginPlay();
	AddWorkload(40);
	nodeType = NodeType::Security;
	id = id_counter;
	id_counter++;
	helpTimer = FTimerHandle();
	sProtection->behaviorAnalizer = true;
	sProtection->spamFilter = true;
	have_recovery_system = true;
}
void ANodeSC::Tick(float DeltaTime)
{
	ANodeBase::Tick(DeltaTime);
}

void ANodeSC::AcceptPacket(APacket* packet)
{
	if (packet->packetType == PacketType::Helpful)
	{
		if (have_recovery_system)
		{
			switch (packet->sHelper->helpState)
			{
			case APacket::Helper::HelpState::SuccessReport:
			{
				int item_num = sApocalypseRescueKit->map_id_vec[packet->source_id];
				auto it = sApocalypseRescueKit->list_apocalypse_timers.begin();
				std::advance(it, item_num);
				GetWorldTimerManager().ClearTimer(*(*it).first);
				delete (*it).first;
				(*it).first = nullptr;
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Help for " + FString::FromInt(packet->source_id) + " successful!");

				AddWorkloadWithDelay(5, 2.0f);
				packet->Destroy();
				return;
			}
			case APacket::Helper::HelpState::DefeatReport:
			{
				int item_num = sApocalypseRescueKit->map_id_vec[packet->source_id];
				auto it = sApocalypseRescueKit->list_apocalypse_timers.begin();
				std::advance(it, item_num);
				(*it).second = true;
				AddWorkloadWithDelay(5, 2.0f);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Help for " + FString::FromInt(packet->source_id) + " failed!");

				AddWorkloadWithDelay(5, 2.0f);
				packet->Destroy();
				return;
			}
			default: break;
			}
		}
		if (packet->sHelper->isAlarm)
		{
			if (!ANodeBase::IsAlarm)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Alarm!!!");
				ANodeBase::IsAlarm = true;
				ANodeBase::politic = Politic::OnlyAllowed;
				ANodeBase::sameSignChance = 100;
				ANodeBase::upSignChance = 50;
				ANodeBase::behaviorChance = 75;
				if (have_recovery_system)
				{
					sApocalypseRescueKit = new ApocalypseRescueKit();
					SaveThisWorld();
				}
			}
			AddWorkloadWithDelay(10, 1.0f);
			packet->Destroy();
			return;
		}
	}
	ANodeBase::AcceptPacket(packet);
}

void ANodeSC::GeneratePacket(int chance)
{
}

void ANodeSC::SaveThisWorld()
{
	auto BlessAndSave = [this](int _i)
	{
		std::vector<ANodeBase*>* nodes = DeterminePath(_i);
		if (nodes && !(*nodes).empty())
		{
			APacket* packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
			packet->InitPacket(PacketType::Helpful, this->id, _i);
			packet->sHelper->helpState = APacket::Helper::HelpState::Healer;

			FTimerHandle* timer = new FTimerHandle();
			sApocalypseRescueKit->list_apocalypse_timers.push_back(std::make_pair(timer, false));
			sApocalypseRescueKit->map_id_vec.insert({ _i, sApocalypseRescueKit->list_size });

			//The timer starts, if there is no response, then the killers start spamming
			GetWorldTimerManager().SetTimer(*timer, [this, _id = _i, vec_num = sApocalypseRescueKit->list_size, path = *nodes]
			{
				//Так как путь статичен, я копирую в стэк вектор узлов, а уже из него создаю копию в куче и указатель на неё
				auto it = sApocalypseRescueKit->list_apocalypse_timers.begin();
				std::advance(it, vec_num);
				if ((*it).second)
				{
					std::vector<ANodeBase*>* nodes = new std::vector<ANodeBase*>{ path };
					APacket* _packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
					_packet->InitPacket(PacketType::Helpful, this->id, _id);
					_packet->sHelper->helpState = APacket::Helper::HelpState::Killer;
					SendPacket(_packet, nodes, (*nodes).begin());
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Killer for " + FString::FromInt(_id) + " coming!");
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Not yet for " + FString::FromInt(_id));
					FTimerHandle timer = FTimerHandle();
					GetWorldTimerManager().SetTimer(timer, [it]
					{
						(*it).second = true;
					}, 1.0f, false, 30.0f);
				}
				
			}, 15.0f, true, 15.0f);

			SendPacket(packet, nodes, (*nodes).begin());

			sApocalypseRescueKit->list_size++;
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Healer for " + FString::FromInt(_i) + " coming!");
	}
	};
	for (int i = 10; i < ANodeTI::id_counter; i++) BlessAndSave(i);
	for (int i = 50; i < ANodeDS::id_counter; i++) BlessAndSave(i);
	for (int i = 70; i < ANodePC::id_counter; i++) BlessAndSave(i);
	for (int i = 30; i < ANodeSC::id_counter; i++)
	{
		if (i != id) BlessAndSave(i);
	}
}
