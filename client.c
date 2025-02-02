#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 2048
#define MAX_COMMAND_SIZE 1000
#define MAX_API_KEY_SIZE 100
#define MAX_TOKENS 10

int login();
void msg(int id_envoyeur);
void pull(int id_client);
void history(int id_client);
void history(int id_client);
void modify();
void block(int id_client);
void timeout();
void ban();


int main() {
    char buffer[BUFFER_SIZE];
    bool connecte = false;
    int id_client = 0;
    int reponse;
    

    // Communication bidirectionnelle
    while (1) {
        int reponse;

        printf("1 : LOGIN\n2 : MSG\n3 : PULL\n4 : HISTORY\n5 : MODIFY\n6 : BLOCK\n7 : TIMEOUT\n8 : BAN\nQue voulez vous faire ? : \n");
        scanf(" %d",&reponse);
        fflush(stdin);

        switch(reponse){
            case 1 :
                login();
                break;
            case 2 :
                msg(id_client);
                break;
            case 3 :
                pull(id_client);
                break;
            case 4 :
                history(id_client);
                break;
            case 5 :
                modify();
                break;
            case 6 :
                block(id_client);
                break;
            case 7 :
                timeout();
                break;
            case 8 :
                ban();
                break;
            default :
                printf("Choix inconnu");
        }
    }


    return 0;
}

int login() {
    char api_key[MAX_API_KEY_SIZE];
    char buffer[BUFFER_SIZE];
    char request[BUFFER_SIZE];
    int client_id;

    struct sockaddr_in server_addr;
    int sock, res;

    // Demande de la clé API à l'utilisateur
    printf("Veuillez entrer votre clé API : ");
    scanf("%s", api_key);

    // Retirer le saut de ligne si présent
    size_t len = strlen(api_key);
    if (api_key[len - 1] == '\n') {
        api_key[len - 1] = '\0';
    }

    // Ensure that the formatted string fits within the request buffer
    if (snprintf(request, BUFFER_SIZE, "LOGIN %s", api_key) >= BUFFER_SIZE) {
        fprintf(stderr, "Erreur : la clé API est trop longue.\n");
        return -1;  // Return error if the formatted string exceeds the buffer size
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Server address configuration
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        close(sock);
        exit(1);
    }

    printf("Connected to server...\n");

    // Envoyer la requête au serveur
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Erreur lors de l'envoi de la requête LOGIN");
        return -1;
    }

    // Recevoir l'identifiant client du serveur
    int ret = recv(sock, buffer, BUFFER_SIZE, 0);
    if (ret < 0) {
        perror("Erreur lors de la réception de l'identifiant client");
        return -1;
    }

    buffer[ret] = '\0'; // Assurez-vous que la chaîne est terminée

    // Convertir l'identifiant en entier
    client_id = atoi(buffer);
    printf("Connexion réussie, votre identifiant client est : %d\n", client_id);

    // Fermer la connexion
    close(sock);

    return client_id;
}


void msg(int id_envoyeur) {
    int id_receveur;
    char message[MAX_COMMAND_SIZE];
    char request[BUFFER_SIZE];  // Use BUFFER_SIZE for request buffer
    char server_response[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int sock, res;

    // Demande de l'identifiant du destinataire
    printf("Entrez l'identifiant du destinataire : ");
    scanf("%d", &id_receveur);
    getchar(); // Consommer le saut de ligne restant

    // Demande du message à envoyer
    printf("Entrez votre message : ");
    fgets(message, MAX_COMMAND_SIZE, stdin);  // Use MAX_COMMAND_SIZE here to match the buffer size

    // Retirer le saut de ligne si présent
    size_t len = strlen(message);
    if (message[len - 1] == '\n') {
        message[len - 1] = '\0';
    }

    // Ensure that the formatted string fits in the request buffer
    int required_size = snprintf(NULL, 0, "MSG %d,%d,%s", id_envoyeur, id_receveur, message) + 1;  // +1 for null terminator
    if (required_size >= BUFFER_SIZE) {
        fprintf(stderr, "Message too long to fit in buffer\n");
        return;  // Exit or handle the error gracefully
    }

    // Construire la requête "MSG <id_envoyeur>,<id_receveur>,<message>"
    snprintf(request, BUFFER_SIZE, "MSG %d,%d,%s", id_envoyeur, id_receveur, message);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Server address configuration
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        close(sock);
        exit(1);
    }

    printf("Connected to server...\n");

    // Envoyer la requête au serveur
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Erreur lors de l'envoi du message");
    } else {
        printf("Message envoyé avec succès.\n");
    }

    // Recevoir la réponse du serveur
    int ret = recv(sock, server_response, BUFFER_SIZE, 0);
    if (ret < 0) {
        perror("Erreur lors de la réception de la réponse du serveur");
    } else {
        server_response[ret] = '\0'; // Assurez-vous que la chaîne est terminée
        printf("Réponse du serveur : %s\n", server_response);
    }

    close(sock);
}



void pull(int id_client) {
    char request[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int sock, res;

    // Construire la requête "PULL <id_client>"
    snprintf(request, BUFFER_SIZE, "PULL %d", id_client);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Server address configuration
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        close(sock);
        exit(1);
    }

    printf("Connected to server...\n");

    // Envoyer la requête au serveur
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Erreur lors de l'envoi de la requête PULL");
    } else {
        printf("Requête PULL envoyée avec succès.\n");
    }

    // Recevoir la réponse du serveur
    int ret = recv(sock, server_response, BUFFER_SIZE, 0);
    if (ret < 0) {
        perror("Erreur lors de la réception de la réponse du serveur");
    } else {
        server_response[ret] = '\0'; // Assurez-vous que la chaîne est terminée
        printf("Réponse du serveur : %s\n", server_response);
    }

    close(sock);
}

void history(int id_client) {
    char request[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    char* id_autre = malloc(BUFFER_SIZE * sizeof(char)); 
    struct sockaddr_in server_addr;
    int sock, res;

    if (id_autre == NULL) {
        perror("Erreur d'allocation mémoire");
        return;
    }

    // Demande de l'identifiant du client avec qui vous voulez l'historique
    printf("Identifiant du client avec qui vous voulez l'historique : ");
    fgets(id_autre, BUFFER_SIZE, stdin);

    // Retirer le saut de ligne si présent
    size_t len = strlen(id_autre);
    if (id_autre[len - 1] == '\n') {
        id_autre[len - 1] = '\0';
    }

    // Construire la requête "HISTORY <id_client> <id_autre>"
    snprintf(request, BUFFER_SIZE, "HISTORY %d %s", id_client, id_autre);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Server address configuration
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        close(sock);
        exit(1);
    }

    printf("Connected to server...\n");

    // Envoyer la requête au serveur
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Erreur lors de l'envoi de la requête PULL");
    } else {
        printf("Requête PULL envoyée avec succès.\n");
    }

    // Recevoir la réponse du serveur
    int ret = recv(sock, server_response, BUFFER_SIZE, 0);
    if (ret < 0) {
        perror("Erreur lors de la réception de la réponse du serveur");
    } else {
        server_response[ret] = '\0'; // Assurez-vous que la chaîne est terminée
        printf("Réponse du serveur : %s\n", server_response);
    }

    // Libérer la mémoire allouée pour id_autre
    free(id_autre);
}


void modify() {
    int id_message;
    char new_message[BUFFER_SIZE];
    char request[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int sock, res;

    // Demande de l'identifiant du message à modifier
    printf("Entrez l'identifiant du message à modifier : ");
    scanf("%d", &id_message);
    getchar(); // Consommer le saut de ligne restant

    // Demande du nouveau message
    printf("Entrez le nouveau message : ");
    fgets(new_message, BUFFER_SIZE, stdin);

    // Retirer le saut de ligne si présent
    size_t len = strlen(new_message);
    if (new_message[len - 1] == '\n') {
        new_message[len - 1] = '\0';
    }

    // Limiter la taille de new_message pour éviter l'écrasement du buffer
    int available_space = MAX_COMMAND_SIZE - 20; // Space for "MODIFY <id_message>,<"
    if (strlen(new_message) > available_space) {
        new_message[available_space] = '\0'; // Truncate if necessary
    }

    // Vérifier la taille de la requête avant de la formatter
    int required_size = snprintf(NULL, 0, "MODIFY %d,%s", id_message, new_message) + 1;  // +1 for null terminator
    if (required_size >= MAX_COMMAND_SIZE) {
        fprintf(stderr, "Erreur : la requête est trop longue.\n");
        return;  // Return or handle the error gracefully
    }

    // Construire la requête "MODIFY <id_message>,<new_message>"
    snprintf(request, BUFFER_SIZE, "MODIFY %d,%s", id_message, new_message);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Server address configuration
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        close(sock);
        exit(1);
    }

    printf("Connected to server...\n");

    // Envoyer la requête au serveur
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Erreur lors de l'envoi de la requête MODIFY");
    } else {
        printf("Requête MODIFY envoyée avec succès.\n");
    }

    // Recevoir la réponse du serveur
    int ret = recv(sock, server_response, BUFFER_SIZE, 0);
    if (ret < 0) {
        perror("Erreur lors de la réception de la réponse du serveur");
    } else {
        server_response[ret] = '\0'; // Assurez-vous que la chaîne est terminée
        printf("Réponse du serveur : %s\n", server_response);
    }

    close(sock);
}



void block(int id_client) {
    int id_autre_client;
    char request[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int sock, res;

    // Demande de l'identifiant du client à bloquer
    printf("Entrez l'identifiant du client à bloquer : ");
    scanf("%d", &id_autre_client);

    // Construire la requête "BLOCK <id_client>,<id_autre_client>"
    snprintf(request, BUFFER_SIZE, "BLOCK %d,%d", id_client, id_autre_client);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Server address configuration
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        close(sock);
        exit(1);
    }

    printf("Connected to server...\n");

    // Envoyer la requête au serveur
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Erreur lors de l'envoi de la requête BLOCK");
    } else {
        printf("Requête BLOCK envoyée avec succès.\n");
    }

    // Recevoir la réponse du serveur
    int ret = recv(sock, server_response, BUFFER_SIZE, 0);
    if (ret < 0) {
        perror("Erreur lors de la réception de la réponse du serveur");
    } else {
        server_response[ret] = '\0'; // Assurez-vous que la chaîne est terminée
        printf("Réponse du serveur : %s\n", server_response);
    }

    close(sock);
}

void timeout() {
    int id_autre_client;
    char request[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int sock, res;

    // Demande de l'identifiant du client à bloquer
    printf("Entrez l'identifiant du client à bloquer : ");
    scanf("%d", &id_autre_client);

    // Construire la requête "TIMEOUT <id_client>,<id_autre_client>"
    snprintf(request, BUFFER_SIZE, "TIMEOUT %d", id_autre_client);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Server address configuration
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        close(sock);
        exit(1);
    }

    printf("Connected to server...\n");

    // Envoyer la requête au serveur
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Erreur lors de l'envoi de la requête TIMEOUT");
    } else {
        printf("Requête TIMEOUT envoyée avec succès.\n");
    }

    // Recevoir la réponse du serveur
    int ret = recv(sock, server_response, BUFFER_SIZE, 0);
    if (ret < 0) {
        perror("Erreur lors de la réception de la réponse du serveur");
    } else {
        server_response[ret] = '\0'; // Assurez-vous que la chaîne est terminée
        printf("Réponse du serveur : %s\n", server_response);
    }

    close(sock);
}


void ban() {
    int id_autre_client;
    char request[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int sock, res;

    // Demande de l'identifiant du client à bannir
    printf("Entrez l'identifiant du client à bannir : ");
    scanf("%d", &id_autre_client);

    // Construire la requête "BAN <id_client>,<id_autre_client>"
    snprintf(request, BUFFER_SIZE, "BAN %d", id_autre_client);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Server address configuration
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        close(sock);
        exit(1);
    }

    printf("Connected to server...\n");

    // Envoyer la requête au serveur
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Erreur lors de l'envoi de la requête BAN");
    } else {
        printf("Requête BAN envoyée avec succès.\n");
    }

    // Recevoir la réponse du serveur
    int ret = recv(sock, server_response, BUFFER_SIZE, 0);
    if (ret < 0) {
        perror("Erreur lors de la réception de la réponse du serveur");
    } else {
        server_response[ret] = '\0'; // Assurez-vous que la chaîne est terminée
        printf("Réponse du serveur : %s\n", server_response);
    }

    close(sock);
}