// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WallComponent.h"

#include "Room.generated.h"

UENUM(BlueprintType)
enum class RoomType : uint8
{
	STANDARD	UMETA(DisplayName = "Empty"),
	L_SHAPE		UMETA(DisplayName = "With Door")
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator properties", meta = (ClampMin = "440.0", ClampMax = "1000.0"))
	float maxLength = 1000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator properties", meta = (ClampMin = "440.0", ClampMax = "1000.0"))
	float maxWidth  = 1000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator properties", meta = (ClampMin = "270.0", ClampMax = "350.0"))
	float maxHeight = 350.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator properties", meta = (ClampMin = "440.0", ClampMax = "1000.0"))
	float Length = 440.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator properties", meta = (ClampMin = "440.0", ClampMax = "1000.0"))
	float Width  = 440.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator properties", meta = (ClampMin = "270.0", ClampMax = "350.0"))
	float Height = 290.0;

	// Static meshes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes")
	UStaticMesh* FloorMesh	 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes")
	UStaticMesh* WallMesh	 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes")
	UStaticMesh* CeilingMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes|Doors")
	TArray<UStaticMesh*> DoorMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Meshes|Windows")
	TArray<UStaticMesh*> WindowMeshes;

	// Offsets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets", DisplayName = "Wall Offset")
	float WallOffset	= 20.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets", DisplayName = "Door Offset")
	float DoorOffset	= 4.5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets", DisplayName = "Window Offset")
	float WindowOffset	= 18.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets", DisplayName = "Window Height Offset")
	float WindowHeightOffset = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets", DisplayName = "Aligment Offset")
	float AligmentOffset = 20.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|Offsets")
	float minimalWallLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|L Shape")
	float cornerX = 200.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurator properties|L Shape")
	float cornerY = 200.0;

	RoomType Type = RoomType::STANDARD;

	UPROPERTY(BlueprintReadOnly)
	TMap<WallDirection, UWallComponent*> Walls;

	// Map for window and door dimensions
	TMap<ObjectType, FVector> ObjDimensions;

	UStaticMeshComponent* floor1 = nullptr;
	UStaticMeshComponent* floor2 = nullptr;

	UStaticMeshComponent* ceiling1 = nullptr;
	UStaticMeshComponent* ceiling2 = nullptr;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void CreateRoom(RoomType type = RoomType::STANDARD);

	UFUNCTION(BlueprintCallable)
	// Updates wall acording with door or window if they added
	void UpdateWall(WallDirection direction);

	UFUNCTION(BlueprintCallable)
	void UpdateAllWalls();

	UFUNCTION(BlueprintCallable)
	// Add door/window or remove it and update wall
	void AddObjectToWall(UWallComponent* wall, float localPos, ObjectType type, int index = 0);

	UFUNCTION(BlueprintCallable)
	void SetCorner(FVector2D corner, bool needUpdateWalls = true);

	// Create and attach statuc mesh to wall component
	UStaticMeshComponent* AddStaticMeshComponent(UWallComponent* WallComponent, UStaticMesh* Mesh, FName Name);

	// Create and attach static mesh to room actor
	UStaticMeshComponent* AddStaticMeshComponent(UStaticMesh* Mesh, FName Name);

	void PrepareWallSegments(UWallComponent* wall);

	void UpdateFloor();

	void UpdateWallTransform(WallDirection direction);
	void UpdateWallLength(WallDirection direction);

	void ClampWallPosition(WallDirection direction);

	// Return vector that represents static mesh dimensions
	FVector GetStaticMeshDimensions(UStaticMesh* Mesh);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
