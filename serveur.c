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
    int sock, ret, cnx;
    int size;
    struct sockaddr_in conn_addr, addr;
    char buffer[BUFFER_SIZE];
    size = sizeof(conn_addr);

    // Créer un socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création du socket");
        exit(1);
    }

    // Configurer l'adresse du serveur
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
        perror("inet_pton échoué");
        close(sock);
        exit(1);
    }

    // Lier le socket à l'adresse et au port
    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("Erreur lors du bind");
        close(sock);
        exit(1);
    }

    // Écouter les connexions entrantes
    ret = listen(sock, 1);
    if (ret < 0) {
        perror("Erreur lors de l'écoute du socket");
        close(sock);
        exit(1);
    }

    printf("Serveur en écoute sur 127.0.0.1:8080...\n");

    // Accepter la connexion entrante
    cnx = accept(sock, (struct sockaddr *)&conn_addr, (socklen_t *)&size);
    if (cnx < 0) {
        perror("Erreur lors de l'acceptation de la connexion");
        close(sock);
        exit(1);
    }

    // Communication bidirectionnelle
    while (1) {
        // Lire le message du serveur à envoyer au client
        printf("Serveur : ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            perror("Erreur lors de la lecture de l'input");
            break;
        }
        
        // Envoyer le message au client
        ret = write(cnx, buffer, strlen(buffer));
        if (ret < 0) {
            perror("Erreur lors de l'envoi du message");
            break;
        }

        // Vérifier si l'utilisateur veut quitter
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Fermeture de la connexion avec le client...\n");
            break;
        }

        // Lire la réponse du client
        ret = read(cnx, buffer, BUFFER_SIZE);
        if (ret < 0) {
            perror("Erreur lors de la lecture du message du client");
            break;
        } else if (ret == 0) {
            printf("Le client a fermé la connexion.\n");
            break;
        }

        printf("Client : %s", buffer);  // Afficher la réponse du client
    }

    // Fermer la connexion et le socket
    close(cnx);
    close(sock);

    return 0;
}
