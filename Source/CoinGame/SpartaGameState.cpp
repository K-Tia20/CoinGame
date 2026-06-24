#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "SpartaPlayerController.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

ASpartaGameState::ASpartaGameState()
{
		Score = 0;
		SpawnedCoinCount = 0;
		CollectedCoinCount = 0;
		LevelDuration = 30.0f;
		CurrentLevelIndex = 0;
		MaxLevels = 3;
		CurrentWaveIndex = 0;
		MaxWaves = 3;
		WaveDurations = {60.0f, 40.0f, 30.0f};
		WaveSpawnLimits = {3, 2, 1};
		CurrentWaveSpawnCount = 0;
}

void ASpartaGameState::BeginPlay()
{
		Super::BeginPlay();
	
		FString CurrentMapName = GetWorld()->GetMapName();
		if (!CurrentMapName.Contains("MenuLevel"))
		{
				StartLevel();
		}
	
		GetWorldTimerManager().SetTimer(
				HUDUpdateTimerHandle,
				this,
				&ASpartaGameState::UpdateHUD,
				0.1f,
				true
		);
}

int32 ASpartaGameState::GetScore() const
{
		return Score;
}

void ASpartaGameState::AddScore(int32 Amount)
{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
				USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
				if (SpartaGameInstance)
				{
						SpartaGameInstance->AddToScore(Amount);
				}
		}
}

void ASpartaGameState::StartLevel()
{
		if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
				if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
				{
						SpartaPlayerController->ShowGameHUD();
				}
		}
	
		if (UGameInstance* GameInstance = GetGameInstance())
		{
				USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
				if (SpartaGameInstance)
				{
						CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
				}
		}

		CurrentWaveIndex = 0;
		StartWave();
}

void ASpartaGameState::StartWave()
{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Level %d Wave %d 시작!"), CurrentLevelIndex + 1, CurrentWaveIndex + 1));

		// 이전 웨이브의 남은 코인들 정리
		TArray<AActor*> FoundCoins;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACoinItem::StaticClass(), FoundCoins);
		for (AActor* Coin : FoundCoins)
		{
				Coin->Destroy();
		}

		CurrentWaveSpawnCount = 0;

		if (WaveDurations.IsValidIndex(CurrentWaveIndex))
		{
				LevelDuration = WaveDurations[CurrentWaveIndex];
		}
		else
		{
				LevelDuration = 30.0f;
		}

		SpawnCoins();

		GetWorldTimerManager().SetTimer(
			LevelTimerHandle,
			this,
			&ASpartaGameState::OnLevelTimeUp,
			LevelDuration,
			false
		);
}

void ASpartaGameState::SpawnCoins()
{
		CurrentWaveSpawnCount++;

		TArray<AActor*> FoundVolumes;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

		const int32 ItemToSpawn = 40;
		
		SpawnedCoinCount = 0;
		CollectedCoinCount = 0;

		if (FoundVolumes.Num() > 0)
		{
				ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
				if (SpawnVolume)
				{
						for (int32 i = 0; i < ItemToSpawn; i++)
						{
								AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
								if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
								{
										SpawnedCoinCount++;
								}
						}
				}
		}

		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("이번 회차 스폰된 코인: %d"), SpawnedCoinCount));
}

void ASpartaGameState::OnLevelTimeUp()
{
		EndWave();
}

void ASpartaGameState::OnCoinCollected()
{
		CollectedCoinCount++;
		
		if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
		{
				if (WaveSpawnLimits.IsValidIndex(CurrentWaveIndex) && CurrentWaveSpawnCount < WaveSpawnLimits[CurrentWaveIndex])
				{
						SpawnCoins();
				}
				else
				{
						EndWave();
				}
		}
}

void ASpartaGameState::EndWave()
{
		GetWorldTimerManager().ClearTimer(LevelTimerHandle);

		CurrentWaveIndex++;
		if (CurrentWaveIndex >= MaxWaves)
		{
				EndLevel();
		}
		else
		{
				StartWave();
		}
}

void ASpartaGameState::EndLevel()
{
		GetWorldTimerManager().ClearTimer(LevelTimerHandle);
	
		if (UGameInstance* GameInstance = GetGameInstance())
		{
				USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
				if (SpartaGameInstance)
				{
						AddScore(Score);
						CurrentLevelIndex++;
						SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;
			
						if (CurrentLevelIndex >= MaxLevels)
						{
								OnGameOver();
								return;
						}
			
						if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
						{
								UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
						}
						else
						{
								OnGameOver();
						}
				}
		}
}

void ASpartaGameState::OnGameOver()
{
		if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
				if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
				{
						SpartaPlayerController->SetPause(true);
						SpartaPlayerController->ShowMainMenu(true);
				}
		}
}

void ASpartaGameState::UpdateHUD()
{
		if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
				ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController);
				{
						if (UUserWidget* HUDWidget = SpartaPlayerController->GetHUDWidget())
						{
								if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
								{
										float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
										TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
								}
				
								if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
								{
										if (UGameInstance* GameInstance = GetGameInstance())
										{
												USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
												if (SpartaGameInstance)
												{
														ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
												}
										}
								}
				
								if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
								{
										LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1)));
								}
								
								if (UTextBlock* WaveIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Wave"))))
								{
										WaveIndexText->SetText(FText::FromString(FString::Printf(TEXT("Wave: %d"), CurrentWaveIndex + 1)));
								}
								
								if (UTextBlock* SpawnCountText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Spawn"))))
								{
										SpawnCountText->SetText(FText::FromString(FString::Printf(TEXT("Spawn: %d"), CurrentWaveSpawnCount)));
								}
						}
				}
		}
}