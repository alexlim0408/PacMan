/*
Title: PacMan in CPP
Date: 2023-08-31
Version: v0.5
Team members:
1. Lee Xin Yi       ; 2103307; MH; +6011-1634 6985   (Lead developer)
2. Lim Jin Yap      ; 2102460; MH; +6017- 351 7048   (Developer & Bug hunter)
3. Chua Yun Juan    ; 2103232; MH; +6011-1675 6408   (Developer & Bug hunter)
4. Joel Lim Yi Hong ; 2102392; MH; +6017- 812 5331   (Developer & Bug hunter)
5. Ng Zhee Horng    ; 2103167; MH; +6018- 988 4834   (Developer & Bug hunter)

© 2023 Lee Xin Yi. All rights reserved.
https://github.com/Leejames2305/Pac_Man
*/

#include <iostream>
#include <iomanip>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>   //This is to sort the scores
#include <filesystem>  //This is to get list of files in directory
#include <cmath>       //This is to calculate distance between player and ghost

using namespace std;
namespace fs = std::filesystem;

const int COLS = 40;    //X-axis
const int ROWS = 20;    //Y-axis

struct Score
{
    int score;
    int timeTaken;
    Score(int scr, int time) : score(scr), timeTaken(time) {}
};

struct dots_coordinates
{
    int dotsX;
    int dotsY;
    bool isDots;
};
dots_coordinates dots[ROWS][COLS];

struct ghost_coordinates
{
    bool is_wall;
};
ghost_coordinates ghost[ROWS + 1][COLS + 1];  //Extra 1 line to build invisible wall

vector<string> txt_files;  //Vector to store all txt files name

int main_menu();                                         // main menu function
void runGame(int* pScore, int* pGlobalTime);             // function to handle game logic and display
void level_editor_menu();                                // function to handle level editor menu
void level_editor();                                     // function to handle level editor
void level_create();                                     // function to handle level creation
bool compareScores(const Score& s1, const Score& s2);    // function to compare scores
void displayScoreboard(const vector<Score>& get_scores); // function to handle scoreboard
string loaded_level(bool previous, bool next);           // string to store level name
void ghost_movement(int x, int y, int& g_x, int& g_y);   // function to handle ghost movement

int files_id = 0;
int globaltime = 0;

int main()
{
    int main_menu_selection = 0;
    int score = 0;

    vector<Score> get_scores = { Score(815, 30) };          // Default scores, 150 points in 10 seconds

    while (main_menu_selection != 4)			            // keep showing main menu
    {
        system("cls");
        main_menu_selection = main_menu(); 		            // call main menu function

        if (main_menu_selection == 1) 			            // if main menu selection is 1, start game
        {
            runGame(&score, &globaltime);                   // call the function to run the game
            get_scores.push_back(Score(score, globaltime)); // Add score to vector
            displayScoreboard(get_scores);                  // Display scoreboard
        }
        else if (main_menu_selection == 2) 		            // if main menu selection is 2, level editor
        {
            level_editor_menu(); 				            // call the function to run the level editor
        }
        else if (main_menu_selection == 3) 		            // if main menu selection is 3, scoreboard
        {
            displayScoreboard(get_scores); 				    // call the function to run the scoreboard
        }
        else if (main_menu_selection == 4) 		            // if main menu selection is 4, exit
        {
            exit(0);
        }
    }
}

int main_menu()
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;                         // Next 4 lines set cursor visibility to true
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);

    cout << "PacMan" << endl;
    cout << "Press 1 to start" << endl;
    cout << "Press 2 for level editor" << endl;
    cout << "Press 3 for ScoreBoard" << endl;
    cout << "Press 4/esc to exit" << endl;
    cout << endl;
    cout << "Current level: " << loaded_level(0, 0) << endl;
    cout << "Press a/d to change level (" << files_id << " out of " << txt_files.size() - 1 << ") " << endl;

    unsigned char CharInput;
    CharInput = _getch(); 	// get key stroke

    if (CharInput == 49) 	// 1 key
    {
        return 1;
    }
    else if (CharInput == 50) // 2 key
    {
        return 2;
    }
    else if (CharInput == 51) // 3 key
    {
        return 3;
    }
    else if (CharInput == 52) // 4 key
    {
        return 4;
    }
    else if (CharInput == 27) // esc key
    {
        return 4;
    }
    else if (CharInput == 97 || CharInput == 65) // a key
    {
        loaded_level(1, 0);
        return 0;
    }
    else if (CharInput == 100 || CharInput == 68) // d key
    {
        loaded_level(0, 1);
        return 0;
    }
    else
    {
        cout << "Invalid input, please try again" << endl;
        Sleep(200);
        return 0;
    }
}

void runGame(int* pScore, int* pGlobalTime)
{
    system("cls");                          // clear console screen, start from empty
    int x = 0, y = 0;                       // record player's position
    int direction = 0;                      // record player's direction; 1 up, 2 left, 3 right, 4 down        
    int score = 0;                          // record player's score
    int timetaken = 0;                      // record time taken
    int game_wall_coords[20][40] = { 0 };  // record wall coordinates, 0 = no wall, 1 = wall
    int g_x = 39, g_y = 19;                 // record ghost's position

    ifstream infile;
    infile.open(loaded_level(0, 0));
    if (!infile)
        infile.close(); //file error

    int xx, yy;
    while (infile >> xx >> yy)
    {
        game_wall_coords[yy][xx] = 1;
    }

    for (int j = 0; j < ROWS + 1; ++j) // initialize invinsible ghost walls
    {
        ghost[j][40].is_wall = true;
    }
    for (int i = 0; i < COLS + 1; ++i)
    {
        ghost[20][i].is_wall = true;
    }

    for (int j = 0; j < ROWS; ++j)       // initialize dots coordinates, and ghost walls 
    {
        for (int i = 0; i < COLS; ++i)
        {
            dots[j][i].dotsX = j;
            dots[j][i].dotsY = j;
            dots[j][i].isDots = true;
            if (game_wall_coords[j][i] == 1) // if wall, set isDots to false
            {
                dots[j][i].isDots = false;
                ghost[j][i].is_wall = true;
            }
            else
            {
                ghost[j][i].is_wall = false;
            }
        }
    }

    for (int count = 0; true; count++) // infinite loop, with count running
    {
        bool isDotsLeft = false; // used to determine any dots left

        if (count % 2 == 0)
        {
            ghost_movement(x, y, g_x, g_y);  // call the function to move the ghost
        }

        if (_kbhit()) // Check key stroke
        {
            unsigned char CharInput = _getch(); // arrow key is a 2-key combination, 244-72 for "up", this get first key
            if (CharInput == 224)               // confirm it is a 2-key combination
            {
                CharInput = _getch(); // get second key
                switch (CharInput)
                {
                case 72:
                    if (y > 0 && (game_wall_coords[y - 1][x] != 1)) y--; direction = 1; break; // up
                case 75:
                    if (x > 0 && (game_wall_coords[y][x - 1] != 1)) x--; direction = 2; break; // left
                case 77:
                    if (x < 39 && (game_wall_coords[y][x + 1] != 1)) x++; direction = 3; break; // right
                case 80:
                    if (y < 19 && (game_wall_coords[y + 1][x] != 1)) y++; direction = 4; break; // down
                }
            }
            else if (CharInput == 27) // esc key quit, other no respond
            {
                break;
            }
        }
        else  //When key is not pressed, keep moving in the same direction
        {
            if (direction == 1 && y > 0 && (game_wall_coords[y - 1][x] != 1)) y--; 		// up momentum
            else if (direction == 2 && x > 0 && (game_wall_coords[y][x - 1] != 1)) x--; 	// left momentum
            else if (direction == 3 && x < 39 && (game_wall_coords[y][x + 1] != 1)) x++; // right momentum
            else if (direction == 4 && y < 19 && (game_wall_coords[y + 1][x] != 1)) y++; // down momentum
            else direction = 0; 					// stop momentum when hit wall
        }

        // Display routine
        COORD coord = { 0, 0 };
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

        SetConsoleCursorPosition(consoleHandle, coord); // Set cursor position to (0,0)
        CONSOLE_CURSOR_INFO cursorInfo;                 // Next 4 lines set cursor visibility to FALSE
        GetConsoleCursorInfo(consoleHandle, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(consoleHandle, &cursorInfo);

        for (int row = 0; row < ROWS * 2; row++) // draw 40x80 screen
        {
            for (int column = 0; column < COLS * 2; column++)
            {
                if ((row == y * 2 || row == y * 2 + 1) && (column == x * 2 || column == x * 2 + 1)) // Print 2X of player when position matched
                {
                    switch (direction) // Print player direction, 1 up, 2 left, 3 right, 4 down
                    {
                    case 0:
                        cout << "X"; break;
                    case 1:
                        cout << "^"; break;
                    case 2:
                        cout << "<"; break;
                    case 3:
                        cout << ">"; break;
                    case 4:
                        cout << "v"; break;
                    }

                    if (dots[y][x].isDots == true)  // Check if there is any dots in player position
                    {
                        dots[y][x].isDots = false; // remove dots when player pass through
                        score++;                    // add score when player pass through
                    }
                }
                else if ((row == g_y * 2 || row == g_y * 2 + 1) && (column == g_x * 2 || column == g_x * 2 + 1)) // Print 2X of ghost when position matched
                {
                    cout << "G"; // Print ghost in ghost position
                }
                else if (game_wall_coords[row / 2][column / 2] == 1)
                {
                    cout << '#';
                }
                else if (dots[row / 2][column / 2].isDots) // Print '.' if dots[coords][coords].isDots is true
                {
                    cout << "."; // Print dots in dots position
                    isDotsLeft = true; // Set isDotsLeft to true if there is any dots left
                }
                else
                {
                    cout << " "; // Print space in non-player position
                }
            }
            cout << "#"; // Print right wall
            cout << endl;
        }
        for (int lastrow = 0; lastrow < 80; lastrow++) //Print bottom wall
            cout << "#";
        cout << endl;
        cout << "Score: " << score << endl;
        cout << "Time taken: " << setw(4) << timetaken++ / 6 << "s" << endl;  //Speed is 50ms, therefor 20 rounds = 1 second; Currently set to 6 for debugging
        cout << "Press esc to quit" << endl;
        // cout << g_x << " " << g_y << " " << ghost[g_y - 1][g_x - 1].is_wall << endl;  // Debugging ghost position and wall
        Sleep(50);  // Speed control

        if (isDotsLeft == false) // Check if there is any dots left
        {
            cout << endl;
            cout << "You win!" << endl;
            cout << "Score: " << score << endl;
            cout << "Time taken: " << setw(4) << timetaken / 6 << "s" << endl;
            cout << "Press any key to continue" << endl;

            globaltime = timetaken / 6;

            _getch();
            break;
        }
        else if ((x == g_x || x == g_x + 1 || x == g_x - 1) && (y == g_y || y == g_y + 1 || y == g_y - 1)) // Check if player position is same / next to ghost position
        {
            cout << endl;
            cout << "You lose!" << endl;
            cout << "Score: " << score << endl;
            cout << "Time taken: " << setw(4) << timetaken / 6 << "s" << endl;
            cout << "Press any key to continue" << endl;

            globaltime = timetaken / 6;

            _getch();
            break;
        }
    }
    globaltime = timetaken / 6;

    *pScore = score;
    *pGlobalTime = globaltime;
}

void level_editor_menu()
{
    unsigned char level_editor_selection = 0;

    while (level_editor_selection != 51)
    {
        system("cls");

        cout << "Level editor" << endl;
        cout << "Press 1 to create a new level" << endl;
        cout << "Press 2 to edit an existing level" << endl;
        cout << "Press 3/esc to return to main menu" << endl;
        cout << endl;
        cout << "Current level: " << loaded_level(0, 0) << endl;
        cout << "Press a/d to change level (" << files_id << " out of " << txt_files.size() - 1 << ") " << endl;


        level_editor_selection = _getch();

        if (level_editor_selection == 49)  // 1 key
        {
            level_create();
        }
        else if (level_editor_selection == 50)  // 2 key
        {
            level_editor();
        }
        else if (level_editor_selection == 51)  // 3 key
        {
            break;
        }
        else if (level_editor_selection == 27)  // esc key
        {
            break;
        }
        else if (level_editor_selection == 97 || level_editor_selection == 65) // a key
        {
            loaded_level(1, 0);
        }
        else if (level_editor_selection == 100 || level_editor_selection == 68) // d key
        {
            loaded_level(0, 1);
        }
        else
        {
            cout << "Invalid input, please try again" << endl;
            Sleep(200);
        }
    }
}

void level_editor()
{
    system("cls"); 		//clear console screen, start from empty

    int x = 0, y = 0; 	//record player's position
    int wall_coords[20][40] = { 0 }; //record wall coordinates, 0 = no wall, 1 = wall

    ifstream infile;
    infile.open(loaded_level(0, 0));
    if (!infile)
        infile.close(); //file error

    int xx, yy;
    while (infile >> xx >> yy)  //Read all wall coordinates from txt file
    {
        wall_coords[yy][xx] = 1;  //Set wall coordinates to 1 if there is a wall
    }

    for (;;)
    {
        //Check key stroke
        if (_kbhit())
        {
            //arrow key is a 2-key combination, 244-72 for "up"
            unsigned char c = _getch(); //get first key
            if (c == 224)
            {
                c = _getch(); //get second key
                switch (c)
                {
                case 72: if (y > 0) y--; break; //up
                case 75: if (x > 0) x--; break; //left
                case 77: if (x < 39) x++; break; //right
                case 80: if (y < 19) y++;        //down
                }
            }
            else if (c == 99 || c == 67)  // When press C, coordinates of wall is recorded
            {
                if ((x != 0 || y != 0) && (x != 39 || y != 19))  //P and G position cannot be a wall
                    wall_coords[y][x] = 1;

                if (x < 39)
                    x++; // Move player to next position
            }
            else if (c == 120 || c == 88)  // When press X, coordinates of wall is removed
            {
                if ((x != 0 || y != 0) && (x != 39 || y != 19))  // P and G position cannot be a removed
                    wall_coords[y][x] = 0;

                if (x < 39)
                    x++; // Move player to next position
            }
            else if (c == 27) //esc key
            {
                ofstream outfile;
                outfile.open(loaded_level(0, 0));
                if (!outfile)
                    outfile.close(); //file error

                for (int r = 0; r < 20; r++)
                {
                    for (int c = 0; c < 40; c++)
                    {
                        if (wall_coords[r][c] == 1)
                            outfile << c << " " << r << endl;
                    }
                }
                break;
            }
        }

        //Display routine
        COORD coord = { 0, 0 };
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

        SetConsoleCursorPosition(consoleHandle, coord); // Set cursor position to (0,0)
        CONSOLE_CURSOR_INFO cursorInfo;                 // Next 4 lines set cursor visibility to FALSE
        GetConsoleCursorInfo(consoleHandle, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(consoleHandle, &cursorInfo);

        for (int r = 0; r < 20; r++)
        {
            for (int c = 0; c < 40; c++)
            {
                if (r == y && c == x)
                    cout << "X";
                else if (r == 0 && c == 0)
                    cout << "P";
                else if (r == 19 && c == 39)
                    cout << "G";
                else if (wall_coords[r][c] == 1)
                    cout << '#';
                else
                    cout << " ";
            }
            cout << "#";
            cout << endl;
        }
        for (int lastrow = 0; lastrow < 40; lastrow++) //Print bottom wall
            cout << "#";

        cout << endl;
        cout << "Press C to create wall, X to remove wall" << endl;
        cout << "Press esc to save and return to level editor menu" << endl;

        //Speed control
        Sleep(100);
    }
}

void level_create()
{
    string custom_level_name;
    system("cls"); 		//clear console screen, start from empty
    cout << "Please enter level name (Enter 0 to quit): ";
    cin >> custom_level_name;

    if (custom_level_name == "0")  //If user enter 0, return to level editor menu
        return;

    ofstream outfile;
    outfile.open(custom_level_name + ".txt");
    if (!outfile)
        outfile.close(); //file error

    cout << "Successfully created level: " << custom_level_name << endl;
    cout << "Please proceed to level editor to edit the level" << endl;
    cout << "Press any key to continue" << endl;
    _getch();
}

string loaded_level(bool previous, bool next)  //Read all txt name, all of the txt will be level
{
    for (const auto& entry : fs::directory_iterator("."))  // get all txt files name in current directory
    {
        if (entry.path().extension() == ".txt")
        {
            string filename = entry.path().filename().string();
            if (find(txt_files.begin(), txt_files.end(), filename) == txt_files.end())
            {
                txt_files.push_back(filename);
            }
        }
    }

    if (previous == true)  //If previous is true, go to previous txt file
    {
        if (files_id != 0)  //Cannot go below 0
            files_id--;
    }
    else if (next == true)  //If next is true, go to next txt file
    {
        if (files_id < txt_files.size() - 1)  //Cannot go above txt_files.size()
            files_id++;
    }
    else
    {
        return txt_files[files_id];
    }

    return txt_files[files_id];  //Return current txt file name
}

bool compareScores(const Score& s1, const Score& s2)
{
    if (s1.score != s2.score)
    {
        return s1.score > s2.score;  // Sorting in descending order
    }

    else
    {
        return s1.timeTaken < s2.timeTaken; // If scores are equal, compare times in ascending order
    }
}

void displayScoreboard(const vector<Score>& get_scores)
{
    vector<Score> scoreboard = get_scores; // Copy the scores

    sort(scoreboard.begin(), scoreboard.end(), compareScores);
    system("cls");
    cout << "Scoreboard" << endl;
    cout << " " << endl;
    cout << "Rank " << "\tScore" << "\tTime taken" << endl;

    for (size_t i = 0; i < scoreboard.size(); ++i) {
        const Score& entry = scoreboard[i];

        cout << i + 1 << "\t" << entry.score << "\t" << entry.timeTaken << "s" << endl;
    }
    cout << "Press any key to continue" << endl;

    _getch();
}

void ghost_movement(int x, int y, int& g_x, int& g_y)
{
    int r_distance = pow(abs(g_x + 1 - x), 2) + pow(abs(g_y - y), 2);
    int l_distance = pow(abs(g_x - 1 - x), 2) + pow(abs(g_y - y), 2);
    int d_distance = pow(abs(g_x - x), 2) + pow(abs(g_y + 1 - y), 2);
    int u_distance = pow(abs(g_x - x), 2) + pow(abs(g_y - 1 - y), 2);
    int distance[4] = { r_distance,l_distance,d_distance,u_distance };

    // Sort the distances of the ghost from the player in ascending order in array, 1st being shortest
    for (int i = 0; i < 4; i++)
    {
        for (int j = (i + 1); j < 4; j++)
        {
            if (distance[i] >= distance[j])
            {
                int temp = distance[i];
                distance[i] = distance[j];
                distance[j] = temp;
            }
        }
    }

    if (distance[0] == u_distance && ghost[g_y - 1][g_x].is_wall == 0)  //prioritise up and down, Ghost move up if u_distance is the shortest, and there is no wall above
    {
        g_y--;

    }
    else if (distance[0] == d_distance && ghost[g_y + 1][g_x].is_wall == 0)  //Ghost move down if d_distance is the shortest & there is no wall below 
    {
        g_y++;

    }
    else if (distance[0] == r_distance && ghost[g_y][g_x + 1].is_wall == 0)  //Ghost move right if r_distance is the shortest there is no wall on the right
    {
        g_x++;

    }
    else if (distance[0] == l_distance && ghost[g_y][g_x - 1].is_wall == 0)  //Ghost move left if l_distance is the shortest & there is no wall on the left
    {
        g_x--;

    }
    else if (distance[1] == u_distance && ghost[g_y - 1][g_x].is_wall == 0)  //If shortest distance is unavailable, move to the next shortest distance, repeat for all 4 directions
    {
        g_y--;

    }
    else if (distance[1] == d_distance && ghost[g_y + 1][g_x].is_wall == 0)
    {
        g_y++;

    }
    else if (distance[1] == r_distance && ghost[g_y][g_x + 1].is_wall == 0)
    {
        g_x++;

    }
    else if (distance[1] == l_distance && ghost[g_y][g_x - 1].is_wall == 0)
    {
        g_x--;

    }
    else if (distance[2] == u_distance && ghost[g_y - 1][g_x].is_wall == 0)
    {
        g_y--;

    }
    else if (distance[2] == d_distance && ghost[g_y + 1][g_x].is_wall == 0)
    {
        g_y++;

    }
    else if (distance[2] == r_distance && ghost[g_y][g_x + 1].is_wall == 0)
    {
        g_x++;

    }
    else if (distance[2] == l_distance && ghost[g_y][g_x - 1].is_wall == 0)
    {
        g_x--;

    }
    else if (distance[3] == u_distance && ghost[g_y - 1][g_x].is_wall == 0)
    {
        g_y--;

    }
    else if (distance[3] == d_distance && ghost[g_y + 1][g_x].is_wall == 0)
    {
        g_y++;

    }
    else if (distance[3] == r_distance && ghost[g_y][g_x + 1].is_wall == 0)
    {
        g_x++;

    }
    else if (distance[3] == l_distance && ghost[g_y][g_x - 1].is_wall == 0)
    {
        g_x--;

    }
}