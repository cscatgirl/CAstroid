#include <rcade.h>
#include <emscripten.h>

const int CANVAS_WIDTH = 336;
const int CANVAS_HEIGHT = 262;
const float PADDLE_WIDTH = 60.0f;
const float PADDLE_HEIGHT = 8.0f;
const float PADDLE_Y = 242.0f;
const float PADDLE_SENSITIVITY = 1.5f;

struct Ball {
    float x, y;
    float dx, dy;
    float radius = 4.0f;
};

struct GameState {
    float paddleX;
    Ball ball;
    bool gameStarted;
} game;

rcade::Canvas* canvas;
rcade::Input* input;

void resetBall() {
    game.ball.x = CANVAS_WIDTH / 2.0f;
    game.ball.y = CANVAS_HEIGHT / 2.0f;
    game.ball.dx = 2.0f;
    game.ball.dy = -3.0f;
}

void updateGame() {
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

    // Reset if ball falls off screen
    if (game.ball.y > CANVAS_HEIGHT) {
        resetBall();
        game.gameStarted = false;
    }
}

void renderGame() {
    canvas->clear("#1a1a2e");

    // Draw paddle
    canvas->fillRect(game.paddleX - PADDLE_WIDTH / 2.0f, PADDLE_Y,
                     PADDLE_WIDTH, PADDLE_HEIGHT, "#eee");

    // Draw ball
    canvas->fillRect(game.ball.x - game.ball.radius,
                     game.ball.y - game.ball.radius,
                     game.ball.radius * 2, game.ball.radius * 2, "#fff");

    if (!game.gameStarted) {
        canvas->fillText("Press A to start", CANVAS_WIDTH / 2, 131,
                        "12px monospace", "#eee", "center");
        canvas->fillText("Use spinner to move paddle", CANVAS_WIDTH / 2, 150,
                        "10px monospace", "#888", "center");
    }
}

void gameLoop() {
    updateGame();
    renderGame();
}

int main() {
    // Initialize game state
    game.paddleX = CANVAS_WIDTH / 2.0f;
    game.gameStarted = false;
    resetBall();

    canvas = new rcade::Canvas(CANVAS_WIDTH, CANVAS_HEIGHT);
    input = new rcade::Input();

    emscripten_set_main_loop(gameLoop, 60, 1);
    return 0;
}
