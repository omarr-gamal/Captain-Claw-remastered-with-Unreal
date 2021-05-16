// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "PaperFlipbookComponent.h" 
#include "Components/CapsuleComponent.h"  
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h" 
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"


AEnemyCharacter::AEnemyCharacter() 
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

	attackCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	attackCollisionBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	attackCollisionBox->SetCollisionProfileName("Trigger");
	attackCollisionBox->SetupAttachment(RootComponent);

	attackCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnOverlapBegin);
	attackCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnOverlapEnd);
}


void AEnemyCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("begin overlap"));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "overlap Begin");
	if (ClawCharacter != nullptr && attackCollisionBox->IsOverlappingActor(ClawCharacter))
	{
		//UE_LOG(LogTemp, Warning, TEXT("overlapping"));
		StartSwording();
	}
}

void AEnemyCharacter::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//UE_LOG(LogTemp, Warning, TEXT("end overlap"));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, "overlap End");
}

//////////////////////////////////////////////////////////////////////////
// Animation

void AEnemyCharacter::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* DesiredAnimation;

	if (isSwording) {
		// if swording then render the sword animation.
		DesiredAnimation = SwordingAnimation;
	}
	else {
		// else render running or idle animation
		DesiredAnimation = (PlayerSpeedSqr > 0.0f) ? WalkingAnimation : IdleAnimation;
	}

	// Are we moving or standing still?
	if (GetSprite()->GetFlipbook() != DesiredAnimation)
	{
		GetSprite()->SetFlipbook(DesiredAnimation);
	}
}

void AEnemyCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("beginplay"));

	SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
	StartMovementTimer();
}

void AEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (isSwording) {
		//UE_LOG(LogTemp, Warning, TEXT("swording"));
	}
	else {
		//UE_LOG(LogTemp, Warning, TEXT("not swording"));
	}
	//UE_LOG(LogTemp, Warning, TEXT("%f"), DeltaSeconds);

	AddMovementInput(FVector(movementDirection, 0.0f, 0.0f), 1);

	//if (ClawCharacter != nullptr && attackCollisionBox->IsOverlappingActor(ClawCharacter))
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("overlapping"));
	//	isSwording = true;
	//}
	//else {
	//	isSwording = false;
	//}
	
	UpdateCharacter();
}


// starts the timer for the swording animation
void AEnemyCharacter::StartSwording()
{
	//UE_LOG(LogTemp, Warning, TEXT("swording"));
	isSwording = true;

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::StopSwording, 0.8f, false);

	GetCharacterMovement()->DisableMovement();
	//GetCharacterMovement()->StopMovementImmediately();
}

// called when the timer for the swording animation ends
void AEnemyCharacter::StopSwording()
{
	if (ClawCharacter != nullptr && attackCollisionBox->IsOverlappingActor(ClawCharacter))
	{
		//UE_LOG(LogTemp, Warning, TEXT("overlapping"));
		StartSwording();
	} else {
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		isSwording = false;
	}
	
}


void AEnemyCharacter::StartMovementTimer()
{
	//UE_LOG(LogTemp, Error, TEXT("started timer"));

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::ChangeMovementDirection, 2.2f, false);
}

void AEnemyCharacter::ChangeMovementDirection()
{
	if (ClawCharacter != nullptr && attackCollisionBox->IsOverlappingActor(ClawCharacter))
	{
		StartMovementTimer();
		return;
	}

	if (movementDirection == 1.0f)
		movementDirection = -1.0f;
	else if (movementDirection == -1.0f)
		movementDirection = 1.0f;
	
	//UE_LOG(LogTemp, Error, TEXT("ended timer"));


	if (movementDirection < 0.0f)
	{
		SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
		// compensate for sword space in sprite
		SetActorLocation(GetActorLocation() + FVector(-130.0f, 0.0f, 0.0f)); 
	}
	else if (movementDirection > 0.0f)
	{
		SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
		// compensate for sword space in sprite
		SetActorLocation(GetActorLocation() + FVector(130.0f, 0.0f, 0.0f));
	}
	

	//SetActorLocation(GetActorLocation() + FVector(-1.0f * movementDirection * 80.0f, 0.0f, 0.0f)); 

	StartIdlenessTimer();
}

void AEnemyCharacter::StartIdlenessTimer()
{
	//UE_LOG(LogTemp, Error, TEXT("started idleness timer"));

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::StartMovementTimer, 0.5f, false);
	//StartMovementTimer();
}

void AEnemyCharacter::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();

	// Now setup the rotation of the controller based on the direction we are travelling
	//const FVector PlayerVelocity = GetVelocity();
	//float TravelDirection = PlayerVelocity.X;
	//// Set the rotation so that the character faces his direction of travel.
	//if (Controller != nullptr)
	//{
	//	if (TravelDirection < 0.0f)
	//	{
	//		SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
	//		//SetActorLocation(GetActorLocation() + FVector(80.0f, 0.0f, 0.0f));
	//		//Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
	//	}
	//	else if (TravelDirection > 0.0f)
	//	{
	//		SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
	//		//SetActorLocation(GetActorLocation() + FVector(-80.0f, 0.0f, 0.0f));
	//		//Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
	//	}
	//}
}
