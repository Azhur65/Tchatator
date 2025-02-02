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

#define BUFFER_SIZE 4096

const int MAX_MSG_SIZE = 1000;

//sends the message msg to the user specified by dest
char* send_message(PGconn* dbConn, char* src, char* dest, char* msg);

//returns a query to send a message
PGresult* insert_message_query(PGconn* dbConn, char* src, char* dest, char* msg);

//checks if the user that sent the request is blocked by the user specified by dest
bool is_blocked(PGconn* dbConn, char* src, char* dest);

//block a conversation beetwin 2 users
void block(PGconn* dbConn, char* src, char* dest);

//checks client api key and tells what type of account it is
char* login(PGconn* dbConn, char* api_key);

char* pull_messages(PGconn* dbConn, char* client_id);
char* get_history(PGconn* dbConn, char* client_id, char* target_id, char* last_message_id, int limit);
char* modify_message(PGconn* dbConn, char* message_id, char* new_message);
char* timeout_user(PGconn* dbConn, char* admin_id, char* client_id);
char* ban_user(PGconn* dbConn, char* admin_id, char* client_id);

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
        
        // Lire la requête du client
        ret = read(cnx, buffer, BUFFER_SIZE);
        if (ret < 0) {
            perror("Erreur lors de la lecture du message du client");
            break;
        } else if (ret == 0) {
            printf("Le client a fermé la connexion.\n");
            break;
        }

        // Supprimer le retour à la ligne
        buffer[strcspn(buffer, "\n")] = 0;

        char *response = "400/ERROR : Commande inconnue";

        // Découper la commande et ses arguments
        char *commande = strtok(buffer, " ");

        if (commande) {
            if (strcmp(commande, "LOGIN") == 0) {
                char *api_key = strtok(NULL, " ");
                if (api_key) {
                    response = login(dbConn, api_key);
                } else {
                    response = "400/ERROR : Argument manquant (API Key)";
                }

            } else if (strcmp(commande, "MSG") == 0) {
                char *src = strtok(NULL, ",");
                char *dest = strtok(NULL, ",");
                char *msg = strtok(NULL, "");
                if (src && dest && msg) {
                    response = send_message(dbConn, src, dest, msg);
                } else {
                    response = "400/ERROR : Arguments manquants (src,dest,msg)";
                }

            } else if (strcmp(commande, "PULL") == 0) {
                char *client_id = strtok(NULL, " ");
                if (client_id) {
                    response = pull_messages(dbConn, client_id);
                } else {
                    response = "400/ERROR : Argument manquant (client_id)";
                }

            } else if (strcmp(commande, "HISTORY") == 0) {
                char *client_id = strtok(NULL, ",");
                char *target_id = strtok(NULL, ",");
                char *last_message_id = strtok(NULL, ",");
                char *limit_str = strtok(NULL, ",");
                int limit = (limit_str != NULL) ? atoi(limit_str) : 10;
                if (client_id && target_id) {
                    response = get_history(dbConn, client_id, target_id, last_message_id, limit);
                } else {
                    response = "400/ERROR : Arguments manquants (client_id, target_id)";
                }

            } else if (strcmp(commande, "MODIFY") == 0) {
                char *message_id = strtok(NULL, ",");
                char *new_msg = strtok(NULL, "");
                if (message_id && new_msg) {
                    response = modify_message(dbConn, message_id, new_msg);
                } else {
                    response = "400/ERROR : Arguments manquants (message_id, new_msg)";
                }

            } else if (strcmp(commande, "BLOCK") == 0) {
                char *src = strtok(NULL, ",");
                char *dest = strtok(NULL, "");
                if (src && dest) {
                    block(dbConn, src, dest);
                    response = "200/OK : Conversation bloquée";
                } else {
                    response = "400/ERROR : Arguments manquants (src, dest)";
                }

            } else if (strcmp(commande, "TIMEOUT") == 0) {
                char *admin_id = strtok(NULL, ",");
                char *client_id = strtok(NULL, "");
                if (admin_id && client_id) {
                    response = timeout_user(dbConn, admin_id, client_id);
                } else {
                    response = "400/ERROR : Arguments manquants (admin_id, client_id)";
                }

            } else if (strcmp(commande, "BAN") == 0) {
                char *admin_id = strtok(NULL, ",");
                char *client_id = strtok(NULL, "");
                if (admin_id && client_id) {
                    response = ban_user(dbConn, admin_id, client_id);
                } else {
                    response = "400/ERROR : Arguments manquants (admin_id, client_id)";
                }
            }
        }

        // Envoyer la réponse au client
        write(cnx, response, strlen(response));

    }

    // Fermer la connexion et le socket
    close(cnx);
    close(sock);

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

    // Vérifier si la conversation est bloquée (avec déblocage automatique si nécessaire)
    if (is_blocked(dbConn, src, dest)) {
        ret = malloc(strlen(BLOCKED) + 1);
        strcpy(ret, BLOCKED);
        return ret;
    }

    // Le reste de la logique d'envoi de message reste inchangé
    if (strlen(msg) >= MAX_MSG_SIZE) {
        ret = malloc(strlen(LONG) + 1);
        strcpy(ret, LONG);
        return ret;
    }

    // Envoyer le message
    PGresult* res = insert_message_query(dbConn, src, dest, msg);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erreur lors de l'envoi du message : %s\n", PQerrorMessage(dbConn));
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

    // Message envoyé avec succès
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

    // Requête pour vérifier si la conversation est bloquée et si le blocage a expiré
    PGresult* res = PQexecParams(
        dbConn,
        "SELECT bloque, date_bloquage FROM chatator.conversation "
        "WHERE (client_id_1 = $1 AND client_id_2 = $2) "
        "OR (client_id_2 = $1 AND client_id_1 = $2);",
        2, NULL, paramValues, NULL, NULL, 0
    );

    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        // Vérifier si la conversation est bloquée
        if (strcmp(PQgetvalue(res, 0, 0), "t") == 0) {
            // Récupérer la date de blocage
            char* date_bloquage_str = PQgetvalue(res, 0, 1);
            time_t date_bloquage = (time_t)atol(date_bloquage_str); // Convertir en timestamp
            time_t now = time(NULL); // Date actuelle

            // Vérifier si le blocage a expiré (plus de 24 heures)
            if (difftime(now, date_bloquage) >= 24 * 60 * 60) {
                // Débloquer la conversation
                const char* unblockParams[2] = { src, dest };
                PGresult* unblockRes = PQexecParams(
                    dbConn,
                    "UPDATE chatator.conversation SET bloque = false, date_bloquage = NULL "
                    "WHERE (client_id_1 = $1 AND client_id_2 = $2) "
                    "OR (client_id_2 = $1 AND client_id_1 = $2);",
                    2, NULL, unblockParams, NULL, NULL, 0
                );

                if (PQresultStatus(unblockRes) != PGRES_COMMAND_OK) {
                    printf("Erreur lors du déblocage de la conversation : %s\n", PQerrorMessage(dbConn));
                } else {
                    printf("Conversation débloquée automatiquement.\n");
                }

                PQclear(unblockRes);
            } else {
                // Le blocage est toujours actif
                blocked = true;
            }
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

char* login(PGconn* dbConn, char* api_key) {
    char *ret;
    const char OKC[] = "201/OKC : Accès client authorisé, ";
    const char OKP[] = "202/OKP : Accès professionel authorisé, ";
    const char OKA[] = "203/OKA : Accès administrateur authorisé, ";
    const char DENIED[] = "403/DENIED : Accès refusé";

    const char* paramValues[1] = { api_key };
    PGresult* res = PQexecParams(
        dbConn,
        "SELECT client_id, status FROM chatator.client WHERE api_key = $1",
        1, NULL, paramValues, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        ret = malloc(strlen(DENIED) + 1);
        strcpy(ret, DENIED);
    } else {
        char* client_id = PQgetvalue(res, 0, 0);
        char* status = PQgetvalue(res, 0, 1);

        if (strcmp(status, "client") == 0) {
            ret = malloc(strlen(OKC) + strlen(client_id) + 1);
            sprintf(ret, "%s%s", OKC, client_id);
        } else if (strcmp(status, "professionnel") == 0) {
            ret = malloc(strlen(OKP) + strlen(client_id) + 1);
            sprintf(ret, "%s%s", OKP, client_id);
        } else if (strcmp(status, "administrateur") == 0) {
            ret = malloc(strlen(OKA) + strlen(client_id) + 1);
            sprintf(ret, "%s%s", OKA, client_id);
        } else {
            ret = malloc(strlen(DENIED) + 1);
            strcpy(ret, DENIED);
        }
    }

    PQclear(res);
    return ret;
}

char* pull_messages(PGconn* dbConn, char* client_id) {
    char *ret;
    const char OK[] = "200/OK : Tous les messages ont été reçus avec succès\n";
    const char ERROR[] = "400/ERROR : Tous les messages n'ont pas pus être receptionnés";

    const char* paramValues[1] = { client_id };
    PGresult* res = PQexecParams(
        dbConn,
        "SELECT message_id, message FROM chatator.message WHERE receveur = $1 AND lu = false",
        1, NULL, paramValues, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        ret = malloc(strlen(ERROR) + 1);
        strcpy(ret, ERROR);
    } else {
        // Construire la réponse avec les messages
        char response[BUFFER_SIZE];
        strcpy(response, OK);

        for (int i = 0; i < PQntuples(res); i++) {
            char message_line[512];
            sprintf(message_line, "%s, %s\n",
                PQgetvalue(res, i, 0),  // message_id
                PQgetvalue(res, i, 1)   // message
            );
            strcat(response, message_line);
        }

        ret = malloc(strlen(response) + 1);
        strcpy(ret, response);
    }

    PQclear(res);
    return ret;
}

char* get_history(PGconn* dbConn, char* client_id, char* target_id, char* last_message_id, int limit) {
    printf("args : %s, %s, %s, %d\n", client_id, target_id, last_message_id, limit);
    char *ret;
    const char OK[] = "200/OK : L'Historique à bien été reçu\n";
    const char ERROR[] = "400/ERROR : L'Historique n'a pas pu être receptionné";
    const char UNDEFINED[] = "404/UNDEFINED : L'identifiant du client est incorrect";

    // Vérifier si les identifiants des clients existent
    const char* paramValues[2] = { client_id, target_id };
    PGresult* res = PQexecParams(
        dbConn,
        "SELECT client_id FROM chatator.client WHERE client_id = $1 OR client_id = $2",
        2, NULL, paramValues, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) < 2) {
        ret = malloc(strlen(UNDEFINED) + 1);
        strcpy(ret, UNDEFINED);
        PQclear(res);
        return ret;
    }
    PQclear(res);

    // Construire la requête SQL en fonction de last_message_id
    char query[4096];
    if (last_message_id == NULL || strcmp(last_message_id, "") == 0) {
        // Récupérer les derniers messages
        sprintf(query,
            "SELECT message_id, message FROM chatator.message "
            "WHERE (envoyeur = $1 AND receveur = $2) OR (envoyeur = $2 AND receveur = $1) "
            "ORDER BY date DESC "
            "LIMIT %d", limit);
    } else {
        // Récupérer les messages précédant last_message_id
        sprintf(query,
            "SELECT message_id, message FROM chatator.message "
            "WHERE ((envoyeur = $1 AND receveur = $2) OR (envoyeur = $2 AND receveur = $1)) "
            "AND message_id < %s "
            "ORDER BY date DESC "
            "LIMIT %d", last_message_id, limit);
    }

    // Exécuter la requête
    res = PQexecParams(
        dbConn,
        query,
        2, NULL, paramValues, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        ret = malloc(strlen(ERROR) + 1);
        strcpy(ret, ERROR);
    } else if (PQntuples(res) == 0) {
        ret = malloc(strlen(UNDEFINED) + 1);
        strcpy(ret, UNDEFINED);
    } else {
        // Construire la réponse avec les messages
        char response[BUFFER_SIZE];
        strcpy(response, OK);

        for (int i = 0; i < PQntuples(res); i++) {
            char message_line[512];
            sprintf(message_line, "%s, %s\n",
                PQgetvalue(res, i, 0),  // message_id
                PQgetvalue(res, i, 1)   // message
            );
            strcat(response, message_line);
        }

        ret = malloc(strlen(response) + 1);
        strcpy(ret, response);
    }

    PQclear(res);
    return ret;
}

char* modify_message(PGconn* dbConn, char* message_id, char* new_message) {
    printf("args : %s, %s\n", message_id, new_message);
    char *ret;
    const char OK[] = "200/OK : Message modifié avec succès";
    const char ERROR[] = "400/ERROR : Le message n'a pas été modifié";
    const char LONG[] = "402/LONG : Le nouveau message est trop long";
    const char UNDEFINED[] = "404/UNDEFINED : L'identifiant du message est incorrect";

    if (strlen(new_message) >= MAX_MSG_SIZE) {
        ret = malloc(strlen(LONG) + 1);
        strcpy(ret, LONG);
        return ret;
    }

    const char* paramValues[2] = { new_message, message_id };
    PGresult* res = PQexecParams(
        dbConn,
        "UPDATE chatator.message SET message = $1, modifie = true WHERE message_id = $2",
        2, NULL, paramValues, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        ret = malloc(strlen(ERROR) + 1);
        strcpy(ret, ERROR);
    } else if (PQntuples(res) == 0) {
        ret = malloc(strlen(UNDEFINED) + 1);
        strcpy(ret, UNDEFINED);
    } else {
        ret = malloc(strlen(OK) + 1);
        strcpy(ret, OK);
    }

    PQclear(res);
    return ret;
}

char* timeout_user(PGconn* dbConn, char* admin_id, char* client_id) {
    char *ret;
    const char OK[] = "200/OK : Messages du client bloqués pendant 24 heures";
    const char ERROR[] = "400/ERROR : Erreur sur le blocage des messages en provenance du client";
    const char DENIED[] = "403/DENIED : Vous n'avez pas accès à cette commande";
    const char UNDEFINED[] = "404/UNDEFINED : Un ou plusieurs identifiant sont incorrects";

    // Vérifier si l'utilisateur est un administrateur
    const char* paramValues[1] = { admin_id };
    PGresult* res = PQexecParams(
        dbConn,
        "SELECT status FROM chatator.client WHERE client_id = $1",
        1, NULL, paramValues, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0 || strcmp(PQgetvalue(res, 0, 0), "administrateur") != 0) {
        ret = malloc(strlen(DENIED) + 1);
        strcpy(ret, DENIED);
        PQclear(res);
        return ret;
    }

    // Bloquer le client pendant 24 heures
    const char* blockParams[2] = { client_id, admin_id };
    res = PQexecParams(
        dbConn,
        "UPDATE chatator.conversation SET bloque = true, date_deblocage = NOW() + INTERVAL '24 hours' WHERE (client_id_1 = $1 OR client_id_2 = $1)",
        1, NULL, blockParams, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        ret = malloc(strlen(ERROR) + 1);
        strcpy(ret, ERROR);
    } else {
        ret = malloc(strlen(OK) + 1);
        strcpy(ret, OK);
    }

    PQclear(res);
    return ret;
}

// Fonction pour bannir un client définitivement (BAN)
char* ban_user(PGconn* dbConn, char* admin_id, char* client_id) {
    char *ret;
    const char OK[] = "200/OK : Messages bloqués entre le client et le professionel définitivement";
    const char ERROR[] = "400/ERROR : Erreur sur le banissement d'un client";
    const char DENIED[] = "403/DENIED : Vous n'avez pas accès à cette commande";
    const char UNDEFINED[] = "404/UNDEFINED : L'identifiant du client n'existe pas";

    // Vérifier si l'utilisateur est un administrateur
    const char* paramValues[1] = { admin_id };
    PGresult* res = PQexecParams(
        dbConn,
        "SELECT status FROM chatator.client WHERE client_id = $1",
        1, NULL, paramValues, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0 || strcmp(PQgetvalue(res, 0, 0), "administrateur") != 0) {
        ret = malloc(strlen(DENIED) + 1);
        strcpy(ret, DENIED);
        PQclear(res);
        return ret;
    }

    // Bannir le client définitivement
    const char* banParams[1] = { client_id };
    res = PQexecParams(
        dbConn,
        "UPDATE chatator.client SET banni = true WHERE client_id = $1",
        1, NULL, banParams, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        ret = malloc(strlen(ERROR) + 1);
        strcpy(ret, ERROR);
    } else {
        ret = malloc(strlen(OK) + 1);
        strcpy(ret, OK);
    }

    PQclear(res);
    return ret;
}
