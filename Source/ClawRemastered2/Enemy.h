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
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay();

protected:
	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* IdleAnimation;

	// The animation to play while walking (patrolling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* WalkingAnimation;

	// The animation to play when getting hurt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* HurtAnimation;

	// The animation to play after death
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* DeadAnimation;

	void UpdateCharacter();

	void TurnRight();
	void TurnLeft(); 

	enum State
	{
		walking,
		idling,
		dead,
		hurt
	};

	State currentState;

	float walkDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* FightSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* DeathSound;

private:
	int patrols = 0;

	// movementDirection will be multiplied by world vector
	// 0 will result in no movement, 1 is right
	// movement and -1 is left movement.
	float movementDirection = 1.0f;

	FTimerHandle EndWalkTimer;

public:
	AEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APaperSpriteActor> BulletClass;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	TSubclassOf<UDamageType> DamageType;

	UHealthComponent* OfficerHealth;

private:
	void HandleDeath();

	void DestroySelf();
};
