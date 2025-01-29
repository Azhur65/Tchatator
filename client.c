#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main() {
    int sock, ret;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Créer un socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création du socket");
        exit(1);
    }

    // Configurer l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton échoué");
        close(sock);
        exit(1);
    }

    // Connexion au serveur
    ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        perror("Erreur lors de la connexion au serveur");
        close(sock);
        exit(1);
    }

    // Communication bidirectionnelle
    while (1) {
        // Lire le message du serveur
        ret = read(sock, buffer, BUFFER_SIZE);
        if (ret < 0) {
            perror("Erreur lors de la lecture du message du serveur");
            break;
        } else if (ret == 0) {
            printf("Le serveur a fermé la connexion.\n");
            break;
        }
        printf("Serveur : %s", buffer);  // Afficher le message du serveur

        // Lire le message du client à envoyer au serveur
        printf("Client : ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            perror("Erreur lors de la lecture de l'input");
            break;
        }

        // Envoyer le message au serveur
        ret = write(sock, buffer, strlen(buffer));
        if (ret < 0) {
            perror("Erreur lors de l'envoi du message");
            break;
        }

        // Vérifier si l'utilisateur veut quitter
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Fermeture de la connexion avec le serveur...\n");
            break;
        }
    }

    // Fermer la connexion
    close(sock);

    return 0;
}
