#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BOARD_SIZE 8
#define MAX_HISTORY 1000

typedef enum { EMPTY, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING } PieceType;
typedef enum { NONE, WHITE, BLACK } PieceColor;

typedef struct {
    PieceType type;
    PieceColor color;
} Piece;

Piece board[BOARD_SIZE][BOARD_SIZE];

// Zmienne do śledzenia ruchów króla i wież (roszada)
int hasWhiteKingMoved = 0, hasBlackKingMoved = 0;
int hasWhiteLeftRookMoved = 0, hasWhiteRightRookMoved = 0;
int hasBlackLeftRookMoved = 0, hasBlackRightRookMoved = 0;

// Zmienne do bicia w przelocie
int enPassantRow = -1, enPassantCol = -1;

// Zmienne do remisu
int halfMoveCounter = 0;
Piece boardHistory[MAX_HISTORY][BOARD_SIZE][BOARD_SIZE];
int historyCount = 0;

// Deklaracje funkcji
void initializeBoard();
void displayBoard();
int isValidMove(int sx, int sy, int dx, int dy, PieceColor turn);
void movePiece(int sx, int sy, int dx, int dy);
int isKingInCheck(PieceColor color);
int canMovePiece(int sx, int sy, int dx, int dy, PieceColor turn);
int isCheckmate(PieceColor turn);
int isStalemate(PieceColor turn);
int isSquareAttacked(int x, int y, PieceColor color);
void promotePawn(int x, int y);
void saveBoardToHistory();
int isThreefoldRepetition();
int isPathClear(int sx, int sy, int dx, int dy);
int isValidChessNotation(const char *input);

// Funkcja sprawdzająca trzykrotne powtórzenie pozycji
int isThreefoldRepetition() {
    int repetitions = 0;
    for (int i = 0; i < historyCount; i++) {
        if (memcmp(boardHistory[i], board, sizeof(board)) == 0) {
            repetitions++;
        }
        if (repetitions >= 3) return 1;
    }
    return 0;
}

// Zapisuje bieżący stan planszy do historii
void saveBoardToHistory() {
    if (historyCount < MAX_HISTORY) {
        memcpy(boardHistory[historyCount], board, sizeof(board));
        historyCount++;
    }
}

// Funkcja sprawdzająca poprawność ruchu
int canMovePiece(int sx, int sy, int dx, int dy, PieceColor turn) {
    Piece src = board[sx][sy];
    Piece dest = board[dx][dy];

    board[dx][dy] = src;
    board[sx][sy] = (Piece){EMPTY, NONE};

    int kingInCheck = isKingInCheck(turn);

    board[sx][sy] = src;
    board[dx][dy] = dest;

    return !kingInCheck;
}

// Funkcja sprawdzająca, czy król gracza jest w szachu
int isKingInCheck(PieceColor color) {
    int kingX = -1, kingY = -1;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].type == KING && board[i][j].color == color) {
                kingX = i;
                kingY = j;
                break;
            }
        }
        if (kingX != -1) break;
    }
    if (kingX == -1 || kingY == -1) return 0;

    return isSquareAttacked(kingX, kingY, color);
}

// Funkcja sprawdzająca, czy gracz jest w macie
int isCheckmate(PieceColor turn) {
    if (!isKingInCheck(turn)) return 0;

    for (int sx = 0; sx < BOARD_SIZE; sx++) {
        for (int sy = 0; sy < BOARD_SIZE; sy++) {
            if (board[sx][sy].color == turn) {
                for (int dx = 0; dx < BOARD_SIZE; dx++) {
                    for (int dy = 0; dy < BOARD_SIZE; dy++) {
                        if (isValidMove(sx, sy, dx, dy, turn) && canMovePiece(sx, sy, dx, dy, turn)) {
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return 1;
}

// Funkcja sprawdzająca, czy gracz jest w sytuacji pata
int isStalemate(PieceColor turn) {
    if (isKingInCheck(turn)) return 0;

    for (int sx = 0; sx < BOARD_SIZE; sx++) {
        for (int sy = 0; sy < BOARD_SIZE; sy++) {
            if (board[sx][sy].color == turn) {
                for (int dx = 0; dx < BOARD_SIZE; dx++) {
                    for (int dy = 0; dy < BOARD_SIZE; dy++) {
                        if (isValidMove(sx, sy, dx, dy, turn) && canMovePiece(sx, sy, dx, dy, turn)) {
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return 1;
}

// Funkcja sprawdzająca, czy dane pole jest atakowane przez przeciwnika
int isSquareAttacked(int x, int y, PieceColor color) {
    PieceColor opponent = (color == WHITE) ? BLACK : WHITE;

    for (int sx = 0; sx < BOARD_SIZE; sx++) {
        for (int sy = 0; sy < BOARD_SIZE; sy++) {
            if (board[sx][sy].color == opponent) {
                if (isValidMove(sx, sy, x, y, opponent)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Sprawdza, czy droga jest wolna dla wieży, gońca i hetmana
int isPathClear(int sx, int sy, int dx, int dy) {
    int xInc = (dx - sx) ? (dx - sx) / abs(dx - sx) : 0;
    int yInc = (dy - sy) ? (dy - sy) / abs(dy - sy) : 0;

    int x = sx + xInc, y = sy + yInc;
    while (x != dx || y != dy) {
        if (board[x][y].type != EMPTY) return 0;
        x += xInc;
        y += yInc;
    }
    return 1;
}

// Funkcja do promocji pionka
void promotePawn(int x, int y) {
    char choice;
    printf("Promote pawn! Choose [Q]ueen, [R]ook, [B]ishop, or K[N]ight: ");
    scanf(" %c", &choice);

    switch (choice) {
        case 'Q': case 'q': board[x][y].type = QUEEN; break;
        case 'R': case 'r': board[x][y].type = ROOK; break;
        case 'B': case 'b': board[x][y].type = BISHOP; break;
        case 'N': case 'n': board[x][y].type = KNIGHT; break;
        default: board[x][y].type = QUEEN; break;
    }
}

// Przenoszenie figury z uwzględnieniem bicia w przelocie, promocji i roszady
void movePiece(int sx, int sy, int dx, int dy) {
    Piece piece = board[sx][sy];
    Piece captured = board[dx][dy];  // Sprawdzenie, czy na docelowym polu jest figura przeciwnika

    // Przeniesienie figury na nowe miejsce
    board[dx][dy] = piece;
    board[sx][sy] = (Piece){EMPTY, NONE};

    // Aktualizacja licznika półruchów
    if (captured.type == EMPTY && piece.type != PAWN) {
        halfMoveCounter++;  // Zwiększamy licznik, jeśli nie było bicia ani ruchu pionka
    } else {
        halfMoveCounter = 0;  // Zerujemy licznik w przypadku bicia lub ruchu pionka
    }

    // Obsługa bicia w przelocie
    if (piece.type == PAWN && abs(sx - dx) == 2) {
        enPassantRow = dx;
        enPassantCol = sy;
    } else if (piece.type == PAWN && dx == enPassantRow && dy == enPassantCol) {
        // Usunięcie zbitego pionka w przypadku bicia w przelocie
        board[sx][dy] = (Piece){EMPTY, NONE};
        enPassantRow = -1;
        enPassantCol = -1;
    } else {
        enPassantRow = -1;
        enPassantCol = -1;
    }

    // Obsługa roszady
    if (piece.type == KING) {
        if (piece.color == WHITE) {
            hasWhiteKingMoved = 1;
            if (dy - sy == 2 && !hasWhiteRightRookMoved) { // Krótka roszada dla białych
                board[7][5] = board[7][7];  // Przeniesienie wieży
                board[7][7] = (Piece){EMPTY, NONE};  // Usunięcie starej wieży
            } else if (sy - dy == 2 && !hasWhiteLeftRookMoved) { // Długa roszada dla białych
                board[7][3] = board[7][0];  // Przeniesienie wieży
                board[7][0] = (Piece){EMPTY, NONE};  // Usunięcie starej wieży
            }
        } else {
            hasBlackKingMoved = 1;
            if (dy - sy == 2 && !hasBlackRightRookMoved) { // Krótka roszada dla czarnych
                board[0][5] = board[0][7];  // Przeniesienie wieży
                board[0][7] = (Piece){EMPTY, NONE};  // Usunięcie starej wieży
            } else if (sy - dy == 2 && !hasBlackLeftRookMoved) { // Długa roszada dla czarnych
                board[0][3] = board[0][0];  // Przeniesienie wieży
                board[0][0] = (Piece){EMPTY, NONE};  // Usunięcie starej wieży
            }
        }
    }

    // Sprawdzenie i realizacja promocji pionka
    if (piece.type == PAWN && (dx == 0 || dx == 7)) {
        promotePawn(dx, dy);  // Promocja na wybraną figurę
    }

    // Zapisanie obecnego stanu planszy do historii dla trzykrotnego powtórzenia
    saveBoardToHistory();
}


// Funkcja do sprawdzania poprawności formatu wejścia
int isValidChessNotation(const char *input) {
    return strlen(input) == 4 && input[0] >= 'a' && input[0] <= 'h' &&
           input[1] >= '1' && input[1] <= '8' && input[2] >= 'a' &&
           input[2] <= 'h' && input[3] >= '1' && input[3] <= '8';
}

// Wyświetlanie planszy
void displayBoard() {
    char whiteSymbols[] = {'.', 'P', 'R', 'N', 'B', 'Q', 'K'};
    char blackSymbols[] = {'.', 'p', 'r', 'n', 'b', 'q', 'k'};
    
    printf("  a b c d e f g h\n");
    for (int i = 0; i < BOARD_SIZE; i++) {  // Wyświetlanie od rzędu 0 do 7
        printf("%d ", 8 - i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            Piece p = board[i][j];
            char symbol = (p.color == WHITE) ? whiteSymbols[p.type] : blackSymbols[p.type];
            printf("%c ", symbol);
        }
        printf("%d\n", 8 - i);
    }
    printf("  a b c d e f g h\n\n");
}


// Inicjalizacja planszy
void initializeBoard() {
    hasWhiteKingMoved = hasBlackKingMoved = 0;
    hasWhiteLeftRookMoved = hasWhiteRightRookMoved = 0;
    hasBlackLeftRookMoved = hasBlackRightRookMoved = 0;
    halfMoveCounter = 0;
    historyCount = 0;
    enPassantRow = enPassantCol = -1;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = (Piece){EMPTY, NONE};
        }
    }

    // Ustawienie pionków
    for (int i = 0; i < BOARD_SIZE; i++) {
        board[1][i] = (Piece){PAWN, BLACK};  // Czarne pionki na drugim wierszu od góry
        board[6][i] = (Piece){PAWN, WHITE};  // Białe pionki na drugim wierszu od dołu
    }

    // Ustawienie figur
    PieceType order[] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
    for (int i = 0; i < BOARD_SIZE; i++) {
        board[0][i] = (Piece){order[i], BLACK};  // Czarne figury na górze
        board[7][i] = (Piece){order[i], WHITE};  // Białe figury na dole
    }
}


int isValidMove(int sx, int sy, int dx, int dy, PieceColor turn) {
    if (sx < 0 || sy < 0 || dx < 0 || dy < 0 || 
        sx >= BOARD_SIZE || sy >= BOARD_SIZE || 
        dx >= BOARD_SIZE || dy >= BOARD_SIZE) {
        return 0; // Ruch poza planszą
    }

    Piece src = board[sx][sy];
    Piece dest = board[dx][dy];

    if (src.color != turn) return 0; // Figury muszą należeć do bieżącego gracza
    if (dest.color == turn) return 0; // Nie można zbić swojej figury

    switch (src.type) {
        case PAWN:
            if (turn == WHITE) {
                if (sx - dx == 1 && sy == dy && dest.type == EMPTY) return 1;
                if (sx - dx == 2 && sy == dy && sx == 6 && dest.type == EMPTY && board[sx - 1][sy].type == EMPTY) return 1;
                if (sx - dx == 1 && abs(sy - dy) == 1 && dest.color == BLACK) return 1;
                if (sx - dx == 1 && abs(sy - dy) == 1 && dx == enPassantRow && dy == enPassantCol) return 1;
            } else {
                if (dx - sx == 1 && sy == dy && dest.type == EMPTY) return 1;
                if (dx - sx == 2 && sy == dy && sx == 1 && dest.type == EMPTY && board[sx + 1][sy].type == EMPTY) return 1;
                if (dx - sx == 1 && abs(sy - dy) == 1 && dest.color == WHITE) return 1;
                if (dx - sx == 1 && abs(sy - dy) == 1 && dx == enPassantRow && dy == enPassantCol) return 1;
            }
            break;
        case ROOK:
            if (sx == dx || sy == dy) return isPathClear(sx, sy, dx, dy);
            break;
        case KNIGHT:
            if ((abs(sx - dx) == 2 && abs(sy - dy) == 1) || (abs(sx - dx) == 1 && abs(sy - dy) == 2)) return 1;
            break;
        case BISHOP:
            if (abs(sx - dx) == abs(sy - dy)) return isPathClear(sx, sy, dx, dy);
            break;
        case QUEEN:
            if (sx == dx || sy == dy || abs(sx - dx) == abs(sy - dy)) return isPathClear(sx, sy, dx, dy);
            break;
        case KING:
            if (abs(sx - dx) <= 1 && abs(sy - dy) <= 1) return 1;

            // Sprawdzenie roszady
            if (src.color == WHITE && !hasWhiteKingMoved && dx == 7 && (dy == 6 || dy == 2)) {
                // Krótka roszada
                if (dy == 6 && !hasWhiteRightRookMoved && isPathClear(sx, sy, dx, 7) &&
                    !isSquareAttacked(sx, sy, BLACK) && !isSquareAttacked(sx, sy + 1, BLACK) && !isSquareAttacked(dx, dy, BLACK)) {
                    return 1;
                }
                // Długa roszada
                if (dy == 2 && !hasWhiteLeftRookMoved && isPathClear(sx, sy, dx, 0) &&
                    !isSquareAttacked(sx, sy, BLACK) && !isSquareAttacked(sx, sy - 1, BLACK) && !isSquareAttacked(dx, dy, BLACK)) {
                    return 1;
                }
            } else if (src.color == BLACK && !hasBlackKingMoved && dx == 0 && (dy == 6 || dy == 2)) {
                // Krótka roszada
                if (dy == 6 && !hasBlackRightRookMoved && isPathClear(sx, sy, dx, 7) &&
                    !isSquareAttacked(sx, sy, WHITE) && !isSquareAttacked(sx, sy + 1, WHITE) && !isSquareAttacked(dx, dy, WHITE)) {
                    return 1;
                }
                // Długa roszada
                if (dy == 2 && !hasBlackLeftRookMoved && isPathClear(sx, sy, dx, 0) &&
                    !isSquareAttacked(sx, sy, WHITE) && !isSquareAttacked(sx, sy - 1, WHITE) && !isSquareAttacked(dx, dy, WHITE)) {
                    return 1;
                }
            }
            break;
        default:
            return 0;
    }
    return 0;
}


// Główna funkcja gry
void playGame() {
    initializeBoard();
    PieceColor turn = WHITE;
    char input[6];

    while (1) {
        displayBoard();

        if (isThreefoldRepetition()) {
            printf("Game is a draw by threefold repetition.\n");
            break;
        }

        if (halfMoveCounter >= 100) {
            printf("Game is a draw by 50-move rule.\n");
            break;
        }    

        if (isCheckmate(turn)) {
            printf("Checkmate! %s wins!\n", turn == WHITE ? "Black" : "White");
            break;
        }
        if (isStalemate(turn)) {
            printf("Stalemate! Game is a draw.\n");
            break;
        }
        if (isKingInCheck(turn == WHITE ? BLACK : WHITE)) {
            printf("Check!\n");
        }
        printf("Turn: %s\n", turn == WHITE ? "White" : "Black");
        printf("Enter move (e.g., e2e4): ");
        fgets(input, 6, stdin);
        input[strcspn(input, "\n")] = 0; 

        if (!isValidChessNotation(input)) {
            printf("Invalid input format! Please use e.g., e2e4.\n");
            continue;
        }
        
        int sx = 8 - (input[1] - '1') - 1;
        int sy = input[0] - 'a';
        int dx = 8 - (input[3] - '1') - 1;
        int dy = input[2] - 'a';

        if (isValidMove(sx, sy, dx, dy, turn) && canMovePiece(sx, sy, dx, dy, turn)) {
            movePiece(sx, sy, dx, dy);
            if (isKingInCheck(turn == WHITE ? BLACK : WHITE)) printf("Check!\n");
            turn = (turn == WHITE) ? BLACK : WHITE;
        } else {
            printf("Invalid move or move puts king in check!\n");
        }
    }
}

int main() {
    playGame();
    return 0;
}
