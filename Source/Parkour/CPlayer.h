// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CPlayer.generated.h"

UCLASS()
class PARKOUR_API ACPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class UCameraComponent* FollowCamera;
private:
	UFUNCTION()
		void MoveForward(float value);
	UFUNCTION()
		void MoveRight(float value);
	UFUNCTION()
		void ExitLedge();
	UFUNCTION()
		void ClimbingLedge();
	UFUNCTION()
		void ClimbingJump();
private:
	void ForwardTracer();
	void HeightTracer();
	void GrabLedge();
	bool HipToLedge();
	FVector MoveToLocation();
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Parkour")
		FVector HeightLocation;
	UPROPERTY(VisibleAnywhere, Category = "Parkour")
		FVector WallLocation;
	UPROPERTY(VisibleAnywhere, Category = "Parkour")
		FVector WallNormal;
	UPROPERTY(VisibleAnywhere, Category = "Parkour")
		bool isClimbingLedge;
	UPROPERTY(VisibleAnywhere, Category = "Parkour")
		bool isHanging;
public:
	void ClimbLedge(bool isClimb);
private:
	UPROPERTY(VisibleAnywhere, Category = "SideMove")
		bool isMoveRight; //가능여부
	UPROPERTY(VisibleAnywhere, Category = "SideMove")
		bool isMoveLeft;
	UPROPERTY(VisibleAnywhere, Category = "SideMove")
		bool isMovingRight;
	UPROPERTY(VisibleAnywhere, Category = "SideMove")
		bool isMovingLeft;
	UPROPERTY(VisibleAnywhere, Category = "SideMove")
		float RightAxis; //오른쪽 축 값
	UPROPERTY(VisibleAnywhere, Category = "SideMove")
		class USceneComponent* RightScene;
	UPROPERTY(VisibleAnywhere, Category = "SideMove")
		class USceneComponent* LeftScene; //씬컴포넌트가 뭐닞
private:
	void RightTracer();
	void LeftTracer();
	void SetSideMove(float Speed, float InterSpeed, bool isRight, bool isLeft); //이동지점
	void MoveInLedge(); //이동
	void MoveEvent(); //매달리고 있는지 판별

public:
	FORCEINLINE bool GetIsMoveRight() { return isMoveRight; }
	FORCEINLINE bool GetIsMoveLeft() { return isMoveLeft; }
	FORCEINLINE bool GetIsMovingRight() { return isMovingRight; }
	FORCEINLINE bool GetIsMovingLeft() { return isMovingLeft; }
};