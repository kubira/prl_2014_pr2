#!/bin/bash

################################################################################
#                                                                              #
# Soubor:   test.sh                                                            #
# Autor:    Radim KUBIŠ, xkubis03                                              #
# Vytvořen: 28. března 2014                                                    #
#                                                                              #
# Popis:                                                                       #
#    Tento skript slouží pro spuštění jednoho testu řadícího algoritmu         #
#    Enumeration-sort nad posloupností čísel k seřazení.                       #
#                                                                              #
#    Jediný nepovinný parametr tohoto skriptu udává, kolik čísel bude          #
#    algoritmus řadit.                                                         #
#                                                                              #
################################################################################

# Pokud nebyl zadán počet čísel k seřazení
if [ $# -lt 1 ]; then
    # Nastavím počet na hodnotu 10
    number_of_values=10;
# Pokud byl zadán požadovaný počet čísel k seřazení
else
    # Nastavím tuto hodnotu pro test
    number_of_values=$1;
fi;

# Přeložím zdrojový kód řadícího programu
mpicc --prefix /usr/local/share/OpenMPI -o es es.c -lrt -std=gnu99

# Vyrobím soubor 'numbers' s náhodnými čísly pro seřazení
dd if=/dev/random bs=1 count=$number_of_values of=numbers &> /dev/null

# Zvýším počet hodnot o +1, tolik bude program obsahovat procesorů
number_of_values=$((number_of_values+1))

# Spustím test řadícího algoritmu
mpirun --prefix /usr/local/share/OpenMPI -np $number_of_values es

# Uklidím vytvořené soubory po ukončení testu
rm -f es numbers
