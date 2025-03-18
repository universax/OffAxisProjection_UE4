// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/SceneCapture2D.h"
#include "OffAxisProjectionSceneCapture2D.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class OFFAXISPROJECTION_API AOffAxisProjectionSceneCapture2D : public ASceneCapture2D
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetTargetQuadPoints", Keywords = "OffAxisProjection SetTargetQuadPoints"), Category = "OffAxisProjection SceneCapture2D")
		void SetTargetQuadPoints(FVector _pa, FVector _pb, FVector _pc);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UpdateProjectionMatrix", Keywords = "OffAxisProjection UpdateProjectionMatrix"), Category = "OffAxisProjection SceneCapture2D")
		void UpdateProjectionMatrix();
	
	void UpdateProjectionMatrix_Internal(USceneCaptureComponent2D* captureComponent2D, FMatrix OffAxisMatrix);
	FMatrix GenerateOffAxisMatrix(FVector _eyeRelativePositon);
	FMatrix FrustumMatrix(float left, float right, float bottom, float top, float nearVal, float farVal);

	FMatrix GetProjectionMatrix(USceneCaptureComponent2D* captureComponent2D);

private:
	float OffAxisNearPlane = .1f;
	float OffAxisFarPlane = 10000.f;

	FVector pa = FVector(80.f, 40.f, 0.f);
	FVector pb = FVector(-80.f, 40.f, 0.f);
	FVector pc = FVector(80.f, 130.f, 0.f);
	FVector pe = FVector::ZeroVector;

	FVector eyePosition = FVector(-90.f, 100.f, 100.f);

	FMatrix finalMat;
};
