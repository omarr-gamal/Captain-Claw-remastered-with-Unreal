// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/BoxComponent.h"
#include "ClawRemastered2Character.h"
#include "BlueOfficerBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AEnemy::AEnemy()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;
	GetCharacterMovement()->MaxFlySpeed = 100.0f;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;

	//UE_LOG(LogTemp, Warning, TEXT("swording"));

	OfficerHealth = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AEnemy::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("beginplay"));

	SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
	currentState = walking;
	walkDirection = 1.0f;
}

void AEnemy::UpdateCharacter()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* CurrentAnimation = GetSprite()->GetFlipbook();

	if (currentState == walking) {
		AddMovementInput(FVector(walkDirection, 0.0f, 0.0f), 1);
		if (CurrentAnimation != WalkingAnimation)
		{
			GetSprite()->SetFlipbook(WalkingAnimation);

			GetWorldTimerManager().SetTimer(EndWalkTimer, this, &AEnemy::TurnLeft, 1.5f, false);
			UE_LOG(LogTemp, Warning, TEXT("right"));
		}
	}
	//else if (currentState == idling) {
	//	DesiredAnimation = IdleAnimation;
	//}
	//else if (currentState == dead) {
	//	DesiredAnimation = DeadAnimation;
	//}
	//else if (currentState == hurt) {
	//	DesiredAnimation = HurtAnimation;
	//}
}


void AEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCharacter();
}

void AEnemy::TurnRight()
{
	GetWorldTimerManager().SetTimer(EndWalkTimer, this, &AEnemy::TurnLeft, 1.5f, false);
	walkDirection *= -1;
	SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
	UE_LOG(LogTemp, Error, TEXT("turn right"));
}

void AEnemy::TurnLeft()
{
	GetWorldTimerManager().SetTimer(EndWalkTimer, this, &AEnemy::TurnRight, 1.5f, false);
	walkDirection *= -1;
	SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
	UE_LOG(LogTemp, Error, TEXT("turn left"));
}

void AEnemy::HandleDeath()
{
	currentState = dead;

	UGameplayStatics::SpawnSound2D(this, DeathSound, 1.0f, 1.0f, 0.0f);

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemy::DestroySelf, 0.6f, false);

	this->SetActorEnableCollision(false);
}

void AEnemy::DestroySelf()
{
	this->Destroy();
}


