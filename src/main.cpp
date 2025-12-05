#include <rcade.h>
#include <emscripten.h>
#include <cmath>
#include <string>

const int CANVAS_WIDTH = 336;
const int CANVAS_HEIGHT = 262;
const float PADDLE_WIDTH = 60.0f;
const float PADDLE_HEIGHT = 8.0f;
const float PADDLE_Y = 242.0f;
const float PADDLE_SENSITIVITY = 1.5f;

// Brick constants
const int BRICK_ROWS = 6;
const int BRICK_COLS = 10;
const float BRICK_WIDTH = 30.0f;
const float BRICK_HEIGHT = 10.0f;
const float BRICK_PADDING = 3.0f;
const float BRICK_OFFSET_TOP = 30.0f;
const float BRICK_OFFSET_LEFT = 18.0f;

struct Ball {
    float x, y;
    float dx, dy;
    float radius = 4.0f;
};

struct Brick {
    float x, y;
    bool active;
    int row; // Used for color
};

struct GameState {
    float paddleX;
    Ball ball;
    Brick bricks[BRICK_ROWS * BRICK_COLS];
    int score;
    int lives;
    bool gameStarted;
    bool gameOver;
    bool gameWon;
} game;

rcade::Canvas* canvas;
rcade::Input* input;

// Brick colors by row
const char* BRICK_COLORS[] = {
    "#e74c3c", // Red
    "#e67e22", // Orange
    "#f39c12", // Yellow
    "#2ecc71", // Green
    "#3498db", // Blue
    "#9b59b6"  // Purple
};

void initBricks() {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            int idx = row * BRICK_COLS + col;
            game.bricks[idx].x = BRICK_OFFSET_LEFT + col * (BRICK_WIDTH + BRICK_PADDING);
            game.bricks[idx].y = BRICK_OFFSET_TOP + row * (BRICK_HEIGHT + BRICK_PADDING);
            game.bricks[idx].active = true;
            game.bricks[idx].row = row;
        }
    }
}

void resetBall() {
    game.ball.x = CANVAS_WIDTH / 2.0f;
    game.ball.y = CANVAS_HEIGHT / 2.0f;
    game.ball.dx = 2.0f;
    game.ball.dy = -3.0f;
}

void resetGame() {
    game.paddleX = CANVAS_WIDTH / 2.0f;
    game.score = 0;
    game.lives = 3;
    game.gameStarted = false;
    game.gameOver = false;
    game.gameWon = false;
    resetBall();
    initBricks();
}

void updateGame() {
    if (game.gameOver || game.gameWon) {
        // Restart game on button press
        if (input->getPlayer1().A) {
            resetGame();
        }
        return;
    }

    if (!game.gameStarted) {
        // Start game on button press
        if (input->getPlayer1().A) {
            game.gameStarted = true;
        }
        return;
    }

    // Update paddle position with spinner
    int spinnerDelta = input->getPlayer1().SPINNER.getStepDelta();
    game.paddleX += spinnerDelta * PADDLE_SENSITIVITY;

    // Clamp paddle to screen
    float halfWidth = PADDLE_WIDTH / 2.0f;
    if (game.paddleX < halfWidth) game.paddleX = halfWidth;
    if (game.paddleX > CANVAS_WIDTH - halfWidth) game.paddleX = CANVAS_WIDTH - halfWidth;

    // Update ball
    game.ball.x += game.ball.dx;
    game.ball.y += game.ball.dy;

    // Ball collision with walls
    if (game.ball.x <= game.ball.radius || game.ball.x >= CANVAS_WIDTH - game.ball.radius) {
        game.ball.dx = -game.ball.dx;
    }
    if (game.ball.y <= game.ball.radius) {
        game.ball.dy = -game.ball.dy;
    }

    // Ball collision with paddle
    if (game.ball.y + game.ball.radius >= PADDLE_Y &&
        game.ball.y - game.ball.radius <= PADDLE_Y + PADDLE_HEIGHT) {
        if (game.ball.x >= game.paddleX - halfWidth &&
            game.ball.x <= game.paddleX + halfWidth) {
            game.ball.dy = -abs(game.ball.dy);

            // Add spin based on hit position
            float hitPos = (game.ball.x - game.paddleX) / halfWidth;
            game.ball.dx += hitPos * 1.5f;
        }
    }

    // Ball collision with bricks
    bool bricksRemaining = false;
    for (int i = 0; i < BRICK_ROWS * BRICK_COLS; i++) {
        if (!game.bricks[i].active) continue;

        bricksRemaining = true;

        Brick& brick = game.bricks[i];

        // Check if ball is overlapping with brick
        if (game.ball.x + game.ball.radius >= brick.x &&
            game.ball.x - game.ball.radius <= brick.x + BRICK_WIDTH &&
            game.ball.y + game.ball.radius >= brick.y &&
            game.ball.y - game.ball.radius <= brick.y + BRICK_HEIGHT) {

            // Deactivate brick
            brick.active = false;

            // Update score (higher rows = more points)
            game.score += (BRICK_ROWS - brick.row) * 10;

            // Calculate which side was hit
            float ballCenterX = game.ball.x;
            float ballCenterY = game.ball.y;
            float brickCenterX = brick.x + BRICK_WIDTH / 2.0f;
            float brickCenterY = brick.y + BRICK_HEIGHT / 2.0f;

            float dx = ballCenterX - brickCenterX;
            float dy = ballCenterY - brickCenterY;

            // Determine bounce direction based on which side was hit
            if (abs(dx / BRICK_WIDTH) > abs(dy / BRICK_HEIGHT)) {
                // Hit from left or right
                game.ball.dx = -game.ball.dx;
            } else {
                // Hit from top or bottom
                game.ball.dy = -game.ball.dy;
            }

            break; // Only process one brick collision per frame
        }
    }

    // Check win condition
    if (!bricksRemaining) {
        game.gameWon = true;
    }

    // Reset if ball falls off screen
    if (game.ball.y > CANVAS_HEIGHT) {
        game.lives--;
        if (game.lives <= 0) {
            game.gameOver = true;
        } else {
            resetBall();
            game.gameStarted = false;
        }
    }
}

void renderGame() {
    canvas->clear("#1a1a2e");

    // Draw bricks
    for (int i = 0; i < BRICK_ROWS * BRICK_COLS; i++) {
        if (game.bricks[i].active) {
            canvas->fillRect(game.bricks[i].x, game.bricks[i].y,
                           BRICK_WIDTH, BRICK_HEIGHT,
                           BRICK_COLORS[game.bricks[i].row]);
        }
    }

    // Draw paddle
    canvas->fillRect(game.paddleX - PADDLE_WIDTH / 2.0f, PADDLE_Y,
                     PADDLE_WIDTH, PADDLE_HEIGHT, "#eee");

    // Draw ball
    canvas->fillRect(game.ball.x - game.ball.radius,
                     game.ball.y - game.ball.radius,
                     game.ball.radius * 2, game.ball.radius * 2, "#fff");

    // Draw score and lives
    std::string scoreText = "Score: " + std::to_string(game.score);
    canvas->fillText(scoreText.c_str(), 10, 15, "10px monospace", "#eee", "left");

    std::string livesText = "Lives: " + std::to_string(game.lives);
    canvas->fillText(livesText.c_str(), CANVAS_WIDTH - 10, 15, "10px monospace", "#eee", "right");

    if (game.gameWon) {
        canvas->fillRect(0, 100, CANVAS_WIDTH, 80, "#1a1a2e");
        canvas->fillText("YOU WIN!", CANVAS_WIDTH / 2, 121,
                        "24px monospace", "#2ecc71", "center");
        std::string finalScore = "Final Score: " + std::to_string(game.score);
        canvas->fillText(finalScore.c_str(), CANVAS_WIDTH / 2, 145,
                        "12px monospace", "#eee", "center");
        canvas->fillText("Press A to play again", CANVAS_WIDTH / 2, 165,
                        "10px monospace", "#888", "center");
    } else if (game.gameOver) {
        canvas->fillRect(0, 100, CANVAS_WIDTH, 80, "#1a1a2e");
        canvas->fillText("GAME OVER", CANVAS_WIDTH / 2, 121,
                        "24px monospace", "#e74c3c", "center");
        std::string finalScore = "Final Score: " + std::to_string(game.score);
        canvas->fillText(finalScore.c_str(), CANVAS_WIDTH / 2, 145,
                        "12px monospace", "#eee", "center");
        canvas->fillText("Press A to play again", CANVAS_WIDTH / 2, 165,
                        "10px monospace", "#888", "center");
    } else if (!game.gameStarted) {
        canvas->fillRect(0, 100, CANVAS_WIDTH, 70, "#1a1a2e");
        canvas->fillText("Press A (or F key) to start", CANVAS_WIDTH / 2, 121,
                        "12px monospace", "#eee", "center");
        canvas->fillText("Use spinner to move paddle", CANVAS_WIDTH / 2, 140,
                        "10px monospace", "#888", "center");
        canvas->fillText("(Arrow keys for keyboard)", CANVAS_WIDTH / 2, 155,
                        "9px monospace", "#666", "center");
    }
}

void gameLoop() {
    updateGame();
    renderGame();
}

int main() {
    canvas = new rcade::Canvas(CANVAS_WIDTH, CANVAS_HEIGHT);
    input = new rcade::Input();

    resetGame();

    emscripten_set_main_loop(gameLoop, 60, 1);
    return 0;
}
