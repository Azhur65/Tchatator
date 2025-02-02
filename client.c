#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_COMMAND_SIZE 1000
#define MAX_KEYS 100
#define MAX_TOKENS 10

int main() {
    int sock, ret;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    char commande[MAX_COMMAND_SIZE];

    const char delimiter_commande[] = " ";
    char *tokens_commande[MAX_TOKENS];
    int token_commande_count = 0;
    int id_client = 0;
    int longueur;
    char* arguments;

    const char delimiter_argument[] = ",";
    char *tokens_argument[MAX_TOKENS];
    int token_argument_count = 0;
    
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
        // Lire le message du client à envoyer au serveur
        printf("Client : ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            perror("Erreur lors de la lecture de l'input");
            break;
        }

        //verification du message à envoyé
        char *tokenc = strtok(buffer, delimiter_commande);
        longueur = strlen(tokenc);
        strncpy(arguments, buffer,longueur + 1);

        if(!strcmp(tokenc,"LOGIN") && !strcmp(tokenc,"MSG") && !strcmp(tokenc,"PULL") && !strcmp(tokenc,"HISTORY") && !strcmp(tokenc,"MODIFY") && !strcmp(tokenc,"BLOCK") && !strcmp(tokenc,"TIMEOUT") && !strcmp(tokenc,"BAN")){
            printf("Commande non reconnue");
            break;
        }else{
            char *tokena = strtok(arguments, delimiter_argument);

            while (tokena != NULL && token_argument_count < MAX_TOKENS) {
                tokens_argument[token_argument_count++] = tokena;
                tokena = strtok(NULL, delimiter_argument);
            }

            // Envoyer le message au serveur
            ret = write(sock, buffer, strlen(buffer));
            if (ret < 0) {
                perror("Erreur lors de l'envoi du message");
                break;
            }

            // Lire le message du serveur
            ret = read(sock, buffer, BUFFER_SIZE);
            if (ret < 0) {
                perror("Erreur lors de la lecture du message du serveur");
                break;
            } else if (ret == 0) {
                printf("Le serveur a fermé la connexion.\n");
                break;
            }

            if(strcmp(tokenc, "LOGIN")){
                if(strcmp(buffer, "201/OKC")){
                    printf("Accès client authorisé");

                }else if(strcmp(buffer,"202/OKP")){
                    printf("Accès professionnel authorisé");

                }else if(strcmp(buffer,"203/OKA")){
                    printf("accès administrateur authorisé");

                }else{
                    printf("Accès refusé");
                }

            }else if(strcmp(tokenc,"MSG")){
                if(strcmp(buffer,"200/OK")){
                    printf("message envoyé\n");

                }else if(strcmp(buffer,"401/BLOCKED")){
                    printf("le destinataire n'authorise pas les messages de votre part\n");

                }else if(strcmp(buffer,"402/LONG")){
                    printf("le message est troplong\n");

                }else if(strcmp(buffer,"404/UNDEFINED")){
                    printf("L'identifiant du client est incorrect\n");

                }else if(strcmp(buffer,"405/SPAM")){
                    printf("Trop de messages ont été envoyés\n");

                }else{
                    printf("400/ERROR : Le message n'a pas été envoyé\n");

                }
            }else if(strcmp(tokenc,"PULL")){
                if(strcmp(buffer,"400/ERROR") || buffer == NULL){
                    printf("Tous les messages n'ont pas pu être réceptionné\n");

                }else{
                    printf("%s\n",buffer);
                }

            }else if(strcmp(tokenc,"HISTORY")){
                if(strcmp(buffer,"404/UNDEFINED")){
                    printf("L'identifiant du est incorrect\n");

                }else if(strcmp(buffer,"400/ERROR") || buffer == NULL){
                    printf("L'historique n'a pas être receptionner\n");

                }else{
                    printf("%s\n",buffer);

                }

            }else if(strcmp(tokenc,"MODIFY")){
                if(strcmp(buffer,"200/OK")){
                    printf("Messaeg modifié avec succès\n");

                }else if(strcmp(buffer,"402/LONG")){
                    printf("le message est trop long\n");

                }else if(strcmp(buffer,"404/UNDEFINED")){
                    printf("L'identifiant du message est incorrect\n");

                }else{
                    printf("Le message n'a pas été modifié\n");

                }

            }else if(strcmp(tokenc,"BLOCK")){
                if(strcmp(buffer,"200/OK")){
                    printf("messages blockés pendant 24 heures\n");

                }else if(strcmp(buffer,"403/DENIED")){
                    printf("vous n'avez pas accès à cette commande\n");

                }else if(strcmp(buffer,"404/UNDEFINED")){
                    printf("identifiants client incorrect\n");

                }else{
                    printf("Erreur sur leblocage du client\n");

                }

            }else if(strcmp(tokenc,"TIMEOUT")){
                if(strcmp(buffer,"200/OK")){
                    printf("messages blockés pendant 24 heures\n");

                }else if(strcmp(buffer,"403/DENIED")){
                    printf("vous n'avez pas accès à cette commande\n");

                }else if(strcmp(buffer,"404/UNDEFINED")){
                    printf("identifiants client incorrect\n");

                }else{
                    printf("Erreur sur leblocage du client\n");
                    
                }

            }else if(strcmp(tokenc,"BAN")){
                if(strcmp(buffer,"200/OK")){
                    printf("messages blockés indefiniment\n");

                }else if(strcmp(buffer,"403/DENIED")){
                    printf("vous n'avez pas accès à cette commande\n");

                }else if(strcmp(buffer,"404/UNDEFINED")){
                    printf("identifiants client incorrect\n");

                }else{
                    printf("Erreur sur leblocage du client\n");
                    
                }

            }else{
                printf("Commande %s non reconnue",tokenc);
            }
        }
    }

    // Fermer la connexion
    close(sock);

    return 0;
}
