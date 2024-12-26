#include <iostream>
#include <windows.h>
#include <vector>
#include <thread>
#include <random>

using namespace std;
void createAssets();
int rotate(int px, int py, int rotation);
bool doesPieceFit(int tetrominoIndex, int rotation, int posX, int posY);

wstring tetromino[7];
int fieldWidth = 12;
int fieldHeight = 18;
unsigned char* playingField = nullptr;

wchar_t* screen = nullptr;
int screenWidth = 80;
int screenHeight = 30;

int main()
{
    // Create assets
    createAssets();
    
    playingField = new unsigned char[fieldWidth * fieldHeight];
    for (int x = 0; x < fieldWidth; x++) 
        for (int y = 0; y < fieldHeight; y++) 
            playingField[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;

    screen = new wchar_t[screenWidth * screenHeight];
    for (int i = 0; i < screenWidth * screenHeight; i++) 
        screen[i] = L' ';
    
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Game Logic

    bool gameOver = false;

    random_device tetrominoGenerator;

    int currentPiece = tetrominoGenerator() % 7;
    int nextPiece = tetrominoGenerator() % 7;
    int currentRotation = 0;
    int currentX = fieldWidth / 2;
    int currentY = 0;

    bool keys[4];
    bool rotationPressed;

    int speed = 20;
    int speedCounter = 0;
    bool forceDown = false;
    int pieceCount = 0;
    int score = 0;

    vector<int> lines;

    while (!gameOver) {
        // GAME TIMING
        this_thread::sleep_for(2.5ms * speed);
        speedCounter++;
        forceDown = (speedCounter == speed);

        // INPUT
        for (int i = 0; i < 4; i++)
            keys[i] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[i])));
            


        // GAME LOGIC
        if (!doesPieceFit(currentPiece, currentRotation, currentX, currentY)) {
            gameOver = true;
            break;
        }

        currentX += (keys[0] && doesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) ? 1 : 0;
        currentX -= (keys[1] && doesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) ? 1 : 0;
        currentY += (keys[2] && doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) ? 1 : 0;

        if (keys[3]) {
            currentRotation += (!rotationPressed && doesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) ? 1 : 0;
            rotationPressed = true;
        }
        else 
            rotationPressed = false;

        if (forceDown) {
            if (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
                currentY++;
            }
            else { 
                // Lock the piece
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                        if (tetromino[currentPiece][rotate(px, py, currentRotation)] == L'X')
                            playingField[(currentY + py) * fieldWidth + (currentX + px)] = currentPiece + 1;

                pieceCount++;
                if (pieceCount % 10 == 0)
                    if (speed >= 10) speed--;
                    

                // Check if we have lines
                
                for (int py = 0; py < 4; py++) 
                    if (currentY + py < fieldHeight - 1) {
                        bool line = true;
                        for (int px = 1; px < fieldWidth -1; px++) 
                            line &= (playingField[(currentY + py) * fieldWidth + px]) != 0;
                        
                        if (line) {
                            for (int px = 1; px < fieldWidth - 1; px++)
                                playingField[(currentY + py) * fieldWidth + px] = 8;

                            lines.push_back(currentY + py);
                        }
                    }
                
                score += 25;
                if (!lines.empty()) score += (1 << lines.size()) * 100;

                // Clean Next Piece space
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                        if (tetromino[nextPiece][rotate(px, py, 0)] == L'X')
                            screen[(5 + py) * screenWidth + fieldWidth + 6 + px] = 0;

                // Choose next piece
                currentRotation = 0;
                currentX = fieldWidth / 2;
                currentY = 0;
                currentPiece = nextPiece;
                nextPiece = tetrominoGenerator() % 7;

                speedCounter = 0;
            }
            speedCounter = 0;
        }

        // RENDER OUTPUT
   
        // Draw Field

        for (int x = 0; x < fieldWidth; x++)
            for (int y = 0; y < fieldHeight; y++)
                screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[playingField[y * fieldWidth + x]];

        for (int px = 0; px < 4; px++)
            for (int py = 0; py < 4; py++)
                if (tetromino[currentPiece][rotate(px, py, currentRotation)] == L'X')
                    screen[(currentY + py + 2) * screenWidth + (currentX + px + 2)] = currentPiece + 65;

        swprintf_s(&screen[2 * screenWidth + fieldWidth + 6 ], 16, L"SCORE: %8d", score);

        // Display next piece 
        swprintf(&screen[4 * screenWidth + fieldWidth + 6], 16, L"NEXT PIECE: ");

        for (int px = 0; px < 4; px++)
            for (int py = 0; py < 4; py++)
                if (tetromino[nextPiece][rotate(px, py, 0)] == L'X')
                    screen[(5 + py) * screenWidth + fieldWidth + 6 + px] = nextPiece + 65;


        // Display Frame
        if (lines.empty())
            WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
        else {
            WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
            this_thread::sleep_for(400ms);

            for (auto &v : lines) 
                for (int px = 1; px < fieldWidth - 1; px++) {
                    for (int py = v; py > 0; py--)
                        playingField[(py * fieldWidth) + px] = playingField[((py - 1) * fieldWidth) + px];
                    playingField[px] = 0;
                }
            lines.clear();
        }


    }

    CloseHandle(hConsole);
    std::cout << "Game Over!! Score: " << score << endl;
    system("pause");

    return 0;

}

void createAssets() {
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"..X.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"....");

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");
}

int rotate(int px, int py, int rotation) {
    int index = 0;
    switch (rotation % 4) {
    case 0: 
        index = py * 4 + px;
        break;
    case 1: 
        index = 12 + py - (px * 4);
        break;
    case 2: 
        index = 15 - (py * 4) - px;
        break;
    case 3: 
        index = 3 - py + (px * 4);
        break;
    }
    return index;
}

bool doesPieceFit(int tetrominoIndex, int rotation, int posX, int posY) {

    for (int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++) {
            int positionIndex = rotate(px, py, rotation);
            int fieldIndex = (posY + py) * fieldWidth + (posX + px);

            if (posX + px >= 0 && posX + px < fieldWidth)
                if (posY + py >= 0 && posY + py < fieldHeight)
                    if (tetromino[tetrominoIndex][positionIndex] == L'X' && playingField[fieldIndex] != 0)
                        return false;
        }

    return true;
}
