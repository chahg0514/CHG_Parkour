// Fill out your copyright notice in the Description page of Project Settings.


#include "CAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "CPlayer.h"

UCAnimInstance::UCAnimInstance()
{
	CurrentSpeed = 0.f;
	isAir = false;
	isHandling = false;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ClimbAsset(TEXT("AnimMontage'/Game/InfinityBladeWarriors/ParkourAnimations/Climb_Montage.Climb_Montage'"));
	if (ClimbAsset.Succeeded())
	{
		ClimbMontage = ClimbAsset.Object;
	}
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	APawn* Pawn = TryGetPawnOwner();
	if (IsValid(Pawn))
	{
		CurrentSpeed = Pawn->GetVelocity().Size2D();
		isAir = Pawn->GetMovementComponent()->IsFalling();

		ACPlayer* MyPlayer = Cast<ACPlayer>(Pawn); //이걸 업데이트에서..?
		if (MyPlayer == nullptr) return;
		isMoveRight = MyPlayer->GetIsMoveRight();
		isMoveLeft = MyPlayer->GetIsMoveLeft();
		isMovingRight = MyPlayer->GetIsMovingRight();
		isMovingLeft = MyPlayer->GetIsMovingLeft();

	}
}

void UCAnimInstance::ClimbLedge(bool isClimb)
{
	isClimbing = isClimb;
	Montage_Play(ClimbMontage); //몽타주 재생
	isHandling = false;
}
