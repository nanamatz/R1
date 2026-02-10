#include "Character/R1Character.h"
#include "Components/WidgetComponent.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "UI/R1HpBarWidget.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"

// Sets default values
AR1Character::AR1Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//HpBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	//HpBarComponent->SetupAttachment(GetRootComponent());

	//ConstructorHelpers::FClassFinder<UUserWidget> HealthBarWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/WBP_HpBar.WBP_HpBar_C'"));
	//if (HealthBarWidgetClass.Succeeded())
	//{
	//	HpBarComponent->SetWidgetClass(HealthBarWidgetClass.Class);
	//	HpBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	//	HpBarComponent->SetDrawAtDesiredSize(true);
	//	HpBarComponent->SetRelativeLocation(FVector(0, 0, 120));
	//}

}

// Called when the game starts or when spawned
void AR1Character::BeginPlay()
{
	Super::BeginPlay();
	//InitHpAndMp();	
	AddCharacterAbility();
}

// Called every frame
void AR1Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AR1Character::HandleGameplayEvent(FGameplayTag EventTag)
{

}

void AR1Character::Highlight()
{
	bHighlighted = true;
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(250);
}

void AR1Character::UnHighlight()
{
	bHighlighted = false;
	GetMesh()->SetRenderCustomDepth(false);
}

void AR1Character::OnDamaged(int32 Damage, TObjectPtr<AR1Character> Attacker)
{
	float Hp = AttributeSet->GetHealth();
	float MaxHp = AttributeSet->GetMaxHealth();


	Hp = FMath::Clamp(Hp - Damage, 0, MaxHp);
	AttributeSet->SetHealth(Hp);
	
	float Ratio = static_cast<float>(Hp) / MaxHp;
	OnHealthChanged(Ratio);

	if (Hp == 0)
	{
		OnDead(Attacker);
	}
	//RefreshHpRatio();
}


void AR1Character::OnDead(TObjectPtr<AR1Character> Attacker)
{
	CreatureState = ECreatureState::Dead;
}

void AR1Character::InitHpAndMp()
{
	if (AttributeSet)
	{
		AttributeSet->InitHealth(AttributeSet->GetHealth());
		AttributeSet->InitMana(AttributeSet->GetMana());
	}
}

UAbilitySystemComponent* AR1Character::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AR1Character::InitAbilitySystem()
{

}

void AR1Character::RegenerateHealth()
{
}

void AR1Character::OnHealthChanged(float Ratio)
{
	// 값이 변하면 델리게이트를 통해 UI들에게 알림!
	if(OnHpChanged.IsBound())
	{
		OnHpChanged.Broadcast(Ratio);
	}
	else
	{
		UE_LOG(LogTemp,Warning, TEXT("OnHpChanged is NOT bound for actor: %s"), *GetName());
	}
}

void AR1Character::AddCharacterAbility()
{
	UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(AbilitySystemComponent);
	if (ASC == nullptr)
	{
		return;
	}

	ASC->AddCharacterAbilities(StartupAbilities);

}

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHpChangedDelegate, float, Ratio)
//{
//
//}
