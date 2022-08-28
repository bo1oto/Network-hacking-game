
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
	sProtection->bSpamFilter = true;
	have_recovery_system = true;
}
void ANodeSC::Tick(float DeltaTime)
{
	ANodeBase::Tick(DeltaTime);
}

void ANodeSC::AcceptPacket(APacket* packet)
{
	if (packet->packetType == EPacketType::Helpful)
	{
		if (have_recovery_system)
		{
			int item_num = sApocalypseRescueKit->map_id_pos[packet->source_id];
			auto it = sApocalypseRescueKit->list_apocalypse_timers.begin();
			std::advance(it, item_num);
			switch (packet->sHelper->helpState)
			{
			case APacket::Helper::EHelpState::SuccessReport:
			{
				GetWorldTimerManager().ClearTimer(*(*it).first);
				delete (*it).first;
				(*it).first = nullptr;
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Help for " + FString::FromInt(packet->source_id) + " successful!");

				AddWorkloadWithDelay(5, 2.0f);
				packet->Destroy();
				return;
			}
			case APacket::Helper::EHelpState::DefeatReport:
			{
				(*it).second = true;
				AddWorkloadWithDelay(5, 2.0f);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Help for " + FString::FromInt(packet->source_id) + " failed!");

				AddWorkloadWithDelay(5, 2.0f);
				packet->Destroy();
				return;
			}
			default: 
				break;
			}
		}
		if (packet->sHelper->isAlarm)
		{
			if (!ANodeBase::IsAlarm)
			{
				ANodeBase::IsAlarm = true;
				ANodeBase::politic = EPolitic::OnlyAllowed;
				ANodeBase::sameSignChance = 100;
				ANodeBase::upSignChance = 50;
				ANodeBase::behaviorChance = 75;
				if (have_recovery_system)
				{
					sApocalypseRescueKit = new ApocalypseRescueKit();
					SaveThisWorld();
				}
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Alarm!!!");
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
		std::vector<ANodeBase*> nodes{};
		DeterminePath(_i, nodes);
		if (!nodes.empty())
		{
			APacket* packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
			packet->InitPacket(EPacketType::Helpful, this->id, _i, std::vector<AActor*>(nodes.begin(), nodes.end()));
			packet->sHelper->helpState = APacket::Helper::EHelpState::Healer;

			FTimerHandle* timer = new FTimerHandle();
			sApocalypseRescueKit->list_apocalypse_timers.push_back(std::make_pair(timer, false));
			sApocalypseRescueKit->map_id_pos.insert({ _i, sApocalypseRescueKit->list_size });

			//The timer starts, if there is no response, then the killers start spamming
			GetWorldTimerManager().SetTimer(*timer, [this, _id = _i, vec_num = sApocalypseRescueKit->list_size, path = nodes]
			{
				bool no_response_flag = true;
				//Так как путь статичен, я копирую в стэк вектор узлов, а уже из него создаю копию в куче и указатель на неё
				auto it = sApocalypseRescueKit->list_apocalypse_timers.begin();
				std::advance(it, vec_num);
				if ((*it).second)
				{
					APacket* _packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
					_packet->InitPacket(EPacketType::Helpful, this->id, _id, std::vector<AActor*>(path.begin(), path.end()));
					_packet->sHelper->helpState = APacket::Helper::EHelpState::Killer;
					SendPacket(_packet, _packet->path.begin());
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Killer for " + FString::FromInt(_id) + " coming!");
				}
				else if (no_response_flag)
				{
					no_response_flag = false;
					FTimerHandle timer = FTimerHandle();
					GetWorldTimerManager().SetTimer(timer, [it]
					{
						(*it).second = true;
					}, 1.0f, false, 30.0f);
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Not yet for " + FString::FromInt(_id));
				}
				
			}, 15.0f, true, 15.0f);

			SendPacket(packet, packet->path.begin());

			sApocalypseRescueKit->list_size++;
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Healer for " + FString::FromInt(_i) + " coming!");
		}
	};
	for (int i = 10; i < ANodeTI::id_counter; ++i)
	{
		BlessAndSave(i);
	}
	for (int i = 50; i < ANodeDS::id_counter; ++i)
	{
		BlessAndSave(i);
	}
	for (int i = 70; i < ANodePC::id_counter; ++i)
	{
		BlessAndSave(i);
	}
	for (int i = 30; i < ANodeSC::id_counter; ++i)
	{
		if (i != id) BlessAndSave(i);
	}
}
