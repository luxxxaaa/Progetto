#include <time.h>  // Libreria per gestire il tempo
#include <stdio.h>
#include <stdlib.h>  // Libreria per funzioni standard di sistema
#include <unistd.h>  // Libreria per funzioni POSIX, come fork e pipe
#include <ncurses.h> // Libreria per gestire l'interfaccia grafica su terminale
#include <sys/wait.h> // Libreria per la gestione dei processi
//ciaoneeeeeeeeeeeee

// Definisce i simboli per i personaggi
#define VESPA    '^'    // Simbolo per la vespa
#define CONTADINO  '#'   // Simbolo per il contadino
#define TRAPPOLE 'x'     // Simbolo per le trappole
#define DELAY     100000 // Ritardo tra i movimenti (in microsecondi)
#define TRAP_COUNT 3     // Numero di trappole

// Struttura per rappresentare la posizione e il tipo di ogni entità
typedef struct messaggio
{
    int x, y;   // Coordinate (x, y)
    int tipo;   // Tipo di entità (1 = Vespa, 2 = Contadino)
} messaggio;

// Prototipi delle funzioni
void procVespa          (int *pipe_fds, messaggio movV);
void procContadino      (int *pipe_fds, messaggio movC);
void procControllo      (int *pipe_fds, messaggio movR, messaggio movV, messaggio movC);

int main()

{
    // Inizializza la modalità ncurses per la grafica a terminale
    initscr(); noecho(); curs_set(0); cbreak();
  
    int pipe_fds[2];  // File descriptor per la pipe (comunicazione tra processi)
    pid_t vespa, contadino;  // PID per i processi vespa e contadino

    // Inizializzazione delle posizioni iniziali di Vespa e Contadino
    messaggio movC = {COLS - 2, LINES - 2, 2};  // Contadino in basso a destra
    messaggio movV = {COLS / 2, LINES / 2, 1};  // Vespa al centro
    messaggio movR;

    // Disegna il bordo dello schermo
    box(stdscr, ACS_VLINE, ACS_HLINE);
    mvaddch(movC.y, movC.x, CONTADINO);  // Disegna il contadino
    mvaddch(movV.y, movV.x, VESPA);      // Disegna la vespa
    refresh();

    // Crea una pipe per la comunicazione tra i processi
    if (pipe(pipe_fds) == -1) {exit(1);}
    
    // Crea il processo per il contadino
    contadino = fork();
    if (contadino < 0)       {exit(2);}
    else if (contadino == 0) {procContadino(pipe_fds, movC);}
    else
    {
        // Crea il processo per la vespa
        vespa = fork();
        if (vespa < 0)       {exit(2);}
        else if (vespa == 0) {procVespa(pipe_fds, movV);}
        else
        {
            // Nel processo padre, esegue la funzione di controllo
            procControllo(pipe_fds, movR, movC, movV);
        }
        
    }    
    
    endwin();  // Termina la modalità ncurses
    exit(3);
}

// Processo che gestisce il movimento della vespa
void procVespa  (int *pipe_fds, messaggio movV) {
    srand(time(NULL) ^ (getpid() << 16));  // Inizializza il generatore di numeri casuali
    close(pipe_fds[0]);  // Chiude il lato di lettura della pipe
    int c;
    int N;
    while (1)
    {
        // Genera un numero casuale per determinare se la vespa deve muoversi
        N = (rand() % 10) + 1;
        
        if (N == 4)
        {
           c = (rand() % 4) + 1;  // Determina la direzione casuale
        }
        
        // Aggiorna la posizione della vespa in base alla direzione scelta
        switch (c)
        {
            case 1: if(movV.y > 1)         movV.y--; break;  // Su
            case 2: if(movV.y < LINES - 2) movV.y++; break;  // Giù
            case 3: if(movV.x > 1)         movV.x--; break;  // Sinistra
            case 4: if(movV.x < COLS - 2)  movV.x++; break;  // Destra
        }

        // Scrive la nuova posizione nella pipe
        write(pipe_fds[1], &movV, sizeof(messaggio));
        usleep(DELAY);  // Ritardo tra i movimenti
    }
}

// Processo che gestisce il movimento del contadino
void procContadino   (int *pipe_fds, messaggio movC) {
    close(pipe_fds[0]);  // Chiude il lato di lettura della pipe
    keypad(stdscr, TRUE);  // Abilita l'uso dei tasti freccia
    int c;

    while (1)
    {
        c = (int) getch();  // Legge l'input dell'utente
        // Aggiorna la posizione del contadino in base all'input dell'utente
        switch (c)
        {
            case KEY_UP:    if(movC.y > 1)         movC.y--; break;  // Su
            case KEY_DOWN:  if(movC.y < LINES - 2) movC.y++; break;  // Giù
            case KEY_LEFT:  if(movC.x > 1)         movC.x--; break;  // Sinistra
            case KEY_RIGHT: if(movC.x < COLS - 2)  movC.x++; break;  // Destra
        }

        // Scrive la nuova posizione nella pipe
        write(pipe_fds[1], &movC, sizeof(messaggio));
    }
}

// Processo di controllo che gestisce la logica del gioco
void procControllo (int *pipe_fds, messaggio movR, messaggio movC, messaggio movV) {
    close(pipe_fds[1]);  // Chiude il lato di scrittura della pipe
    int viteV = 3;  // Vita della vespa
    int viteC = 3;  // Vita del contadino
    time_t start_time = time(NULL);  // Inizializza il timer
    char timer_str[10];

    messaggio trappole[TRAP_COUNT];  // Array per le trappole
    srand(time(NULL));
    for (int i = 0; i < TRAP_COUNT; i++) {
        // Genera una posizione casuale per ciascuna trappola
        trappole[i].x = (rand() % (COLS - 2)) + 1;
        trappole[i].y = (rand() % (LINES - 2)) + 1;
        mvaddch(trappole[i].y, trappole[i].x, TRAPPOLE);  // Disegna la trappola
    }
    refresh();
    time_t trap_start_time = time(NULL);  // Tempo di inizio per il posizionamento delle trappole

    while (1)
    {
        // Calcola il tempo trascorso
        time_t current_time = time(NULL);
        int elapsed_time = (int)difftime(current_time, start_time);
        int minutes = elapsed_time / 60;
        int seconds = elapsed_time % 60;
        snprintf(timer_str, sizeof(timer_str), "%02d:%02d", minutes, seconds);
        mvprintw(0, (COLS / 2) - 5, "Tempo: %s", timer_str);  // Mostra il timer al centro in alto

        // Aggiorna la posizione delle trappole ogni 10 secondi
        if (difftime(current_time, trap_start_time) >= 10) {
            for (int i = 0; i < TRAP_COUNT; i++) {
                mvaddch(trappole[i].y, trappole[i].x, ' ');  // Cancella la vecchia posizione
                // Genera una nuova posizione per la trappola
                trappole[i].x = (rand() % (COLS - 2)) + 1;
                trappole[i].y = (rand() % (LINES - 2)) + 1;
                mvaddch(trappole[i].y, trappole[i].x, TRAPPOLE);  // Ridisegna la trappola nella nuova posizione
            }
            trap_start_time = current_time;
        }

        // Legge la nuova posizione dalla pipe
        read(pipe_fds[0], &movR, sizeof(messaggio));

        // Aggiorna la posizione del contadino
        if (movR.tipo == 2)
        {
            mvaddch(movC.y, movC.x, ' ');  // Cancella la vecchia posizione del contadino
            movC.x = movR.x;
            movC.y = movR.y;
            mvaddch(movC.y, movC.x, CONTADINO);  // Ridisegna il contadino
        }
        // Aggiorna la posizione della vespa
        else if (movR.tipo == 1)
        {
            mvaddch(movV.y, movV.x, ' ');  // Cancella la vecchia posizione della vespa
            movV.x = movR.x;
            movV.y = movR.y;
            mvaddch(movV.y, movV.x, VESPA);  // Ridisegna la vespa
        }

        // Controllo collisioni con le trappole
        for (int i = 0; i < TRAP_COUNT; i++) {
            if (movV.x == trappole[i].x && movV.y == trappole[i].y) {

                viteV--;  // Se la vespa colpisce una trappola, perde una vita

                

                
                if (viteC < 6)
                    viteC++;  // Se la vespa colpisce una trappola, il contadino guadagna una vita
                
            }
        }

        // Controllo collisioni tra Vespa e Contadino
        if (movC.x == movV.x && movC.y == movV.y) {
            viteC--;  // Se il contadino e la vespa si incontrano, il contadino perde una vita
        }

        // Controlla se la vespa ha perso tutte le vite
        if (viteV == 0 ){
            mvprintw(LINES / 2, COLS / 2, "Vittoria!");
            refresh();
            usleep(DELAY * 20);
            endwin();
            _exit(0);
        }     
        // Controlla se il contadino ha perso tutte le vite
        if (viteC == 0 ){
            mvprintw(LINES / 2, COLS / 2, "Sconfitta!");
            refresh();
            usleep(DELAY * 20);
            endwin();
            _exit(0);
        }
        refresh();  // Aggiorna lo schermo
    }
}
