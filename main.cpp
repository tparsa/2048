#include <cstdio>
#include <cstdlib>
#include <termios.h>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <sys/select.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/time.h>


const int GENERAL_MENU_STATE = 0;
const int GAME_MODE_MENU_STATE = 1;
const int X_2_GAME_MODE = 2;
const int X_3_GAME_MODE = 3;
const int PAUSED_GAMED = 5;
const int CONTINUE_GAME = 6;
const int ONGOING_GAME = 100;
const int GAME_OVER = 101;
const int EXIT_GAME = 0;
const int DEMO_GAME = 7;

const char NEW_GAME_MENU_OPTION = '1';
const char CONTINUE_GAME_MENU_OPTION = '2';
const char DEMO_GAME_OPTION = '3';
const char EXIT_GAME_MENU_OPTION = '4';
const char X_2_GAME_MODE_OPTION = '1';
const char X_3_GAME_MODE_OPTION = '2';

const char* WANRING_NO_PAUSED_GAME = "No paused game found";
const char* INVALID_OPTION = "Invalid option";
const char* WARNING_GAME_IS_PAUSED = "Game is paused";
const char* GAME_OVER_MESSAGE = "Game Over!";
const char* WARNING_CANT_MOVE_IN_THAT_DIRECTION = "Can't move in that direction";
const char* WIN_MESSAGE = "You achieved the goal! You can continue playing";
const char* WON_BUT_TABLE_IS_FULL = "You won! but the table is full";

const char PAUSE_AND_GO_TO_MENU = 'e';
const char ARROW_UP = char(65);
const char ARROW_DOWN = char(66);
const char ARROW_RIGHT = char(67);
const char ARROW_LEFT = char(68);
const char KEY_UP = 'w';
const char KEY_DOWN = 's';
const char KEY_RIGHT = 'd';
const char KEY_LEFT = 'a';
const char ESCAPE_CHARACTER = char(27);
const char UNDO_ACTION = 'z';

const int MOVE_DIRECTION_UP[2] = {1, 0};
const int MOVE_DIRECTION_DOWN[2] = {-1, 0};
const int MOVE_DIRECTION_RIGHT[2] = {0, -1};
const int MOVE_DIRECTION_LEFT[2] = {0, 1};

const int BASE_TO_POWER_OF_TWO_ADDITION_POWER_THRESHOLD = 8;
const int MAX_TABLE_SIZE = 100;
const int WIN_POWER = 11;


void gotoxy(int x, int y)
{
    printf("%c[%d;%df", 0x1B, y, x);
}


void clrscr(void)
{
    system("clear");
}


static struct termios old, nw;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
    tcgetattr(0, &old); //grab old terminal i/o settings
    nw = old; //make new settings same as old settings
    nw.c_lflag &= ~ICANON; //disable buffered i/o
    nw.c_lflag &= echo ? ECHO : ~ECHO; //set echo mode
    tcsetattr(0, TCSANOW, &nw); //apply terminal io settings
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
    tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch(void)
{
    char ch;
    initTermios(0);
    ch = getchar();
    resetTermios();
    return ch;
}

void bold_print(const char* message)
{
    printf("\e[1m%s\e[0m\n", message);
}

void bold_print(int message)
{
    printf("\e[1m%d\e[0m\n", message);
}

void warning(const char* message)
{
    if ((message == NULL) || (message[0] == '\0'))
        return;
    gotoxy(150, 3);
    bold_print(message);
    gotoxy(0, 0);
}

void show_menu(int state, const char* message)
{
    clrscr();
    warning(message);
    if (state == GENERAL_MENU_STATE)
        printf("1: Start a new game\n2: Resume paused game\n3: Demo Game\n4: Exit\n");
    else if (state == GAME_MODE_MENU_STATE)
        printf("1: New 2048 Game\n2: New 177147 (3^11) Game\n");
}

int _game_mode_menu(const char* message)
{
    show_menu(GAME_MODE_MENU_STATE, message);
    char game_mode = getch();
    if (game_mode == X_2_GAME_MODE_OPTION)
        return X_2_GAME_MODE;
    else if (game_mode == X_3_GAME_MODE_OPTION)
        return X_3_GAME_MODE;
    else
        return _game_mode_menu(INVALID_OPTION);
}

int menu(int state, const char* message)
{
    warning(message);
    if (state == GENERAL_MENU_STATE || state == PAUSED_GAMED)
    {
        show_menu(GENERAL_MENU_STATE, message);
        char option = getch();
        if (option == NEW_GAME_MENU_OPTION)
            return _game_mode_menu(NULL);
        else if (option == CONTINUE_GAME_MENU_OPTION)
        {
            if (state != PAUSED_GAMED)
                return menu(state, WANRING_NO_PAUSED_GAME);
            return CONTINUE_GAME;
        }
        else if (option == DEMO_GAME_OPTION)
            return DEMO_GAME;
        else if (option == EXIT_GAME_MENU_OPTION)
            return EXIT_GAME;
    }
    else if (state == GAME_MODE_MENU_STATE)
        return _game_mode_menu(message);
    return -1;
}

void clear_table(int table[][MAX_TABLE_SIZE])
{
    for (int i = 0; i < MAX_TABLE_SIZE; i++)
        for (int j = 0; j < MAX_TABLE_SIZE; j++)
            table[i][j] = 0;
}

void add_n_random_base_or_next_power_to_table(int table[][MAX_TABLE_SIZE], int base, int table_size, int n, bool next_power)
{
    while(n--)
    {
        int row, col;
        do {
            row = rand() % table_size;
            col = rand() % table_size;
        } while(table[row][col] != 0);
        int num = base;
        if (next_power && rand() % 100 > 50)
            num = base*base;
        table[row][col] = num;
    }
}

void initiate_game(int table[][MAX_TABLE_SIZE], int base, int table_size)
{
    clear_table(table);
    add_n_random_base_or_next_power_to_table(table, base, table_size, 2, false);
    // table[0][0] = 4;
    // table[1][0] = 32;
    // table[1][3] = 2;
    // table[2][0] = 32;
    // table[2][1] = 4;
    // table[2][2] = 2;
    // table[3][0] = 64;
    // table[3][1] = 8;

    // table[0][0] = 8;
    // table[0][1] = 4;
    // table[0][2] = 0;
    // table[0][3] = 2;
    // table[1][0] = 32;
    // table[1][1] = 8;
    // table[1][2] = 4;
    // table[1][3] = 2;
    // table[2][0] = 128;
    // table[2][1] = 64;
    // table[2][2] = 8;
    // table[2][3] = 4;
    // table[3][0] = 64;
    // table[3][1] = 16;
    // table[3][2] = 32;
    // table[3][3] = 8;
}

int int_len(int n)
{
    int ret = 0;
    while(n > 0)
    {
        ret += 1;
        n /= 10;
    }
    return ret;
}

const char* to_string(int n)
{
    if (n == 0)
        return "0";
    int len = int_len(n);
    char* str = (char *) malloc(len);
    int idx = 0;
    while(n > 0)
    {
        str[len - idx - 1] = char(n % 10 + '0');
        n /= 10;
        idx++;
    }
    return str;
}

void draw_table(int table[][MAX_TABLE_SIZE], int table_size)
{
    for (int i = 0; i < table_size; i++)
    {
        for (int j = 0; j < table_size; j++)
            printf("%d\t\t", table[i][j]);
        printf("\n");
    }
}

void draw_screen(int table[][MAX_TABLE_SIZE], int table_size, int high_score, const char* message)
{
    clrscr();
    gotoxy(20, 1);
    char high_score_message[100] = "High Score: ";
    strcat(high_score_message, to_string(high_score));
    bold_print(high_score_message);
    warning(message);
    gotoxy(0, 3);
    draw_table(table, table_size);
}

int goto_menu(int state, const char* message)
{
    int ret_state = menu(state, message);
    char invalid_option_message[100];
    if (message != NULL)
    {
        strcpy(invalid_option_message, message);
        strcat(invalid_option_message, "\n");
    }
    strcat(invalid_option_message, INVALID_OPTION);
    while (ret_state == -1)
        ret_state = menu(state, invalid_option_message);
    return ret_state;
}

bool valid_key_move(char action)
{
    bool is_arrow = (action == ARROW_UP || action == ARROW_DOWN || action == ARROW_LEFT || action == ARROW_RIGHT);
    bool is_normal_key = (action == KEY_UP || action == KEY_DOWN || action == KEY_LEFT || action == KEY_RIGHT);
    return (is_arrow || is_normal_key);
}

void shift_direction(int table[][MAX_TABLE_SIZE], int table_size, const int direction[])
{
    for (int x = 0; x < table_size; x++)
    {
        for (int i = 0; i < table_size; i++)
            for (int j = 0; j < table_size; j++)
            {
                if (table[i][j] == 0)
                    continue;
                int new_row = i;
                int new_col = j;
                for (int k = 1; k < table_size; k++)
                {
                    int new_i = i - k * direction[0];
                    int new_j = j - k * direction[1];
                    if (new_i < 0 || new_i >= table_size)
                        break;
                    if (new_j < 0 || new_j >= table_size)
                        break;
                    if (table[new_i][new_j] == 0)
                    {
                        new_row = new_i;
                        new_col = new_j;
                    }
                    else
                        break;
                }
                if (new_row != i || new_col != j)
                {
                    table[new_row][new_col] = table[i][j];
                    table[i][j] = 0;
                }
            }
    }
}

int simulate_direction(int table[][MAX_TABLE_SIZE], int base, int table_size, const int direction[])
{
    int score = 0;
    int start_i = 0, start_j = 0, end_i = table_size, end_j = table_size, step_i = 1, step_j = 1;
    if (direction[0] == -1)
    {
        start_i = table_size;
        end_i = 0;
        step_i = -1;
    }
    if (direction[1] == -1)
    {
        start_j = table_size;
        end_j = 0;
        step_j = -1;
    }
    for (int i = start_i; i != end_i; i+=step_i)
    {
        for (int j = start_j; j != end_j; j+=step_j)
        {
            if (table[i][j] == 0)
                continue;
            bool merge = true;
            for (int dis = 0; dis < base; dis++)
            {
                int new_i = i + dis * direction[0];
                int new_j = j + dis * direction[1];
                if (new_i < 0 || new_i >= table_size)
                {
                    merge = false;
                    break;
                }
                if (new_j < 0 || new_j >= table_size)
                {
                    merge = false;
                    break;
                }
                if (table[i][j] != table[new_i][new_j])
                {
                    merge = false;
                    break;
                }
            }
            if (merge)
            {
                table[i][j] = base * table[i][j];
                score += table[i][j];
                for (int dis = 1; dis < base; dis++)
                    table[i + dis * direction[0]][j + dis * direction[1]] = 0;
            }
        }
    }
    return score;
}

int simulate_move_up(int table[][MAX_TABLE_SIZE], int base, int table_size)
{
    shift_direction(table, table_size, MOVE_DIRECTION_UP);
    int score = simulate_direction(table, base, table_size, MOVE_DIRECTION_UP);
    shift_direction(table, table_size, MOVE_DIRECTION_UP);
    return score;
}

int simulate_move_down(int table[][MAX_TABLE_SIZE], int base, int table_size)
{
    shift_direction(table, table_size, MOVE_DIRECTION_DOWN);
    int score = simulate_direction(table, base, table_size, MOVE_DIRECTION_DOWN);
    shift_direction(table, table_size, MOVE_DIRECTION_DOWN);
    return score;
}

int simulate_move_right(int table[][MAX_TABLE_SIZE], int base, int table_size)
{
    shift_direction(table, table_size, MOVE_DIRECTION_RIGHT);
    int score = simulate_direction(table, base, table_size, MOVE_DIRECTION_RIGHT);
    shift_direction(table, table_size, MOVE_DIRECTION_RIGHT);
    return score;
}

int simulate_move_left(int table[][MAX_TABLE_SIZE], int base, int table_size)
{
    shift_direction(table, table_size, MOVE_DIRECTION_LEFT);
    int score = simulate_direction(table, base, table_size, MOVE_DIRECTION_LEFT);
    shift_direction(table, table_size, MOVE_DIRECTION_LEFT);
    return score;
}

int simulate_move(int table[][MAX_TABLE_SIZE], int next_table[][MAX_TABLE_SIZE], int base, char action, int table_size)
{
    for (int i = 0; i < table_size; i++)
        for (int j = 0; j < table_size; j++)
            next_table[i][j] = table[i][j];
    if (action == ARROW_UP || action == KEY_UP)
        return simulate_move_up(next_table, base, table_size);
    else if (action == ARROW_DOWN || action == KEY_DOWN)
        return simulate_move_down(next_table, base, table_size);
    else if (action == ARROW_RIGHT || action == KEY_RIGHT)
        return simulate_move_right(next_table, base, table_size);
    else if (action == ARROW_LEFT || action == KEY_LEFT)
        return simulate_move_left(next_table, base, table_size);
    return 0;
}

bool actable_move(int table[][MAX_TABLE_SIZE], int base, int table_size, char action)
{
    int next_table[MAX_TABLE_SIZE][MAX_TABLE_SIZE];
    clear_table(next_table);
    simulate_move(table, next_table, base, action, table_size);
    for (int i = 0; i < MAX_TABLE_SIZE; i++)
        for (int j = 0; j < MAX_TABLE_SIZE; j++)
            if (table[i][j] != next_table[i][j])
            {
                return true;
            }
    return false;
}

bool valid_unlock_move(int table[][MAX_TABLE_SIZE], int base, int table_size, char action)
{
    return valid_key_move(action) && actable_move(table, base, table_size, action);
}

int get_highest_num_of_table(int table[][MAX_TABLE_SIZE], int table_size)
{
    int mx = 0;
    for (int i = 0; i < table_size; i++)
        for (int j = 0; j < table_size; j++)
            if (table[i][j] > mx)
                mx = table[i][j];
    return mx;
}

int power(int base, int p)
{
    int ret = 1;
    while(p--)
        ret *= base;
    return ret;
}

int move(int table[][MAX_TABLE_SIZE], char action, int base, int table_size)
{
    int next_table[MAX_TABLE_SIZE][MAX_TABLE_SIZE];
    int score = simulate_move(table, next_table, base, action, table_size);
    for (int i = 0; i < table_size; i++)
        for (int j = 0; j < table_size; j++)
            table[i][j] = next_table[i][j];
    int highest_num_of_table = get_highest_num_of_table(table, table_size);
    bool f = false;
    if (highest_num_of_table >= power(base, BASE_TO_POWER_OF_TWO_ADDITION_POWER_THRESHOLD))
        f = true;
    add_n_random_base_or_next_power_to_table(table, base, table_size, 1, f);
    return score;
}

bool check_game_over(int table[][MAX_TABLE_SIZE], int base, int table_size)
{
    for (int i = 0; i < table_size; i++)
        for (int j = 0; j < table_size; j++)
            if (table[i][j] == 0)
                return false;
    return (valid_unlock_move(table, base, table_size, KEY_UP)
            || valid_unlock_move(table, base, table_size, KEY_DOWN)
            || valid_unlock_move(table, base, table_size, KEY_RIGHT)
            || valid_unlock_move(table, base, table_size, KEY_LEFT));
}

char convert_arrow_to_normal(char action)
{
    if (action == ARROW_UP)
        return KEY_UP;
    if (action == ARROW_DOWN)
        return KEY_DOWN;
    if (action == ARROW_RIGHT)
        return KEY_RIGHT;
    if (action == ARROW_LEFT)
        return KEY_LEFT;
    return 0;
}

char get_arrow_action()
{
    char action = getch();
    action = getch();
    return convert_arrow_to_normal(action);
}

void copy_table(int prev_table[][MAX_TABLE_SIZE], int table[][MAX_TABLE_SIZE], int table_size)
{
    for (int i = 0; i < table_size; i++)
        for (int j = 0; j < table_size; j++)
            prev_table[i][j] = table[i][j];
}

void demo_game() {
    fd_set set;
    struct timeval tv;
    struct termios t;

    memset(&tv, 0, sizeof(tv));

    tcgetattr(0, &t);
    t.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &t);

    int table[MAX_TABLE_SIZE][MAX_TABLE_SIZE];
    int base = 2;
    int table_size = base * base;
    int score = 0, high_score = 0;
    char message[300];
    strcpy(message, "");
    bool won = false;
    initiate_game(table, base, table_size);

    while (true) {
        FD_ZERO(&set);
        FD_SET(0, &set);
        select(1, &set, 0, 0, &tv);

        if (FD_ISSET(0, &set)) {
            char ch = getch();
            if (ch == PAUSE_AND_GO_TO_MENU)
                return;
        }
        draw_screen(table, table_size, high_score, message);
        if (check_game_over(table, base, table_size))
        {
            if (!won)
                strcpy(message, GAME_OVER_MESSAGE);
            else
                strcpy(message, WON_BUT_TABLE_IS_FULL);
            continue;
        }
        char action;
        do {
            int random = rand() % 4;
            action = convert_arrow_to_normal(char(65 + random));
        } while (!valid_unlock_move(table, base, table_size, action));

        score += move(table, action, base, table_size);
        message[0] = action;
        message[1] = '\0';
        if (score > high_score)
            high_score = score;
        if (get_highest_num_of_table(table, table_size) == power(base, WIN_POWER))
        {
            won = true;
            strcpy(message, WIN_MESSAGE);
        }

        sleep(1);
    }
}

int main()
{
    srand(time(NULL));

    int state = goto_menu(GENERAL_MENU_STATE, NULL);
    int table[MAX_TABLE_SIZE][MAX_TABLE_SIZE];
    int prev_table[MAX_TABLE_SIZE][MAX_TABLE_SIZE];
    int game_mode = -1, game_state = -1;
    char message[300];
    int score = 0, high_score = 0, prev_score = 0;
    while (true)
    {
        if (state == X_2_GAME_MODE || state == X_3_GAME_MODE)
        {
            if (game_state != PAUSED_GAMED)
                game_mode = state;
            int base = state;
            int table_size = state*state;
            if (game_state != PAUSED_GAMED)
                initiate_game(table, base, table_size);
            game_state = ONGOING_GAME;
            bool won = false;
            while (game_state == ONGOING_GAME)
            {
                draw_screen(table, table_size, high_score, message);
                char action = getch();
                if (action == ESCAPE_CHARACTER)
                    action = get_arrow_action();
                if (action == PAUSE_AND_GO_TO_MENU)
                {
                    game_state = PAUSED_GAMED;
                }
                else if(action == UNDO_ACTION)
                {
                    copy_table(table, prev_table, table_size);
                    score = prev_score;
                }
                else if (valid_unlock_move(table, base, table_size, action))
                {
                    copy_table(prev_table, table, table_size);
                    prev_score = score;
                    score += move(table, action, base, table_size);
                    strcpy(message, "");
                    if (score > high_score)
                        high_score = score;
                    if (get_highest_num_of_table(table, table_size) == power(base, WIN_POWER))
                    {
                        won = true;
                        strcpy(message, WIN_MESSAGE);
                    }
                }
                else{
                    strcpy(message, WARNING_CANT_MOVE_IN_THAT_DIRECTION);
                }
                if (check_game_over(table, base, table_size))
                {
                    if (!won)
                        strcpy(message, GAME_OVER_MESSAGE);
                    else
                        strcpy(message, WON_BUT_TABLE_IS_FULL);
                    game_state = GAME_OVER;
                }
            }
            if (game_state == PAUSED_GAMED) {
                state = goto_menu(PAUSED_GAMED, WARNING_GAME_IS_PAUSED);
                if (state == X_2_GAME_MODE || state == X_3_GAME_MODE)
                    game_state = -1;
            }
            else if (game_state == GAME_OVER)
            {
                strcpy(message, GAME_OVER_MESSAGE);
                strcat(message, " Score: ");
                strcat(message, to_string(score));
                state = goto_menu(GENERAL_MENU_STATE, message);
            }
        }
        else if (state == CONTINUE_GAME)
            state = game_mode;
        else if (state == DEMO_GAME)
        {
            demo_game();
            state = goto_menu(GENERAL_MENU_STATE, message);
        }
        else if (state == EXIT_GAME)
            break;
    }
    return 0;
}