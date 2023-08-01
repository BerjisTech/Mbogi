// Fill out your copyright notice in the Description page of Project Settings.

#include "LocationBox.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "NavigationHelperComponent.h"
#include <vcruntime.h>

// Sets default values
ALocationBox::ALocationBox() {
  // Set this actor to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  // Create the BoxComponent and attach it to the root component
  BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
  RootComponent = BoxCollision;

  // Set up the overlap event
  BoxCollision->OnComponentBeginOverlap.AddDynamic(
      this, &ALocationBox::OnOverlapBegin);
}

// Called when the game starts or when spawned
void ALocationBox::BeginPlay() {
  Super::BeginPlay();

  CalculateDistances();
}

// Called every frame
void ALocationBox::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
}

void ALocationBox::CalculateDistances() {
  for (auto &Element : Neighbors) {
    FNeighborData &Data = Element.Value;
    if (Data.Neighbor) {
      Data.Distance = FVector::Distance(GetActorLocation(),
                                        Data.Neighbor->GetActorLocation());
    }
  }
}

void ALocationBox::UpdateNeighborDistances() {
  for (auto &NeighborPair : Neighbors) {
    ALocationBox *Neighbor = NeighborPair.Value.Neighbor;
    if (Neighbor && Neighbor != this) {
      float CalculatedDistance =
          FVector::Dist(GetActorLocation(), Neighbor->GetActorLocation());
      NeighborPair.Value.Distance = CalculatedDistance;
    }
  }
}

void ALocationBox::OnOverlapBegin(UPrimitiveComponent *OverlappedComp,
                                  AActor *OtherActor,
                                  UPrimitiveComponent *OtherComp,
                                  int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult &SweepResult) {
  // Print string other actor name
  // Print other actor name to the screen
  GEngine->AddOnScreenDebugMessage(
      -1, 5.f, FColor::Blue,
      FString::Printf(TEXT("Other actor name: %s"), *OtherActor->GetName()));

  //  Print string LocationType
  UEnum *LocationTypeEnum = StaticEnum<ELocationType>();
  FString LocationTypeString =
      LocationTypeEnum
          ->GetDisplayNameTextByValue(static_cast<int32>(LocationType))
          .ToString();
  GEngine->AddOnScreenDebugMessage(
      -1, 5.f, FColor::Green,
      FString::Printf(TEXT("LocationType: %s"), *LocationTypeString));

  switch (LocationType) {
  case ELocationType::Location_Area:
    // TODO: Display the location on the HUD
    break;

  case ELocationType::Location_Point: {
    // Get the NavigationHelperComponent from the other actor
    UNavigationHelperComponent *NavigationHelper =
        Cast<UNavigationHelperComponent>(OtherActor->GetComponentByClass(
            UNavigationHelperComponent::StaticClass()));

    if (NavigationHelper) {
      // Get the target location from the NavigationHelperComponent
      ALocationBox *TargetLocationHelper = NavigationHelper->TargetLocation;

      if (TargetLocationHelper != NULL) {

        // Calculate the new path from this location to the target location
        TArray<ALocationBox *> NewPath = FindPathTo(TargetLocationHelper);

        // Set the new path
        NavigationHelper->Path = NewPath;
        // Debug print paths
        FString DebugMessage =
            FString::Printf(TEXT("Path from %s to %s:"), *LocationName,
                            *TargetLocationHelper->LocationName);
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, DebugMessage);
      } else {
        FString DebugMessage = TEXT("TargetLocationHelper is null");
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, DebugMessage);
      }
    }
    break;
  }
  }
}

float ALocationBox::Heuristic(ALocationBox *Other) {
  if (Other == nullptr) {
    // Handle error, log a warning, or return a default value
    GEngine->AddOnScreenDebugMessage(
        -1, 5.f, FColor::Red, TEXT("Other is null in Heuristic function"));
    return 0.0f;
  } else {
    GEngine->AddOnScreenDebugMessage(
        -1, 5.f, FColor::Red, TEXT("Other is not null in Heuristic function"));
  }
  return FVector::Distance(GetActorLocation(), Other->GetActorLocation());
}

void ALocationBox::SetTargetLocation(ALocationBox *NewTargetLocation) {
  TargetLocation = NewTargetLocation;
}

TArray<ALocationBox *> ALocationBox::GetPathToTarget() {
  if (TargetLocation) {
    return FindPathTo(TargetLocation);
  }

  return TArray<ALocationBox *>();
}

void ALocationBox::DestroyActorDelayed(AActor *ActorToDestroy) {
  if (ActorToDestroy) {
    ActorToDestroy->Destroy();
  }
}

TArray<ALocationBox *> ALocationBox::FindPathTo(ALocationBox *Goal) {
  // The set of nodes already evaluated
  TSet<ALocationBox *> ClosedSet;

  // The set of currently discovered nodes that are not evaluated yet
  TSet<ALocationBox *> OpenSet;
  OpenSet.Add(this);

  // The map of navigated nodes
  TMap<ALocationBox *, ALocationBox *> CameFrom;

  // The cost of getting from start to that node
  TMap<ALocationBox *, float> GScore;
  GScore.Add(this, 0.0f);

  // The total cost of getting from the start to the goal through that node
  TMap<ALocationBox *, float> FScore;
  FScore.Add(this, Heuristic(Goal));

  while (OpenSet.Num() > 0) {
    // Get the node in OpenSet having the lowest FScore[] value
    ALocationBox *Current = nullptr;
    for (ALocationBox *Node : OpenSet) {
      float *CurrentFScore = FScore.Find(Current);
      float *NodeFScore = FScore.Find(Node);

      if (Current == nullptr ||
          (NodeFScore && (!CurrentFScore || *NodeFScore < *CurrentFScore))) {
        Current = Node;
      }
    }

    if (Current == Goal) {
      // Reconstruct and return the found path
      TArray<ALocationBox *> Path;
      while (CameFrom.Contains(Current)) {
        Path.Insert(Current, 0);
        Current = CameFrom[Current];
      }
      Path.Insert(Current, 0);

      // Spawn BP_SplineTool_Tiedtke_Wires at each location in the path
      for (ALocationBox *Location : Path) {
        FTransform Transform(Location->GetActorLocation());
        AActor *SpawnedActor = GetWorld()->SpawnActor<AActor>(
            BP_SplineTool_Tiedtke_Wires_Class, Transform);

        // Set a timer to destroy the actor after 10 seconds
        FTimerHandle TimerHandle;
        FTimerDelegate TimerDel;
        TimerDel.BindUFunction(this, FName("DestroyActorDelayed"),
                               SpawnedActor);
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 10.0f,
                                               false);
      }

      // Construct the path string for debugging
      FString PathString = TEXT("Path: ");
      for (ALocationBox *Location : Path) {
        PathString += Location->GetName() + TEXT(" -> ");
      }

      // Display the path on screen
      GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, PathString);

      return Path;
    }

    OpenSet.Remove(Current);
    ClosedSet.Add(Current);

    // For each neighbor of Current
    for (auto &Element : Current->Neighbors) {
      ALocationBox *Neighbor = Element.Value.Neighbor;
      if (ClosedSet.Contains(Neighbor)) {
        continue;
      }

      // The distance from start to a neighbor
      float TentativeGScore = GScore[Current] + Element.Value.Distance;
      float *CurrentGScore = GScore.Find(Neighbor);

      if (!OpenSet.Contains(Neighbor)) {
        OpenSet.Add(Neighbor);
      } else if (CurrentGScore && TentativeGScore >= *CurrentGScore) {
        continue;
      }

      CameFrom.Add(Neighbor, Current);
      GScore.Add(Neighbor, TentativeGScore);
      FScore.Add(Neighbor, GScore[Neighbor] + Heuristic(Neighbor));
    }
  }

  // Return an empty array if there is no path
  GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                                   TEXT("No path found!"));
  return TArray<ALocationBox *>();
}
