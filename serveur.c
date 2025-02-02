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
char* send_message(PGconn* dbConn, char* src, char* dest, char* msg);

//returns a query to send a message
PGresult* insert_message_query(PGconn* dbConn, char* src, char* dest, char* msg);

//checks if the user that sent the request is blocked by the user specified by dest
bool is_blocked(PGconn* dbConn, char* src, char* dest);

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
    /*
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
    close(sock);*/

    //TEST
    printf("block\n");
    block(dbConn, "Co-0001", "Co-0002");

    printf("send_message\n");
    send_message(dbConn, "Co-0003", "Co-0002", "message de test");

    PQfinish(dbConn);

    return EXIT_SUCCESS;
}

char* send_message(PGconn* dbConn, char* src, char* dest, char* msg) {

    char *ret;

    const char OK[] = "200/OK : Message envoyé";
    const char ERROR[] = "400/ERROR : Le message n'a pas été envoyé";
    const char BLOCKED[] = "401/BLOCKED : Le destinataire n'autorise pas les messages de votre part";
    const char LONG[] = "402/LONG : Le message est trop long";
    const char UNDEFINED[] = "404/UNDEFINED : L'identifiant du client est incorrect";

    if (is_blocked(dbConn, src, dest)) {
        ret = malloc(strlen(BLOCKED) + 1);
        strcpy(ret, BLOCKED);
        return ret;
    }

    if (strlen(msg) >= MAX_MSG_SIZE) {
        ret = malloc(strlen(LONG) + 1);
        strcpy(ret, LONG);
        return ret;
    }

    // Call insert_message_query and store the result in res
    PGresult* res = insert_message_query(dbConn, src, dest, msg);

    // Check if there was an error inserting the message
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    // Print the detailed error message for debugging
    fprintf(stderr, "Error executing query: %s\n", PQerrorMessage(dbConn));
    if (strstr(PQerrorMessage(dbConn), "message_fk_2") != NULL) {
        ret = malloc(strlen(UNDEFINED) + 1);
        strcpy(ret, UNDEFINED);
    } else {
        ret = malloc(strlen(ERROR) + 1);
        strcpy(ret, ERROR);
    }
    PQclear(res);
    return ret;
}


    // If everything went well, return the success message
    ret = malloc(strlen(OK) + 1);
    strcpy(ret, OK);
    PQclear(res);
    return ret;
}

PGresult* insert_message_query(PGconn* dbConn, char* src, char* dest, char* msg) {
    const char* paramValues[4];
    time_t currentTime = time(NULL);  // Get the current Unix timestamp
    
    // Convert the timestamp into a string to ensure it can be passed properly
    char timestampStr[20];  // Buffer to hold the timestamp as a string
    snprintf(timestampStr, sizeof(timestampStr), "%ld", (long)currentTime);
    
    paramValues[0] = msg;
    paramValues[1] = src;
    paramValues[2] = dest;
    paramValues[3] = timestampStr;  // Pass the Unix timestamp as a string to the query

    // Execute the query
    PGresult* res = PQexecParams(
        dbConn,
        "INSERT INTO chatator.message(message, envoyeur, receveur, date, modifie) "
        "VALUES($1, $2, $3, to_timestamp($4), false);",  // Use `to_timestamp()` to convert Unix timestamp to PostgreSQL timestamp
        4,  // Number of parameters
        NULL,  // Param descriptions
        paramValues,  // The actual parameter values
        NULL,  // Output columns
        NULL,  // Output lengths
        0  // Flags
    );

    return res;  // Return the result pointer for further handling
}





bool is_blocked(PGconn* dbConn, char* src, char* dest) {
    bool blocked = false;
    const char* paramValues[2] = { src, dest };
    
    PGresult* res = PQexecParams(
        dbConn,
        "SELECT bloque FROM chatator.conversation WHERE (client_id_1 = $1 AND client_id_2 = $2) "
        "OR (client_id_2 = $1 AND client_id_1 = $2);",
        2,
        NULL,
        paramValues,
        NULL,
        NULL,
        0
    );
    
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        if (strcmp(PQgetvalue(res, 0, 0), "t") == 0) {
            blocked = true;
        }
    }
    PQclear(res);
    return blocked;
}


void block(PGconn* dbConn, char* src, char* dest) {
    const char* paramValues[3];
    paramValues[0] = src;
    paramValues[1] = dest;

    // Get current timestamp as a string
    char timestampStr[20];
    snprintf(timestampStr, sizeof(timestampStr), "%ld", time(NULL));
    paramValues[2] = timestampStr;

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
        printf("Error updating bloque: %s\n", PQerrorMessage(dbConn));
    } else {
        printf("Update successful. Block set to true.\n");
    }

    PQclear(res);

    res = PQexecParams(
        dbConn,
        "UPDATE chatator.conversation SET date_deblocage = to_timestamp($3) "
        "WHERE (client_id_1 = $1 AND client_id_2 = $2) "
        "OR (client_id_2 = $1 AND client_id_1 = $2);",
        3,               // number of parameters
        NULL,            // parameter types (let PostgreSQL infer)
        paramValues,     // parameter values
        NULL,            // parameter lengths
        NULL,            // parameter formats (text/binary)
        0                // result format (0 for text)
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Error updating date_deblocage: %s\n", PQerrorMessage(dbConn));
    } else {
        printf("Update successful. Timestamp set to now.\n");
    }

    PQclear(res);
}

