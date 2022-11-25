// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ObjectComponent.h"

#include "WallComponent.generated.h"

UENUM(BlueprintType)
enum class WallDirection : uint8
{
	NORTH = 0	UMETA(DisplayName = "North"), // X+
	SOUTH = 1	UMETA(DisplayName = "South"), // X-
	WEST  = 2	UMETA(DisplayName = "West"),  // Y-
	EAST  = 3	UMETA(DisplayName = "East"),   // Y+
	NORTH_EAST = 4	UMETA(DisplayName = "Northeast"), 
	SOUTH_EAST = 5	UMETA(DisplayName = "Southeast"),
	NORTH_WEST = 6	UMETA(DisplayName = "Northwest"),
	SOUTH_WEST = 7	UMETA(DisplayName = "Southwest")
};

UENUM(BlueprintType)
enum class SegmentDirection : uint8
{
	TOP		UMETA(DisplayName = "TOP"),
	BOTTOM	UMETA(DisplayName = "BOTTOM"),
	LEFT	UMETA(DisplayName = "LEFT"),
	RIGHT	UMETA(DisplayName = "RIGHT")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DYNAMIC_INTERIOR_API UWallComponent : public USceneComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	WallDirection Direction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int leftAligment = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int rightAligment = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Length;

	UPROPERTY()
	// For storing door or window mesh if exist
	TArray<UObjectComponent*> Objects;

	// Segments between objects
	TArray<TWeakObjectPtr<UStaticMeshComponent>> HorizontalSegments;

	// Segments below and above objects
	TArray<TWeakObjectPtr<UStaticMeshComponent>> VerticalSegments;

	// Sets default values for this component's properties
	UWallComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
