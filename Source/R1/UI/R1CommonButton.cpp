


#include "UI/R1CommonButton.h"
#include "Components/TextBlock.h"

void UR1CommonButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (ButtonName)
	{
		ButtonName->SetText(ButtonText);
	}
}

