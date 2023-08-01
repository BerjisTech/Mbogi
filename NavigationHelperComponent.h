// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "LocationBox.h"
#include "NavigationHelperComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MBOGI_API UNavigationHelperComponent : public UActorComponent {
  GENERATED_BODY()

public:
  // Sets default values for this component's properties
  UNavigationHelperComponent();

  // Target Location
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Navigation")
  ALocationBox *TargetLocation;

  // Path
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Navigation")
  TArray<ALocationBox *> Path;

  // TODO: Add your methods here

protected:
  // Called when the game starts
  virtual void BeginPlay() override;

public:
  // Called every frame
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;
};