// Fill out your copyright notice in the Description page of Project Settings.


#include "WallComponent.h"
#include "UObject/UObjectGlobals.h"

// Sets default values for this component's properties
UWallComponent::UWallComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	

	

	// BoundingBox->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}


// Called when the game starts
void UWallComponent::BeginPlay()
{
	Super::BeginPlay();

	BoundingBox = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass(), NAME_None);
	BoundingBox->SetWorldScale3D(FVector(1.0));
	BoundingBox->SetHiddenInGame(false);
	BoundingBox->SetBoxExtent(FVector(50.0f));

	static FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
	BoundingBox->AttachToComponent(this, rules);
	
	BoundingBox->RegisterComponent();
	BoundingBox->SetRelativeLocation(FVector(0.0));

	BoundingBox->SetCollisionProfileName(FName("BlockAll"));
}


// Called every frame
void UWallComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

