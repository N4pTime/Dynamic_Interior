// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectComponent.h"

FVector UObjectComponent::GetDimensions()
{
	if (!IsValid(this->GetStaticMesh()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid static mesh."));
		return FVector(0.0);
	}

	auto Box = this->GetStaticMesh()->GetBoundingBox();

	return Box.Max - Box.Min;
}

