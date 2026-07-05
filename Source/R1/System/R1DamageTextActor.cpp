#include "System/R1DamageTextActor.h"
#include "Components/WidgetComponent.h"
#include "UI/R1DamageTextWidget.h"

AR1DamageTextActor::AR1DamageTextActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	DamageTextWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageTextWidgetComp"));
	DamageTextWidgetComp->SetupAttachment(RootComp);
	DamageTextWidgetComp->SetWidgetSpace(EWidgetSpace::Screen); // Float in 3D but face screen
	DamageTextWidgetComp->SetDrawSize(FVector2D(250.0f, 100.0f));

	static ConstructorHelpers::FClassFinder<UR1DamageTextWidget> WidgetAsset(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/System/Combat/WBP_DamageText.WBP_DamageText_C'"));
	if (WidgetAsset.Succeeded())
	{
		DamageTextWidgetComp->SetWidgetClass(WidgetAsset.Class);
	}
}

void AR1DamageTextActor::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSpanSeconds);
}

void AR1DamageTextActor::SetDamageInfo(const FR1DamageInfo& DamageInfo)
{
	if (UR1DamageTextWidget* DamageWidget = Cast<UR1DamageTextWidget>(DamageTextWidgetComp->GetUserWidgetObject()))
	{
		DamageWidget->SetDamageInfo(DamageInfo);
	}
}
