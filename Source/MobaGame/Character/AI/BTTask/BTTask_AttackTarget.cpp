// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_AttackTarget.h"
#include "../AIController/MobaGameAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

EBTNodeResult::Type UBTTask_AttackTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (Blackboard_Actor.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		if (UBlackboardComponent* MyBlackBoard = OwnerComp.GetBlackboardComponent())
		{
			if (AMobaGameAIController* AIConroller = Cast<AMobaGameAIController>(OwnerComp.GetOwner()))
			{
				if (AMobaGameCharacter* InTarget = Cast<AMobaGameCharacter>(MyBlackBoard->GetValueAsObject(Blackboard_Actor.SelectedKeyName)))
				{
					AIConroller->NormalAttack(InTarget);
					return EBTNodeResult::Succeeded;
				}
			}
		}
	}

	return EBTNodeResult::Failed;
}

void UBTTask_AttackTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		Blackboard_Actor.ResolveSelectedKey(*BBAsset);
	}
}