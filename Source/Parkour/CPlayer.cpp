// Fill out your copyright notice in the Description page of Project Settings.


#include "CPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "CAnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ACPlayer::ACPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//ĸ��������Ʈ
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	//ĳ���� �����Ʈ ������Ʈ
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 540, 0); //bOrientRotationToMovement�� ���̴�, �����̴� �������� ȸ���� �� 1�ʸ��� ȸ���� ��..?
	GetCharacterMovement()->JumpZVelocity = 600; //���ʰ��� �ֱ� ��
	GetCharacterMovement()->AirControl = 0.2f; //���Ͻ� ������ �� �ִ� ��(0~1��)

	//��Ʈ�ѷ� ȸ��
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//�޽�
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("SkeletalMesh'/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Tusk.SK_CharM_Tusk'"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}
	GetMesh()->SetRelativeLocation(FVector(0, 0, -97.f)); //�̰� �޽� ���� �����ִ°Ű���
	GetMesh()->SetRelativeRotation(FRotator(0, -90.f, 0));

	//�ִϸ��̼� 
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimAsset(TEXT("AnimBlueprint'/Game/Blueprint/ABP_Player.ABP_Player_C'")); //�ִ��ν��Ͻ��� Ŭ������ ����;ߵ�

	if (AnimAsset.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimAsset.Class);
		//�ִϸ��̼� ����

	}
	//������ ��
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;

	//ī�޶�
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = true;

	//Ʈ���̽� ��ġ
	RightScene = CreateDefaultSubobject<USceneComponent>(TEXT("RightScene"));
	LeftScene = CreateDefaultSubobject<USceneComponent>(TEXT("LeftScene"));
	RightScene->SetupAttachment(GetCapsuleComponent());
	LeftScene->SetupAttachment(GetCapsuleComponent());
	RightScene->SetRelativeLocation(FVector(40, 70, 40));
	LeftScene->SetRelativeLocation(FVector(40, -70, 40));
}

// Called when the game starts or when spawned
void ACPlayer::BeginPlay()
{
	Super::BeginPlay();
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%s"), UEngineTypes::ConvertToCollisionChannel(ETraceTypeQuery::TraceTypeQuery7)));
}

// Called every frame
void ACPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ForwardTracer();
	HeightTracer();
	if (isHanging)
	{
		LeftTracer();
		RightTracer();
		MoveEvent();
	}

}

// Called to bind functionality to input
void ACPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) //�̰� ���� 5�� 17�� 7�� 16��
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//����
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACPlayer::ClimbingJump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	//�̵�
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACPlayer::MoveForward); //���⿡ �Ű������� ������ ������µ� �˾Ƽ� �Ǵ°ǰ�
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACPlayer::MoveRight); //��������Ʈ�� �ּҸ� �������ִ°���
	//ȸ��
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ACPlayer::AddControllerYawInput);
	//&ACPlayer�� �ǰ� &APawn�� �ǳ�..?
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACPlayer::AddControllerPitchInput);
	//�׷� ����
	PlayerInputComponent->BindAction(TEXT("ExitLedge"), IE_Pressed, this, &ACPlayer::ExitLedge);

}

void ACPlayer::MoveForward(float value)
{
	if (IsValid(GetController()) && value != 0.0f && !isHanging)
	{
		const FRotator Rotation = GetController()->GetControlRotation(); 
		const FRotator YawRotation(0, Rotation.Yaw, 0); //z���� �����
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); //�չ����� �𷺼ǿ� �־��ְ�
		AddMovementInput(Direction, value); //�չ������� �̵�
	}
}

void ACPlayer::MoveRight(float value)
{
	RightAxis = value; //-1~1���� �־���
	if (IsValid(GetController()) && value != 0.0f && !isHanging)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
	}
}

void ACPlayer::ExitLedge()
{

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	//�߷��� �ٽ� ����
	UCAnimInstance* Anim = Cast<UCAnimInstance>(GetMesh()->GetAnimInstance());
	if (Anim != nullptr && isHanging)
	{
		Anim->SetHandling(false);
		isHanging = false;
	}
}

void ACPlayer::ClimbingLedge()
{
	if (!isClimbingLedge)
	{
		UCAnimInstance* Anim = Cast<UCAnimInstance>(GetMesh()->GetAnimInstance());
		if (Anim == nullptr) return;
		Anim->ClimbLedge(true);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		isClimbingLedge = true;
		isHanging = false;
	}
}

void ACPlayer::ClimbingJump()
{
	if(isHanging)
	{
		ClimbingLedge();
	}
	else
	{
		Jump();
	}
}

void ACPlayer::ForwardTracer()
{
	FVector start, end, direction;
	start = GetActorLocation();
	direction = GetActorForwardVector();
	end = start + (direction * 150.f);
	TArray<AActor*> IgnoreActors;
	FHitResult hitResult;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Ehllo"));
	/*if (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_GameTraceChannel1))
	{
		WallLocation = hitResult.Location;
		WallNormal = hitResult.Normal; 
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Ehllo"));
	}*/
	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), start, end, 20.f, 
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1),
		false, IgnoreActors, EDrawDebugTrace::ForOneFrame, hitResult, true))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, TEXT("Trace!"));
		
		WallLocation = hitResult.Location;
		WallNormal = hitResult.Normal;
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("%f %f %f"), WallNormal.X, WallNormal.Y, WallNormal.Z));
		
	}//�ٲٱ� ����
	
}

void ACPlayer::HeightTracer()
{

	FVector start, end, direction,pos;
	pos = GetActorLocation();
	direction = GetActorForwardVector() * 70.; //������ 70 �̵�
	start = FVector(pos.X, pos.Y, pos.Z + 500.f) + direction;  //���� 500 �̵�
	end = FVector(start.X, start.Y, start.Z - 500.f); //��ǥ������ �̷��� �ϸ� ������ �Ʒ��� ��� �ȴ�.
	TArray<AActor*> IgnoreActors;
	FHitResult hitResult;
	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), start, end, 20.f, 
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1),
		false, IgnoreActors, EDrawDebugTrace::ForOneFrame, hitResult, true))
	{
		HeightLocation = hitResult.Location;
		if (HipToLedge() && !isClimbingLedge)
		{
			GrabLedge();
		}
	}
}

void ACPlayer::GrabLedge() //�Ŵ޸��� �Լ�
{
	UCAnimInstance* Anim = Cast<UCAnimInstance>(GetMesh()->GetAnimInstance());
	if (Anim != nullptr)
	{
		Anim->SetHandling(true); //�Ŵ޷���
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);//�߷��� ������
		isHanging = true; //�Ŵ޷���
		FRotator XVector = UKismetMathLibrary::MakeRotFromX(WallNormal); //���� �չ����� ����
		FRotator TargetRotation = FRotator(XVector.Pitch, XVector.Yaw - 180, XVector.Roll);
		//���� �չ����� 180�� ȸ���� ������ ����. y���� ȸ����Ű�ϱ� ��������̵� �ݴ������ ���ϴ°���.

		GetCapsuleComponent()->SetRelativeLocation(MoveToLocation()); //Relative�� World��  ������ ����
		//�Ŵ޸� ��ġ
		GetCapsuleComponent()->SetRelativeRotation(TargetRotation);
		//���� ��ֺ����� �ݴ����
		GetCharacterMovement()->StopMovementImmediately(); //�̵�����

	}
}

bool ACPlayer::HipToLedge()
{
	FVector SockerPos = GetMesh()->GetSocketLocation(FName(TEXT("pelvis"))); //���� �̸����� ��ġ ������°�
	float Distance = SockerPos.Z - HeightLocation.Z; //���� ���̶� ���̸� ����

	return UKismetMathLibrary::InRange_FloatFloat(Distance, -50.f, 0); //-50 ���� ũ�� 0���� ������ 

	//�״ϱ� ���� �� ���κ��̶� �㸮�κ��� ���̰� 50����, 0���� ũ�ٴ°�
	// ĳ������ �㸮�� ������ �� �Ʒ��� �ִ���, �����κ��̶� ������ġ or �� ���� ���� �ʴ��� üũ
	//InRange_FloatFloat
	//(Value >= Min && Value <= Max) 
}

FVector ACPlayer::MoveToLocation()
{
	FVector normal = WallNormal * FVector(30.f, 30.f, 0.f);
	FVector pos = FVector(normal.X + WallLocation.X, normal.Y + WallLocation.Y, HeightLocation.Z - 110.f);
	
	return pos;
	//x, y�����δ� �����̿� ��ֺ��ͷκ��� 22��ŭ, z�����δ� -120��ŭ ������ ��ġ
}

void ACPlayer::ClimbLedge(bool isClimb)
{
	isClimbingLedge = isClimb;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void ACPlayer::RightTracer()
{
	FVector start, end;
	start = RightScene->GetComponentLocation();
	end = RightScene->GetComponentLocation();
	TArray<AActor*> IgnoreActors;
	FHitResult hitResult;
	if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), start, end, 20.f, 60.f, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1), false,
		IgnoreActors, EDrawDebugTrace::ForOneFrame, hitResult, true))
	{
		isMoveRight = true;
	}
	else { isMoveRight = false;}
	

}

void ACPlayer::LeftTracer()
{
	FVector start, end;
	start = LeftScene->GetComponentLocation();
	end = LeftScene->GetComponentLocation();
	TArray<AActor*> IgnoreActors;
	FHitResult hitResult;
	if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), start, end, 20.f, 60.f, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1), false,
		IgnoreActors, EDrawDebugTrace::ForOneFrame, hitResult, true))
	{
		isMoveLeft = true;
	}
	else { isMoveLeft = false; }
}

void ACPlayer::SetSideMove(float Speed, float InterSpeed, bool isRight, bool isLeft)
{
	FVector CurrentPos = GetActorLocation();
	FVector Target = (GetActorRightVector() * Speed) + CurrentPos;
	FVector Pos = FMath::VInterpTo(CurrentPos, Target, GetWorld()->GetDeltaSeconds(), InterSpeed);
	//VInterpTO : ������ ��������
	//��������: A�� B ������ �߰����� ã�´�.
	//ex) 3�� 13�� 0.5(50%0 ���� 8�̴�.
	SetActorLocation(Pos);
	isMovingRight = isRight;
	isMovingLeft = isLeft;
}

void ACPlayer::MoveInLedge()
{
	if (isMoveRight && RightAxis > 0.f)
	{
		SetSideMove(20.f, 5.f, true, false);
	}
	if (isMoveLeft && RightAxis < 0.f)
	{
		SetSideMove(-20.f, 5.f, false, true);
	}
	if (RightAxis == 0.f)
	{
		isMovingRight = false;
		isMovingLeft = false;
	}
}

void ACPlayer::MoveEvent()
{
	if (isHanging)//�Ŵ޸��� ������
	{
		MoveInLedge(); //�������� ����
	}
	else
	{
		isMovingRight = false;
		isMovingLeft = false;
	}
}

