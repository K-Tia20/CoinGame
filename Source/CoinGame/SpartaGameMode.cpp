#include "SpartaGameMode.h"
#include "SpartaCharacter.h"
#include "SpartaPlayerController.h"
#include "SpartaGameState.h"

ASpartaGameMode::ASpartaGameMode()
{
	PlayerControllerClass = ASpartaPlayerController::StaticClass();
	DefaultPawnClass = ASpartaCharacter ::StaticClass();
	// 우리가 만든 GameState로 설정
	GameStateClass = ASpartaGameState::StaticClass();
}