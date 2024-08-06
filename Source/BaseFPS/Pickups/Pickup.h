// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USphereComponent;
class APickupInstance;

/**
 * This is essentially a "pickup spawner", but we're calling it a "pickup" for short. During gameplay,
 * this will spawn in {@code PickupInstance} objects for characters to pickup and interact with.
 * Outside of just being an empty transform in the game world, it's responsible for tracking/managing
 * when to spawn it's next pickup.
 *
 * @note for reasons mentioned above, this does not replicate or even load on clients. It is strictly used by
 * host to spawn in objects.
 */
UCLASS(Abstract, Blueprintable)
class BASEFPS_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditInstanceOnly, Category="Pickup")
	TSubclassOf<APickupInstance> PickupType;

	/** if true, this spawner is actively spawning pickup items */
	UPROPERTY(Transient)
	uint8 bIsActive:1;
	
	/** spawn pickup on begin play, or wait a full {@code RespawnTime} before spawning? */
	UPROPERTY(EditInstanceOnly, Category="Pickup")
	uint8 bDeferredSpawn:1;
	
	/** does this pickup respawn, or is it a one-time pickup? */
	UPROPERTY(EditInstanceOnly, Category="Pickup")
	uint8 bIsRespawning:1;

	/** the respawn time used for this pickup, */
	UPROPERTY(Transient)
	float RespawnTime;

	/** handle used to track respawn timer */
	FTimerHandle RespawnTimerHandle;
	
	/** use this actor's default respawn time (true), or use the pickup instance's respawn time (false)? */
	UPROPERTY(EditInstanceOnly, Category="Pickup")
	uint8 bOverrideRespawnTime:1;
	
	/** the pickup instance's respawn time (copied over when set in editor) */
	UPROPERTY(EditAnywhere, Category="Pickup", meta=(EditCondition="bOverrideRespawnTime", EditConditionHides))
	float RespawnTimeOverride;
	
public:
	void Activate();
	void Deactivate();

	UFUNCTION()
	void SpawnPickup();

	/** handles clean up and sets time to respawn another item if active */
	UFUNCTION()
	void OnPickupDestroyed(AActor* DestroyedPickup);

	/************************************************************************/
	/* Visuals                                                              */
	/************************************************************************/
protected:
	/** Billboard used to visualize actors location when pickup is not set */
	UPROPERTY(VisibleDefaultsOnly, Category="Pickup")
	UBillboardComponent* Billboard;
	
	/************************************************************************/
	/* Editor Only                                                          */
	/************************************************************************/
protected:
#if WITH_EDITORONLY_DATA
	
	/** the mesh that represents this pickup in the game world */
	UPROPERTY(Transient)
	UMeshComponent* EditorMesh;
	
	/** material used by the editor mesh to signify this pickup spawn is delayed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Pickup")
	UMaterialInstance* DeferredSpawnMaterial;
	
#endif

#if WITH_EDITOR
	virtual void PreEditUndo() override;
	virtual void PostEditUndo() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void PickupTypeChanged();
	void CreateEditorMesh();
#endif
	
	/************************************************************************/
	/* Utilities                                                            */
	/************************************************************************/
public:
	/**
	 * Static utility function used to create the pickup mesh for a pickup actor
	 * @note don't try to use the pickup mesh's collision anything, just use the mesh for visuals (seems to be a bug in collison system)
	 * @param PickupActor the pickup actor, can be an actual pickup or a pickup spawner 
	 * @param MeshPtr (reference to) the pointer to the PickupActor's mesh
	 * @param PickupType the pickup instance type this actor represents
	 */
	static void CreatePickupMesh(AActor* PickupActor, UMeshComponent*& MeshPtr, TSubclassOf<APickupInstance> PickupType);

private:
	/**
	 * (Internal only) Creates and registers a new component with the provided pickup actor
	 * @param PickupActor the pickup actor, can be an actual pickup or a pickup spawner
	 * @param Template the template the new component is based on
	 * @param AttachToComponent the scene component to attach to
	 * @return pointer to the created/registered actor component
	 */
	static USceneComponent* CreateAndRegisterFromTemplate(AActor* PickupActor, USceneComponent* Template, USceneComponent* AttachToComponent);
	
	/**
	 * Static utility to recursively traverse the component tree and re-create the child meshes attached to a mesh template
	 * @param PickupActor the pickup actor, can be an actual pickup or a pickup spawner 
	 * @param PickupType the pickup instance type this actor represents
	 * @param CurrentAttachment the current mesh template/attachment
	 * @param TemplateName the name of the current mesh template/attachment
	 * @param NativeCompList the list of native components (non-BP)
	 * @param BPNodes the list of BP nodes
	 */
	static void CreatePickupMeshAttachments(AActor* PickupActor, TSubclassOf<APickupInstance> PickupType, USceneComponent* CurrentAttachment, FName TemplateName, const TArray<USceneComponent*>& NativeCompList, const TArray<class USCS_Node*>& BPNodes);
};
