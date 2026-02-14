#include "Character/R1Character.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "GameplayEffect.h"
#include "DataTable/CharacterStatsRow.h"

// Sets default values
AR1Character::AR1Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AR1Character::BeginPlay()
{
	Super::BeginPlay();
	AddCharacterAbility();
	ApplyCharacterEffect();
}

// Called every frame
void AR1Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AR1Character::HandleGameplayEvent(FGameplayTag EventTag)
{
	if(AbilitySystemComponent)
	{
		FGameplayEventData EventData;
		EventData.EventTag = EventTag;
		EventData.Instigator = this;

		AbilitySystemComponent->HandleGameplayEvent(EventTag,&EventData);
	}
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

	//OnHealthChanged(Ratio);//이 부분을 뺴고 나중에 GameplayEffect로 대신해줘야 함.

	if (Hp == 0)
	{
		OnDead(Attacker);
	}
	//RefreshHpRatio();
}


void AR1Character::OnDead(TObjectPtr<AR1Character> Attacker)
{
	if (CreatureState == ECreatureState::Dead)
	{
		return;
	}

	SetCreatureState(ECreatureState::Dead);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OnDeadDelegate.Broadcast(this, Attacker.Get());
}

UAbilitySystemComponent* AR1Character::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AR1Character::InitAbilitySystem()
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



void AR1Character::SetCreatureState(ECreatureState InState)
{
	CreatureState = InState;

	if (CreatureState != ECreatureState::Dead)
	{
		return;
	}

	// GAS 컴포넌트 가져오기
	UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(GetAbilitySystemComponent());

	if (ASC)
	{
		// 1. 죽었을 때: Dead 태그 부착
		if (CreatureState == ECreatureState::Dead)
		{
			// AddLooseGameplayTag: 코드로 강제로 태그를 붙이는 함수
			ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Dead")));
		}
		// 2. 살았을 때: Dead 태그 제거
		else
		{
			ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Dead")));
		}
	}
}

void AR1Character::InitAttributes()
{
	if (!AbilitySystemComponent || !CharacterStatTable || !InitStatEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitAttributes: Missing ASC, Table, or GE Class!"));
		return;
	}

	FR1CharacterStatsRow* StatData = CharacterStatTable->FindRow<FR1CharacterStatsRow>(CharacterRowName, TEXT("InitAttributes"));

	if (StatData)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitStatEffectClass, 1.0f, Context);

		if (SpecHandle.IsValid())
		{
			// 3. [핵심] SetByCaller로 값 주입하기
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.MaxHealth")),
				StatData->MaxHealth
			);

			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.MaxMana")),
				StatData->MaxMana
			);

			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.BaseDamage")),
				StatData->BaseDamage
			);

			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.BaseDefence")),
				StatData->BaseDefence
			);

			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.AttackRange")),
				StatData->AttackRange
			);

			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.AttackRadius")),
				StatData->AttackRadius
			);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.AggroRange")),
				StatData->AggroRange
			);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.MaxExp")),
				StatData->MaxExp
			);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.Xp")),
				StatData->Xp
			);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Attribute.Level")),
				StatData->Level
			);

			// 4. 내용물이 채워진 GE를 나 자신에게 적용
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	if (!StatData)
	{
		// 여기가 뜨면 RowName 오타 (Player vs player 등)
		UE_LOG(LogTemp, Error, TEXT("InitAttributes: '%s' 행을 찾을 수 없습니다!"), *CharacterRowName.ToString());
		return;
	}

	// 4. 값 확인
	UE_LOG(LogTemp, Warning, TEXT("데이터 찾음! 레벨: %f, 경험치 통: %f, 경험치: %f,공격력: %f, 체력: %f"), StatData->Level,StatData->MaxExp,StatData->Xp,StatData->BaseDamage, StatData->MaxHealth);

}


void AR1Character::InitializeCharacterAttribute()
{
	//TODO
}

void AR1Character::ApplyCharacterEffect()
{
	//TODO
	UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(AbilitySystemComponent);
	if (ASC == nullptr)
	{
		return;
	}
	ASC->ApplyCharacterEffects(StartupEffects);
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
