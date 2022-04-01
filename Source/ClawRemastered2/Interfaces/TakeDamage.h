// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TakeDamage.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTakeDamage : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CLAWREMASTERED2_API ITakeDamage
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void OnDamageTaken(float DamageAmount, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser) = 0;
};
