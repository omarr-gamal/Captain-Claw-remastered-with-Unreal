// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "HealthComponent.h"
#include "Sound/SoundBase.h"
#include "Enemy.generated.h"


class APaperSpriteActor;

/**
 * 
 */
UCLASS()
class CLAWREMASTERED2_API AEnemy : public APaperCharacter
{
	GENERATED_BODY()

protected:
	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* IdleAnimation;

	// The animation to play while walking (patrolling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* WalkingAnimation;

	// The animation to play while aggroed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* AggroedAnimation;

	// The animation to play when getting hurt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* HurtAnimation;

	// The animation to play after death
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* DeadAnimation;

	enum State
	{
		walking,
		idling,
		aggroed,
		dead,
		hurt
	};

	State currentState;

	float walkDirection;
	float toClawCharacterDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	float walkDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	float idlingDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* FightSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* DeathSound;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	TSubclassOf<UDamageType> DamageType;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* OfficerIdleSightCollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* OfficerWalkSightCollisionBox;

private:
	int patrols = 0;

	// movementDirection will be multiplied by world vector
	// 0 will result in no movement, 1 is right
	// movement and -1 is left movement.
	float movementDirection = 1.0f;

	FTimerHandle EndWalkTimer;

	UHealthComponent* OfficerHealth;

public:
	AEnemy();

private:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay();

	UFUNCTION()
	void OnOverlapBeginIdleSightCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEndIdleSightCollisionBox(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnOverlapBeginWalkSightCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEndWalkSightCollisionBox(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void HandleDeath();

	void DestroySelf();

protected:
	virtual void ActAggroed();

	void UpdateCharacter();

	void TurnRight();
	void TurnLeft();

	UPrimitiveComponent* CheckIfClawInSight();

	void UpdateRotation();
	void UpdateToClawCharacterDirection(UPrimitiveComponent* clawCapsule);

	void SetRotationToRight();
	void SetRotationToLeft();
};
