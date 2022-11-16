// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WallComponent.generated.h"

UENUM(BlueprintType)
enum class WallType : uint8
{
	EMPTY		UMETA(DisplayName = "Empty"),
	WITH_DOOR	UMETA(DisplayName = "With Door"),
	WITH_WINDOW	UMETA(DisplayName = "With Window")
};

UENUM(BlueprintType)
enum class WallDirection : uint8
{
	NORTH = 0	UMETA(DisplayName = "North"), // X+
	SOUTH = 1	UMETA(DisplayName = "South"), // X-
	WEST  = 2	UMETA(DisplayName = "West"),  // Y-
	EAST  = 3	UMETA(DisplayName = "East")   // Y+
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
	WallType Type = WallType::EMPTY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	WallDirection Direction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int leftAligment = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int rightAligment = 20;

	UPROPERTY()
	// For storing door or window mesh if exist
	UStaticMeshComponent* Object = nullptr;
	TMap<SegmentDirection, UStaticMeshComponent*> WallSegments;

	// Sets default values for this component's properties
	UWallComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};