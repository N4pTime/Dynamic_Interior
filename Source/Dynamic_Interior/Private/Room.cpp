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

	// Keep Door/Window dimensions
	ObjDimensions.Add(WallType::WITH_DOOR, GetStaticMeshDimensions(DoorMesh));
	ObjDimensions.Add(WallType::WITH_WINDOW, GetStaticMeshDimensions(WindowMesh));

	CreateRoom(RoomType::STANDARD);
}

// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARoom::CreateRoom(RoomType type)
{
	Type = type;

	if (type == RoomType::STANDARD)
	{
		// Create walls
		for (int idx = 0; idx < 4; ++idx)
		{
			WallDirection dir = static_cast<WallDirection>(idx);

			UWallComponent* wall = NewObject<UWallComponent>(GetRootComponent(), UWallComponent::StaticClass(), FName("Wall" + FString::FromInt(idx + 1)));

			// Keep references for editing door/window aligments
			Walls.Add(dir, wall);

			wall->RegisterComponent();
			wall->AttachTo(this->RootComponent);

			wall->Type = WallType::EMPTY;
			wall->Direction = dir;

			// Setup wall segments
			for (int i = 0; i < 4; ++i)
			{
				SegmentDirection segdir = static_cast<SegmentDirection>(i);

				auto WallSegment = AddStaticMeshComponent(wall, WallMesh, FName(wall->GetName() + "_Segment" + FString::FromInt(i)));
				wall->WallSegments.Add(segdir, WallSegment);

				// WallSegment->SetVisibility(false);
			}
		}
	}
	else if (type == RoomType::L_SHAPE)
	{
		// Create 4 walls
		for (int idx = 0; idx < 6; ++idx)
		{
			WallDirection dir = static_cast<WallDirection>(idx);

			UWallComponent* wall = NewObject<UWallComponent>(GetRootComponent(), UWallComponent::StaticClass(), FName("Wall" + FString::FromInt(idx + 1)));

			// Keep references for editing door/window aligments
			Walls.Add(dir, wall);

			wall->RegisterComponent();
			wall->AttachTo(this->RootComponent);

			wall->Type = WallType::EMPTY;
			wall->Direction = dir;

			// Setup wall segments
			for (int i = 0; i < 4; ++i)
			{
				SegmentDirection segdir = static_cast<SegmentDirection>(i);

				auto WallSegment = AddStaticMeshComponent(wall, WallMesh, FName(wall->GetName() + "_Segment" + FString::FromInt(i)));
				wall->WallSegments.Add(segdir, WallSegment);

				// WallSegment->SetVisibility(false);
			}
		}
	}

	// Create room
	UpdateAllWalls();
}

void ARoom::SetCorner(FIntPoint corner)
{
	cornerX = FMath::Clamp((float)corner.X, minimalWallLength, Length - minimalWallLength);
	cornerY = FMath::Clamp((float)corner.Y, minimalWallLength, Width  - minimalWallLength);

	// Update corner walls
	UpdateWall(WallDirection::NORTH);
	UpdateWall(WallDirection::EAST);
	UpdateWall(WallDirection::NORTH_EAST);
	UpdateWall(WallDirection::SOUTH_EAST);
}

// Create segments (wall static meshes) and adds door/window by type
void ARoom::UpdateWall(WallDirection Direction)
{
	auto wall = Walls[Direction];

	// Update wall scale
	if (Type == RoomType::STANDARD)
	{
		switch (Direction)
		{
		case WallDirection::NORTH:
		case WallDirection::SOUTH:
			wall->Length = Width + WallOffset;
			break;
		case WallDirection::WEST:
		case WallDirection::EAST:
			wall->Length = Length + WallOffset;
			break;
		}
	}
	else if (Type == RoomType::L_SHAPE)
	{
		switch (Direction)
		{
		case WallDirection::SOUTH:
			//TODO: Set relative loc
			wall->Length = Width + WallOffset;
			break;
		case WallDirection::WEST:
			wall->Length = Length + WallOffset;
			break;
		case WallDirection::NORTH:
			wall->Length = cornerY + WallOffset;
			break;
		case WallDirection::NORTH_EAST:
			wall->Length = Length - cornerX;
			break;
		case WallDirection::EAST:
			wall->Length = cornerX + WallOffset;
			break;
		case WallDirection::SOUTH_EAST:
			wall->Length = Width - cornerY;
			break;
		}
	}

	// Update wall segments
	if (wall->Type == WallType::EMPTY)
	{
		// Destroy Door/Window
		if (IsValid(wall->Object)) wall->Object->DestroyComponent();
		
		UStaticMeshComponent* WallSegment = wall->WallSegments[SegmentDirection::TOP];

		// Set visible current / hide others
		WallSegment->SetVisibility(true);
		wall->WallSegments[SegmentDirection::BOTTOM]->SetVisibility(false);
		wall->WallSegments[SegmentDirection::LEFT]->SetVisibility(false);
		wall->WallSegments[SegmentDirection::RIGHT]->SetVisibility(false);

		WallSegment->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
		WallSegment->SetWorldScale3D(FVector(1.0, wall->Length, Height));
	}
	else if (wall->Type == WallType::WITH_DOOR)
	{
		auto& WallSegments = wall->WallSegments;

		// Hide one segment
		wall->WallSegments[SegmentDirection::BOTTOM]->SetVisibility(false);
		wall->WallSegments[SegmentDirection::TOP]->SetVisibility(true);
		wall->WallSegments[SegmentDirection::LEFT]->SetVisibility(true);
		wall->WallSegments[SegmentDirection::RIGHT]->SetVisibility(true);

		// Destroy old object and create new door mesh
		if (IsValid(wall->Object)) 
			wall->Object->DestroyComponent();
		wall->Object = AddStaticMeshComponent(wall, DoorMesh, FName(wall->GetName() + "_Door"));

		// Get door mesh dimensions
		auto DoorDimensions = ObjDimensions[WallType::WITH_DOOR];
		auto DoorWidth  = DoorDimensions.Y;
		auto DoorHeight = DoorDimensions.Z;

		// Set transform for wall sections and door
		WallSegments[SegmentDirection::LEFT]->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
		WallSegments[SegmentDirection::LEFT]->SetRelativeScale3D(FVector(1.0, wall->leftAligment, DoorHeight));

		// For door
		wall->Object->SetRelativeLocation(FVector(DoorOffset, wall->leftAligment, 0.0));

		WallSegments[SegmentDirection::RIGHT]->SetRelativeLocation(FVector(0.0, wall->leftAligment + DoorWidth, 0.0));
		WallSegments[SegmentDirection::RIGHT]->SetRelativeScale3D(FVector(1.0, wall->Length - (wall->leftAligment + DoorWidth), DoorHeight));

		WallSegments[SegmentDirection::TOP]->SetRelativeLocation(FVector(0.0, 0.0, DoorHeight));
		WallSegments[SegmentDirection::TOP]->SetRelativeScale3D(FVector(1.0, wall->Length, Height - DoorHeight));
	}
	else if (wall->Type == WallType::WITH_WINDOW)
	{
		auto& WallSegments = wall->WallSegments;

		// Show all segments
		wall->WallSegments[SegmentDirection::TOP]->SetVisibility(true);
		wall->WallSegments[SegmentDirection::LEFT]->SetVisibility(true);
		wall->WallSegments[SegmentDirection::RIGHT]->SetVisibility(true);
		wall->WallSegments[SegmentDirection::BOTTOM]->SetVisibility(true);

		// Destroy old object and create new door mesh
		if (IsValid(wall->Object) && wall->Object->IsValidLowLevel()) 
			wall->Object->DestroyComponent();
		wall->Object = AddStaticMeshComponent(wall, WindowMesh, FName(wall->GetName() + "_Window"));

		// Get door mesh dimensions
		auto WindowDimensions = ObjDimensions[WallType::WITH_WINDOW];
		auto WindowWidth  = WindowDimensions.Y - 5.3f; //NOTE: вычитаю, чтобы убрать зазор у окна
		auto WindowHeight = WindowDimensions.Z;

		// Set transform for wall sections and window
		WallSegments[SegmentDirection::LEFT]->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
		WallSegments[SegmentDirection::LEFT]->SetRelativeScale3D(FVector(1.0, wall->leftAligment, Height));

		// For window
		wall->Object->SetRelativeLocation(FVector(WindowOffset, wall->leftAligment, WindowHeightOffset));

		WallSegments[SegmentDirection::RIGHT]->SetRelativeLocation(FVector(0.0, wall->leftAligment + WindowWidth, 0.0));
		WallSegments[SegmentDirection::RIGHT]->SetRelativeScale3D(FVector(1.0, wall->Length - (wall->leftAligment + WindowWidth), Height));

		WallSegments[SegmentDirection::BOTTOM]->SetRelativeLocation(FVector(0.0, wall->leftAligment, 0.0));
		WallSegments[SegmentDirection::BOTTOM]->SetRelativeScale3D(FVector(1.0, WindowWidth, WindowHeightOffset));

		WallSegments[SegmentDirection::TOP]->SetRelativeLocation(FVector(0.0, wall->leftAligment, WindowHeightOffset + WindowHeight));
		WallSegments[SegmentDirection::TOP]->SetRelativeScale3D(FVector(1.0, WindowWidth, Height - (WindowHeightOffset + WindowHeight)));
	}
	
	UpdateWallTransform(Direction);
}

void ARoom::UpdateWallTransform(WallDirection direction)
{
	auto wall = Walls[direction];

	if (Type == RoomType::STANDARD)
	{
		switch (wall->Direction)
		{
		case WallDirection::NORTH:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(0.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(Length, 0.0, 0.0));
			break;
		case WallDirection::SOUTH:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(180.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(0.0, Width, 0.0));
			break;
		case WallDirection::WEST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
			break;
		case WallDirection::EAST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(Length, Width, 0.0));
			break;
		}
	}
	else if (Type == RoomType::L_SHAPE)
	{
		switch (wall->Direction)
		{
		case WallDirection::NORTH:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(0.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(Length, 0.0, 0.0));
			break;
		case WallDirection::SOUTH:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(180.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(0.0, Width, 0.0));
			break;
		case WallDirection::WEST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
			break;
		case WallDirection::EAST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(cornerX, Width, 0.0));
			break;
		case WallDirection::NORTH_EAST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(Length, cornerY, 0.0));
			break;
		case WallDirection::SOUTH_EAST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(0.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(cornerX, cornerY + WallOffset, 0.0));
			break;
		}
	}
}

void ARoom::UpdateAllWalls()
{
	// Create/update floor mesh
	if (!IsValid(floor))
		floor = AddStaticMeshComponent(FloorMesh, "Floor");
	floor->SetRelativeLocation(FVector(0.0));
	floor->SetRelativeScale3D(FVector(Length, Width, 1.0));

	// Create/update ceiling mesh
	if (!IsValid(ceiling))
		ceiling = AddStaticMeshComponent(CeilingMesh, "Ceiling");
	ceiling->SetRelativeLocation(FVector(0.0, 0.0, Height));
	ceiling->SetRelativeScale3D(FVector(Length, Width, 1.0));
	
	// Update wall segments
	for (auto& p : Walls)
	{
		auto& Direction = p.Key;
		auto& wall = p.Value;

		// Update aligments
		SetLeftAligment(Direction, wall->leftAligment);
	}
}

void ARoom::SetWallType(WallDirection Direction, WallType type)
{
	auto wall = Walls[Direction];

	wall->Type = type;

	// Update aligments
	SetLeftAligment(Direction, wall->leftAligment);
}

void ARoom::SetLeftAligment(WallDirection Direction, int value)
{
	auto wall = Walls[Direction];

	if (wall->Type != WallType::EMPTY)
	{
		auto ObjWidth = ObjDimensions[wall->Type].Y;

		auto WallWidth = (Direction == WallDirection::NORTH || Direction == WallDirection::SOUTH) ? Width : Length;
		wall->leftAligment = FMath::Clamp((float)value, AligmentOffset, WallWidth - AligmentOffset - ObjWidth);
		wall->rightAligment = WallWidth - wall->leftAligment - ObjWidth;
	}

	UpdateWall(Direction);
}

void ARoom::SetRightAligment(WallDirection Direction, int value)
{
	auto wall = Walls[Direction];

	auto ObjWidth = ObjDimensions[wall->Type].Y;

	auto WallWidth = (Direction == WallDirection::NORTH || Direction == WallDirection::SOUTH) ? Width : Length;
	wall->rightAligment = FMath::Clamp((float)value, AligmentOffset, WallWidth - AligmentOffset - ObjWidth);
	wall->leftAligment = WallWidth - wall->rightAligment - ObjWidth;

	UpdateWall(Direction);
}

UStaticMeshComponent* ARoom::AddStaticMeshComponent(UWallComponent* WallComponent, UStaticMesh* Mesh, FName Name)
{
	if (!IsValid(WallComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Wall component was null."));
		return nullptr;
	}

	auto Obj = NewObject<UStaticMeshComponent>(WallComponent, UStaticMeshComponent::StaticClass(), Name);
	if (!IsValid(Obj))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot create static mesh component."));
		return nullptr;
	}

	Obj->SetStaticMesh(Mesh);
	Obj->RegisterComponent();

	static FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
	Obj->AttachToComponent(WallComponent, rules);

	AddInstanceComponent(Obj);

	return Obj;
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

	// AddInstanceComponent(Obj);

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


