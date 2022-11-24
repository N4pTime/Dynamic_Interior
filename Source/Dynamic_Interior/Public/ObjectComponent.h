// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "ObjectComponent.generated.h"

/**
 * 
 */
UCLASS()
class DYNAMIC_INTERIOR_API UObjectComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	float aligment = 0.0;
};
