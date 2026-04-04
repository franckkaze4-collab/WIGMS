#include <stdio.h>
#define MAX 50

typedef struct {
    int gift_id;
    char name[100];
    float value;
    int guest_id;
} Gift;

Gift gifts[MAX];
int count = 0;

// ---- Fonctions principales ----

void register_gift() {
    printf("\n--- Enregistrement d’un nouveau cadeau ---\n");

    printf("Gift ID           : ");
    scanf("%d", &gifts[count].gift_id);

    printf("Nom du Cadeau     : ");
    scanf("%s", gifts[count].name);

    printf("Valeur du Cadeau  : ");
    scanf("%f", &gifts[count].value);

    printf("Guest ID          : ");
    scanf("%d", &gifts[count].guest_id);

    count++;

    printf("\n? Cadeau enregistré avec succès !\n");
}

void display_dashboard() {
    int i;
    float total = 0;

    if (count == 0) {
        printf("\n??  Aucun cadeau enregistré.\n");
        return;
    }

    printf("\n==============================================================\n");
    printf(" ??  TABLEAU DE BORD DES CADEAUX DE MARIAGE\n");
    printf("==============================================================\n");
    printf("| %-5s | %-20s | %-10s | %-10s |\n", "ID", "Nom", "Valeur (€)", "Guest ID");
    printf("--------------------------------------------------------------\n");

    for (i = 0; i < count; i++) {
        printf("| %-5d | %-20s | %-10.2f | %-10d |\n",
               gifts[i].gift_id, gifts[i].name, gifts[i].value, gifts[i].guest_id);
        total += gifts[i].value;
    }

    printf("--------------------------------------------------------------\n");
    printf("| %-28s | TOTAL: %-10.2f€ |\n", " ", total);
    printf("==============================================================\n\n");
}

int main() {
    int choice;

    do {
        printf("\n========= Gestion des Cadeaux de Mariage =========\n");
        printf("1. Enregistrer un Cadeau\n");
        printf("2. Afficher le Tableau de Bord\n");
        printf("0. Quitter\n");
        printf("==================================================\n");
        printf("Votre choix: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                register_gift();
                break;

            case 2:
                display_dashboard();
                break;

            case 0:
                printf("\n?? Merci d’avoir utilisé le gestionnaire !\n");
                break;

            default:
                printf("\n? Choix invalide. Réessayez.\n");
        }

    } while (choice != 0);

    return 0;
}
