// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Nodes.h"
//#include "NodeSC.h"

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
	//min + (rand() % (max - min + 1))
	/*if (nodeState != NodeState::Overloaded && nodeState != NodeState::Offline && !nodeLinks.empty())
	{
		GeneratePacket(std::rand() % (101));
	}*/

}

void ANodeSC::CheckPacket(APacket* packet, std::vector<ANodeBase*>* vec, std::vector<ANodeBase*>::iterator it, float time)
{
	//“ут считаем time и проводим соответствующую обработку
	time = 0.1f;
	//ѕровер€ем пакет, исход€ из собственных средств
	if (it != vec->end() - 1)//≈сли это не узел назначени€, то мы что то делаем
	{
		//„то то делаем
		time += 0.5f;
	}
	ANodeBase::CheckPacket(packet, vec, it, time);
}
void ANodeSC::AcceptPacket(APacket* packet)
{
	/* „то есть только у узла безопасности?
		*	1. —истема сканировани€ сети (интеллектуальна€ Xd)
		*		ћожно сделать так, что все узлы высылают свои пакеты узлу безопасности и он на их основе принимает решение..но это слишком массивно и тупо
		*	2. ћожно ему присвоить навыки отвоЄвывани€ узлов
		*		¬ режиме тревоги, если известен захваченный узел, он отвоЄвываетс€ каким то образом
		*	3. ¬ общем то апгрейд систем защиты других узлов и есть отдельна€ механика
	*/
	if (packet->packetType == PacketType::Helpful)
	{
		//“ут плохо получаетс€
		if (have_recovery_system)
		{
			switch (packet->sHelper->helpState)
			{
			case APacket::Helper::HelpState::SuccessReport:
			{
				int item_num = sApocalypseRescueKit->map_id_vec[packet->source_id];
				auto it = sApocalypseRescueKit->list_apocalypse_timers.begin();
				std::advance(it, item_num);
				GetWorldTimerManager().ClearTimer(*std::get<0>(*it));
				delete std::get<0>(*it);
				std::get<0>(*it) = nullptr;
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
				std::get<1>(*it) = true;
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
				/* “ревога:
				*	1. ”величивает веро€тность обнаружени€ вредоносного пакета
				*	2. ѕереводит политику в режим всЄ запрещено
				*	3. ѕри наличии механики и таких узлов, начинаетс€ процес отвоЄвывани€ узлов
				*	4. ¬озможно сканирование сети становитс€ более агрессивным (что повышает нагрузку на сеть)
				*	5. ¬ зависимости от времени атаки, запускаетс€ процесс спасени€ от апокалипсиса
				*/
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
	/* SC генерирует:
	* : ничего
	* : пакеты проверки (что-то подобное хочетс€ реализовать)
	* : полезные пакеты (если происходит атака, если нет то ничего)
	* : может быть обычные пакеты (пакеты проверки?) 
	* “ут надо шансы подвергать поправке на ?безопасника? узла и прочее (если будет)
	*/
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
			SendPacket(packet, nodes, (*nodes).begin());

			FTimerHandle* timer = new FTimerHandle();
			sApocalypseRescueKit->list_apocalypse_timers.push_back(std::tuple<FTimerHandle*, bool>{timer, false});
			sApocalypseRescueKit->map_id_vec.insert({ _i, sApocalypseRescueKit->list_size });

			//«апускаетс€ таймер, если ответа не приходит, то начинают спамитьс€ киллеры
			GetWorldTimerManager().SetTimer(*timer, [this, _id = _i, vec_num = sApocalypseRescueKit->list_size, path = *nodes]
			{
				//“ак как путь статичен, € копирую в стэк вектор узлов, а уже из него создаю копию в куче и указатель на неЄ
				int item_num = sApocalypseRescueKit->map_id_vec[vec_num];
				auto it = sApocalypseRescueKit->list_apocalypse_timers.begin();
				std::advance(it, item_num);
				if (std::get<1>(*it))
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
				}
				
			}, 20.0f, true, 20.0f);

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