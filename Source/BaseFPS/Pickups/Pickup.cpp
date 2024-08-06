// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/Pickup.h"

#include "BaseFPS.h"
#include "PickupInstance.h"

// Sets default values
APickup::APickup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	bNetLoadOnClient = false;
	
	Billboard = ObjectInitializer.CreateDefaultSubobject<UBillboardComponent>(this, TEXT("Root"));
	Billboard->SetSprite(LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/S_Actor.S_Actor")));
	Billboard->SetRelativeScale3D(FVector(1.0f));
	Billboard->SetHiddenInGame(true);
	RootComponent = Billboard;

	bIsActive = false;
	bDeferredSpawn = false;
	bIsRespawning = true;
	bOverrideRespawnTime = false;
	RespawnTimeOverride = 30.0f;
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		Activate();
	}
}


void APickup::Activate()
{
	if (PickupType)
	{
		bIsActive = true;
		RespawnTime = PickupType.GetDefaultObject()->GetRespawnTime();
		RespawnTime = bOverrideRespawnTime ||  FMath::IsNearlyZero(RespawnTime) ? RespawnTimeOverride : RespawnTime;
		if (!bDeferredSpawn || FMath::IsNearlyZero(RespawnTime))
		{
			SpawnPickup();
		}
		else
		{
			GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &APickup::SpawnPickup, RespawnTime, false);
		}
	}
	else
	{
		Deactivate();
	}
	
}

void APickup::Deactivate()
{
	bIsActive = false;
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
}

void APickup::SpawnPickup()
{
	if (HasAuthority() && bIsActive && PickupType)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (APickupInstance* Pickup = GetWorld()->SpawnActor<APickupInstance>(PickupType, GetActorLocation(), GetActorRotation(), SpawnParams))
		{
			Pickup->SetOwner(this);
			if (bIsRespawning)
			{
				Pickup->OnDestroyed.AddDynamic(this, &APickup::APickup::OnPickupDestroyed);
			}
			else
			{
				Deactivate();
			}
		}
	}
}

void APickup::OnPickupDestroyed(AActor* DestroyedPickup)
{
	if (bIsActive)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &APickup::SpawnPickup, RespawnTime, false);
	}
}

/************************************************************************/
/* Editor Only                                                          */
/************************************************************************/

#if WITH_EDITOR

void APickup::PreEditUndo()
{
	RemoveComponentTree(EditorMesh);
	EditorMesh = nullptr;
	Super::PreEditUndo();
}

void APickup::PostEditUndo()
{
	Super::PostEditUndo();
	PickupTypeChanged();
}

void APickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		// responsible for mesh creation on editor startup
		PickupTypeChanged();
	}
}

void APickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// responsible for mesh creation after editing actor
	if (const FProperty* PropertyChanged = PropertyChangedEvent.Property)
	{
		if (PropertyChanged->GetName() == FString(TEXT("PickupType")))
		{
			PickupTypeChanged();
		}
	}
	
}

void APickup::PickupTypeChanged()
{
	CreateEditorMesh();
	Billboard->SetVisibility(EditorMesh == nullptr);
}

void APickup::CreateEditorMesh()
{
	CreatePickupMesh(this, EditorMesh, PickupType);

	if (EditorMesh)
	{
		EditorMesh->bIsEditorOnly = true;
		EditorMesh->SetHiddenInGame(true);

		// switch to ghost material to represent deferred spawn
		if (bDeferredSpawn && DeferredSpawnMaterial)
		{
			for (int i = 0; i < EditorMesh->GetNumMaterials(); i++)
			{
				EditorMesh->SetMaterial(i, DeferredSpawnMaterial);
			}

			// FIXME (aleforte) should use recursive approach to set child materials
			TArray<USceneComponent*> Attachments = EditorMesh->GetAttachChildren();
			for (USceneComponent* Attached : Attachments)
			{
				if (UMeshComponent* Mesh = Cast<UMeshComponent>(Attached))
				{
					for (int i = 0; i < Mesh->GetNumMaterials(); i++)
					{
						Mesh->SetMaterial(i, DeferredSpawnMaterial);
					}
				}
			}
		}
	}
}

#endif

/************************************************************************/
/* Utilities                                                            */
/************************************************************************/

void APickup::CreatePickupMesh(AActor* PickupActor, UMeshComponent*& MeshPtr, TSubclassOf<APickupInstance> PickupType)
{
	if (!PickupType)
	{
		RemoveComponentTree(MeshPtr);
		MeshPtr = nullptr;
	}
	else
	{
		UMeshComponent* NewMesh = PickupType.GetDefaultObject()->GetEditorMeshTemplate();
		if (NewMesh == nullptr)
		{
			RemoveComponentTree(MeshPtr);
			MeshPtr = nullptr;
		}
		else
		{
			if (MeshPtr != nullptr)
			{
				RemoveComponentTree(MeshPtr);
				MeshPtr = nullptr;
			}

			UE_LOG(LogBaseFPS, Warning, TEXT("Creating new editor mesh (Actor=%s)..."), *PickupActor->GetName());
			MeshPtr = Cast<UMeshComponent>(CreateAndRegisterFromTemplate(PickupActor, NewMesh, PickupActor->GetRootComponent()));
			
			// collect components from native (C++) class
			TArray<USceneComponent*> NativeCompList;
			PickupType.GetDefaultObject()->GetComponents(NativeCompList);
			// collect components from blueprint (BP) class
			TArray<USCS_Node*> ConstructionNodes;
			{
				TArray<const UBlueprintGeneratedClass*> ParentBPClassStack;
				UBlueprintGeneratedClass::GetGeneratedClassesHierarchy(PickupType, ParentBPClassStack);
				for (int32 i = ParentBPClassStack.Num() - 1; i >= 0; i--)
				{
					const UBlueprintGeneratedClass* CurrentBPGClass = ParentBPClassStack[i];
					if (CurrentBPGClass->SimpleConstructionScript)
					{
						ConstructionNodes += CurrentBPGClass->SimpleConstructionScript->GetAllNodes();
					}
				}
			}
			CreatePickupMeshAttachments(PickupActor, PickupType, MeshPtr, NewMesh->GetFName(), NativeCompList, ConstructionNodes);
		}
	}
}

USceneComponent* APickup::CreateAndRegisterFromTemplate(AActor* PickupActor, USceneComponent* Template, USceneComponent* AttachToComponent)
{
	// create & setup new pickup mesh
	USceneComponent* NewComp = NewObject<UMeshComponent>(PickupActor, Template->GetClass(), NAME_None, RF_NoFlags, Template);
	NewComp->SetShouldUpdatePhysicsVolume(false);
	NewComp->bUseAttachParentBound = false;
	NewComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	NewComp->SetHiddenInGame(true);
	NewComp->bIsEditorOnly = true;
	if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(NewComp))
	{
		SkelMesh->SetForceRefPose(true);
	}

	// removing children (added separately)
	TArray<USceneComponent*> Children = NewComp->GetAttachChildren();
	for (USceneComponent* Child: Children)
	{
		Child->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	}

	// register mesh as new component
	NewComp->bAutoRegister = false;
	NewComp->RegisterComponent();
	NewComp->AttachToComponent(AttachToComponent, FAttachmentTransformRules::KeepRelativeTransform, NewComp->GetAttachSocketName());

	// handle collision setup last, seems to only work after registration
	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(NewComp))
	{
		PrimComp->SetSimulatePhysics(false);
		PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	return NewComp;
}

void APickup::CreatePickupMeshAttachments(AActor* PickupActor, TSubclassOf<APickupInstance> PickupType,
	USceneComponent* CurrentAttachment, FName TemplateName, const TArray<USceneComponent*>& NativeCompList,
	const TArray<USCS_Node*>& BPNodes)
{
	for (int32 i = 0; i < NativeCompList.Num(); i++)
	{
		if (NativeCompList[i]->GetAttachParent() == CurrentAttachment)
		{
			// create & register new component
			USceneComponent* NewComp = CreateAndRegisterFromTemplate(PickupActor, NativeCompList[i], CurrentAttachment);
			// recurse to add child attachments
			CreatePickupMeshAttachments(PickupActor, PickupType, NewComp, NativeCompList[i]->GetFName(), NativeCompList, BPNodes);
		}
	}
	for (int32 i = 0; i < BPNodes.Num(); i++)
	{
		USceneComponent* ComponentTemplate = Cast<USceneComponent>(BPNodes[i]->GetActualComponentTemplate(Cast<UBlueprintGeneratedClass>(PickupType)));
		if (BPNodes[i]->ComponentTemplate != nullptr && BPNodes[i]->ParentComponentOrVariableName == TemplateName)
		{
			// create & register new component
			USceneComponent* NewComp = CreateAndRegisterFromTemplate(PickupActor, Cast<USceneComponent>(BPNodes[i]->ComponentTemplate), CurrentAttachment);
			// recurse to add child attachments
			CreatePickupMeshAttachments(PickupActor, PickupType, NewComp, BPNodes[i]->GetVariableName(), NativeCompList, BPNodes);
		}
	}
}

