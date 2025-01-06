#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define LOG_FILE "/var/log/port_monitor.log" // plik, do którego skrypt zapisuje
#define INTERVAL 10  // Interwał sprawdzania w sekundach

void monitor_ports(){
    FILE *fp;
    char buffer[256];

    // Otwarcie pliku do nadpisywania
    fp = fopen(LOG_FILE, "a");
    if (fp == NULL){
        syslog(LOG_ERR, "Nie można otworzyć pliku logu: %s", LOG_FILE);
        exit(EXIT_FAILURE);
    }
    
    // Zapisanie aktualnej daty i czasu
    time_t now = time(NULL);
    fprintf(fp, "\nCzas sprawdzenia: %s\n", ctime(&now));

    // Uruchomienie komendy "ss -tuln" i przypisanie do zmiennej
    FILE *ss_output = popen("ss -tuln", "r");

    // Wydobycie outputu komendy do buffera i wpisanie buffera do pliku log
    while (fgets(buffer, sizeof(buffer), ss_output) != NULL){
        fprintf(fp, "%s", buffer);
    }

    pclose(ss_output);
    fclose(fp);
}

void daemonize(){
    pid_t pid;

    // Utworzenie procesu potomnego
    pid = fork();

    if(pid < 0){
        exit(EXIT_FAILURE);
    }

    // Zakończenie procesu nadrzędnego
    if(pid > 0){
        exit(EXIT_SUCCESS);
    }

    // Utworzenie nowej sesji
    if(setsid() < 0){
        exit(EXIT_FAILURE);
    }

    // Ignoruj sygnał SIGHUP
    // Sygnał wyłącza skrypt przy wyłączeniu terminala
    signal(SIGHUP, SIG_IGN);

    // Utwórz kolejny proces potomny, aby nie trzymać kontroli nad terminalem
    pid = fork();

    if(pid < 0){
        exit(EXIT_FAILURE);
    }

    if(pid > 0){
        exit(EXIT_SUCCESS);
    }

    // Zainicjuj syslog
    openlog("port_monitor", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Demon port_monitor uruchomiony.");
}

int main() {
    // Demonizacja procesu
    daemonize();

    // Główna pętla demona
    while(1){
        monitor_ports();  // Monitorowanie otwartych portów
        sleep(INTERVAL);  // Oczekiwanie przez określony czas
    }

    // Zamknij syslog (nigdy nie zostanie osiągnięte)
    closelog();

    return EXIT_SUCCESS;
}
