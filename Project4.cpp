#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>

using namespace std;

class TicTacToe {
private:
    char board[3][3];
    char humanMarker;
    char aiMarker;

    void resetBoard() {
        char cell = '1';
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                board[i][j] = cell++;
            }
        }
    }

    void drawBoard() const {
        cout << "\n=========================================\n";
        cout << "            TIC TAC TOE GAME             \n";
        cout << "=========================================\n\n";

        cout << "     |     |     \n";
        cout << "  " << board[0][0] << "  |  " << board[0][1] << "  |  " << board[0][2] << " \n";
        cout << "_____|_____|_____\n";
        cout << "     |     |     \n";
        cout << "  " << board[1][0] << "  |  " << board[1][1] << "  |  " << board[1][2] << " \n";
        cout << "_____|_____|_____\n";
        cout << "     |     |     \n";
        cout << "  " << board[2][0] << "  |  " << board[2][1] << "  |  " << board[2][2] << " \n";
        cout << "     |     |     \n\n";
    }

    bool isMovesLeft() const {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (board[i][j] != 'X' && board[i][j] != 'O')
                    return true;
        return false;
    }

    int evaluateBoard() const {
        // Checking Rows for Win/Loss
        for (int row = 0; row < 3; row++) {
            if (board[row][0] == board[row][1] && board[row][1] == board[row][2]) {
                if (board[row][0] == aiMarker) return +10;
                else if (board[row][0] == humanMarker) return -10;
            }
        }

        // Checking Columns for Win/Loss
        for (int col = 0; col < 3; col++) {
            if (board[0][col] == board[1][col] && board[1][col] == board[2][col]) {
                if (board[0][col] == aiMarker) return +10;
                else if (board[0][col] == humanMarker) return -10;
            }
        }

        // Checking Diagonals for Win/Loss
        if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
            if (board[0][0] == aiMarker) return +10;
            else if (board[0][0] == humanMarker) return -10;
        }

        if (board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
            if (board[0][2] == aiMarker) return +10;
            else if (board[0][2] == humanMarker) return -10;
        }

        return 0; // Draw or game in progress
    }

    // Unbeatable AI Minimax Algorithm
    int minimax(int depth, bool isMax) {
        int score = evaluateBoard();

        if (score == 10) return score - depth;
        if (score == -10) return score + depth;
        if (!isMovesLeft()) return 0;

        if (isMax) {
            int best = -1000;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] != 'X' && board[i][j] != 'O') {
                        char temp = board[i][j];
                        board[i][j] = aiMarker;
                        best = max(best, minimax(depth + 1, !isMax));
                        board[i][j] = temp;
                    }
                }
            }
            return best;
        } else {
            int best = 1000;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] != 'X' && board[i][j] != 'O') {
                        char temp = board[i][j];
                        board[i][j] = humanMarker;
                        best = min(best, minimax(depth + 1, !isMax));
                        board[i][j] = temp;
                    }
                }
            }
            return best;
        }
    }

    void findBestMove() {
        int bestVal = -1000;
        int bestRow = -1;
        int bestCol = -1;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] != 'X' && board[i][j] != 'O') {
                    char temp = board[i][j];
                    board[i][j] = aiMarker;
                    int moveVal = minimax(0, false);
                    board[i][j] = temp;

                    if (moveVal > bestVal) {
                        bestRow = i;
                        bestCol = j;
                        bestVal = moveVal;
                    }
                }
            }
        }
        board[bestRow][bestCol] = aiMarker;
    }

    bool placeMarker(int slot, char marker) {
        int row = (slot - 1) / 3;
        int col = (slot - 1) % 3;

        if (slot < 1 || slot > 9 || board[row][col] == 'X' || board[row][col] == 'O') {
            return false;
        }

        board[row][col] = marker;
        return true;
    }

public:
    void start() {
        char playAgain;
        do {
            int mode;
            cout << "\n=========================================\n";
            cout << "            SELECT GAME MODE             \n";
            cout << "=========================================\n";
            cout << " 1. Player vs Player (2 Players)\n";
            cout << " 2. Player vs Unbeatable AI (Single Player)\n";
            cout << " Select Option (1-2): ";
            while (!(cin >> mode) || (mode != 1 && mode != 2)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << " [!] Invalid choice. Enter 1 or 2: ";
            }

            resetBoard();

            if (mode == 1) {
                // Two Player Logic
                char currentMarker = 'X';
                int player = 1;
                while (evaluateBoard() == 0 && isMovesLeft()) {
                    drawBoard();
                    int slot;
                    cout << " Player " << player << " (" << currentMarker << "), choose a slot (1-9): ";
                    if (!(cin >> slot) || !placeMarker(slot, currentMarker)) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << " [!] Invalid move! Try again.\n";
                        continue;
                    }
                    currentMarker = (currentMarker == 'X') ? 'O' : 'X';
                    player = (player == 1) ? 2 : 1;
                }

                drawBoard();
                int score = evaluateBoard();
                if (score == 0) cout << " [=] Game Draw!\n";
                else cout << " [✓] Player " << (currentMarker == 'X' ? 2 : 1) << " Wins!\n";

            } else {
                // AI Mode Logic
                humanMarker = 'X';
                aiMarker = 'O';

                while (evaluateBoard() == 0 && isMovesLeft()) {
                    drawBoard();
                    int slot;
                    cout << " Your Turn (X), choose slot (1-9): ";
                    if (!(cin >> slot) || !placeMarker(slot, humanMarker)) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << " [!] Invalid move! Try again.\n";
                        continue;
                    }

                    if (evaluateBoard() == 0 && isMovesLeft()) {
                        cout << "\n [AI Thinking...]\n";
                        findBestMove();
                    }
                }

                drawBoard();
                int score = evaluateBoard();
                if (score == 10) cout << " [!] AI Wins! Better luck next time.\n";
                else if (score == -10) cout << " [✓] Amazing! You defeated the AI!\n";
                else cout << " [=] Game Draw! Well played.\n";
            }

            cout << "\nDo you want to play again? (Y/N): ";
            cin >> playAgain;

        } while (playAgain == 'y' || playAgain == 'Y');

        cout << "\nThank you for playing!\n";
    }
};

int main() {
    TicTacToe game;
    game.start();
    return 0;
}
