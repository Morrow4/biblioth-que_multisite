#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "../header/fonctions_bdd.h"
#include "../header/utilitaire.h"

void ajout_livre(MYSQL *conn)
{
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[5];
    char ISBN[14];
    char titre[255];
    char auteur[101];
    char edition[101];
    char genre[101];

    printf("Veuillez saisir les informations du livre :\n\n");

    do
    {
        printf("ISBN (13 caractères) : ");
        scanf("%13s", ISBN);
        if (strlen(ISBN) != 13)
        {
            printf("L'ISBN doit avoir précisément 13 caractères. Veuillez réessayer.\n");
        }
    } while (strlen(ISBN) != 13);

    printf("Titre : ");
    scanf(" %254[^\n]", titre);

    printf("Auteur : ");
    scanf(" %100[^\n]", auteur);

    printf("Edition : ");
    scanf(" %100[^\n]", edition);

    printf("Genre : ");
    scanf(" %100[^\n]", genre);

    // Préparer la requête SQL pour l'ajout du livre
    const char *query = "INSERT INTO Livre (ISBN, Titre, Auteur, Edition, Genre) VALUES (?, ?, ?, ?, ?)";
    stmt = mysql_stmt_init(conn);

    if (!stmt)
    {
        fprintf(stderr, "Échec de l'initialisation de la requête préparée : %s\n", mysql_error(conn));
        return;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        fprintf(stderr, "Échec de la préparation de la requête : %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    memset(bind, 0, sizeof(bind));

    // Lier les variables d'entrée à la requête préparée
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = ISBN;
    bind[0].buffer_length = strlen(ISBN);

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = titre;
    bind[1].buffer_length = strlen(titre);

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = auteur;
    bind[2].buffer_length = strlen(auteur);

    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = edition;
    bind[3].buffer_length = strlen(edition);

    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = genre;
    bind[4].buffer_length = strlen(genre);

    if (mysql_stmt_bind_param(stmt, bind))
    {
        fprintf(stderr, "Échec de la liaison des paramètres : %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    // Exécuter la requête préparée
    if (mysql_stmt_execute(stmt))
    {
        fprintf(stderr, "Erreur lors de l'exécution de la requête : %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    printf("Succès de l'ajout du livre !\n");

    // Fermer la requête préparée
    mysql_stmt_close(stmt);
}

void mise_a_jour_livre(MYSQL *conn, char *ISBN)
{
    // Saisie des nouvelles informations du livre
    char titre[255];
    char auteur[101];
    char edition[101];
    char genre[101];

    printf("Veuillez saisir les nouvelles informations du livre (sans espace) :\n");

    printf("Nouveau Titre : ");
    scanf("%254s", titre);

    printf("Nouvel Auteur : ");
    scanf("%100s", auteur);

    printf("Nouvelle Edition : ");
    scanf("%100s", edition);

    printf("Nouveau Genre : ");
    scanf("%100s", genre);

    // Préparer la requête SQL pour la mise à jour du livre
    char query[1024];
    sprintf(query, "UPDATE Livre SET Titre='%s', Auteur='%s', Edition='%s', Genre='%s' WHERE ISBN='%s'",
            titre, auteur, edition, genre, ISBN);

    // Exécuter la requête SQL
    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Erreur lors de la mise a jour du livre : %s\n", mysql_error(conn));
        return;
    }

    printf("Succes de la mise a jour du livre !\n");
}

void suppression_livre(MYSQL *conn, char *ISBN)
{
    // Préparer la requête SQL pour la suppression du livre
    char query[1024];
    sprintf(query, "DELETE FROM Livre WHERE ISBN='%s'", ISBN);

    // Exécuter la requête SQL
    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Erreur lors de la suppression du livre : %s\n", mysql_error(conn));
        return;
    }

    printf("Succes de la suppression du livre !\n");
}

void recherche_ISBN(MYSQL *conn, char *titre, char *auteur)
{
    // Préparer la requête SQL pour la recherche d'ISBN par titre ou auteur
    char query[1024];
    sprintf(query, "SELECT ISBN FROM Livre WHERE Titre='%s' OR Auteur='%s'", titre, auteur);

    // Exécuter la requête SQL
    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Erreur lors de la recherche d'ISBN : %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (!result)
    {
        fprintf(stderr, "Aucun résultat retourné par la requête\n");
        return;
    }

    // Afficher les ISBNs trouvés
    printf("Résultats de la recherche d'ISBN pour le livre avec titre \"%s\" ou auteur \"%s\" :\n", titre, auteur);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        printf("ISBN: %s\n", row[0]);
        printf("------------------------------\n");
    }

    mysql_free_result(result);
}

// Fonction pour afficher tous les livres avec leur disponibilité
void afficher_tous_les_livres(MYSQL *conn)
{
    // Requête SQL pour récupérer les livres avec leur disponibilité
    const char *query = "SELECT Livre.ISBN, Livre.Titre, Livre.Auteur, Livre.Edition, Livre.Genre, CASE WHEN EXISTS (SELECT 1 FROM Exemplaire WHERE Exemplaire.ISBN = Livre.ISBN AND Exemplaire.Disponibilite = true) THEN 'Disponible' ELSE 'Indisponible' END AS Disponibilite FROM Livre";

    // Exécuter la requête SQL
    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Erreur lors de la récupération des livres : %s\n", mysql_error(conn));
        return;
    }

    // Récupérer le résultat de la requête
    MYSQL_RES *result = mysql_store_result(conn);

    // Vérifier si le résultat est valide
    if (!result)
    {
        fprintf(stderr, "Aucun résultat retourné par la requête\n");
        return;
    }

    // Afficher les en-têtes
    printf("%-15s %-70s %-40s %-15s\n", "ISBN", "Titre", "Auteur", "Disponibilité");
    printf("-----------------------------------------------------------------------------------------------------------------------------------------------\n");

    // Afficher chaque livre avec sa disponibilité
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        // Afficher les détails du livre
        printf("%-15s %-70s %-40s %-15s\n", row[0], row[1], row[2], (row[3] && atoi(row[3]) ? "Disponible" : "Indisponible"));
    }

    // Libérer la mémoire du résultat
    mysql_free_result(result);
}