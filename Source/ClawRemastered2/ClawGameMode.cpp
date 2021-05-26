 // Fill out your copyright notice in the Description page of Project Settings.


#include "ClawGameMode.h"


void AClawGameMode::BeginPlay()
{
	Super::BeginPlay();

}


void AClawGameMode::ActorDied(AActor* DeadActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Actor Died"));
}


void AClawGameMode::HandleGameOver(bool PlayerWon)
{
	// to be implemented in the future
}


void AClawGameMode::HandleGameStart()
{
	// to be implemented in the future
}