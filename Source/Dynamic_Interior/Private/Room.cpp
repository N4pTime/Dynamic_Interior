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

	// Create walls
	for (int idx = 0; idx < 4; ++idx)
	{
		WallDirection dir = static_cast<WallDirection>(idx);

		UWallComponent* W = NewObject<UWallComponent>(GetRootComponent(), UWallComponent::StaticClass(), FName("Wall" + FString::FromInt(idx + 1)));
		
		// Keep references for editing door/window aligments
		Walls.Add(dir, W);
		
		W->RegisterComponent();
		W->AttachTo(this->RootComponent);

		W->Type = WallType::EMPTY;
		W->Direction = dir;
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
	auto W = Walls[Direction];

	// Update wall segments
	if (W->Type == WallType::EMPTY)
	{
		// Destroy Door/Window
		if (IsValid(W->Object)) W->Object->DestroyComponent();

		// Remove previous segments
		if (W->WallSegments.Num() != 1)
		{
			for (auto& p : W->WallSegments)
			{
				auto& Segment = p.Value;
				Segment->DestroyComponent();
			}

			W->WallSegments.Reset();
		}
		
		UStaticMeshComponent* WallSegment = nullptr;

		// Get wall segment
		if (W->WallSegments.Contains(SegmentDirection::TOP))
			WallSegment = W->WallSegments[SegmentDirection::TOP];
		else
		{
			// Else create segment
			WallSegment = AddStaticMeshComponent(W, WallMesh, FName(W->GetName() + "_Segment"));
			W->WallSegments.Add(SegmentDirection::TOP, WallSegment);
		}

		switch (Direction)
		{
		case WallDirection::NORTH:
		case WallDirection::SOUTH:
			//TODO: Set relative loc
			WallSegment->SetWorldScale3D(FVector(1.0, Width + WallOffset, Height));
			break;
		case WallDirection::WEST:
		case WallDirection::EAST:
			WallSegment->SetWorldScale3D(FVector(1.0, Length + WallOffset, Height));
			break;
		}
	}
	else if (W->Type == WallType::WITH_DOOR)
	{
		auto& WallSegments = W->WallSegments;

		// Create 3 wall sections
		if (!WallSegments.Contains(SegmentDirection::TOP))
			WallSegments.Add(SegmentDirection::TOP, AddStaticMeshComponent(W, WallMesh, FName(W->GetName() + "_Segment_Top")));

		if (!WallSegments.Contains(SegmentDirection::LEFT))
			WallSegments.Add(SegmentDirection::LEFT, AddStaticMeshComponent(W, WallMesh, FName(W->GetName() + "_Segment_Left")));

		if (!WallSegments.Contains(SegmentDirection::RIGHT))
			WallSegments.Add(SegmentDirection::RIGHT, AddStaticMeshComponent(W, WallMesh, FName(W->GetName() + "_Segment_Right")));

		if (WallSegments.Contains(SegmentDirection::BOTTOM))
		{
			WallSegments[SegmentDirection::BOTTOM]->DestroyComponent();
			WallSegments.Remove(SegmentDirection::BOTTOM);
		}

		// Destroy old object and create new door mesh
		if (IsValid(W->Object)) 
			W->Object->DestroyComponent();
		W->Object = AddStaticMeshComponent(W, DoorMesh, FName(W->GetName() + "_Door"));

		// Get door mesh dimensions
		auto DoorDimensions = ObjDimensions[WallType::WITH_DOOR];
		auto DoorWidth  = DoorDimensions.Y;
		auto DoorHeight = DoorDimensions.Z;

		// Set wall length
		auto WallLength = Width + WallOffset;
		if (Direction == WallDirection::WEST || Direction == WallDirection::EAST)
			WallLength = Length + WallOffset;

		// Set transform for wall sections and door
		WallSegments[SegmentDirection::LEFT]->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
		WallSegments[SegmentDirection::LEFT]->SetRelativeScale3D(FVector(1.0, W->leftAligment, DoorHeight));

		// For door
		W->Object->SetRelativeLocation(FVector(DoorOffset, W->leftAligment, 0.0));

		WallSegments[SegmentDirection::RIGHT]->SetRelativeLocation(FVector(0.0, W->leftAligment + DoorWidth, 0.0));
		WallSegments[SegmentDirection::RIGHT]->SetRelativeScale3D(FVector(1.0, WallLength - (W->leftAligment + DoorWidth), DoorHeight));

		WallSegments[SegmentDirection::TOP]->SetRelativeLocation(FVector(0.0, 0.0, DoorHeight));
		WallSegments[SegmentDirection::TOP]->SetRelativeScale3D(FVector(1.0, WallLength, Height - DoorHeight));
	}
	else if (W->Type == WallType::WITH_WINDOW)
	{
		auto& WallSegments = W->WallSegments;

		// Create 3 wall sections
		if (!WallSegments.Contains(SegmentDirection::TOP))
			WallSegments.Add(SegmentDirection::TOP, AddStaticMeshComponent(W, WallMesh, FName(W->GetName() + "_Segment_Top")));

		if (!WallSegments.Contains(SegmentDirection::LEFT))
			WallSegments.Add(SegmentDirection::LEFT, AddStaticMeshComponent(W, WallMesh, FName(W->GetName() + "_Segment_Left")));

		if (!WallSegments.Contains(SegmentDirection::RIGHT))
			WallSegments.Add(SegmentDirection::RIGHT, AddStaticMeshComponent(W, WallMesh, FName(W->GetName() + "_Segment_Right")));

		if (!WallSegments.Contains(SegmentDirection::BOTTOM))
			WallSegments.Add(SegmentDirection::BOTTOM, AddStaticMeshComponent(W, WallMesh, FName(W->GetName() + "_Segment_Bottom")));

		// Destroy old object and create new door mesh
		if (IsValid(W->Object) && W->Object->IsValidLowLevel()) 
			W->Object->DestroyComponent();
		W->Object = AddStaticMeshComponent(W, WindowMesh, FName(W->GetName() + "_Window"));

		// Get door mesh dimensions
		auto WindowDimensions = ObjDimensions[WallType::WITH_WINDOW];
		auto WindowWidth  = WindowDimensions.Y - 5.3f; //NOTE: вычитаю, чтобы убрать зазор у окна
		auto WindowHeight = WindowDimensions.Z;

		// Set wall length
		auto WallLength = Width + WallOffset;
		if (Direction == WallDirection::WEST || Direction == WallDirection::EAST)
			WallLength = Length + WallOffset;

		// Set transform for wall sections and window
		WallSegments[SegmentDirection::LEFT]->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
		WallSegments[SegmentDirection::LEFT]->SetRelativeScale3D(FVector(1.0, W->leftAligment, Height));

		// For window
		W->Object->SetRelativeLocation(FVector(WindowOffset, W->leftAligment, WindowHeightOffset));

		WallSegments[SegmentDirection::RIGHT]->SetRelativeLocation(FVector(0.0, W->leftAligment + WindowWidth, 0.0));
		WallSegments[SegmentDirection::RIGHT]->SetRelativeScale3D(FVector(1.0, WallLength - (W->leftAligment + WindowWidth), Height));

		WallSegments[SegmentDirection::BOTTOM]->SetRelativeLocation(FVector(0.0, W->leftAligment, 0.0));
		WallSegments[SegmentDirection::BOTTOM]->SetRelativeScale3D(FVector(1.0, WindowWidth, WindowHeightOffset));

		WallSegments[SegmentDirection::TOP]->SetRelativeLocation(FVector(0.0, W->leftAligment, WindowHeightOffset + WindowHeight));
		WallSegments[SegmentDirection::TOP]->SetRelativeScale3D(FVector(1.0, WindowWidth, Height - (WindowHeightOffset + WindowHeight)));
	}
	
	// Set transform for wall components
	switch (Direction)
	{
	case WallDirection::NORTH:
		W->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(0.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
		W->SetRelativeLocation(FVector(Length, 0.0, 0.0));
		break;
	case WallDirection::SOUTH:
		W->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(180.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
		W->SetRelativeLocation(FVector(0.0, Width, 0.0));
		break;
	case WallDirection::WEST:
		W->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(-90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
		W->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
		break;
	case WallDirection::EAST:
		W->SetWorldRotation(GetActorRotation().Vector().RotateAngleAxis(90.0f, FVector(0.0, 0.0, 1.0f)).Rotation());
		W->SetRelativeLocation(FVector(Length, Width, 0.0));
		break;
	}
}

void ARoom::UpdateAllSegments()
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
		auto& W = p.Value;

		// Update aligments
		SetLeftAligment(Direction, W->leftAligment);
	}
}

void ARoom::SetWallType(WallDirection Direction, WallType type)
{
	auto W = Walls[Direction];

	W->Type = type;

	// Update aligments
	SetLeftAligment(Direction, W->leftAligment);
}

void ARoom::SetLeftAligment(WallDirection Direction, int value)
{
	auto W = Walls[Direction];

	if (W->Type != WallType::EMPTY)
	{
		auto ObjWidth = ObjDimensions[W->Type].Y;

		auto WallWidth = (Direction == WallDirection::NORTH || Direction == WallDirection::SOUTH) ? Width : Length;
		W->leftAligment = FMath::Clamp((float)value, AligmentOffset, WallWidth - AligmentOffset - ObjWidth);
		W->rightAligment = WallWidth - W->leftAligment - ObjWidth;
	}

	UpdateSegments(Direction);
}

void ARoom::SetRightAligment(WallDirection Direction, int value)
{
	auto W = Walls[Direction];

	auto ObjWidth = ObjDimensions[W->Type].Y;

	auto WallWidth = (Direction == WallDirection::NORTH || Direction == WallDirection::SOUTH) ? Width : Length;
	W->rightAligment = FMath::Clamp((float)value, AligmentOffset, WallWidth - AligmentOffset - ObjWidth);
	W->leftAligment = WallWidth - W->rightAligment - ObjWidth;

	UpdateSegments(Direction);
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

	// AddInstanceComponent(Obj);

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


