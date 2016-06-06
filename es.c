/*
 * Soubor:    es.c
 * Autor:     Radim KUBIŠ, xkubis03
 * Vytvořeno: 28. března 2014
 *
 * Popis:
 *    Tento program byl vytvořen jako 2. projekt do předmětu Paralelní
 *    a distribuované algoritmy (PRL).
 *
 *    Náplní projektu bylo vytvořit pomocí knihovny OpenMPI program
 *    implementující řadící algoritmus Enumeration-sort, který byl
 *    prezentován na přednášce.
 *
 * Poznámka:
 *    Pro změnu mezi měřením délky trvání algoritmu a výpisem řazených
 *    hodnot je třeba zakomentovat/odkomentovat řádky s výskytem slova
 *    MEASUREMENT v komentáři.
 */

#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

// Tag pro zasílání vlastních čísel X jednotlivým procesorům
#define TAGX   0
// Tag pro zasílání všech vstupních hodnot Xi do algoritmu
#define TAGY   1
// Tag pro zasílání ohodnocených čísel do registrů Z
#define TAGZ   2
// Tag pro posouvání seřazených čísel vpravo na konci algoritmu
#define TAGEND 3

/*
 * Funkce pro výpočet a výpis doby trvání řazení vstupní posloupnosti čísel.
 *
 * start - struktura s hodnotami počátečního času algoritmu
 * stop  - struktura s hodnotami koncového času algoritmu
 *
 */
void getTimeDiff(struct timespec *start, struct timespec *stop) {
    // Proměnná pro uložení doby trvání řadícího algoritmu
    double diff = 0;

    // Výpočet rozdílu celých sekund a převod na nanosekundy
    diff = ((stop->tv_sec - start->tv_sec) * 1000000000);
    // Výpočet rozdílu nanosekund a přičtení rozdílu do celkové doby trvání
    diff += (stop->tv_nsec-start->tv_nsec);

    // Výpis doby trvání algoritmu v milisekundách na standardní výstup
    fprintf(stdout, "%f\n", (diff / 1000000));
}

/*
 * Funkce main - hlavní funkce programu.
 *
 * argc - počet vstupních parametrů příkazové řádky
 * argv - pole vstupních parametrů příkazové řádky
 *
 * Návratová hodnota:
 *         0 - program proběhl v pořádku
 *     jinak - program byl ukončen s chybou
 */
int main(int argc, char *argv[]) {
    // Proměnná pro uložení ranku procesoru
    int myRank = 0;
    // Proměnná pro uložení hodnoty čísla procesoru (X a Z)
    int myNumber = 0;
    // Proměnná pro ukládání přijmuté hodnoty čísla posloupnosti (Y = Xi)
    int receivedNumber = 0;
    // Proměnná pro uložení počtu procesorů algoritmu
    int numberOfProcessors = 0;
    // Proměnná pro inkrementaci při přijetí menšího čísla (registr C)
    int numberOfGreaterThenMy = 1;
    // Proměnná pro stav MPI
    MPI_Status stat;

    // Inicializace MPI
    MPI_Init(&argc, &argv);
    // Získání počtu procesorů algoritmu
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcessors);
    // Získání ranku procesoru
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    // Pokud jsem procesor s rankem roota
    if(myRank == 0) {
        // Proměnná pro deskriptor vstupního souboru
        FILE *inputFile = NULL;
        // Pole pro ukládání načtené posloupnosti čísel ze souboru
        // a seřazených čísel na konci algoritmu
        int inputNumbers[(numberOfProcessors - 1)];
        // Struktura pro počáteční čas algoritmu
        struct timespec start;
        // Struktura pro koncový čas algoritmu
        struct timespec finish;

        // Otevření vstupního binárního souboru s posloupností čísel
        inputFile = fopen("numbers", "r");

        // V cyklu načítám ze souboru tolik čísel, kolik je procesorů
        for(int i = 1; i < numberOfProcessors; i++) {
            // Načtení jednoho čísla ze souboru
            inputNumbers[i - 1] = fgetc(inputFile);
            // Výpis právě načteného čísla odděleného mezerou
            fprintf(stdout, "%d ", inputNumbers[i - 1]); /* MEASUREMENT */
        }
        // Výpis konce řádku za načtenou posloupností čísel ze souboru
        fprintf(stdout, "\x8\n"); /* MEASUREMENT */

        // Uzavření vstupního souboru
        fclose(inputFile);

        // Inicializace počátečního času algoritmu
        //clock_gettime(CLOCK_MONOTONIC_RAW, &start); /* MEASUREMENT */

        // Rozeslání vlastních čísel jednotlivým procesorům
        for(int i = 1; i < numberOfProcessors; i++) {
            // Odeslání vlastního čísla jednomu procesoru
            MPI_Send(&(inputNumbers[i - 1]), 1, MPI_INT, i, TAGX, MPI_COMM_WORLD);
        }

        // Postupné zasílání vstupní posloupnosti čísel prvnímu procesoru
        for(int i = 1; i < numberOfProcessors; i++) {
            // Odeslání jednoho čísla prvnímu procesoru
            MPI_Send(&(inputNumbers[i - 1]), 1, MPI_INT, 1, TAGY, MPI_COMM_WORLD);
        }

        // Přijímání seřazené posloupnosti čísel od posledního procesoru
        for(int i = 1; i < numberOfProcessors; i++) {
            // Příjem jednoho seřazeného čísla od posledního procesoru
            MPI_Recv(&(inputNumbers[i - 1]), 1, MPI_INT, (numberOfProcessors - 1), TAGEND, MPI_COMM_WORLD, &stat);
        }

        // Inicializace koncového času algoritmu
        //clock_gettime(CLOCK_MONOTONIC_RAW, &finish); /* MEASUREMENT */

        // Výpis seřazené posloupnosti čísel
        for(int i = (numberOfProcessors-1); i > 0; i--) {
            // Výpis jednoho seřazeného čísla odděleného koncem řádku
            fprintf(stdout, "%d\n", inputNumbers[i - 1]); /* MEASUREMENT */
        }

        // Volání funkce pro výpočet a výpis délky řazení algoritmu
        //getTimeDiff(&start, &finish); /* MEASUREMENT */
    } else {
        // Pokud nejsem rootovský procesor,
        // příjmu od roota vlastní číslo procesoru (hodnota X)
        MPI_Recv(&myNumber, 1, MPI_INT, 0, TAGX, MPI_COMM_WORLD, &stat);

        // Předávání hodnot na vstupu vpravo (X1 ... Xrank-1)
        for(int i = 1; i < myRank; i++) {
            // Příjem jednoho čísla vstupu (Xi) od levého procesoru
            MPI_Recv(&receivedNumber, 1, MPI_INT, (myRank - 1), TAGY, MPI_COMM_WORLD, &stat);

            // Porovnání přijatého čísla (Xi) s vlastním číslem (X)
            if(myNumber >= receivedNumber) {
                // Pokud je moje vlastní číslo (X) větší nebo rovno než přijaté
                // číslo (Xi), inkrementuji (registr C)
                numberOfGreaterThenMy++;
            }

            // Pokud nejsem poslední procesor,
            // předávám přijaté číslo(Xi) do dalšího procesoru vpravo
            if(myRank < (numberOfProcessors - 1)) {
                // Předání jednoho čísla (Xi) vstupu vpravo
                MPI_Send(&receivedNumber, 1, MPI_INT, (myRank + 1), TAGY, MPI_COMM_WORLD);
            }
        }

        // Předávání hodnot na vstupu vpravo (Xrank ... Xn)
        for(int i = myRank; i < numberOfProcessors; i++) {
            // Příjem jednoho čísla vstupu (Xi) od levého procesoru
            MPI_Recv(&receivedNumber, 1, MPI_INT, (myRank - 1), TAGY, MPI_COMM_WORLD, &stat);

            // Porovnání přijatého čísla (Xi) s vlastním číslem (X)
            if(myNumber > receivedNumber) {
                // Pokud je moje vlastní číslo (X) větší než přijaté
                // číslo (Xi), inkrementuji (registr C)
                numberOfGreaterThenMy++;
            }

            // Pokud nejsem poslední procesor,
            // předávám přijaté číslo(Xi) do dalšího procesoru vpravo
            if(myRank < (numberOfProcessors - 1)) {
                // Předání jednoho čísla (Xi) vstupu vpravo
                MPI_Send(&receivedNumber, 1, MPI_INT, (myRank + 1), TAGY, MPI_COMM_WORLD);
            }
        }

        // Odeslání mého vlastního čísla (X) do registru Z procesoru
        // s rankem podle počtu menších čísel (registr C)
        MPI_Send(&myNumber, 1, MPI_INT, numberOfGreaterThenMy, TAGZ, MPI_COMM_WORLD);

        // Přijem hodnoty do mého registru Z od kteréhokoliv jiného procesoru
        MPI_Recv(&myNumber, 1, MPI_INT, MPI_ANY_SOURCE, TAGZ, MPI_COMM_WORLD, &stat);

        // Postupné zasílání (posouvání) mého registru Z vpravo
        // a příjem hodnoty registru Z zleva
        for(int i = 1; i <= myRank; i++) {
            // Zaslání registru Z vpravo
            MPI_Send(&myNumber, 1, MPI_INT, ((myRank + 1) % numberOfProcessors), TAGEND, MPI_COMM_WORLD);

            // Pokud ještě mám co přijímat zleva
            if(i < myRank) {
                // Příjmu obsah registru Z zleva
                MPI_Recv(&myNumber, 1, MPI_INT, (myRank - 1), TAGEND, MPI_COMM_WORLD, &stat);
            }
        }
    }

    // Ukončení práce s MPI
    MPI_Finalize();

    // Program doběhl bez chyby, vrací hodnotu 0
    return 0;
}
