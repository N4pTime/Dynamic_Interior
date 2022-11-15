// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Room.generated.h"

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
	NORTH	UMETA(DisplayName = "North"), // X+
	SOUTH	UMETA(DisplayName = "South"), // X-
	WEST	UMETA(DisplayName = "West"),  // Y-
	EAST	UMETA(DisplayName = "East")   // Y+
};

UENUM(BlueprintType)
enum class SegmentDirection : uint8
{
	TOP		UMETA(DisplayName = "TOP"),
	BOTTOM	UMETA(DisplayName = "BOTTOM"),
	LEFT	UMETA(DisplayName = "LEFT"),
	RIGHT	UMETA(DisplayName = "RIGHT")
};

class Wall
{
public:

	FString Name;

	WallType Type = WallType::EMPTY;
	WallDirection Direction;

	int leftAligment = 20;
	int rightAligment = 20;

	// For storing door or window mesh if exist
	UStaticMeshComponent* Object = nullptr;
	TMap<SegmentDirection, UStaticMeshComponent*> WallSegments;
};

UCLASS()
class DYNAMIC_INTERIOR_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();

protected:
	
	// Room dimensions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator properties", meta = (ClampMin = "200.0", ClampMax = "500.0"))
	float Length = 200.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator properties", meta = (ClampMin = "200.0", ClampMax = "500.0"))
	float Width  = 200.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator properties", meta = (ClampMin = "270.0", ClampMax = "350.0"))
	float Height = 270.0;

	// Static meshes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes")
	UStaticMesh* FloorMesh	 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes")
	UStaticMesh* WallMesh	 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes")
	UStaticMesh* CeilingMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes")
	UStaticMesh* DoorMesh	 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes")
	UStaticMesh* WindowMesh	 = nullptr;

	// Offsets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets", DisplayName = "Wall Offset")
	float WallOffset	= 20.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets", DisplayName = "Door Offset")
	float DoorOffset	= 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets", DisplayName = "Window Offset")
	float WindowOffset	= 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets", DisplayName = "Aligment Offset")
	float AligmentOffset = 20.0;

	UStaticMeshComponent* floor   = nullptr;
	UStaticMeshComponent* ceiling = nullptr;
	TMap<WallDirection, Wall> Walls;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void UpdateSegments(WallDirection direction);

	UFUNCTION(BlueprintCallable)
	void UpdateAllSegments();

	UFUNCTION(BlueprintCallable)
	void SetWallType(WallDirection direction, WallType type);

	UFUNCTION(BlueprintCallable)
	void SetLeftAligment(WallDirection direction, int value);

	UFUNCTION(BlueprintCallable)
	void SetRightAligment(WallDirection direction, int value);

	UStaticMeshComponent* AddStaticMeshComponent(UStaticMesh* Mesh, FName Name);
	FVector GetStaticMeshDimensions(UStaticMesh* Mesh);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
