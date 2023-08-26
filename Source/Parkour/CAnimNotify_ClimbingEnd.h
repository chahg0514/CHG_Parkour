// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_ClimbingEnd.generated.h"

/**
 * 
 */
UCLASS()
class PARKOUR_API UCAnimNotify_ClimbingEnd : public UAnimNotify
{
	GENERATED_BODY()
public:
	FString GetNotifyName_Implementation() const; //이것도 정의 만들어줘야하는데 안뜨니까 직접 만들어줘야함
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
