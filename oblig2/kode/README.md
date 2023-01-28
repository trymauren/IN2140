Leser superblokkfilen rekursivt, og lager structs for å representere inoder i loading_recursive(),
som kalles av load_inodes().

Alle tester fungerer.

For å kjøre programmet med full valgrind- sjekk:

$ bash run.sh

For å rydde:

$ bash clean.sh
