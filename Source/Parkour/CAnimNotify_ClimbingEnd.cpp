// Fill out your copyright notice in the Description page of Project Settings.


#include "CAnimNotify_ClimbingEnd.h"
#include "CPlayer.h"
#include "CAnimInstance.h"

FString UCAnimNotify_ClimbingEnd::GetNotifyName_Implementation() const
{
	return TEXT("ClimbingEnd");
}

void UCAnimNotify_ClimbingEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	UCAnimInstance* Anim = Cast<UCAnimInstance>(MeshComp->GetAnimInstance());
	if (Anim == nullptr) return;
	ACPlayer* Player = Cast<ACPlayer>(MeshComp->GetOwner());
	if (Player == nullptr) return;
	Player->ClimbLedge(false);
	Anim->SetClimbing(false);
}
