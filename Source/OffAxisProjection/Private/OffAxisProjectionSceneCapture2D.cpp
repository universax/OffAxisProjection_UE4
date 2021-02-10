// Fill out your copyright notice in the Description page of Project Settings.


#include "OffAxisProjectionSceneCapture2D.h"
#include "Engine.h"

void AOffAxisProjectionSceneCapture2D::BeginPlay()
{
	Super::BeginPlay();

	USceneCaptureComponent2D* captureComponent2D = GetCaptureComponent2D();
	if (captureComponent2D)
	{
		captureComponent2D->bUseCustomProjectionMatrix = true;
	}
}

void AOffAxisProjectionSceneCapture2D::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	USceneCaptureComponent2D* captureComponent2D = GetCaptureComponent2D();
	if (captureComponent2D)
	{
		FVector pos = GetActorLocation();
		GEngine->AddOnScreenDebugMessage(10, 1, FColor::Green, FString::Printf(TEXT("SceneCapture2d pos: (%f, %f, %f)"), pos.X, pos.Y, pos.Z));
		eyePosition.X = pos.Y;
		eyePosition.Y = pos.Z;
		eyePosition.Z = pos.X;
		UpdateProjectionMatrix_Internal(captureComponent2D, GenerateOffAxisMatrix(eyePosition));
	}
}

void AOffAxisProjectionSceneCapture2D::SetTargetQuadPoints(FVector _pa, FVector _pb, FVector _pc)
{
	pa = FVector(_pa.Y, _pa.Z, _pa.X);
	pb = FVector(_pb.Y, _pb.Z, _pb.X);
	pc = FVector(_pc.Y, _pc.Z, _pc.X);
}

void AOffAxisProjectionSceneCapture2D::UpdateProjectionMatrix_Internal(USceneCaptureComponent2D* captureComponent2D, FMatrix OffAxisMatrix)
{
	FMatrix stereoProjectionMatrix = OffAxisMatrix;

	FMatrix axisChanger; //rotates everything to UE4 coordinate system.
	axisChanger.SetIdentity();
	axisChanger.M[0][0] = 0.0f;
	axisChanger.M[1][1] = 0.0f;
	axisChanger.M[2][2] = 0.0f;
	axisChanger.M[0][2] = 1.0f;
	axisChanger.M[1][0] = 1.0f;
	axisChanger.M[2][1] = 1.0f;

	//View->ViewRotation = s_ViewRotation;
	FMatrix tmpMat = axisChanger * stereoProjectionMatrix;
	GEngine->AddOnScreenDebugMessage(11, 4, FColor::Red, FString::Printf(TEXT("tmpMat SC2D: %s"), *tmpMat.ToString()));

	FMatrix calcedMatrix = GetProjectionMatrix(captureComponent2D);
	//GEngine->AddOnScreenDebugMessage(16, 4, FColor::Red, FString::Printf(TEXT("calced SC2D: %s"), *calcedMatrix.ToString()));

	if (captureComponent2D)
	{
		//const FTransform& Transform = captureComponent2D->GetComponentToWorld();
		//FMatrix ViewMatrix = Transform.ToInverseMatrixWithScale();
		//ViewMatrix = ViewMatrix * FMatrix(
		//	FPlane(0, 0, 1, 0),
		//	FPlane(1, 0, 0, 0),
		//	FPlane(0, 1, 0, 0),
		//	FPlane(0, 0, 0, 1));
		const FVector curLocation = GetActorLocation();
		FMatrix ViewMatrix = FMatrix(
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(curLocation.X, curLocation.Y, curLocation.Z, 1)
		);

		FMatrix finalMat = ViewMatrix * tmpMat;
		GEngine->AddOnScreenDebugMessage(16, 4, FColor::Red, FString::Printf(TEXT("Transform inv SC2D: %s"), *ViewMatrix.ToString()));
		GEngine->AddOnScreenDebugMessage(18, 4, FColor::Red, FString::Printf(TEXT("Result SC2D: %s"), *finalMat.ToString()));

		captureComponent2D->CustomProjectionMatrix = finalMat;
	}
}




FMatrix AOffAxisProjectionSceneCapture2D::GenerateOffAxisMatrix(FVector _eyeRelativePosition)
{
	FMatrix result;

	float l, r, b, t, n, f, nd;

	n = OffAxisNearPlane;
	f = OffAxisFarPlane;

	//this is analog to: http://csc.lsu.edu/~kooima/articles/genperspective/
	//Careful: coordinate system! y-up, x-right (UE4 uses inverted LHS)

	//pa = lower left, pb = lower right, pc = upper left, eye pos
	pe = FVector(_eyeRelativePosition.X, _eyeRelativePosition.Y, _eyeRelativePosition.Z);

	// Compute the screen corner vectors.
	FVector va, vb, vc;
	va = pa - pe;
	vb = pb - pe;
	vc = pc - pe;

	// Compute an orthonormal basis for the screen.
	FVector vr, vu, vn;
	vr = pb - pa;
	vr.Normalize();
	vu = pc - pa;
	vu.Normalize();
	vn = -FVector::CrossProduct(vr, vu);
	vn.Normalize();

	// Find the distance from the eye to screen plane.
	float d = -FVector::DotProduct(va, vn);

	nd = n / d;

	// Find the extent of the perpendicular projection.
	l = FVector::DotProduct(vr, va) * nd;
	r = FVector::DotProduct(vr, vb) * nd;
	b = FVector::DotProduct(vu, va) * nd;
	t = FVector::DotProduct(vu, vc) * nd;

	// Load the perpendicular projection.
	result = FrustumMatrix(l, r, b, t, n, f);
	//GEngine->AddOnScreenDebugMessage(40, 2, FColor::Red, FString::Printf(TEXT("FrustumMatrix_ORIG: %s"), *result.ToString()));
	GEngine->AddOnScreenDebugMessage(12, 2, FColor::Orange, FString::Printf(TEXT("pa: %s"), *pa.ToString()));
	GEngine->AddOnScreenDebugMessage(13, 2, FColor::Orange, FString::Printf(TEXT("pb: %s"), *pb.ToString()));
	GEngine->AddOnScreenDebugMessage(14, 2, FColor::Orange, FString::Printf(TEXT("pc: %s"), *pc.ToString()));
	GEngine->AddOnScreenDebugMessage(15, 2, FColor::Orange, FString::Printf(TEXT("pe: %s"), *pe.ToString()));
	////GEngine->AddOnScreenDebugMessage(5, 2, FColor::Black, FString::Printf(TEXT("vr: %s"), *vu.ToString()));
	////GEngine->AddOnScreenDebugMessage(6, 2, FColor::Black, FString::Printf(TEXT("vu: %s"), *vr.ToString()));
	////GEngine->AddOnScreenDebugMessage(7, 2, FColor::Black, FString::Printf(TEXT("vn: %s"), *vn.ToString()));
	////GEngine->AddOnScreenDebugMessage(8, 4, FColor::Red, FString::Printf(TEXT("Frustum: %f \t %f \t %f \t %f \t %f \t %f \t "), l, r, b, t, n, f));
	GEngine->AddOnScreenDebugMessage(19, 2, FColor::Orange, FString::Printf(TEXT("Eye-Screen-Distance SC2D: %f"), d));
	//GEngine->AddOnScreenDebugMessage(10, 4, FColor::Orange, FString::Printf(TEXT("nd: %f"), nd));

	//Move the apex of the frustum to the origin.
	result = FTranslationMatrix(-pe) * result;

	//scales matrix for UE4 and RHI
	result *= 1.0f / result.M[0][0];

	result.M[2][2] = 0.f; //?
	result.M[3][2] = n; //?

	return result;
}

FMatrix AOffAxisProjectionSceneCapture2D::FrustumMatrix(float left, float right, float bottom, float top, float nearVal, float farVal)
{
	//column-major order
	FMatrix Result;
	Result.SetIdentity();
	Result.M[0][0] = (2.0f * nearVal) / (right - left);
	Result.M[1][1] = (2.0f * nearVal) / (top - bottom);
	Result.M[2][0] = -(right + left) / (right - left);
	Result.M[2][1] = -(top + bottom) / (top - bottom);
	Result.M[2][2] = (farVal) / (farVal - nearVal);
	Result.M[2][3] = 1.0f;
	Result.M[3][2] = -(farVal * nearVal) / (farVal - nearVal);
	Result.M[3][3] = 0.0f;

	return Result;
}

FMatrix AOffAxisProjectionSceneCapture2D::GetProjectionMatrix(USceneCaptureComponent2D* captureComponent2D)
{
	UTextureRenderTarget2D* RenderTexture = captureComponent2D->TextureTarget;
	if (RenderTexture)
	{
		FMinimalViewInfo Info;
		Info.Location = captureComponent2D->GetComponentTransform().GetLocation();
		Info.Rotation = captureComponent2D->GetComponentTransform().GetRotation().Rotator();
		Info.FOV = captureComponent2D->FOVAngle;
		Info.ProjectionMode = captureComponent2D->ProjectionType;
		Info.AspectRatio = float(RenderTexture->SizeX) / float(RenderTexture->SizeY);
		Info.OrthoNearClipPlane = 0.1f;   // not quit shure where to get this...
		Info.OrthoFarClipPlane = 10000;
		Info.bConstrainAspectRatio = true;

		return Info.CalculateProjectionMatrix();
	}
	//GEngine->AddOnScreenDebugMessage(12, 4, FColor::Red, FString::Printf(TEXT("TextureTarget is Null")));
	return FMatrix();
}
