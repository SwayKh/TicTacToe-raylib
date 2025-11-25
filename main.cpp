// ============================================================================
// Header File Documentation
// ============================================================================
// raylib.h is the main header file of the Raylib library, which provides:
// - Window creation and management
// - 2D rendering (textures, shapes, text)
// - Input handling (mouse, keyboard)
// - Audio management (sound loading and playback)
// - Mathematical types (Vector2, Color, etc.)
// This file must be included before using any Raylib functions.
#include "raylib.h"

// ============================================================================
// ENUM DECLARATIONS
// ============================================================================

// -----------------------------------------------------------------------------
// enum SceneName
// -----------------------------------------------------------------------------
// Represents the different screens (scenes) of the game.
// Values:
//   SCENE_MENU    → Main Menu screen
//   SCENE_GAME    → Game screen where Tic-Tac-Toe is played
//   SCENE_CREDITS → Credits screen
enum SceneName { SCENE_MENU = 1, SCENE_GAME = 2, SCENE_CREDITS = 3 };

// -----------------------------------------------------------------------------
// enum Player
// -----------------------------------------------------------------------------
// Represents the state of each tile (cell) on the Tic-Tac-Toe board.
// Values:
//   EMPTY    → tile not taken yet
//   PLAYER_X → tile contains X
//   PLAYER_O → tile contains O
enum Player { EMPTY = 0, PLAYER_X = 1, PLAYER_O = 2 };

// ============================================================================
// STRUCT: GameState
// ============================================================================
// Holds **all mutable game state** used during gameplay and scene transitions.
// This allows all functions to receive & modify shared game data cleanly.
// ----------------------------------------------------------------------------
//
// MEMBER VARIABLES:
//
// scene         → current screen (menu/game/credits)
// darkMode      → bool flag for dark or light theme
// pressed       → bool flag for click debouncing to prevent multi-input
// board[9]      → integer array storing the 3×3 grid
// gameOver      → indicates whether the round has ended
// turn          → whose turn it is (PLAYER_X / PLAYER_O)
// winner        → winner of the round (1,2) or draw (3)
// mousePos      → stores latest mouse cursor position
//
struct GameState {
  SceneName scene = SCENE_MENU;
  bool darkMode = false;
  bool pressed = false;

  int board[9] = {0};
  bool gameOver = false;
  int turn = PLAYER_X;
  int winner = 0;

  Vector2 mousePos;
};

// ============================================================================
// STRUCT: Assets
// ============================================================================
// Holds **all loaded textures and sounds** used in the game.
// Centralizing assets prevents scattering textures/sounds across the program.
//
// MEMBER VARIABLES:
//   bgLight / bgDark       → background images for themes
//   menuTitleLight/Dark    → title images for menu
//   buttonLight/Dark       → button textures
//   tileBlank/X/O          → board tile textures
//   sndPress / sndPlace / sndWin → audio effects
//
struct Assets {
  Texture2D bgLight, bgDark;
  Texture2D menuTitleLight, menuTitleDark;
  Texture2D buttonLight, buttonDark;
  Texture2D tileBlank, tileX, tileO;

  Sound sndPress, sndPlace, sndWin;
};

// ============================================================================
// FUNCTION: LoadAssets
// ============================================================================
// ============= Objective =============
// Load all textures and sound effects used in the game at startup.
//
// ============= Input Parameters =============
// None.
//
// ============= Output =============
// Returns an Assets struct containing all loaded textures/sounds.
//
// ============= Return Value =============
// Assets → holds all game textures and sound buffers
//
// ============= Side Effects =============
// - Loads textures from disk into GPU memory.
// - Loads sound data into RAM.
// - If files are missing or corrupt, raylib may crash or show missing textures.
//
// ============= Approach =============
// A single function returns a fully populated struct, making it easier to pass
// assets around all scenes and keep code organized.
//
Assets LoadAssets() {
  Assets A;

  // Load theme backgrounds
  A.bgLight = LoadTexture("resources/BackgroundLight.png");
  A.bgDark = LoadTexture("resources/BackgroundDark.png");

  // Load menu titles
  A.menuTitleLight = LoadTexture("resources/MenuTitleLight.png");
  A.menuTitleDark = LoadTexture("resources/MenuTitleDark.png");

  // Load button skins
  A.buttonLight = LoadTexture("resources/ButtonLight.png");
  A.buttonDark = LoadTexture("resources/ButtonDark.png");

  // Load tile graphics
  A.tileBlank = LoadTexture("resources/BlankTile.png");
  A.tileX = LoadTexture("resources/Cross.png");
  A.tileO = LoadTexture("resources/Circle.png");

  // Load sound effects
  A.sndPress = LoadSound("resources/BtnPress.wav");
  A.sndPlace = LoadSound("resources/Place.wav");
  A.sndWin = LoadSound("resources/Win.wav");

  return A;
}

// ============================================================================
// FUNCTION: ResetBoard
// ============================================================================
// ============= Objective =============
// Reset the Tic-Tac-Toe board to a clean empty state before a new game starts.
//
// ============= Input Parameters =============
// GameState &G → reference to game state that must be reset.
//
// ============= Output =============
// The state G is modified directly.
//
// ============= Return Value =============
// None.
//
// ============= Side Effects =============
// - Clears all board cells
// - Resets turn order, winner state, and gameOver flag
//
// ============= Approach =============
// Iterate through the 9 cells and mark each as EMPTY. Reset all relevant flags.
//
void ResetBoard(GameState &G) {
  for (int i = 0; i < 9; i++)
    G.board[i] = EMPTY;

  G.turn = PLAYER_X;
  G.winner = 0;
  G.gameOver = false;
}

// ============================================================================
// FUNCTION: CheckWinner
// ============================================================================
// ============= Objective =============
// Determine whether either player (X or O) has won by forming a row, column,
// or diagonal of identical symbols. Also detect draw conditions.
//
// ============= Input Parameters =============
// GameState &G → game state holding board values and winner flags
// const Assets &A → provides access to sound effects for win/draw
//
// ============= Output =============
// Updates G.winner and G.gameOver when a win or draw is detected.
//
// ============= Return Value =============
// None.
//
// ============= Side Effects =============
// - Plays sound A.sndWin on win or draw.
// - Modifies G.board, G.winner, G.gameOver.
//
// ============= Approach =============
// - Define an array containing all 8 possible winning triplets.
// - For each combination, check if the 3 board positions have the same non-zero
// value.
// - If found: set winner and trigger game over.
// - If board is full and no winner exists: declare draw.
// ----------------------------------------------------------------------------
void CheckWinner(GameState &G, const Assets &A) {
  // All 8 winning lines: 3 rows, 3 columns, 2 diagonals.
  const int WINS[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6},
                          {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

  // Loop through each winning combination.
  for (auto &w : WINS) {
    int a = w[0], b = w[1], c = w[2];

    // Check if non-empty AND all three are the same.
    if (G.board[a] != EMPTY && G.board[a] == G.board[b] &&
        G.board[b] == G.board[c]) {
      // Set winner to X or O depending on tile value.
      G.winner = G.board[a];
      G.gameOver = true;

      // Play win sound.
      PlaySound(A.sndWin);
      return;
    }
  }

  // Check draw condition: board full but no winner.
  bool full = true;
  for (int i = 0; i < 9; i++)
    if (G.board[i] == EMPTY)
      full = false;

  if (full) {
    G.winner = 3; // 3 means draw
    G.gameOver = true;
    PlaySound(A.sndWin);
  }
}

// ============================================================================
// FUNCTION: HandleGameInput
// ============================================================================
// ============= Objective =============
// Handle tile-clicking input from the user during gameplay.
// Allows players to place X or O on the board.
//
// ============= Input Parameters =============
// GameState &G → reference to game state for board/turn updates
// const Assets &A → contains placement sound effect
//
// ============= Output =============
// Directly modifies G.board, G.turn, G.pressed.
//
// ============= Return Value =============
// None.
//
// ============= Side Effects =============
// - Plays tile placement sound.
// - Mutates global game state.
// - Changes turn order.
//
// ============= Approach =============
// - Prevent repeated clicks using G.pressed (debouncing).
// - Detect which tile was clicked by matching mouse position to tile bounds.
// - If clicked tile is empty: place symbol, switch turn.
// - Call CheckWinner() to update game status.
// ----------------------------------------------------------------------------
void HandleGameInput(GameState &G, const Assets &A) {
  // Clicking too quickly? Debounce.
  if (G.pressed)
    return;

  // Must be an actual click.
  if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    return;

  // Do not accept clicks after game ended.
  if (G.gameOver)
    return;

  // Precomputed tile X positions for 3 columns.
  float xs[3] = {12.5f, 112.5f, 212.5f};

  // Precomputed tile Y positions for 3 rows.
  float ys[3] = {62.5f, 162.5f, 262.5f};

  // Loop through each cell of the 3×3 board.
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++) {
      int idx = r * 3 + c;

      float x = xs[c];
      float y = ys[r];

      // Check if mouse is inside this tile rectangle.
      if (G.mousePos.x >= x && G.mousePos.x <= x + 75 && G.mousePos.y >= y &&
          G.mousePos.y <= y + 75 && G.board[idx] == EMPTY) {
        // Place X or O.
        G.board[idx] = G.turn;

        // Play tile placement sound.
        PlaySound(A.sndPlace);

        // Switch turn.
        G.turn = (G.turn == PLAYER_X ? PLAYER_O : PLAYER_X);

        // Check if this move wins the game.
        CheckWinner(G, A);

        // Mark click as consumed.
        G.pressed = true;
        return;
      }
    }
}

// ============================================================================
// FUNCTION: DrawBoard
// ============================================================================
// ============= Objective =============
// Render the 3×3 grid of Tic-Tac-Toe tiles with their current states.
//
// ============= Input Parameters =============
// GameState &G → provides board layout
// const Assets &A → provides tile textures
//
// ============= Output =============
// Draws textures to the screen.
//
// ============= Return Value =============
// None.
//
// ============= Side Effects =============
// - Renders textures directly to GPU buffer.
//
// ============= Approach =============
// - Convert board indices (0–8) into row/column.
// - Compute drawing offsets.
// - Draw texture depending on tile state.
// ----------------------------------------------------------------------------
void DrawBoard(GameState &G, const Assets &A) {
  // Precomputed offsets relative to screen center.
  float startX[3] = {-137.5f, -37.5f, 62.5f};
  float startY[3] = {-137.5f, -37.5f, 62.5f};

  // Draw each of the 9 tiles.
  for (int i = 0; i < 9; i++) {
    int r = i / 3; // row index
    int c = i % 3; // column index

    float x = GetScreenWidth() / 2 + startX[c];
    float y = GetScreenHeight() / 2 + startY[r];

    // Draw empty tile.
    if (G.board[i] == EMPTY)
      DrawTexture(A.tileBlank, x, y, WHITE);

    // Draw X.
    else if (G.board[i] == PLAYER_X)
      DrawTexture(A.tileX, x, y, MAROON);

    // Draw O.
    else
      DrawTexture(A.tileO, x, y, BLUE);
  }
}

// ============================================================================
// FUNCTION: DrawGameScene
// ============================================================================
// ============= Objective =============
// Render the entire game screen: background, turn indicator, grid lines,
// tiles, and "Play Again" button if game is finished.
//
// ============= Input Parameters =============
// GameState &G → contains board, turn, winner, theme mode
// const Assets &A → provides textures/sounds for rendering
//
// ============= Output =============
// Draws complete game scene to screen.
//
// ============= Return Value =============
// None.
//
// ============= Side Effects =============
// - Renders textures and text to GPU buffer.
// - Shows interactive elements (play again button).
//
// ============= Approach =============
// - Draw background depending on dark/light mode.
// - Draw turn text or winner text.
// - Draw grid lines.
// - Render all 9 tiles using DrawBoard().
// ----------------------------------------------------------------------------
void DrawGameScene(GameState &G, const Assets &A) {
  // Draw background theme.
  DrawTexture(G.darkMode ? A.bgDark : A.bgLight, 0, 0, WHITE);

  // ------------------------------------------------------------------------
  // Display turn or winner
  // ------------------------------------------------------------------------

  if (!G.gameOver) {
    // If still playing, show whose turn it is.

    if (G.turn == PLAYER_X)
      DrawText("X turn", 70, 5, 50, G.darkMode ? WHITE : BLACK);
    else
      DrawText("O turn", 70, 5, 50, G.darkMode ? WHITE : BLACK);

  } else {
    // Game over → show result.

    if (G.winner == PLAYER_X)
      DrawText("X wins", 70, 5, 50, G.darkMode ? WHITE : BLACK);

    if (G.winner == PLAYER_O)
      DrawText("O wins", 70, 5, 50, G.darkMode ? WHITE : BLACK);

    if (G.winner == 3)
      DrawText("Draw", 100, 5, 50, G.darkMode ? WHITE : BLACK);

    // Draw "PLAY AGAIN" button graphic.
    DrawTexture(G.darkMode ? A.buttonDark : A.buttonLight, 50, 345, WHITE);

    // Draw text on top of the button.
    DrawText("PLAY AGAIN", 60, 355, 30, G.darkMode ? WHITE : BLACK);
  }

  // ------------------------------------------------------------------------
  // Grid lines
  // ------------------------------------------------------------------------
  // Color depends on theme.
  Color gridColor = G.darkMode ? GRAY : BLACK;

  // Vertical Line 1
  DrawRectangle(GetScreenWidth() / 2 - 55, GetScreenHeight() / 2 - 137, 10, 275,
                gridColor);

  // Vertical Line 2
  DrawRectangle(GetScreenWidth() / 2 + 45, GetScreenHeight() / 2 - 137, 10, 275,
                gridColor);

  // Horizontal Line 1
  DrawRectangle(GetScreenWidth() / 2 - 137, GetScreenHeight() / 2 - 55, 275, 10,
                gridColor);

  // Horizontal Line 2
  DrawRectangle(GetScreenWidth() / 2 - 137, GetScreenHeight() / 2 + 45, 275, 10,
                gridColor);

  // ------------------------------------------------------------------------
  // Draw the 3×3 tiles
  // ------------------------------------------------------------------------
  DrawBoard(G, A);
}

// ============================================================================
// FUNCTION: DrawMenu
// ============================================================================
// ============= Objective =============
// Render the main menu with Play, Dark/Light Mode Toggle, Credits, and Exit.
//
// ============= Input Parameters =============
// GameState &G → for checking theme mode
// const Assets &A → contains menu textures
//
// ============= Output =============
// Draws menu background, title, and buttons.
//
// ============= Return Value =============
// None.
//
// ============= Side Effects =============
// - Visual menu rendering.
//
// ============= Approach =============
// - Draw light/dark theme background.
// - Draw title texture.
// - Draw 4 interactive buttons.
// ----------------------------------------------------------------------------
void DrawMenu(GameState &G, const Assets &A) {
  // Draw appropriate background.
  DrawTexture(G.darkMode ? A.bgDark : A.bgLight, 0, 0, WHITE);

  // Draw menu title based on theme mode.
  DrawTexture(G.darkMode ? A.menuTitleDark : A.menuTitleLight, 0, 0, WHITE);

  // Select appropriate button texture and text color.
  Texture2D btn = G.darkMode ? A.buttonDark : A.buttonLight;
  Color txt = G.darkMode ? WHITE : BLACK;

  // -------------------------------
  // "PLAY" button
  // -------------------------------
  DrawTexture(btn, 50, 100, WHITE);
  DrawText("PLAY", 110, 112, 30, txt);

  // -------------------------------
  // "DARK MODE" or "LIGHT MODE"
  // -------------------------------
  DrawTexture(btn, 50, 175, WHITE);

  if (G.darkMode)
    DrawText("LIGHT MODE", 60, 187, 30, txt);
  else
    DrawText("DARK MODE", 60, 187, 30, txt);

  // -------------------------------
  // "CREDITS" button
  // -------------------------------
  DrawTexture(btn, 50, 250, WHITE);
  DrawText("CREDITS", 85, 262, 30, txt);

  // -------------------------------
  // "EXIT" button
  // -------------------------------
  DrawTexture(btn, 50, 325, WHITE);
  DrawText("EXIT", 113, 337, 30, txt);
}

// ============================================================================
// FUNCTION: HandleMenuInput
// ============================================================================
// ============= Objective =============
// Handle clicks on the Main Menu buttons: Play, Theme Toggle, Credits, Exit.
//
// ============= Input Parameters =============
// GameState &G → modifies selected menu option
// const Assets &A → plays button press sounds
//
// ============= Output =============
// Updates G.scene or G.darkMode, or closes window.
//
// ============= Return Value =============
// None.
//
// ============= Side Effects =============
// - Changes scene
// - Toggles theme
// - Closes the program window
// - Plays click sounds
//
// ============= Approach =============
// - Check if mouse click occurs inside known button rectangles.
// - Perform associated action.
// ----------------------------------------------------------------------------
void HandleMenuInput(GameState &G, const Assets &A) {
  // Only react to actual left-click events.
  if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    return;

  float x = G.mousePos.x;
  float y = G.mousePos.y;

  // -------------------------------
  // PLAY button (50,100)-(250,150)
  // -------------------------------
  if (x >= 50 && x <= 250 && y >= 100 && y <= 150) {
    G.scene = SCENE_GAME;
    PlaySound(A.sndPress);
  }

  // -------------------------------
  // DARK/LIGHT MODE button
  // -------------------------------
  if (x >= 50 && x <= 250 && y >= 175 && y <= 225) {
    G.darkMode = !G.darkMode; // toggle theme
    PlaySound(A.sndPress);
  }

  // -------------------------------
  // CREDITS button
  // -------------------------------
  if (x >= 50 && x <= 250 && y >= 250 && y <= 300) {
    G.scene = SCENE_CREDITS;
    PlaySound(A.sndPress);
  }

  // -------------------------------
  // EXIT button
  // -------------------------------
  if (x >= 50 && x <= 250 && y >= 325 && y <= 375) {
    PlaySound(A.sndPress);
    CloseWindow(); // exits game loop
  }
}

// ============================================================================
// FUNCTION: DrawCredits
// ============================================================================
// ============= Objective =============
// Render the Credits screen displaying acknowledgements and a “BACK” button.
//
// ============= Input Parameters =============
// GameState &G → to determine theme mode
// const Assets &A → for textures used in credits UI
//
// ============= Output =============
// Draws credits text and interactive elements.
//
// ============= Return Value =============
// None.
//
// ============= Side Effects =============
// - Renders textures/text to the active frame.
//
// ============= Approach =============
// - Draw background based on theme.
// - Draw BACK button.
// - Draw credits text.
// ----------------------------------------------------------------------------
void DrawCredits(GameState &G, const Assets &A) {
  // Draw background.
  DrawTexture(G.darkMode ? A.bgDark : A.bgLight, 0, 0, WHITE);

  // Choose button and text color based on theme.
  Texture2D btn = G.darkMode ? A.buttonDark : A.buttonLight;
  Color txt = G.darkMode ? WHITE : BLACK;

  // Draw BACK button.
  DrawTexture(btn, 50, 325, WHITE);
  DrawText("BACK", 113, 337, 30, txt);

  // Credits title.
  DrawText("CREDITS", 58, 5, 40, GRAY);

  // Credit lines.
  DrawText("Sumit, Raghav, Vijay", 30, 55, 20, GRAY);
  DrawText("raylib - Graphics Library", 30, 85, 20, GRAY);

  // Clickable link text: raylib.com
  DrawText("raylib.com", 110, 115, 20, SKYBLUE);

  DrawText("You - Playing the game <3", 30, 145, 20, GRAY);
}

// ============================================================================
// FUNCTION: HandleCreditsInput
// ============================================================================
// ============= Objective =============
// Handle user clicks on the Credits screen, including:
// - Clicking BACK to return to menu
// - Clicking the raylib.com link to open browser
//
// ============= Input Parameters =============
// GameState &G → modified to change scene
// const Assets &A → used to play sound effects
//
// ============= Output =============
// Modifies G.scene depending on user actions.
//
// ============= Return Value =============
// None.
//
// ============= Side Effects =============
// - May open a URL in the default system browser.
// - Changes the active scene.
// - Plays click sounds.
//
// ============= Approach =============
// - Only process left-click events.
// - Check if click lies inside BACK button rectangle.
// - Check if click lies inside raylib.com text area.
// ----------------------------------------------------------------------------
void HandleCreditsInput(GameState &G, const Assets &A) {
  // Only process actual clicks.
  if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    return;

  float x = G.mousePos.x;
  float y = G.mousePos.y;

  // BACK button: returns to menu.
  if (x >= 50 && x <= 250 && y >= 325 && y <= 375) {
    G.scene = SCENE_MENU;
    PlaySound(A.sndPress);
  }

  // Clickable raylib.com link.
  if (x >= 0 && x <= 400 && y >= 115 && y <= 135) {
    OpenURL("https://www.raylib.com/");
    PlaySound(A.sndWin);
  }
}

// ============================================================================
// FUNCTION: main
// ============================================================================
// ============= Objective =============
// The central function that initializes the game, loads assets, enters the
// main game loop, and handles scene switching among:
//   - Main Menu
//   - Game Scene
//   - Credits Scene
//
// ============= Input Parameters =============
// None.
//
// ============= Output =============
// Renders the entire game window frame-by-frame until the user closes it.
//
// ============= Return Value =============
// int → returns 0 on normal, successful program termination.
//
// ============= Side Effects =============
// - Opens a graphical window.
// - Initializes audio hardware.
// - Allocates GPU textures and sound buffers.
// - Runs an infinite loop until CloseWindow() or OS close event occurs.
// - Handles all gameplay, input, drawing, and state transitions.
//
// ============= Approach =============
// - Initialize window & audio.
// - Load all textures/sounds.
// - Initialize GameState struct.
// - Enter main loop:
//       - Read mouse position
//       - BeginDrawing()
//       - Switch based on G.scene
//       - Draw corresponding UI
//       - Process input for that scene
//       - EndDrawing()
// - Exit when WindowShouldClose() becomes true.
// ----------------------------------------------------------------------------
int main() {
  // ------------------------------------------------------------------------
  // Window Initialization
  // ------------------------------------------------------------------------
  // Create the window with fixed width and height.
  InitWindow(300, 400, "Tic Tac Toe");

  // Initialize audio system so sound effects work.
  InitAudioDevice();

  // Limit the game loop to 60 frames per second.
  SetTargetFPS(60);

  // ------------------------------------------------------------------------
  // Create GameState instance to hold dynamic state.
  // ------------------------------------------------------------------------
  GameState G; // Defaults to menu scene, X turn, board empty.

  // ------------------------------------------------------------------------
  // Load all assets (textures & sounds) at startup.
  // ------------------------------------------------------------------------
  Assets A = LoadAssets();

  // =========================================================================
  // MAIN GAME LOOP
  // =========================================================================
  // This loop continues running until:
  // - The user closes the window (clicking X)
  // - Or CloseWindow() is called explicitly (Exit button)
  // WindowShouldClose() queries OS events to know if the window must shut.
  while (!WindowShouldClose()) {
    // ---------------------------------------------------------------------
    // Update: Read current mouse position for this frame.
    // ---------------------------------------------------------------------
    G.mousePos = GetMousePosition();

    // ---------------------------------------------------------------------
    // Begin drawing the new frame.
    // ---------------------------------------------------------------------
    BeginDrawing();

    // ---------------------------------------------------------------------
    // SCENE SWITCHING LOGIC
    // ---------------------------------------------------------------------
    switch (G.scene) {
    // =====================================================================
    // SCENE: MAIN MENU
    // =====================================================================
    case SCENE_MENU:
      // Draw the menu UI (buttons, background, title).
      DrawMenu(G, A);

      // Handle input for menu: Play, Theme Toggle, Credits, Exit.
      HandleMenuInput(G, A);
      break;

    // =====================================================================
    // SCENE: GAMEPLAY SCENE
    // =====================================================================
    case SCENE_GAME:

      // Draw the entire game interface:
      // - background
      // - current turn OR winner text
      // - grid lines
      // - tiles
      DrawGameScene(G, A);

      // --------------------------------------------------------------
      // If the game is still running, accept tile inputs.
      // --------------------------------------------------------------
      if (!G.gameOver) {
        // Detects clicking on tiles and placing X/O.
        HandleGameInput(G, A);

      } else {
        // --------------------------------------------------------------
        // GAME OVER → check for "Play Again" button click.
        // --------------------------------------------------------------
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && G.mousePos.x >= 50 &&
            G.mousePos.x <= 250 && G.mousePos.y >= 345 && G.mousePos.y <= 395) {
          // Reset game board and game state.
          ResetBoard(G);

          // Play click sound.
          PlaySound(A.sndPress);
        }
      }
      break;

    // =====================================================================
    // SCENE: CREDITS
    // =====================================================================
    case SCENE_CREDITS:

      // Draw credits UI (background, text, LINK, back button).
      DrawCredits(G, A);

      // Handle clicks: BACK or raylib.com link.
      HandleCreditsInput(G, A);
      break;
    }

    // ---------------------------------------------------------------------
    // End drawing for the frame. All draw calls now appear on screen.
    // ---------------------------------------------------------------------
    EndDrawing();

    // ---------------------------------------------------------------------
    // CLICK DEBOUNCING RESET
    // ---------------------------------------------------------------------
    // If left mouse button is no longer down,
    // reset G.pressed so new clicks can be registered.
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      G.pressed = false;
  }

  // =========================================================================
  // CLEAN EXIT
  // =========================================================================
  // When the game loop ends, returning 0 signals successful termination.
  return 0;
}
