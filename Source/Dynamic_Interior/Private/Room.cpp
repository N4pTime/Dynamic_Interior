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
	
	if (!FloorMesh || !WallMesh || !CeilingMesh || DoorMeshes.Num() == 0 || WindowMeshes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Some mesh not set."));
		return;
	}

	CreateRoom(RoomType::STANDARD);

	
}

// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	this->SetActorEnableCollision(true);

	
}

void ARoom::EnableBoundingBoxes(bool value)
{
	auto type = value ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision;

	for (auto& boxComponent : WallBoundingBoxes)
		boxComponent->SetCollisionEnabled(type);
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

			static FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
			wall->AttachToComponent(this->RootComponent, rules);
			AddInstanceComponent(wall);

			wall->RegisterComponent();

			wall->Direction = dir;

			// Add bounding box to array
			WallBoundingBoxes.Add(wall->BoundingBox);

			// Create corner segment
			auto cornerMesh = AddStaticMeshComponent(WallMesh, FName("Corner" + FString::FromInt(idx + 1)));
			CornerSegments.Add(MakeWeakObjectPtr(cornerMesh));

			cornerMesh->RegisterComponent();
			cornerMesh->AttachToComponent(this->RootComponent, rules);
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

			static FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
			wall->AttachToComponent(this->RootComponent, rules);

			wall->RegisterComponent();

			wall->Direction = dir;

			// Add bounding box to array
			WallBoundingBoxes.Add(wall->BoundingBox);

			// Create corner segment
			auto cornerMesh = AddStaticMeshComponent(WallMesh, FName("Corner" + FString::FromInt(idx + 1)));
			CornerSegments.Add(MakeWeakObjectPtr(cornerMesh));

			cornerMesh->RegisterComponent();
			cornerMesh->AttachToComponent(this->RootComponent, rules);
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

	// Disable bounding boxed
	EnableBoundingBoxes(false);

	// Create room
	UpdateAllWalls();
}

// Create segments (wall static meshes) and adds door/window by type
void ARoom::UpdateWall(WallDirection Direction)
{
	auto wall = Walls[Direction];

	// Update wall scale
	UpdateWallTransform(Direction);
	PrepareWallSegments(wall);

	// Update wall segments
	if (wall->Objects.Num() == 0)
	{
		auto WallSegment = wall->HorizontalSegments[0];

		WallSegment->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
		WallSegment->SetWorldScale3D(FVector(1.0, wall->Length, Height));
	}
	else
	{
		// Sort objects by their offset
		wall->Objects.Sort([](const UObjectComponent& c1, const UObjectComponent& c2)
			{
				return c1.offset < c2.offset;
			});

		// Set transform to horizontal segments
		float currentOffset = 0.0;
		for (int idx = 0; idx < wall->HorizontalSegments.Num(); ++idx)
		{
			auto segment = wall->HorizontalSegments[idx];

			if (idx == wall->HorizontalSegments.Num() - 1)
			{
				segment->SetRelativeLocation(FVector(0.0, currentOffset, 0.0));
				segment->SetRelativeScale3D(FVector(1.0, wall->Length - currentOffset, Height));
			}
			else
			{
				auto obj	  = wall->Objects[idx];
				auto objWidth = obj->GetDimensions().Y;

				segment->SetRelativeLocation(FVector(0.0, currentOffset, 0.0));
				segment->SetRelativeScale3D(FVector(1.0, obj->offset - currentOffset, Height));

				currentOffset += (obj->offset - currentOffset) + objWidth;
			}
		}

		// Set transform to vertical segments
		int segIndex = 0;
		for (auto& obj : wall->Objects)
		{
			auto objHeight = obj->GetDimensions().Z;
			auto objWidth = obj->GetDimensions().Y;

			if (obj->type == ObjectType::DOOR)
			{
				auto segment = wall->VerticalSegments[segIndex++];

				segment->SetRelativeLocation(FVector(0.0, obj->offset, objHeight));
				segment->SetRelativeScale3D(FVector(1.0, objWidth, Height - objHeight));
			}
			else if (obj->type == ObjectType::WINDOW)
			{
				auto segmentBottom  = wall->VerticalSegments[segIndex++];
				auto segmentTop		= wall->VerticalSegments[segIndex++];

				segmentBottom->SetRelativeLocation(FVector(0.0, obj->offset, 0.0));
				segmentBottom->SetRelativeScale3D(FVector(1.0, objWidth, WindowHeightOffset));

				segmentTop->SetRelativeLocation(FVector(0.0, obj->offset, WindowHeightOffset + objHeight));
				segmentTop->SetRelativeScale3D(FVector(1.0, objWidth, Height - (WindowHeightOffset + objHeight)));
			}
		}

		// Transform objects
		for (auto& obj : wall->Objects)
		{
			switch (obj->type)
			{
			case ObjectType::DOOR:
				obj->SetRelativeLocation(FVector(DoorOffset, obj->offset, 0.0));
				break;
			case ObjectType::WINDOW:
				obj->SetRelativeLocation(FVector(WindowOffset, obj->offset, WindowHeightOffset));
				break;
			}

			// Rotate obj by 180 degrees if needed
			switch (wall->Direction)
			{
			case WallDirection::SOUTH:
			case WallDirection::EAST:
			case WallDirection::NORTH_EAST:
				obj->SetWorldRotation(wall->GetComponentRotation().Vector().RotateAngleAxis(180.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
				obj->AddRelativeLocation(FVector(WallOffset / 2.0, obj->GetDimensions().Y, 0.0));
				break;
			}
		}
	}
}

void ARoom::PrepareWallSegments(UWallComponent* wall)
{
	int numDoors = 0;
	int numWindows = 0;

	// Count doors/windows
	for (const auto& obj : wall->Objects)
	{
		if (obj->type == ObjectType::DOOR)
			numDoors++;
		if (obj->type == ObjectType::WINDOW)
			numWindows++;
	}

	if (!numDoors && !numWindows)
	{
		wall->HorizontalSegments.SetNumUninitialized(1);
		wall->VerticalSegments.SetNumUninitialized(0);

		auto& segment = wall->HorizontalSegments[0];
		FName name = FName(wall->GetName() + "_SegH");

		segment = MakeWeakObjectPtr<UStaticMeshComponent>(AddStaticMeshComponent(wall, WallMesh, name));
	}
	else
	{
		// Update count of segments
		wall->HorizontalSegments.SetNumUninitialized(wall->Objects.Num() + 1);
		wall->VerticalSegments.SetNumUninitialized(numDoors + numWindows * 2);

		// Create horizontal segments on unallocated elements
		for (auto& segment : wall->HorizontalSegments)
		{
			if (segment.IsValid()) continue;

			auto index = wall->HorizontalSegments.Find(segment);
			FName name = FName(wall->GetName() + "_SegH" + FString::FromInt(index));

			segment = MakeWeakObjectPtr<UStaticMeshComponent>(AddStaticMeshComponent(wall, WallMesh, name));
		}

		// Create vertical segments on unallocated elements
		for (auto& segment : wall->VerticalSegments)
		{
			if (segment.IsValid()) continue;

			auto index = wall->VerticalSegments.Find(segment);
			FName name = FName(wall->GetName() + "_SegV" + FString::FromInt(index));

			segment = MakeWeakObjectPtr<UStaticMeshComponent>(AddStaticMeshComponent(wall, WallMesh, name));
		}
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
			wall->Length = Width;
			break;
		case WallDirection::WEST:
		case WallDirection::EAST:
			wall->Length = Length;
			break;
		}
	}
	else if (Type == RoomType::L_SHAPE)
	{
		switch (direction)
		{
		case WallDirection::SOUTH:
			//TODO: Set relative loc
			wall->Length = Width;
			break;
		case WallDirection::WEST:
			wall->Length = Length;
			break;
		case WallDirection::NORTH:
			wall->Length = cornerY;
			break;
		case WallDirection::NORTH_EAST:
			wall->Length = Length - cornerX - WallOffset;
			break;
		case WallDirection::EAST:
			wall->Length = cornerX;
			break;
		case WallDirection::SOUTH_EAST:
			wall->Length = Width - cornerY - WallOffset;
			break;
		}
	}
}

void ARoom::UpdateWallTransform(WallDirection direction)
{
	auto wall = Walls[direction];

	UpdateWallLength(direction);

	// Update wall box component transform
	wall->BoundingBox->SetRelativeLocation(FVector(AligmentOffset / 2.0, wall->Length / 2.0, Height / 2.0));
	wall->BoundingBox->SetRelativeScale3D(FVector(AligmentOffset + 0.001, wall->Length, Height * 1.3) / 100.0);

	// Update wall world position
	if (Type == RoomType::STANDARD)
	{
		switch (wall->Direction)
		{
		case WallDirection::NORTH:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(0.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(wall->Length, 0.0, 0.0));
			break;
		case WallDirection::SOUTH:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(0.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(-WallOffset, 0.0, 0.0));
			break;
		case WallDirection::WEST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
			break;
		case WallDirection::EAST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(0.0, wall->Length + WallOffset, 0.0));
			break;
		}
	}
	else if (Type == RoomType::L_SHAPE)
	{
		switch (wall->Direction)
		{
		case WallDirection::NORTH:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(0.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(wall->Length, 0.0, 0.0));
			break;
		case WallDirection::SOUTH:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(0.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(-WallOffset, 0.0, 0.0));
			break;
		case WallDirection::WEST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
			break;
		case WallDirection::EAST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(0.0f, wall->Length + WallOffset, 0.0));
			break;
		case WallDirection::NORTH_EAST:
			wall->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
			wall->SetRelativeLocation(FVector(cornerX + WallOffset, cornerY + WallOffset, 0.0));
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
	ClampDimensions();

	// Update corners
	SetCorner({ cornerX, cornerY }, false);

	// Create/update floor mesh
	UpdateFloor();

	// Update wall segments
	for (auto& p : Walls)
	{
		auto& Direction = p.Key;
		auto& wall = p.Value;

		UpdateWall(Direction);
	}
}

void ARoom::AddObjectToWall(UWallComponent* wall, float localPos, ObjectType type, int index)
{
	if (!IsValid(wall))
	{
		UE_LOG(LogTemp, Warning, TEXT("Wall component was null."));
		return;
	}

	// Get static mesh
	UStaticMesh* Mesh = nullptr;
	if (type == ObjectType::DOOR)
		Mesh = DoorMeshes[index];
	else if (type == ObjectType::WINDOW)
		Mesh = WindowMeshes[index];

	// Get mesh dimensions
	FVector meshDimensions	= GetStaticMeshDimensions(Mesh);
	float meshOffset		= (int)(localPos - meshDimensions.Y / 2.0);
	float requiredSpace		= meshDimensions.Y + 2 * AligmentOffset;

	// Correct object placing
	if (wall->Objects.Num() != 0)
	{
		// Search for right object
		auto obj = wall->Objects.FindByPredicate([&localPos](const UObjectComponent* comp)
			{
				return comp->offset > localPos;
			});

		// 1. Check on the left of the right object
		if (nullptr != obj)
		{
			// Get left obj
			auto rightObj = *obj;
			auto rightObjIndex  = wall->Objects.Find(rightObj);

			// 2. Check space between left and right objects
			if (rightObjIndex != 0)
			{
				auto leftObj  = wall->Objects[rightObjIndex - 1];

				// Awaible space between two objs
				float awaibleSpace = rightObj->offset - (leftObj->offset + leftObj->GetDimensions().Y);

				// If awaible space less than mesh width + two offsets -> return
				if (awaibleSpace < requiredSpace)
					return;

				// Check where need to clamp
				// If true -> close to rightObject
				if (rightObj->offset - localPos < localPos - (leftObj->offset + leftObj->GetDimensions().Y))
				{
					// Clamp offset for current object
					if (meshOffset > rightObj->offset - meshDimensions.Y - AligmentOffset)
						meshOffset = rightObj->offset - meshDimensions.Y - AligmentOffset;
				}
				// If true -> close to leftObject
				else
				{
					// Clamp offset for current object
					if (meshOffset < leftObj->offset + leftObj->GetDimensions().Y + AligmentOffset)
						meshOffset = leftObj->offset + leftObj->GetDimensions().Y + AligmentOffset;
				}
			}
			else
			{
				// Awaible space between two objs
				float awaibleSpace = rightObj->offset;

				// If awaible space less than mesh width + two offsets -> return
				if (awaibleSpace < requiredSpace)
					return;

				// Clamp offset for current object
				if (meshOffset > rightObj->offset - meshDimensions.Y - AligmentOffset)
					meshOffset = rightObj->offset - meshDimensions.Y - AligmentOffset;
			}
		}
		// 3. Check on the right of the last object
		else
		{
			auto lastObj = wall->Objects.Last();

			// Check if clicked on lastObj space
			if (lastObj->offset + lastObj->GetDimensions().Y > localPos)
				return;

			// Awaible space between two objs
			float awaibleSpace = wall->Length - (lastObj->offset + lastObj->GetDimensions().Y);

			// If awaible space less than mesh width + two offsets -> return
			if (awaibleSpace < requiredSpace)
				return;

			// Clamp offset for current object
			if (meshOffset < lastObj->offset + lastObj->GetDimensions().Y + AligmentOffset)
				meshOffset = lastObj->offset + lastObj->GetDimensions().Y + AligmentOffset;
		}
	}
	
	// Create object and attach to wall component
	auto name = FName(wall->GetName() + "Obj" + FString::FromInt(wall->Objects.Num()));
	auto obj = NewObject<UObjectComponent>(wall, UObjectComponent::StaticClass(), name);
	if (!IsValid(obj))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot create static mesh component."));
		return;
	}

	obj->SetStaticMesh(Mesh);
	obj->RegisterComponent();

	static FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
	obj->AttachToComponent(wall, rules);

	AddInstanceComponent(obj);

	// Clamp obj offset between wall corners
	obj->offset = FMath::Clamp(meshOffset, AligmentOffset, wall->Length - (AligmentOffset + meshDimensions.Y));
	obj->type = type;

	// Add object to wall component
	wall->Objects.Add(obj);
	 
	// Update wall
	UpdateWall(wall->Direction);
}

void ARoom::RemoveObjectFromWall(UObjectComponent* obj)
{
	if (!IsValid(obj))
	{
		return;
	}

	auto wall = (UWallComponent*)obj->GetAttachParent();

	if (wall->Objects.Remove(obj) > 0)
	{
		obj->DestroyComponent();
		UpdateWall(wall->Direction);
	}
		
}

void ARoom::MoveObject(UObjectComponent* obj, float newPos)
{
	if (!IsValid(obj))
	{
		return;
	}

	auto wall = (UWallComponent*)obj->GetAttachParent();

	obj->offset = FMath::Clamp(newPos, AligmentOffset, wall->Length - AligmentOffset - obj->GetDimensions().Y);

	UpdateWall(wall->Direction);
}

void ARoom::ClampDimensions()
{
	if (Type == RoomType::STANDARD)
	{
		// Check minimal length
		int minLength = minimalWallLength;

		auto w1 = Walls[WallDirection::WEST];
		if (w1->Objects.Num() > 0)
		{
			auto obj = w1->Objects.Last();
			int min = obj->offset + obj->GetDimensions().Y + AligmentOffset;
			if (minLength < min)
				minLength = min;
		}

		auto w2 = Walls[WallDirection::EAST];
		if (w2->Objects.Num() > 0)
		{
			auto obj = w2->Objects.Last();
			int min = obj->offset + obj->GetDimensions().Y + AligmentOffset;
			if (minLength < min)
				minLength = min;
		}

		if (Length < minLength)
			Length = minLength;

		// Check minimal width
		int minWidth = minimalWallLength;

		auto w3 = Walls[WallDirection::NORTH];
		if (w3->Objects.Num() > 0)
		{
			auto obj = w3->Objects.Last();
			int min = obj->offset + obj->GetDimensions().Y + AligmentOffset;
			if (minWidth < min)
				minWidth = min;
		}

		auto w4 = Walls[WallDirection::SOUTH];
		if (w4->Objects.Num() > 0)
		{
			auto obj = w4->Objects.Last();
			int min = obj->offset + obj->GetDimensions().Y + AligmentOffset;
			if (minWidth < min)
				minWidth = min;
		}

		if (Width < minWidth)
			Width = minWidth;
	}
	else if (Type == RoomType::L_SHAPE)
	{
		// Check minimal length
		int minLength = minimalWallLength * 2;

		auto w1 = Walls[WallDirection::WEST];
		if (w1->Objects.Num() > 0)
		{
			auto obj = w1->Objects.Last();
			int min = obj->offset + obj->GetDimensions().Y + AligmentOffset;
			if (minLength < min)
				minLength = min;
		}

		auto w2 = Walls[WallDirection::NORTH_EAST];
		// auto w3 = Walls[WallDirection::EAST];

		if (w2->Objects.Num() > 0)
		{
			auto obj = w2->Objects.Last();
			float min = (cornerX + WallOffset) + obj->offset + obj->GetDimensions().Y + AligmentOffset;
			if (minLength < min)
				minLength = min;
		}

		if (Length < minLength)
			Length = minLength;

		// Check minimal width
		int minWidth = minimalWallLength * 2;

		auto w3 = Walls[WallDirection::SOUTH];
		if (w3->Objects.Num() > 0)
		{
			auto obj = w3->Objects.Last();
			int min = obj->offset + obj->GetDimensions().Y + AligmentOffset;
			if (minWidth < min)
				minWidth = min;
		}

		auto w4 = Walls[WallDirection::SOUTH_EAST];
		// auto w5 = Walls[WallDirection::NORTH];

		if (w4->Objects.Num() > 0)
		{
			auto obj = w4->Objects.Last();
			float min = (cornerY + WallOffset) + obj->offset + obj->GetDimensions().Y + AligmentOffset;
			if (minWidth < min)
				minWidth = min;
		}

		if (Width < minWidth)
			Width = minWidth;
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

		// Set corner transform
		/*auto corner1 = CornerSegments[0];
		corner1->SetRelativeLocation(FVector(-WallOffset, -WallOffset, 0.0));
		corner1->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner1->SetVisibility(true);

		auto corner2 = CornerSegments[1];
		corner2->SetRelativeLocation(FVector(Length, -WallOffset, 0.0));
		corner2->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner2->SetVisibility(true);

		auto corner3 = CornerSegments[2];
		corner3->SetRelativeLocation(FVector(Length, Width, 0.0));
		corner3->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner3->SetVisibility(true);

		auto corner4 = CornerSegments[3];
		corner4->SetRelativeLocation(FVector(-WallOffset, Width, 0.0));
		corner4->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner4->SetVisibility(true);*/
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

		// Set corner transform
		auto corner1 = CornerSegments[0];
		corner1->SetRelativeLocation(FVector(-WallOffset, -WallOffset, 0.0));
		corner1->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner1->SetVisibility(true);

		auto corner2 = CornerSegments[1];
		corner2->SetRelativeLocation(FVector(Length, -WallOffset, 0.0));
		corner2->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner2->SetVisibility(true);

		auto corner3 = CornerSegments[2];
		corner3->SetRelativeLocation(FVector(Length, cornerY, 0.0));
		corner3->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner3->SetVisibility(true);

		auto corner4 = CornerSegments[3];
		corner4->SetRelativeLocation(FVector(cornerX, cornerY, 0.0));
		corner4->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner4->SetVisibility(true);

		auto corner5 = CornerSegments[4];
		corner5->SetRelativeLocation(FVector(cornerX, Width, 0.0));
		corner5->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner5->SetVisibility(true);

		auto corner6 = CornerSegments[5];
		corner6->SetRelativeLocation(FVector(-WallOffset, Width, 0.0));
		corner6->SetRelativeScale3D(FVector(1.0, WallOffset, Height));
		corner6->SetVisibility(true);
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

	static FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
	Obj->AttachToComponent(this->RootComponent, rules);

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


