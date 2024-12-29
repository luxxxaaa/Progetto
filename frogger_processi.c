#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <sys/wait.h>

#define RANA "(°'°)" // Simbolo per la rana con il centro '
#define GRANATA 'o'  // Simbolo per le granate
#define DELAY 100000
#define WINDOW_HEIGHT 24
#define WINDOW_WIDTH 32

typedef struct messaggio {
    int x, y;
    int tipo;        // 1 = Rana, 2 = Granata
    int direzione;   // Direzione per le granate
    char forma[10];  // Forma del personaggio
} messaggio;

void procRana(int *pipe_fds, messaggio movRana);
void procGranata(int *pipe_fds, messaggio granata);
void procControllo(int *pipe_fds);
void cancellaRana(int x, int y);

int main() {
    initscr(); noecho(); curs_set(0); cbreak();

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int offset_y = (LINES - WINDOW_HEIGHT) / 2;
    int offset_x = (COLS - WINDOW_WIDTH) / 2;

    box(stdscr, ACS_VLINE, ACS_HLINE);
    refresh();

    pid_t rana = fork();
    if (rana == 0) {
        messaggio movRana = {offset_x + WINDOW_WIDTH / 2, offset_y + WINDOW_HEIGHT - 2, 1, 0, RANA};
        procRana(pipe_fds, movRana);
    } else {
        procControllo(pipe_fds);
    }

    endwin();
    return 0;
}

void procRana(int *pipe_fds, messaggio movRana) {
    close(pipe_fds[0]);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    int c;

    while (1) {
        c = getch();
        switch (c) {
            case KEY_UP:    if (movRana.y > 1)         movRana.y--; break;
            case KEY_DOWN:  if (movRana.y < LINES - 2) movRana.y++; break;
            case KEY_LEFT:  if (movRana.x > 2)         movRana.x--; break;
            case KEY_RIGHT: if (movRana.x < COLS - 3)  movRana.x++; break;
            case ' ': { // Barra spaziatrice per sparare
                messaggio granataDx = {movRana.x + 3, movRana.y, 2, 1, "o"};
                messaggio granataSx = {movRana.x - 1, movRana.y, 2, -1, "o"};

                if (fork() == 0) {
                    procGranata(pipe_fds, granataDx);
                }
                if (fork() == 0) {
                    procGranata(pipe_fds, granataSx);
                }
                break;
            }
        }
        write(pipe_fds[1], &movRana, sizeof(messaggio));
        usleep(50000);
    }
}

void procGranata(int *pipe_fds, messaggio granata) {
    close(pipe_fds[0]);

    while (1) {
        granata.x += granata.direzione;
        if (granata.x < 1 || granata.x >= COLS - 1) break;
        write(pipe_fds[1], &granata, sizeof(messaggio));
        usleep(50000);
    }

    exit(0);
}

void procControllo(int *pipe_fds) {
    close(pipe_fds[1]);
    messaggio entita;

    while (1) {
        if (read(pipe_fds[0], &entita, sizeof(messaggio)) > 0) {
            if (entita.tipo == 1) { // Rana
                // Cancella la vecchia posizione della rana
                cancellaRana(entita.x, entita.y);

                // Disegna la rana nella nuova posizione
                mvprintw(entita.y, entita.x - 2, "%s", entita.forma);
            } else if (entita.tipo == 2) { // Granata
                mvaddch(entita.y, entita.x - entita.direzione, ' '); // Cancella la vecchia posizione della granata
                mvaddch(entita.y, entita.x, GRANATA); // Disegna la granata
            }
        }
        refresh();
    }
}

void cancellaRana(int x, int y) {
    // Cancella la riga sopra
    mvprintw(y - 1, x - 2, "     ");
    // Cancella la riga attuale
    mvprintw(y, x - 2, "     ");
    // Cancella la riga sotto
    mvprintw(y + 1, x - 2, "     ");
}
