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
	//캡슐컴포넌트
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	//캐릭터 무브먼트 컴포넌트
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 540, 0); //bOrientRotationToMovement와 엮이는, 움직이는 방향으로 회전할 때 1초마다 회전할 양..?
	GetCharacterMovement()->JumpZVelocity = 600; //기초값이 있긴 함
	GetCharacterMovement()->AirControl = 0.2f; //낙하시 제어할 수 있는 양(0~1값)

	//컨트롤러 회전
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//메쉬
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("SkeletalMesh'/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Tusk.SK_CharM_Tusk'"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}
	GetMesh()->SetRelativeLocation(FVector(0, 0, -97.f)); //이건 메쉬 높이 맞춰주는거겠지
	GetMesh()->SetRelativeRotation(FRotator(0, -90.f, 0));

	//애니메이션 
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimAsset(TEXT("AnimBlueprint'/Game/Blueprint/ABP_Player.ABP_Player_C'")); //애님인스턴스는 클래스로 갖고와야됨

	if (AnimAsset.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimAsset.Class);
		//애니메이션 적용

	}
	//스프링 암
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;

	//카메라
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = true;

	//트레이스 위치
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
void ACPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) //이거 설명 5월 17일 7시 16분
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//점프
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACPlayer::ClimbingJump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	//이동
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACPlayer::MoveForward); //여기에 매개변수를 지정을 안해줬는데 알아서 되는건가
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACPlayer::MoveRight); //델리게이트로 주소만 전달해주는거임
	//회전
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ACPlayer::AddControllerYawInput);
	//&ACPlayer도 되고 &APawn도 되네..?
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACPlayer::AddControllerPitchInput);
	//그랩 중지
	PlayerInputComponent->BindAction(TEXT("ExitLedge"), IE_Pressed, this, &ACPlayer::ExitLedge);

}

void ACPlayer::MoveForward(float value)
{
	if (IsValid(GetController()) && value != 0.0f && !isHanging)
	{
		const FRotator Rotation = GetController()->GetControlRotation(); 
		const FRotator YawRotation(0, Rotation.Yaw, 0); //z방향 갖고옴
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); //앞방향을 디렉션에 넣어주고
		AddMovementInput(Direction, value); //앞방향으로 이동
	}
}

void ACPlayer::MoveRight(float value)
{
	RightAxis = value; //-1~1값을 넣어줌
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
	//중력을 다시 받음
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
		
	}//바꾸기 전꺼
	
}

void ACPlayer::HeightTracer()
{

	FVector start, end, direction,pos;
	pos = GetActorLocation();
	direction = GetActorForwardVector() * 70.; //앞으로 70 이동
	start = FVector(pos.X, pos.Y, pos.Z + 500.f) + direction;  //위로 500 이동
	end = FVector(start.X, start.Y, start.Z - 500.f); //목표지점을 이렇게 하면 위에서 아래로 쏘게 된다.
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

void ACPlayer::GrabLedge() //매달리는 함수
{
	UCAnimInstance* Anim = Cast<UCAnimInstance>(GetMesh()->GetAnimInstance());
	if (Anim != nullptr)
	{
		Anim->SetHandling(true); //매달렸음
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);//중력을 없애줌
		isHanging = true; //매달렸음
		FRotator XVector = UKismetMathLibrary::MakeRotFromX(WallNormal); //벽의 앞방향을 구함
		FRotator TargetRotation = FRotator(XVector.Pitch, XVector.Yaw - 180, XVector.Roll);
		//벽의 앞방향을 180도 회전한 방향을 구함. y축을 회전시키니깐 어느방향이든 반대방향을 구하는거임.

		GetCapsuleComponent()->SetRelativeLocation(MoveToLocation()); //Relative랑 World랑  차이점 뭐지
		//매달릴 위치
		GetCapsuleComponent()->SetRelativeRotation(TargetRotation);
		//벽의 노멀벡터의 반대방향
		GetCharacterMovement()->StopMovementImmediately(); //이동중지

	}
}

bool ACPlayer::HipToLedge()
{
	FVector SockerPos = GetMesh()->GetSocketLocation(FName(TEXT("pelvis"))); //뼈대 이름으로 위치 갖고오는거
	float Distance = SockerPos.Z - HeightLocation.Z; //뼈대 높이랑 높이를 뺀다

	return UKismetMathLibrary::InRange_FloatFloat(Distance, -50.f, 0); //-50 보다 크고 0보다 작은지 

	//그니까 벽의 맨 윗부분이랑 허리부분의 차이가 50이하, 0보다 크다는건
	// 캐릭터의 허리가 적당히 벽 아래에 있는지, 벽윗부분이랑 같은위치 or 더 위에 있지 않는지 체크
	//InRange_FloatFloat
	//(Value >= Min && Value <= Max) 
}

FVector ACPlayer::MoveToLocation()
{
	FVector normal = WallNormal * FVector(30.f, 30.f, 0.f);
	FVector pos = FVector(normal.X + WallLocation.X, normal.Y + WallLocation.Y, HeightLocation.Z - 110.f);
	
	return pos;
	//x, y축으로는 벽높이와 노멀벡터로부터 22만큼, z축으로는 -120만큼 떨어진 위치
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
	//VInterpTO : 벡터의 선형보간
	//선형보간: A와 B 사이의 중간값을 찾는다.
	//ex) 3과 13의 0.5(50%0 값은 8이다.
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
	if (isHanging)//매달리고 있으면
	{
		MoveInLedge(); //러지무브 실행
	}
	else
	{
		isMovingRight = false;
		isMovingLeft = false;
	}
}

