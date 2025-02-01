#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libpq-fe.h>
#include <stdbool.h>
#include <time.h>

#define BUFFER_SIZE 1024

const int MAX_MSG_SIZE = 1000;

//sends the message msg to the user specified by dest
char* send_message(PGconn* dbConn, int src, int dest, char* msg);

//returns a query to send a message
char* insert_message_query(int src, int dest, char* msg);

//checks if the user that sent the request is blocked by the user specified by dest
bool is_blocked(PGconn* dbConn, int src, int dest);

//block a conversation beetwin 2 users
void block(PGconn* dbConn, char* src, char* dest);

int main() {
    int sock, ret, cnx;
    int size;
    struct sockaddr_in conn_addr, addr;
    char buffer[BUFFER_SIZE];
    size = sizeof(conn_addr);

    // Connection a la bdd
    const char CONN_INFO[] = "host=srfc.ventsdouest.dev port=5432 dbname=sae user=sae password=escapade-Venait-s1gner connect_timeout=30";
    PGconn* dbConn = PQconnectdb(CONN_INFO);
    if (PQstatus(dbConn) != CONNECTION_OK) {
        printf("Error with database connection : %s\n", PQerrorMessage(dbConn));
        PQfinish(dbConn);
        exit(1);
    }

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

    PQfinish(dbConn);

    return EXIT_SUCCESS;
}

char* send_message(PGconn* dbConn, int src, int dest, char* msg) {
    char *ret;
    //return codes and messages
    const char OK[] = "200/OK : Message envoyé";
    const char ERROR[] = "400/ERROR : Le message n'a pas été envoyé";
    const char BLOCKED[] = "401/BLOCKED : Le destinataire n'autorise pas les messages de votre part";
    const char LONG[] = "402/LONG : Le message est trop long";
    const char UNDEFINED[] = "404/UNDEFINED : L'identifiant du client est incorrect";
    const char SPAM[] = "405/SPAM : Le message n'a pas été envoyé car trop de messages ont été envoyés en peu de temps";

    //checks
    if (is_blocked(dbConn, src, dest)) {
        ret = malloc(sizeof(BLOCKED));
        strcpy(ret, BLOCKED);
        return ret;
    }

    if (strlen(msg) >= MAX_MSG_SIZE) {
        ret = malloc(sizeof(LONG));
        strcpy(ret, LONG);
        return ret;
    }

    //send to db
    PGresult* res = PQexec(dbConn, insert_message_query(src, dest, msg));
    char* error = PQresultErrorMessage(res);
    if (strcmp(error, "") != 0) {
        if (strstr(error, "message_fk_2") != NULL) {
            ret = malloc(sizeof(UNDEFINED));
            strcpy(ret, UNDEFINED);
            return ret;
        } else {
            ret = malloc(sizeof(ERROR));
            strcpy(ret, ERROR);
            return ret;
        }
    }

    PQclear(res);
    ret = malloc(sizeof(OK));
    strcpy(ret, OK);
    return ret;
}

char* insert_message_query(int src, int dest, char* msg) {
    char* ret;
    char temp[2048];

    char queryBase[] = "INSERT INTO chatator.message(message, envoyeur, receveur, date, modifie) VALUES(";
    sprintf(temp, "%s, %d, %d, %li, false)", msg, src, dest, (long) time(NULL));
    ret = malloc(sizeof(queryBase) + sizeof(char) * (strlen(temp) + 1));
    ret = strcat(queryBase, temp);

    return ret;
}

bool is_blocked(PGconn* dbConn, int src, int dest) {
    bool blocked = false;
    char query[4096];
    
    sprintf(query, "SELECT date_deblocage FROM chatator.conversation WHERE (client_id_1 = %d AND client_id_2 = %d) OR (client_id_2 = %d AND client_id_1 = %d)", src, dest, src, dest);
    PGresult* res = PQexec(dbConn, query);
    if (strstr(PQgetvalue(res, 0, 0), "true") != NULL) blocked = true;


    return blocked;
}

void block(PGconn* dbConn, char* src, char* dest) {
    const char* paramValues[2];
    paramValues[0] = src;
    paramValues[1] = dest;

    PGresult* res = PQexecParams(
        dbConn,
        "UPDATE chatator.conversation SET bloque = true "
        "WHERE (client_id_1 = $1 AND client_id_2 = $2) "
        "OR (client_id_2 = $1 AND client_id_1 = $2);",
        2,               // number of parameters
        NULL,            // parameter types (let PostgreSQL infer)
        paramValues,     // parameter values
        NULL,            // parameter lengths
        NULL,            // parameter formats (text/binary)
        0                // result format (0 for text)
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Error updating data: %s\n", PQerrorMessage(dbConn));
    } else {
        printf("Update successful. Block set to true.\n");
    }

    PQclear(res);
}
