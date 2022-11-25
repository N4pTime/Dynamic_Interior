// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "ObjectComponent.generated.h"

UENUM(BlueprintType)
enum class ObjectType : uint8
{
	DOOR	UMETA(DisplayName = "With Door"),
	WINDOW	UMETA(DisplayName = "With Window")
};

/**
 * 
 */
UCLASS()
class DYNAMIC_INTERIOR_API UObjectComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ObjectType type;

	float offset = 0.0;

	UFUNCTION(BlueprintCallable)
	// Return vector that represents static mesh dimensions
	FVector GetDimensions();
};
