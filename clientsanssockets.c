#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MAX_COMMAND_SIZE 1000
#define MAX_KEYS 100
#define MAX_TOKENS 10

bool est_connecte(int id);

int main(){
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
    
    scanf("%s",commande);

    char *tokenc = strtok(commande, delimiter_commande);
    longueur = strlen(tokenc);
    strncpy(arguments, commande,longueur + 1);

    char *tokena = strtok(arguments, delimiter_argument);

    while (tokena != NULL && token_argument_count < MAX_TOKENS) {
        tokens_argument[token_argument_count++] = tokena;
        tokena = strtok(NULL, delimiter_argument);
    }

    if(strcmp(tokenc,"LOGIN")){
        char* key;
        char* status;
        int i = 0;
        char* keys[MAX_KEYS] = // requette des cles api
        status = // requette status

        while(key != keys[i]){
            if(strcmp(key,keys[i])){
                if(strcmp(status,"client")){
                    printf("201/OKC : Accès client authorisé");
                    id_client = // id associé au compte ayant la cle
                }else if(strcmp(status,"professionel")){
                    printf("202/OKP : Accès professionel authorisé");
                    id_client = // id associé au compte ayant la cle
                }else{
                    printf("203/OKA : Accès administrateur authorisé");
                    id_client = // id associé au compte ayant la cle
                }
            }
            i++;
        }

        if(id_client == 0){
            printf("403/DENIED : Accès refusé");
        }

    }else if(strcmp(tokenc,"MSG")){
        if(est_connecte(id_client)){
            int i = 0;
            int* all_id[MAX_KEYS] = // requete de tout les id client;
            while(all_id[i] != token_argument[0]){
                i++;
            }

            if(all_id[i] == id_client){
                bool blocked = // requete blocage
                
                if(blocked){
                    printf("401/BLOCKED : Le destinataire n'authorise pas les messages de votre part");
                }else if(strlen(tokens_argument[1]) > MAX_COMMAND_SIZE){
                    printf("402/LONG : Le message est trop long");
            }else{
                printf("404/UNDEFINED : L'identifiant du client est incorrect");
            }
        }

    }else if(strcmp(tokenc,"PULL")){

    }else if(strcmp(tokenc,"HISTORY")){

    }else if(strcmp(tokenc,"MODIFY")){

    }else if(strcmp(tokenc,"BLOCK")){

    }else if(strcmp(tokenc,"TIMEOUT")){

    }else if(strcmp(tokenc,"BAN")){

    }else{
        printf("Commande %s non reconnue",tokenc);
    }

    return EXIT_SUCCESS;
}

bool est_connecte(int id){
    if(id != 0){
        return true;
    }
}