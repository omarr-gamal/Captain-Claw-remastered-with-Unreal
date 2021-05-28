// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClawRemastered2Character.h"
#include "PaperFlipbookComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnemyCharacter.h"
#include "ClawBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"

DEFINE_LOG_CATEGORY_STATIC(SideScrollerCharacter, Log, All);

//////////////////////////////////////////////////////////////////////////
// AClawRemastered2Character

AClawRemastered2Character::AClawRemastered2Character()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Set the size of our collision capsule.
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 75.0f);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	

	// Create an orthographic camera (no perspective) and attach it to the boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	SideViewCameraComponent->OrthoWidth = 2048.0f;
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Prevent all automatic rotation behavior on the camera, character, and camera component
	CameraBoom->SetUsingAbsoluteRotation(true);
	SideViewCameraComponent->bUsePawnControlRotation = false;
	SideViewCameraComponent->bAutoActivate = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxFlySpeed = 600.0f;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

    // 	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
    // 	TextComponent->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
    // 	TextComponent->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
    // 	TextComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    // 	TextComponent->SetupAttachment(RootComponent);

	ClawHealth = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// initializes the enemy's box collision 
	attackCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	attackCollisionBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	attackCollisionBox->SetCollisionProfileName("Trigger");
	attackCollisionBox->SetupAttachment(RootComponent); 

	BulletSpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Bullet Spawn Point"));
	BulletSpawnLocation->SetupAttachment(RootComponent);

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;
}


//////////////////////////////////////////////////////////////////////////
// Animation

void AClawRemastered2Character::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* DesiredAnimation;

	if (isDead) {
		DesiredAnimation = DeadAnimation;
	}
	else if (isHurt) {
		DesiredAnimation = HurtAnimation;
	}
	else if (GetCharacterMovement()->IsFalling()) {
		// if falling then render falling animation.
		// UE_LOG(LogTemp, Warning, TEXT("Text"));
		DesiredAnimation = JumpingAnimation;
	}
	else if (isSwording) {
		// if swording then render the sword animation.
		DesiredAnimation = SwordingAnimation;
	}
	else {
		// else render running or idle animation
		DesiredAnimation = (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
	}

	// Are we moving or standing still?
	if( GetSprite()->GetFlipbook() != DesiredAnimation )
	{
		GetSprite()->SetFlipbook(DesiredAnimation);
	}
}

void AClawRemastered2Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!isDead && ClawHealth->GetHealth() <= 0) {
		HandleDeath();
	}

	UpdateCharacter();
}


//////////////////////////////////////////////////////////////////////////
// Input

void AClawRemastered2Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Note: the 'Jump' action and the 'MoveRight' axis are bound to actual keys/buttons/sticks in DefaultInput.ini (editable from Project Settings..Input)
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &AClawRemastered2Character::MoveRight);
	PlayerInputComponent->BindAction("Sword", IE_Pressed, this, &AClawRemastered2Character::StartSwording);
	PlayerInputComponent->BindAction("Pistol", IE_Pressed, this, &AClawRemastered2Character::StartPistoling);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AClawRemastered2Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AClawRemastered2Character::TouchStopped);
}

void AClawRemastered2Character::MoveRight(float Value)
{
	/*UpdateChar();*/

	// Apply the input to the character motion
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
}

// starts the timer for the swording animation
void AClawRemastered2Character::StartSwording()
{
	if (isSwording == false && GetCharacterMovement()->IsFalling() == false)
	{
		isSwording = true;

		if (GetActorRotation().Yaw >= 0)
		{
			//SetActorLocation(GetActorLocation() + FVector(10.0f, 0.0f, 0.0f));
			GetSprite()->SetWorldLocation(GetSprite()->GetComponentLocation() + FVector(43.0f, 0.0f, 0.0f));
		}
		else 
		{
			GetSprite()->SetWorldLocation(GetSprite()->GetComponentLocation() + FVector(-43.0f, 0.0f, 0.0f));
		}
		
		StartDamaging();

		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &AClawRemastered2Character::StopSwording, 0.6f, false);

		//GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}
}

// called when the timer for the swording animation ends
void AClawRemastered2Character::StopSwording()
{
	isSwording = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (GetActorRotation().Yaw >= 0)
	{
		//SetActorLocation(GetActorLocation() + FVector(10.0f, 0.0f, 0.0f));
		GetSprite()->SetWorldLocation(GetSprite()->GetComponentLocation() + FVector(-43.0f, 0.0f, 0.0f));
	}
	else
	{
		GetSprite()->SetWorldLocation(GetSprite()->GetComponentLocation() + FVector(43.0f, 0.0f, 0.0f));
	}
}

// somehow this method gets called twice for a single hit.
void AClawRemastered2Character::StartDamaging()
{
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AClawRemastered2Character::StopDamaging, 0.3f, false);
}

void AClawRemastered2Character::StopDamaging()
{
	TSet<AActor*> OverlappingActors;

	attackCollisionBox->GetOverlappingActors(OverlappingActors);
	for (auto& Actor : OverlappingActors)
	{
		if (Actor->IsA(AEnemyCharacter::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("1"));

			UGameplayStatics::ApplyDamage(Actor, 300, GetOwner()->GetInstigatorController(), this, DamageType);

			break;
		}
	}
}

void AClawRemastered2Character::StartPistoling()
{
	//UE_LOG(LogTemp, Warning, TEXT("pistoling"));
	if (BulletClass)
	{
		FRotator SpawnRotation = GetActorRotation();
		FVector SpawnLocation;

		if (GetActorRotation().Yaw >= 0) SpawnLocation = GetActorLocation() + FVector(70.0, 0.0f, 25.0f); 
		else SpawnLocation = GetActorLocation() + FVector(-70.0, 0.0f, 25.0f);

		GetWorld()->SpawnActor<AClawBullet>(BulletClass, SpawnLocation, SpawnRotation);
	}
}

void AClawRemastered2Character::StopPistoling()
{
}


// start hurt and stop hurt are implemented but not used yet. I realised it would 
// require either a major refactor of the entire project or very nasty code to use them.
void AClawRemastered2Character::StartHurt()
{
	isHurt = true;

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AClawRemastered2Character::StopHurt, 0.1f, false);
}

void AClawRemastered2Character::StopHurt()
{
	isHurt = false;
}

void AClawRemastered2Character::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Jump on any touch
	Jump();
}

void AClawRemastered2Character::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Cease jumping once touch stopped
	StopJumping();
}

void AClawRemastered2Character::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();

	// Now setup the rotation of the controller based on the direction we are travelling
	const FVector PlayerVelocity = GetVelocity();	
	float TravelDirection = PlayerVelocity.X;
	// Set the rotation so that the character faces his direction of travel.
	if (Controller != nullptr)
	{
		if (TravelDirection < 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
		}
		else if (TravelDirection > 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
	}
}

void AClawRemastered2Character::HandleDeath()
{
	isDead = true;

	GetCharacterMovement()->DisableMovement(); 
	FVector ClawLocation = GetActorLocation();

	//TODO: Disable Claw movement and make him fall 
	//SetActorLocation(ClawLocation + FVector(0.0f, 1.0f, 0.0f));
}