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
void msg(char* id_envoyeur);
void pull(char* id_client);
void history(char* id_client);
void history(char* id_client);
void modify();
void block(char* id_client);
void timeout();
void ban();


int main() {
    char buffer[BUFFER_SIZE];
    bool connecte = false;
    char* id_client = "Co-0001";
    int reponse,sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }
    

    // Communication bidirectionnelle
    while (1) {

        printf("1 : LOGIN\n2 : MSG\n3 : PULL\n4 : HISTORY\n5 : MODIFY\n6 : BLOCK\n7 : TIMEOUT\n8 : BAN\nQue voulez vous faire ? : \n");
        scanf(" %d",&reponse);

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

    close(sock);    
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
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        exit(1);
    }


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

    // Fermer la connexio

    return client_id;
}


void msg(char* id_envoyeur) {
    char id_receveur[MAX_API_KEY_SIZE];
    char message[MAX_COMMAND_SIZE];
    char request[BUFFER_SIZE];  // Use BUFFER_SIZE for request buffer
    char server_response[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int sock, res;

    // Demande de l'identifiant du destinataire
    printf("Entrez l'identifiant du destinataire : ");
    scanf("%s", id_receveur);
    getchar(); // Consommer le saut de ligne restant

    // Demande du message à envoyer
    printf("Entrez votre message : ");
    scanf("%s",message);  // Use MAX_COMMAND_SIZE here to match the buffer size

    // Retirer le saut de ligne si présent
    size_t len = strlen(message);
    if (message[len - 1] == '\n') {
        message[len - 1] = '\0';
    }

    // Ensure that the formatted string fits in the request buffer
    int required_size = snprintf(NULL, 0, "MSG %s,%s,%s", id_envoyeur, id_receveur, message) + 1;  // +1 for null terminator
    if (required_size >= BUFFER_SIZE) {
        fprintf(stderr, "Message too long to fit in buffer\n");
        return;  // Exit or handle the error gracefully
    }

    // Construire la requête "MSG <id_envoyeur>,<id_receveur>,<message>"
    snprintf(request, BUFFER_SIZE, "MSG %s,%s,%s", id_envoyeur, id_receveur, message);

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
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        exit(1);
    }


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

}



void pull(char* id_client) {
    char request[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int sock, res;

    // Construire la requête "PULL <id_client>"
    snprintf(request, BUFFER_SIZE, "PULL %s", id_client);

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
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        exit(1);
    }


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

}

void history(char* id_client) {
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
    scanf("%s",id_autre);

    // Retirer le saut de ligne si présent
    size_t len = strlen(id_autre);
    if (id_autre[len - 1] == '\n') {
        id_autre[len - 1] = '\0';
    }

    // Construire la requête "HISTORY <id_client> <id_autre>"
    snprintf(request, BUFFER_SIZE, "HISTORY %s,%s", id_client, id_autre);

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


    // Envoyer la requête au serveur
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Erreur lors de l'envoi de la requête HISTORY");
    } else {
        printf("Requête HISTORY envoyée avec succès.\n");
    }

    // Recevoir la réponse du serveur
    int ret = recv(sock, server_response, BUFFER_SIZE, 0);
    if (ret < 0) {
        perror("Erreur lors de la réception de la réponse du serveur");
    } else {
        server_response[ret] = '\0'; // Assurez-vous que la chaîne est terminée
        printf("Réponse du serveur : %s\n", server_response);
    }

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
    scanf("%s",new_message);


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
}


void block(char* id_client) {
    char id_autre_client[MAX_API_KEY_SIZE];
    char request[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int sock, res;

    // Demande de l'identifiant du client à bloquer
    printf("Entrez l'identifiant du client à bloquer : ");
    scanf("%s", id_autre_client);

    // Construire la requête "BLOCK <id_client>,<id_autre_client>"
    snprintf(request, BUFFER_SIZE, "BLOCK %s,%s", id_client, id_autre_client);
    
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
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        exit(1);
    }


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
        exit(1);
    }

    // Connect to the server
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Error connecting to server");
        exit(1);
    }


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

    // Construire la requête "BAN <id_autre_client>"
    snprintf(request, BUFFER_SIZE, "BAN %d", id_autre_client);

    // Créer une socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création de la socket");
        return;
    }

    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Échec de inet_pton");
        close(sock);
        return;
    }

    // Connexion au serveur
    res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0) {
        perror("Erreur de connexion au serveur");
        close(sock);
        return;
    }

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

    // Fermer la connexion socket
    close(sock);
}
