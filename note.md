## BIND 

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

bind(fd, (struct sockaddr *)&addr, sizeof(addr));

## struct sockaddr_in     vs     struct addrinfo

**struct sockaddr_in** = L'ADRESSE elle-même, format brut kernel.
Trois champs utiles (famille, port, IP), du binaire prêt-à-bind : c'est le formulaire administratif que le kernel exige.
Tu la remplis à la main — comme on vient de le faire : famille, htons(port), INADDR_ANY.
Bas niveau, aucun service : si tu veux l'adresse de « google.com », débrouille-toi — elle ne sait stocker qu'un résultat, pas le trouver.

**struct addrinfo** = la FICHE que getaddrinfo() te rend — le résultat de l'ANNUAIRE.
Tu te souviens de getaddrinfo côté ops (ton getaddrinfo failed d'Inception — la famille annuaire !) : 
c'est la fonction qui résout — un nom (« localhost », « google.com ») ou un service (« 8080 », « http ») → des adresses utilisables. 
Et sa réponse est une liste chaînée de struct addrinfo, où chaque maillon dit : 
« voici UNE possibilité : famille (ai_family), type (ai_socktype), protocole (ai_protocol)... et — regarde le champ ai_addr — un pointeur vers... une struct sockaddr toute remplie ! » — La poupée russe : addrinfo CONTIENT le sockaddr. 
getaddrinfo fait le travail que tu faisais à la main (le htons, le remplissage, le ce choix de famille) et te rend des structures prêtes à donner à bind/connect telles quelles.



**struct sockaddr_in** = L'ADRESSE elle-même, format brut kernel. Trois champs utiles (famille, port, IP), du binaire prêt-à-bind : c'est le formulaire administratif que le kernel exige. Tu la remplis à la main — comme on vient de le faire : famille, htons(port), INADDR_ANY. Bas niveau, aucun service : si tu veux l'adresse de « google.com », débrouille-toi — elle ne sait stocker qu'un résultat, pas le trouver.

**struct addrinfo** = la FICHE que getaddrinfo() te rend — le résultat de l'ANNUAIRE. Tu te souviens de getaddrinfo côté ops (ton getaddrinfo failed d'Inception — la famille annuaire !) : c'est la fonction qui résout — un nom (« localhost », « google.com ») ou un service (« 8080 », « http ») → des adresses utilisables. Et sa réponse est une liste chaînée de struct addrinfo, où chaque maillon dit : « voici UNE possibilité : famille (ai_family), type (ai_socktype), protocole (ai_protocol)... et — regarde le champ ai_addr — un pointeur vers... une struct sockaddr toute remplie ! » — La poupée russe : addrinfo CONTIENT le sockaddr. getaddrinfo fait le travail que tu faisais à la main (le htons, le remplissage, le choix de famille) et te rend des structures prêtes à donner à bind/connect telles quelles.hoix de famille) et te rend des structures prêtes à donner à bind/connect telles quelles.



struct sockaddr_in addr;                    // _in = internet (IPv4)

addr.sin_family = AF_INET;                  // la famille (encore elle)
addr.sin_port = htons(8080);                // LE PORT — et le htons de ce matin
//   entre en scène : sans lui → 36895 !
addr.sin_addr.s_addr = INADDR_ANY;          // L'ADRESSE — le mystère à élucider



