// MovingActor.cpp


#include "MovingActor.h"

AMovingActor::AMovingActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComp->SetupAttachment(SceneRoot);
	
	MoveSpeed = 200.0f;
	MaxRange = 500.0f;
	MoveDirection = 1;
	
}

void AMovingActor::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();
}

void AMovingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float FrameMovement = MoveSpeed * MoveDirection * DeltaTime;
	
	FVector CurrentLocation = GetActorLocation();
	FVector MovementVector = FVector(FrameMovement, 0.0f, 0.0f);
	FVector NextLocation = CurrentLocation + MovementVector;
	SetActorLocation(NextLocation);
	
	float DistanceMoved = FVector::Dist(StartLocation, NextLocation);
	
	if (DistanceMoved >= MaxRange)
	{
		MoveDirection *= -1;
	}
}