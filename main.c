// Projekt 3
// Gra planszowa 'Samotnik'
// Kacper Zając


#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <conio.h>
#endif

char board[7][16];              // plansza gry
int current_y = 3;              // srodek planszy
int current_x = 7;
int remaining_pegs;             // pozostale pola
int remaining_moves = 0;        // ilość możliwych do wykonania ruchow
_Bool error = 0;                // proba wykonania ruchu z miejsca niedozwolonego
_Bool admin_on = 0;             // oznacza wlaczony tryb administratora
_Bool howtoplay = 0;            // wyswietla instrukcje do gry
FILE *file;

#ifdef linux                            // odpowiedni sposob poruszania sie za pomoca strzalek
#include <unistd.h>
#include <termios.h>                    // NA PRZYSZLOSC CZY NIE LEPIEJ #IFDEF _WIN32 int WIN = 1; a potem IF (WIN) to WIN A NIE TO LINUX ?
#include <stdio_ext.h>

char getch(){
    char buf=0;
    struct termios old={0};
    fflush(stdout);
    if(tcgetattr(0, &old)<0)
        perror("tcsetattr()");
    old.c_lflag&=~ICANON;
    old.c_lflag&=~ECHO;
    old.c_cc[VMIN]=1;
    old.c_cc[VTIME]=0;
    if(tcsetattr(0, TCSANOW, &old)<0)
        perror("tcsetattr ICANON");
    if(read(0,&buf,1)<0)
        perror("read()");
    old.c_lflag|=ICANON;
    old.c_lflag|=ECHO;
    if(tcsetattr(0, TCSADRAIN, &old)<0)
        perror ("tcsetattr ~ICANON");
    return buf;
}

int get_key() {
    int ch;
    ch = getch();
    if (ch == '\033') { // if the first value is esc
        getch(); // skip the [
        switch (getch()) { // the real value
            case 'A':
                return 'U';
            case 'B':
                return 'D';
            case 'C':
                return 'R';
            case 'D':
                return 'L';
            default:
                break;
        }
    }
    return ch;
}
#endif                                  // do odpowiedniego systemu

#ifdef _WIN32
int get_key() {
    int ch;
    if ((ch = getch()) != 27 != 13) {
        if (ch == 0 || ch == 224)
            ch = getch();
        else return ch;
    }
    return ch;
}
#endif

void move();

void arrows() {
    if (admin_on) {
        board[current_y][current_x - 1] = '>';
        board[current_y][current_x + 1] = '<';
    } else {
        board[current_y][current_x - 1] = '<';
        board[current_y][current_x + 1] = '>';
    }
}

void mark() {
    board[current_y][current_x - 1] = '|';
    board[current_y][current_x + 1] = '|';
}

void clear_arrows() {
    board[current_y][current_x - 1] = ' ';
    board[current_y][current_x + 1] = ' ';
}

void count_remaining() {
    int i, j;
    remaining_moves = 0;
    for (i = 0; i < 7; i++) {
        for (j = 0; j < 15; j++) {
            if (board[i][j] == 'o') {
                if (board[i][j - 4] == ('.')) {
                    if (board[i][j - 2] == 'o') remaining_moves++;
                }
                if (board[i][j + 4] == ('.')) {
                    if (board[i][j + 2] == 'o') remaining_moves++;
                }
                if (board[i - 2][j] == ('.')) {
                    if (board[i - 1][j] == 'o') remaining_moves++;
                }
                if (board[i + 2][j] == ('.')) {
                    if (board[i + 1][j] == 'o') remaining_moves++;
                }
            }
        }
    }
}

void restart() {
    int i = 0, j = 0;
    for (i = 0; i < 7; i++)
        for (j = 0; j < 15; j++)        // tworzy najpierw puste pola
            board[i][j] = ' ';
    for (i = 2; i < 5; i++)             // nastepnie twory pole pionowe
        for (j = 1; j < 14; j++) {
            board[i][j++] = 'o';        // pola gry j - 1 3 5 7 9 11 13
            board[i][j] = ' ';
        }
    for (i = 0; i < 7; i++) {           // a na koncu pole poziome
        for (j = 5; j < 10; j++) {
            board[i][j++] = 'o';
            board[i][j] = ' ';
        }
        board[i][15] = '\n';
    }
    board[3][7] = '.';                  // srodek pola
    remaining_pegs = 32;
    current_y = 3;
    current_x = 7;
    count_remaining();
    arrows();
}

void save_file() {
    char filename[50];
    int i, j;
    printf("\n\nPodaj adres pliku: ");
    scanf("%s", filename);
    file = fopen(filename, "r");
    if (file != NULL) {
        printf("\n\nPodany plik juz istnieje. Czy chcesz go nadpisac? (t/n)");
        if (get_key() == 'n') return;
    }
    file = fopen(filename, "w+");
    for (i = 0; i < 7; i++) {
        {
            for (j = 0; j < 16; j++)
                fprintf(file, "%c", board[i][j]);
        }
    }
    fprintf(file, "%d %d %d", current_x, current_y, remaining_pegs);
    fclose(file);
}

void open_file() {
    char filename[50];
    int i, j;
    printf("\n\nPodaj nazwe pliku: ");      // wybierz czy utwórz czy zapisz / jak nie to utwórz
    scanf("%s", filename);
    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Podany plik nie istnieje. Nacisnij Enter, aby kontynuowac.");
        getchar();
        getchar();
        return;
    }
    clear_arrows();
    remaining_pegs = 0;
    for (i = 0; i < 7; i++) {
        {
            for (j = 0; j < 16; j++)
                fscanf(file, "%c", &board[i][j]);
        }
    }
    fscanf(file, "%d %d %d", &current_x, &current_y, &remaining_pegs);
    fclose(file);
}

void show_board() {
    int i;
    int j;

#ifdef _WIN32
    system("cls");
#endif
#ifdef linux
    system("clear");
#endif

    printf("    samotnik  ");
    printf("\n---------------\n\n");

    for (i = 0; i < 7; i++)
        for (j = 0; j < 16; j++) printf("%c", board[i][j]);

    if (admin_on) {
        printf("\n\nZnajdujesz sie w trybie administratora.\nNacisnij A, aby opuscic. Aby wyjsc z gry - nacisnij E.");
        return;
    }
    printf("\n\nNa planszy zostalo %d\nLiczba dostepnych ruchow - %d", remaining_pegs, remaining_moves);
    if (!howtoplay) printf("\n\nNacisnij S, aby wyswietlic pomoc.");
    if (error) printf("\n\nZ tego miejsca nie jest mozliwy zaden ruch.");
    if (howtoplay)
        printf("\n\nSterowanie:\nPo planszy gracz porusza sie przy pomocy strzalek i przycisku Enter."
                       "\n\nPrzyciski:\nR - restart, E - wyjscie z gry, Z - zapisuje stan gry"
                       "\nX - wczytuje stan gry z pliku, A - wlacza tryb administratora\nS - zamyka podpowiedzi"
                       "\n\nZwyciestwo:\nCelem gry jest zostawienie na planszy jak najmniejszej liczby pionkow.\nPionka bije sie"
                       " przeskakujac go w pionielub poziomie.\nNie mozna poruszac sie na ukos i bic kilku pionkow jednoczesnie.");
}

int relation() {

    if (current_x - 4 > 0) {
        if (board[current_y][current_x - 4] == ('.')) {
            if (board[current_y][current_x - 2] == 'o') return 1;
        }
    }
    if (current_x + 4 < 14) {
        if (board[current_y][current_x + 4] == ('.')) {
            if (board[current_y][current_x + 2] == 'o') return 1;
        }
    }
    if (current_y - 2 >= 0) {
        if (board[current_y - 2][current_x] == ('.')) {
            if (board[current_y - 1][current_x] == 'o') return 1;
        }
    }
    if (current_y + 2 < 7) {
        if (board[current_y + 2][current_x] == ('.')) {
            if (board[current_y + 1][current_x] == 'o') return 1;
        }
    }
    return 0;


}

void jump() {
    int direction = get_key();
    switch (direction) {
        case 13:
        case 10:
            arrows();
            break;
        case 80:            //dol
        case 'D':
            if (current_y > 4 || (current_y > 2 && current_x < 4 && current_x > 10)) break;
            if (board[current_y + 2][current_x] == ('.')) {
                if (board[current_y + 1][current_x] == 'o') {
                    board[current_y][current_x] = '.';
                    board[current_y + 1][current_x] = '.';
                    board[current_y + 2][current_x] = 'o';
                    clear_arrows();
                    current_y = current_y + 2;
                    arrows();
                    remaining_pegs--;
                }
            } else {
                clear_arrows();
                if (!(current_y == 6 || (current_y == 4 && (current_x < 4 || current_x > 10)))) current_y++;
                arrows();
            }
            break;
        case 77:            //prawo
        case 'R':
            if (current_x > 9 || (current_x > 5 && current_y < 1 && current_y > 5)) break;
            if (board[current_y][current_x + 4] == ('.')) {
                if (board[current_y][current_x + 2] == 'o') {
                    board[current_y][current_x] = '.';
                    board[current_y][current_x + 2] = '.';
                    board[current_y][current_x + 4] = 'o';
                    clear_arrows();
                    current_x = current_x + 4;
                    arrows();
                    remaining_pegs--;
                }
            } else {
                clear_arrows();
                if (!(current_x == 13 || (current_x == 9 && (current_y < 2 || current_y > 4))))
                    current_x += current_x + 2;
                arrows();
            }
            break;
        case 72:            //gora
        case 'U':
            if (current_y < 2 || (current_y < 4 && current_x < 4 && current_x > 10)) break;
            if (board[current_y - 2][current_x] == ('.')) {
                if (board[current_y - 1][current_x] == 'o') {
                    board[current_y][current_x] = '.';
                    board[current_y - 1][current_x] = '.';
                    board[current_y - 2][current_x] = 'o';
                    clear_arrows();
                    current_y = current_y - 2;
                    arrows();
                    remaining_pegs--;
                }
            } else {
                clear_arrows();
                if (!(current_y == 0 || (current_y == 2 && (current_x < 4 || current_x > 10)))) current_y--;
                arrows();
            }
            break;
        case 75:            //lewo
        case 'L':
            if (current_x < 5 || (current_x < 9 && current_y < 2 && current_y > 4)) break;
            if (board[current_y][current_x - 4] == ('.')) {
                if (board[current_y][current_x - 2] == 'o') {
                    board[current_y][current_x] = '.';
                    board[current_y][current_x - 2] = '.';
                    board[current_y][current_x - 4] = 'o';
                    clear_arrows();
                    current_x = current_x - 4;
                    arrows();
                    remaining_pegs--;
                }
            } else {
                clear_arrows();
                if (!(current_x == 1 || (current_x == 5 && (current_y < 2 || current_y > 4))))
                    current_x = current_x - 2;
                arrows();
            }
            break;
        default:
            break;

    }
}

void end() {
    int ch;
    if (remaining_pegs == 1) {
        printf("\n\nGratulacje. Wygrales!\nCzy chcesz zagrac jeszcze raz? (t/n)");
    }
    if (remaining_moves == 0 && remaining_pegs != 1) {
        printf("\n\nNie masz juz wolnego ruchu. Czy chcesz sprobowac ponownie? (t/n)");
    }
#ifdef _WIN32
    fseek(stdin, 0, SEEK_END);
#endif
#ifdef linux
    __fpurge(stdin);
#endif
    ch = getchar();
    if (ch == 'n') exit(0);
    restart();
    show_board();
}

void move() {
    show_board();
    error = 0;
    int direction = get_key();
    switch (direction) {
        case 'e':
            break;
        case 's':
            if (admin_on) move();
            if (howtoplay == 1) howtoplay = 0;
            else howtoplay = 1;
            move();
        case 13:
        case 10:
            if (admin_on) {
                if (board[current_y][current_x] == 'o') {
                    board[current_y][current_x] = '.';
                    remaining_pegs--;
                } else {
                    board[current_y][current_x] = 'o';
                    remaining_pegs++;
                }
            } else {
                if ((board[current_y][current_x] == 'o') && relation()) {
                    mark();
                    show_board();
                    jump();
                } else
                    error = 1;
                count_remaining();
                show_board();
                if (remaining_pegs == 1 || remaining_moves == 0)
                    (end());
            }
            move();
            break;
        case 72:            //gora
        case 'U':
            if ((current_y > 0 && current_x > 4 && current_x < 10) || current_y > 2) {
                clear_arrows();
                current_y--;
                arrows();
            }
            move();
            break;
        case 77:            //prawo
        case 'R':
            if ((current_x < 12 && current_y > 1 && current_y < 5) || current_x < 8) {
                clear_arrows();
                current_x++;
                current_x++;
                arrows();
            }
            move();
            break;
        case 80:            //dol
        case 'D':
            if ((current_y < 6 && current_x > 4 && current_x < 10) || current_y < 4) {
                clear_arrows();
                current_y++;
                arrows();
            }
            move();
            break;
        case 75:            //lewo
        case 'L':
            if ((current_x > 2 && current_y > 1 && current_y < 5) || current_x > 6) {
                clear_arrows();
                current_x--;
                current_x--;
                arrows();
            }
            move();
            break;
        case 'r':
            if (admin_on) move();
            restart();
            move();
            break;
        case 'x':
            if (admin_on) move();
            open_file();
            move();
            break;
        case 'z':
            if (admin_on) move();
            save_file();
            move();
            break;
        case 'a':
            if (admin_on) {
                admin_on = 0;
                count_remaining();
                arrows();
                if (remaining_pegs == 1 || remaining_moves == 0)
                    (end());
                move();
            } else {
                admin_on = 1;
                arrows();
                move();
            }
            break;
        default:
            move();
            break;

    }
}

int main() {
    restart();
    show_board();
    move();
    return 0;
}
