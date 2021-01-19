// Copyright Epic Games, Inc. All Rights Reserved.

#include "MobaGamePlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Character/CharacterInstance/MobaGameCharacter.h"
#include "Engine/World.h"
#include "MobaPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Tool/ScreenMoveUnits.h"
AMobaGamePlayerController::AMobaGamePlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AMobaGamePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
	{
		if (bMoveToMouseCursor)
		{
			MoveToMouseCursor();
		}
		FScreenMoveUnits().ListenScreenMove(this, 10.f);
	}
}

void AMobaGamePlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AMobaGamePlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AMobaGamePlayerController::OnSetDestinationReleased);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMobaGamePlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AMobaGamePlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &AMobaGamePlayerController::OnResetVR);
}

void AMobaGamePlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMobaGamePlayerController::MoveToMouseCursor()
{
	if (AMobaPawn* MyPawn = Cast<AMobaPawn>(GetPawn()))
	{
		// Trace to see what is under the mouse cursor		
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
		if (LocalPlayer && LocalPlayer->ViewportClient)
		{
			FVector2D MousePosition;
			if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
			{
				// Early out if we clicked on a HUD hitbox
				/*if (GetHUD() != NULL && GetHUD()->GetHitBoxAtCoordinates(MousePosition, true))
				{
					return;
				}*/

				FVector WorldOrigin;
				FVector WorldDirection;
				if (UGameplayStatics::DeprojectScreenToWorld(this, MousePosition, WorldOrigin, WorldDirection) == true)
				{
					VerifyMouseWorldPostionClickOnServer(WorldOrigin, WorldDirection);
				}
			}
		}
	}
}

void AMobaGamePlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (AMobaPawn* MyPawn = Cast<AMobaPawn>(GetPawn()))
	{
		FVector2D ScreenSpaceLocation(Location);

		// Trace to see what is under the touch location
		FHitResult HitResult;
		GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
		if (HitResult.bBlockingHit)
		{
			// We hit something, move there
			MyPawn->CharacterMoveToOnServer(HitResult.ImpactPoint);
		}
	}
}

void AMobaGamePlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void AMobaGamePlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

void AMobaGamePlayerController::VerifyMouseWorldPostionClickOnServer_Implementation(const FVector& WorldOrigin, const FVector& WorldDirection)
{
	if (AMobaPawn* MyPawn = Cast<AMobaPawn>(GetPawn()))
	{
		FHitResult HitResult;
		FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(ClickableTrace), false);

		auto TracePos = [&](ECollisionChannel InChannel)->bool
		{
			return GetWorld()->LineTraceSingleByChannel(HitResult, WorldOrigin, WorldOrigin + WorldDirection * HitResultTraceDistance, InChannel, CollisionQueryParams);
		};
		
		if (TracePos(ECC_GameTraceChannel1))
		{
			if (HitResult.bBlockingHit)
			{
				MyPawn->CharacterMoveToTargetAttackOnServer(HitResult.ImpactPoint, Cast<APawn>(HitResult.Actor));
				return;
			}
		}

		if (TracePos(ECC_Visibility))
		{
			if (HitResult.bBlockingHit)
			{
				MyPawn->CharacterMoveToOnServer(HitResult.ImpactPoint);
			}
		}
	}
}

bool AMobaGamePlayerController::VerifyMouseWorldPostionClickOnServer_Validate(const FVector& WorldOrigin, const FVector& WorldDirection)
{
	return true;
}
