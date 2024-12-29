#include <time.h>  // Libreria per gestire il tempo
#include <stdio.h>
#include <stdlib.h>  // Libreria per funzioni standard di sistema
#include <unistd.h>  // Libreria per funzioni POSIX, come fork e pipe
#include <ncurses.h> // Libreria per gestire l'interfaccia grafica su terminale
#include <sys/wait.h> // Libreria per la gestione dei processi

#define RANA '(°°)' // Simbolo per la rana
#define COCCODRILLO_DX '~~~OOO^==' // Simbolo per il coccodrillo verso destra
#define COCCODRILLO_SX '==^OOO~~~' // Simbolo per il coccodrillo verso sinistra
#define DELAY 100000 // Ritardo tra i movimenti (in microsecondi)

// Struttura per rappresentare la posizione e il tipo di ogni entità
typedef struct messaggio {
    int x, y;          // Coordinate (x, y) del personaggio
    int tipo;          // Tipo di entità (1 = Rana, 2 = Coccodrillo)
    char forma[10];    // Forma del personaggio (es. '(°°)' o '~~~OOO^==')
} messaggio;

// Prototipi delle funzioni
void procRana(int *pipe_fds, messaggio movR);
void procCoccodrillo(int *pipe_fds, messaggio movC);
void procControllo(int *pipe_fds, messaggio movR, messaggio movC);

int main() {
    // Inizializza la modalità ncurses per la grafica a terminale
    initscr(); noecho(); curs_set(0); cbreak();
  
    int pipe_fds[2];  // File descriptor per la pipe (comunicazione tra processi)
    pid_t rana, coccodrillo;  // PID per i processi rana e coccodrillo

    // Inizializzazione delle posizioni iniziali di Rana e Coccodrillo

}

// Processo che gestisce il movimento della rana
void procRana(int *pipe_fds, messaggio movR) {
    close(pipe_fds[0]);  // Chiude il lato di lettura della pipe
    keypad(stdscr, TRUE);  // Abilita l'uso dei tasti freccia
    int c;

    while (1) {
        c = (int) getch();  // Legge l'input dell'utente
        // Aggiorna la posizione della rana in base all'input dell'utente
        switch (c) {
            case KEY_UP:    if (movR.y > 1)         movR.y--; break;  // Su
            case KEY_DOWN:  if (movR.y < LINES - 2) movR.y++; break;  // Giù
            case KEY_LEFT:  if (movR.x > 1)         movR.x--; break;  // Sinistra
            case KEY_RIGHT: if (movR.x < COLS - 2)  movR.x++; break;  // Destra
        }

        // Scrive la nuova posizione nella pipe
        write(pipe_fds[1], &movR, sizeof(messaggio));
    }
}