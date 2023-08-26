// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PARKOUR_API UCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UCAnimInstance();
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override; //애니메이션 틱함수
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
		float CurrentSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
		bool isAir;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
		bool isHandling;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
		bool isClimbing; //올라가는 중인지?

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
		bool isMoveRight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
		bool isMoveLeft;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
		bool isMovingRight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
		bool isMovingLeft;
public:
	FORCEINLINE void SetHandling(bool isHandle)
	{
		isHandling = isHandle;
	}
	FORCEINLINE void SetClimbing(bool isClimb)
	{
		isClimbing = isClimb;
	}
private:
	UPROPERTY(VisibleAnywhere, Category = "Parkour")
		UAnimMontage* ClimbMontage; //올라가는 몽타주 변수
public:
	void ClimbLedge(bool isClimb);
	
};
