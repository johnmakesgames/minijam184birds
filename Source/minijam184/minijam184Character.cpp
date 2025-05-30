// Copyright Epic Games, Inc. All Rights Reserved.

#include "minijam184Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


//////////////////////////////////////////////////////////////////////////
// Aminijam184Character

Aminijam184Character::Aminijam184Character()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.55f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void Aminijam184Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void Aminijam184Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (isJumping)
	{
		jumpHeldTime += DeltaSeconds;

		if (jumpHeldTime >= 1.0f)
		{
			StartGentleFall();
		}
	}

	dashTimer -= DeltaSeconds;

	//UE_LOG(LogTemp, Warning, TEXT("Remaining Jumps %d"), remainingJumps);
	if (remainingJumps < maxJumps)
	{
		jumpRechargeTimer += DeltaSeconds;
		//UE_LOG(LogTemp, Warning, TEXT("Recharge Timer %f"), jumpRechargeTimer);
		if (jumpRechargeTimer >= jumpRechargeTime)
		{
			remainingJumps++;
			jumpRechargeTimer = 0;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Score %d"), score)

	if (Controller != nullptr)
	{
		if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying)
		{
			const FVector UpDirection = FollowCamera->GetComponentRotation().Vector();

			if (UpDirection.Z < -0.5f)
			{
				glideSpeed += FMath::Abs(UpDirection.Z);
			}
			else
			{
				glideSpeed -= FMath::Abs(UpDirection.Z) * 0.75f;
			}

			UE_LOG(LogTemp, Error, TEXT("Speed %f"), glideSpeed);

			if (glideSpeed < 0)
			{
				glideSpeed = 0;
			}
			else
			{
				AddMovementInput(UpDirection, glideSpeed);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void Aminijam184Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &Aminijam184Character::Move);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &Aminijam184Character::AirDash);
		EnhancedInputComponent->BindAction(GlideAction, ETriggerEvent::Triggered, this, &Aminijam184Character::StartGlide);
		EnhancedInputComponent->BindAction(GlideAction, ETriggerEvent::Completed, this, &Aminijam184Character::StopGlide);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &Aminijam184Character::Look);

	}

}

void Aminijam184Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// add movement 
		if (GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Flying)
		{
			// get forward vector
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			// get right vector 
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void Aminijam184Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}

	flyingDirection.Pitch += LookAxisVector.Y;
	flyingDirection.Yaw += LookAxisVector.X;
}

bool Aminijam184Character::CanJumpInternal_Implementation() const
{
	return jumpHeldTime < 1.0f;
}

void Aminijam184Character::Jump()
{
	if (remainingJumps > 0)
	{
		if (!isJumping)
		{
			isJumping = true;
		}

		Super::Jump();
	}
}

void Aminijam184Character::StopJumping()
{
	isJumping = false;
	remainingJumps = FMath::Max(0, remainingJumps -1);
	jumpHeldTime = 0;
	StopGentleFall();
	Super::StopJumping();
}

void Aminijam184Character::StartGentleFall()
{
	GetCharacterMovement()->GravityScale = 0.15f;
}

void Aminijam184Character::StopGentleFall()
{
	GetCharacterMovement()->GravityScale = 1.0f;
}

void Aminijam184Character::StartGlide()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	//flyingDirection = GetActorForwardVector();

	//APlayerController* PC = Cast<APlayerController>(Controller);
	//PC->SetIgnoreLookInput(false);
}

void Aminijam184Character::StopGlide()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

	APlayerController* PC = Cast<APlayerController>(Controller);
	glideSpeed = 0;
	//PC->SetIgnoreLookInput(true);
}

void Aminijam184Character::AirDash()
{
	UE_LOG(LogTemp, Warning, TEXT("Dash Timer %f"), dashTimer);

	if (dashTimer < 0.0f)
	{
		GetCharacterMovement()->AddImpulse(GetActorForwardVector() * 100000, false);
		dashTimer = 5.0f;
	}
}
