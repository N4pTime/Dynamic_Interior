// Fill out your copyright notice in the Description page of Project Settings.
#pragma optimize("", off)


#include "Room.h"

// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();
	
	if (!FloorMesh || !WallMesh || !CeilingMesh || !DoorMesh || !WindowMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Some mesh not set."));
		return;
	}

	// Create walls
	for (int idx = 0; idx < 4; ++idx)
	{
		WallDirection dir = static_cast<WallDirection>(idx);

		Wall W;
		W.Type = WallType::EMPTY;
		W.Direction = dir;
		W.Name = FString("Wall" + FString::FromInt(idx + 1));

		// Keep references for editing door/window aligments
		Walls.Add(dir, W);
	}

	// Create room
	UpdateAllSegments();
}

// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Create segments (wall static meshes) and adds door/window by type
void ARoom::UpdateSegments(WallDirection Direction)
{
	Wall& W = Walls[Direction];

	// Update wall segments
	if (W.Type == WallType::EMPTY)
	{
		// Destroy Door/Window
		if (IsValid(W.Object)) W.Object->DestroyComponent();

		// Remove previous segments
		if (W.WallSegments.Num() != 1)
		{
			for (auto& p : W.WallSegments)
			{
				auto& Segment = p.Value;
				Segment->DestroyComponent();
			}

			W.WallSegments.Reset();
		}

		// Create wall section
		UStaticMeshComponent* WallSegment = nullptr;

		if (W.WallSegments.Contains(SegmentDirection::TOP))
			WallSegment = W.WallSegments[SegmentDirection::TOP];
		else
		{
			// Create segment
			WallSegment = AddStaticMeshComponent(WallMesh, FName(W.Name + "_Segment"));
			W.WallSegments.Add(SegmentDirection::TOP, WallSegment);
		}

		// Add transform to wall section
		switch (Direction)
		{
		case WallDirection::NORTH:
			WallSegment->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(0.0f, FVector(0.0, 0.0, 1.0f)).Rotation());

			WallSegment->SetWorldScale3D(FVector(1.0, Width, Height));
			WallSegment->SetWorldLocation(FVector(Length, 0.0, 0.0));
			break;
		case WallDirection::SOUTH:
			WallSegment->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(180.0f, FVector(0.0, 0.0, 1.0f)).Rotation());

			WallSegment->SetWorldScale3D(FVector(1.0, Width, Height));
			WallSegment->SetWorldLocation(FVector(0.0, Width, 0.0));
			break;
		case WallDirection::WEST:
			WallSegment->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());

			WallSegment->SetWorldScale3D(FVector(1.0, Length, Height) + FVector(0.0, WallOffset * 2, 0.0));
			WallSegment->SetWorldLocation(FVector(-WallOffset, 0.0, 0.0));
			break;
		case WallDirection::EAST:
			WallSegment->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			
			WallSegment->SetWorldScale3D(FVector(1.0, Length, Height) + FVector(0.0, WallOffset * 2, 0.0));
			WallSegment->SetWorldLocation(FVector(WallOffset + Length, Width, 0.0));
			break;
		}
	}
	else if (W.Type == WallType::WITH_DOOR)
	{
		auto& WallSegments = W.WallSegments;

		// Create 3 wall sections
		if (!WallSegments.Contains(SegmentDirection::TOP))
			WallSegments.Add(SegmentDirection::TOP, AddStaticMeshComponent(WallMesh, FName(W.Name + "_Segment_Top")));

		if (!WallSegments.Contains(SegmentDirection::LEFT))
			WallSegments.Add(SegmentDirection::LEFT, AddStaticMeshComponent(WallMesh, FName(W.Name + "_Segment_Left")));

		if (!WallSegments.Contains(SegmentDirection::RIGHT))
			WallSegments.Add(SegmentDirection::RIGHT, AddStaticMeshComponent(WallMesh, FName(W.Name + "_Segment_Right")));

		if (WallSegments.Contains(SegmentDirection::BOTTOM))
		{
			WallSegments[SegmentDirection::BOTTOM]->DestroyComponent();
			WallSegments.Remove(SegmentDirection::BOTTOM);
		}

		// Destroy old object and create new door mesh
		if (IsValid(W.Object)) W.Object->DestroyComponent();
		W.Object = AddStaticMeshComponent(DoorMesh, FName(W.Name + "_Door"));

		// Get door mesh dimensions
		auto DoorDimensions = GetStaticMeshDimensions(DoorMesh);
		auto DoorWidth  = DoorDimensions.Y;
		auto DoorHeight = DoorDimensions.Z;

		if (Direction == WallDirection::WEST)
		{
			auto Location = FVector(-WallOffset, 0.0, 0.0);
			auto Rotation = GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation();
			auto Scale	  = FVector(1.0, Length, Height) + FVector(0.0, WallOffset * 2, 0.0);

			WallSegments[SegmentDirection::LEFT]->SetWorldLocation(Location);
			WallSegments[SegmentDirection::LEFT]->SetWorldRotation(Rotation);
			WallSegments[SegmentDirection::LEFT]->SetRelativeScale3D(FVector(1.0, WallOffset + W.leftAligment, Height));

			WallSegments[SegmentDirection::RIGHT]->SetWorldLocation(FVector(W.leftAligment + DoorWidth, 0.0, 0.0));
			WallSegments[SegmentDirection::RIGHT]->SetWorldRotation(Rotation);
			WallSegments[SegmentDirection::RIGHT]->SetRelativeScale3D(FVector(1.0, Length + WallOffset - DoorWidth, Height));

			WallSegments[SegmentDirection::TOP]->SetWorldLocation(Location + FVector(0.0, 0.0, DoorHeight));
			WallSegments[SegmentDirection::TOP]->SetWorldRotation(Rotation);
			WallSegments[SegmentDirection::TOP]->SetRelativeScale3D(Scale + FVector(0.0, 0.0, -DoorHeight));

			// For door
			W.Object->SetWorldLocation(FVector(W.leftAligment, 0.0, 0.0));
			W.Object->SetWorldRotation(Rotation);
		}
			
	}
	else if (W.Type == WallType::WITH_WINDOW)
	{

	}
}

void ARoom::UpdateAllSegments()
{
	// Floor
	if (!IsValid(floor))
		floor = AddStaticMeshComponent(FloorMesh, "Floor");
	floor->SetWorldScale3D(FVector(Length, Width, 1.0));

	// Ceiling
	/*if (!IsValid(ceiling))
		ceiling = AddStaticMeshComponent(CeilingMesh, "Ceiling");
	ceiling->SetWorldScale3D(FVector(Length, Width, 1.0));
	ceiling->SetWorldLocation(GetActorLocation() + FVector(0.0, 0.0, Height));*/

	for (auto& p : Walls)
	{
		auto& Direction = p.Key;
		UpdateSegments(Direction);
	}
}

void ARoom::SetWallType(WallDirection Direction, WallType type)
{
	Wall& W = Walls[Direction];

	W.Type = type;

	// Create door/window mesh or delete it

	UpdateSegments(Direction);
}

void ARoom::SetLeftAligment(WallDirection Direction, int value)
{
	Wall& W = Walls[Direction];

	W.leftAligment = value;
	UpdateSegments(Direction);
}

void ARoom::SetRightAligment(WallDirection Direction, int value)
{
	Wall& W = Walls[Direction];

	W.rightAligment = value;
	UpdateSegments(Direction);
}

UStaticMeshComponent* ARoom::AddStaticMeshComponent(UStaticMesh* Mesh, FName Name)
{
	auto Obj = NewObject<UStaticMeshComponent>(GetRootComponent(), UStaticMeshComponent::StaticClass(), Name);
	if (!IsValid(Obj))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot create static mesh component."));
		return nullptr;
	}

	Obj->SetStaticMesh(Mesh);
	Obj->RegisterComponent();
	Obj->AttachTo(this->RootComponent);

	AddInstanceComponent(Obj);

	return Obj;
}

FVector ARoom::GetStaticMeshDimensions(UStaticMesh* Mesh)
{
	if (!IsValid(Mesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid static mesh."));
		return FVector(0.0);
	}

	auto Box = Mesh->GetBoundingBox();

	return Box.Max - Box.Min;
}


