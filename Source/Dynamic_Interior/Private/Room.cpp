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

	CreateRoom(RoomType::L_SHAPE);
}

// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	this->SetActorEnableCollision(true);

	
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

				auto WallSegment = (UObjectComponent*)AddStaticMeshComponent(wall, WallMesh, FName(wall->GetName() + "_Segment" + FString::FromInt(i)));
				wall->WallSegments.Add(segdir, WallSegment);

				// WallSegment->SetVisibility(false);
			}
		}

		// Prepare floor mesh
		floor1 = AddStaticMeshComponent(FloorMesh, "Floor1");
		floor1->SetVisibility(false);

		// Prepare ceiling mesh
		ceiling1 = AddStaticMeshComponent(CeilingMesh, "Ceiling");
		ceiling1->SetVisibility(false);
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

				auto WallSegment = (UObjectComponent*)AddStaticMeshComponent(wall, WallMesh, FName(wall->GetName() + "_Segment" + FString::FromInt(i)));
				wall->WallSegments.Add(segdir, WallSegment);

				// WallSegment->SetVisibility(false);
			}
		}

		// Prepare floor mesh
		floor1 = AddStaticMeshComponent(FloorMesh, "Floor1");
		floor1->SetVisibility(false);

		floor2 = AddStaticMeshComponent(FloorMesh, "Floor2");
		floor2->SetVisibility(false);

		// Prepare ceiling mesh
		ceiling1 = AddStaticMeshComponent(CeilingMesh, "Ceiling1");
		ceiling1->SetVisibility(false);

		ceiling2 = AddStaticMeshComponent(CeilingMesh, "Ceiling2");
		ceiling2->SetVisibility(false);
	}

	// Create room
	UpdateAllWalls();
}

// Create segments (wall static meshes) and adds door/window by type
void ARoom::UpdateWall(WallDirection Direction)
{
	auto wall = Walls[Direction];

	// Update wall scale
	UpdateWallLength(Direction);
	UpdateWallTransform(Direction);
	UpdateWallAligment(Direction);

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
}

void ARoom::UpdateWallLength(WallDirection direction)
{
	auto wall = Walls[direction];

	// Update wall scale
	if (Type == RoomType::STANDARD)
	{
		switch (direction)
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
		switch (direction)
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
	// Clamp room dimensions
	if (Type == RoomType::STANDARD)
	{
		if (Length < minimalWallLength)
			Length = minimalWallLength;

		if (Width < minimalWallLength)
			Width = minimalWallLength;
	}
	else if (Type == RoomType::L_SHAPE)
	{
		if (Length < minimalWallLength * 2)
			Length = minimalWallLength * 2;

		if (Width < minimalWallLength * 2)
			Width = minimalWallLength * 2;
	}

	// Update corners
	SetCorner({ cornerX, cornerY }, false);

	// Create/update floor mesh
	UpdateFloor();

	// Update wall segments
	for (auto& p : Walls)
	{
		auto& Direction = p.Key;
		auto& wall = p.Value;

		// Update aligments
		SetLeftAligment(Direction, wall->leftAligment);
	}
}

void ARoom::AddObjectToWall(UWallComponent* wall, float localPos, WallType type)
{
	if (!IsValid(wall) || wall->Type != WallType::EMPTY)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wall component was null."));
		return;
	}

	FVector objWidth;
	if (type == WallType::WITH_DOOR)
	{
		objWidth = ObjDimensions[WallType::WITH_DOOR];
	}
	else if (type == WallType::WITH_WINDOW)
	{
		objWidth = ObjDimensions[WallType::WITH_WINDOW];
	}

	wall->Type = type;

	// Update aligments
	SetLeftAligment(wall->Direction, localPos - objWidth.Y / 2.0);
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

		auto WallWidth = wall->Length - WallOffset;
		wall->leftAligment = FMath::Clamp((float)value, AligmentOffset, WallWidth - AligmentOffset - ObjWidth);
		wall->rightAligment = WallWidth - wall->leftAligment - ObjWidth;
	}

	UpdateWall(Direction);
}

void ARoom::SetRightAligment(WallDirection Direction, int value)
{
	auto wall = Walls[Direction];

	if (wall->Type != WallType::EMPTY)
	{
		auto ObjWidth = ObjDimensions[wall->Type].Y;

		auto WallWidth = wall->Length - WallOffset;
		wall->rightAligment = FMath::Clamp((float)value, AligmentOffset, WallWidth - AligmentOffset - ObjWidth);
		wall->leftAligment = WallWidth - wall->rightAligment - ObjWidth;
	}

	UpdateWall(Direction);
}

void ARoom::UpdateWallAligment(WallDirection direction)
{
	auto wall = Walls[direction];

	if (wall->Type != WallType::EMPTY)
	{
		auto ObjWidth = ObjDimensions[wall->Type].Y;

		auto WallWidth = wall->Length - WallOffset;
		wall->leftAligment = FMath::Clamp((float)wall->leftAligment, AligmentOffset, WallWidth - AligmentOffset - ObjWidth);
		wall->rightAligment = WallWidth - wall->leftAligment - ObjWidth;
	}
}

void ARoom::SetCorner(FVector2D corner, bool needUpdateWalls)
{
	cornerX = FMath::Clamp(corner.X, minimalWallLength, Length - minimalWallLength);
	cornerY = FMath::Clamp(corner.Y, minimalWallLength, Width - minimalWallLength);

	// Update corner walls
	if (needUpdateWalls)
	{
		UpdateWall(WallDirection::NORTH);
		UpdateWall(WallDirection::EAST);
		UpdateWall(WallDirection::NORTH_EAST);
		UpdateWall(WallDirection::SOUTH_EAST);

		UpdateFloor();
	}
}

void ARoom::UpdateFloor()
{
	if (Type == RoomType::STANDARD)
	{
		floor1->SetRelativeLocation(FVector(0.0));
		floor1->SetRelativeScale3D(FVector(Length, Width, 1.0));
		floor1->SetVisibility(true);

		// Create/update ceiling mesh
		ceiling1->SetRelativeLocation(FVector(0.0, 0.0, Height));
		ceiling1->SetRelativeScale3D(FVector(Length, Width, 1.0));
		ceiling1->SetVisibility(true);
	}
	else if (Type == RoomType::L_SHAPE)
	{
		floor1->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
		floor1->SetRelativeScale3D(FVector(cornerX, Width, 1.0));

		ceiling1->SetRelativeLocation(FVector(0.0, 0.0, Height));
		ceiling1->SetRelativeScale3D(FVector(cornerX, Width, 1.0));

		floor2->SetRelativeLocation(FVector(cornerX, 0.0, 0.0));
		floor2->SetRelativeScale3D(FVector(Length - cornerX, cornerY, 1.0));

		ceiling2->SetRelativeLocation(FVector(cornerX, 0.0, Height));
		ceiling2->SetRelativeScale3D(FVector(Length - cornerX, cornerY, 1.0));

		// Set visible
		floor1->SetVisibility(true);
		floor2->SetVisibility(true);
		ceiling1->SetVisibility(true);
		ceiling2->SetVisibility(true);
	}
	
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


