// Fill out your copyright notice in the Description page of Project Settings.


#include "MobyGameCharacterBTService.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "../AIController/MobyGameAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void UMobyGameCharacterBTService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (BlackboardKey_Target.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() &&
		BlackboardKey_Distance.SelectedKeyType == UBlackboardKeyType_Float::StaticClass() &&
		BlackboardKey_Location.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		if (AMobyGameAIController* OwnerController = Cast<AMobyGameAIController>(OwnerComp.GetOwner()))
		{
			if (AMobyGameCharacter* OwnerCharacter = Cast<AMobyGameCharacter>(OwnerController->GetPawn()))
			{
				if (UBlackboardComponent* MyBlackBoard = OwnerComp.GetBlackboardComponent())
				{
					AMobyGameCharacter* InTarget = Cast<AMobyGameCharacter>(MyBlackBoard->GetValueAsObject(BlackboardKey_Target.SelectedKeyName));
					if (!InTarget)
					{
						InTarget = OwnerController->FindTarget();
					}

					float Distance = 999999.0f;
					if (InTarget)
					{
						Distance = FVector::Dist(InTarget->GetActorLocation(), OwnerCharacter->GetActorLocation());

						float RangeAttack = OwnerCharacter->GetCharacterAttribute()->RangeAttack;
						if (Distance > RangeAttack)
						{
							//ƫ�ƽ��� ����ת��
							MyBlackBoard->SetValueAsVector(BlackboardKey_Location.SelectedKeyName, InTarget->GetActorLocation());
						}
						else
						{
							//ƫ�ƽ��� ����ת��
							FVector D = InTarget->GetActorLocation() - OwnerCharacter->GetActorLocation();
							D.Normalize();
							FVector Pos = OwnerCharacter->GetActorLocation() + D * 20;
							MyBlackBoard->SetValueAsVector(BlackboardKey_Location.SelectedKeyName, Pos);
						}
					}

					MyBlackBoard->SetValueAsFloat(BlackboardKey_Distance.SelectedKeyName, Distance);
				}
			}
		}
	}
}

void UMobyGameCharacterBTService::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		BlackboardKey_Target.ResolveSelectedKey(*BBAsset);
		BlackboardKey_Distance.ResolveSelectedKey(*BBAsset);
		BlackboardKey_Location.ResolveSelectedKey(*BBAsset);
	}
}