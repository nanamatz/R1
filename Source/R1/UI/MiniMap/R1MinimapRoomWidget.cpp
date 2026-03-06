


#include "UI/MiniMap/R1MinimapRoomWidget.h"
#include "Components/Image.h"

void UR1MinimapRoomWidget::UpdateRoomState(ER1MinimapRoomState NewState, ER1RoomContentType RoomType)
{
	if (!Image_Background || !Image_Icon) return;

	// 초기화 (기본적으로 아이콘은 숨김)
	ESlateVisibility IconVisibility = ESlateVisibility::Hidden;

	switch (NewState)
	{
	case ER1MinimapRoomState::Hidden:
		// 미발견: 배경 컴포넌트 자체를 아예 숨겨버립니다.
		Image_Background->SetVisibility(ESlateVisibility::Hidden);
		break;

	case ER1MinimapRoomState::Discovered:
		// 발견됨: 어두운 배경 텍스처 사용
		Image_Background->SetVisibility(ESlateVisibility::Visible);
		if (BgTexture_Discovered)
		{
			Image_Background->SetBrushFromTexture(BgTexture_Discovered);
		}

		if (RoomIconMap.Contains(RoomType) && RoomIconMap[RoomType] != nullptr)
		{
			IconVisibility = ESlateVisibility::SelfHitTestInvisible;
			Image_Icon->SetBrushFromTexture(RoomIconMap[RoomType]);
		}
		break;

	case ER1MinimapRoomState::Visited:
		// 방문 완료: 중간 밝기 배경 텍스처 사용
		Image_Background->SetVisibility(ESlateVisibility::Visible);
		if (BgTexture_Visited)
		{
			Image_Background->SetBrushFromTexture(BgTexture_Visited);
		}

		if (RoomIconMap.Contains(RoomType) && RoomIconMap[RoomType] != nullptr)
		{
			IconVisibility = ESlateVisibility::SelfHitTestInvisible;
			Image_Icon->SetBrushFromTexture(RoomIconMap[RoomType]);
		}
		break;

	case ER1MinimapRoomState::Current:
		// 현재 위치: 가장 밝은 배경 텍스처 사용
		Image_Background->SetVisibility(ESlateVisibility::Visible);
		if (BgTexture_Current)
		{
			Image_Background->SetBrushFromTexture(BgTexture_Current);
		}
		if (RoomIconMap.Contains(RoomType) && RoomIconMap[RoomType] != nullptr)
		{
			IconVisibility = ESlateVisibility::SelfHitTestInvisible;
			Image_Icon->SetBrushFromTexture(RoomIconMap[RoomType]);
		}
		break;
	}

	Image_Icon->SetVisibility(IconVisibility);
}
