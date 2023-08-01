#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LocationBox.generated.h"

UENUM(BlueprintType)
enum class ELocationType : uint8 {
  Location_Area UMETA(DisplayName = "Area"),
  Location_Point UMETA(DisplayName = "Point")
};

USTRUCT(BlueprintType)
struct FNeighborData {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Location Data")
  ALocationBox *Neighbor;

  UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Location Data")
  float Distance;
};

UCLASS()
class MBOGI_API ALocationBox : public AActor {
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ALocationBox();

  UPROPERTY(VisibleAnywhere, Category = "Box Collision")
  class UBoxComponent *BoxCollision;

  UPROPERTY(EditAnywhere, Category = "Location Data")
  FString LocationName;

  UPROPERTY(EditAnywhere, Category = "Location Data")
  ELocationType LocationType;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Location Data")
  TMap<FString, FNeighborData> Neighbors;

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Neighbors")
  void UpdateNeighborDistances();

  UPROPERTY(EditAnywhere, Category = "Visual")
  TSubclassOf<AActor> BP_SplineTool_Tiedtke_Wires_Class;

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  // Sets the target location
  UFUNCTION(BlueprintCallable, Category = "Pathfinding")
  void SetTargetLocation(ALocationBox *NewTargetLocation);

  // Returns the path to the target location
  UFUNCTION(BlueprintCallable, Category = "Pathfinding")
  TArray<ALocationBox *> GetPathToTarget();

  // DestroyActorDelayed destroys specified actor after specified delay
  UFUNCTION(BlueprintCallable, Category = "Pathfinding")
  void DestroyActorDelayed(AActor *ActorToDestroy);

private:
  void CalculateDistances();

  float Heuristic(ALocationBox *Other);

  // The target location
  UPROPERTY()
  ALocationBox *TargetLocation;

  // FindPathTo
  TArray<ALocationBox *> FindPathTo(ALocationBox *Goal);

  UFUNCTION()
  void OnOverlapBegin(class UPrimitiveComponent *OverlappedComp,
                      class AActor *OtherActor,
                      class UPrimitiveComponent *OtherComp,
                      int32 OtherBodyIndex, bool bFromSweep,
                      const FHitResult &SweepResult);
};