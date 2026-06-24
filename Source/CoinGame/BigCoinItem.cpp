#include "BigCoinItem.h"

ABigCoinItem::ABigCoinItem()
{
	// 상위 CoinItem의 PointValue를 50으로 덮어쓰기
	PointValue = 50;
	ItemType = "BigCoin";
	
	//회전속도추가함
	RotationSpeed = 90.0f;
}

void ABigCoinItem::ActivateItem(AActor* Activator)
{
	// 부모의 기본 점수 획득 로직 사용
	Super::ActivateItem(Activator);

	// 빅 코인만의 추가 동작(이펙트, 사운드 재생 등)을 여기서 추가할 수 있음
}

//회전기능추가함
void ABigCoinItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FMath::IsNearlyZero(RotationSpeed))
	{
		AddActorLocalRotation(FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f));
	}
}