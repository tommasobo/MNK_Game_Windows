#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

struct snodo;
struct listaptr {struct listaptr*next;struct snodo*nodo;}; // Lista di "fratelli" (puntatori a strutture snodo)
struct snodo{unsigned char casella;char valore;struct listaptr*figlio;}; // Ogni nodo corrisponde a una possibile mossa nella partita
//Il campo casella corrisponde all'indice della casella che è stata modificata, il campo valore corrisponde all'esito finale
//dell'albero (calcolato con minmax), figlio è la testa della lista di figli
struct doppioint{int vittorie;int scenaritotali;}; // coppia di interi usati per capire quale scenario offre la maggior probabilità di vittoria tra pari

static unsigned int M=0,N=0,K=0;
static char *BOARD=NULL; //Stringa che contiene lo stato del gioco al momento corrente
//Nella stringa le caselle sono codificate con 3 (casella vuota), 2 (pedina avversario) e 1 (nostra pedina)
struct snodo NODOBASE={0,2,NULL}; // Inizio dell'albero

//getcoord prende in input le coordinate (riga;colonna) e restituisce l'indice corrispondente nella stringa
char getcoord(unsigned int righe,unsigned int colonne){
	return (char)(colonne+righe*N);}

//getrowcol prende in input l'indice di stringa e restituisce le corrispondenti coordinate (riga;colonna)
void getrowcol(char coord,unsigned int*righe,unsigned int*colonne){
	*righe=coord/N;
	*colonne=coord%N;}

//uccidi_nodo elimina il nodo Testa e tutti i suoi figli ricorsivamente
void uccidi_nodo(struct snodo*Testa){
	struct listaptr*ptr;
	while(Testa->figlio!=NULL){ // questo ciclo elimina tutti i figli
		uccidi_nodo(Testa->figlio->nodo); // chiamata ricorsiva
		ptr=Testa->figlio->next;
		free(Testa->figlio);
		Testa->figlio=ptr;}
	free(Testa);} // infine viene eliminata la Testa

//uccidi_fratelli elimina da "padre" tutti i figli eccetto "figliosuperstite"
void uccidi_fratelli(struct snodo*padre,struct snodo*figliosuperstite){
	struct listaptr*ptr=padre->figlio,*ptr2;
	while(ptr!=NULL){ // ciclo eseguito per ogni figlio
		if(ptr->nodo==figliosuperstite){ // se il figlio corrente va salvato:
			padre->figlio=ptr;
			ptr=ptr->next;
			padre->figlio->next=NULL;}
		else{ // se il figlio corrente va eliminato:
			uccidi_nodo(ptr->nodo);
			ptr2=ptr;
			ptr=ptr->next;
			free(ptr2);}}}

//valuta restituisce 2 se vince il computer, 0 se vince l'avversario, 1 se patta e 3 se la partita non è ancora finita
char valuta(char* griglia) {
    int pc = 0, human = 0, flag = 0;
    unsigned int posx, posy;
	unsigned int k,i;
    for (i = 0; i < (M*N); i++) {
        if (griglia[i] == 3) // Se trovo una cella vuota setto un flag
            flag=1;
        if (griglia[i] == 1 || griglia[i] == 2) {
            getrowcol(i,&posy,&posx); // ottengo la posizione in coordinate della cella che sto valutando
            if (posx+K <= N) { // Controllo di non uscire dalla griglia
                griglia[i] == 1 ? pc++ : human++;
                for (k = 1; k < K; k++) { // Valuto le caselle a destra
                    if (griglia[i+k] == griglia[i]) {
                        griglia[i+k] == 1 ? pc++ : human++;}
                    else { // Se trovo una casella diverso resetto i valori di human e pc
                        pc = 0;
                        human = 0;
                        break; // Esco dal ciclo for
                    }
                }
            }
            if (posy+K <= M && pc != K && human !=K) { // Controllo di non uscire dalla casella e che qualcuno non abbia già vinto
                if (griglia[i] == 1)
                    pc = 1;
                else
                    human = 1;
                for (k = 1; k < K; k++) { // Valuto le caselle in verticale
                    if (griglia[i+(k*N)] == griglia[i])
                        griglia[i+(k*N)] == 1 ? pc++ : human++;
                    else {
                        pc = 0;
                        human = 0;
                        break;}
                }
            }
            if (posy + K <= M && posx + K <= N && pc != K && human !=K) { // Controllo di non uscire dalla casella e che qualcuno non abbia già vinto
                if (griglia[i] == 1)
                    pc = 1;
                else
                    human = 1;
                for (k = 1; k < K; k++) { // Valuto le caselle diagonali a destra
                    if (griglia[i+k*(N+1)] == griglia[i])
                        griglia[i+k*(N+1)] == 1 ? pc++ : human++;
                    else {
                        pc = 0;
                        human = 0;
                        break;}
                }
            }
            if (posy + K <= M  && (int)posx - (int)K + 1 >= 0&& pc != K && human !=K) { // Controllo di non uscire dalla casella e che qualcuno non abbia già vinto
                if (griglia[i] == 1)
                    pc = 1;
                else
                    human = 1;
                    for (k = 1; k < K; k++) { // Valuto le caselle diagonali a sinistra
                        if (griglia[i+k*(N-1)] == griglia[i])
                            griglia[i+k*(N-1)] == 1 ? pc++ : human++;
                        else{
                            pc = 0;
                            human = 0;
                            break;}
                    }
                }
            }
        if (pc == K)  // Se il pc è uguale a K significa che ha vinto
            return 2;
        else if (human == K)
            return 0;
        pc = 0;
        human = 0;}
	if (flag) // Se ho settato il flag significa che ho trovato almeno una casella vuota e quindi la partita non è ancora finita
            return 3;
        else // Altrimenti se non ho trovato una casella vuota significa che la partita è finita in patta
			return 1;}

// genera_albero restituisce una coppia di interi che rappresentano le vittorie sul numero di possibili scenari dell'albero generato.
// in caso di errori restituisce una coppia di -1
// il parametro A è il nodo padre dell'albero da generare
// il parametro giocatore può avere valore 0 (sta giocando il computer), 1 (sta giocando l'avversario) e 2 (giocatore impossibile, usato per inizializzare l'albero)
// il parametro griglia è una stringa che codifica lo stato della board in quella particolare diramazione di mosse-contromosse, le caselle sono codificate dai
//    caratteri 1 (pedina del computer), 2 (pedina del giocatore umano) e 3 (casella libera)
struct doppioint genera_albero(struct snodo*A,char giocatore,char*griglia){
	struct doppioint migliore,corrente,errore;
	unsigned char i,j,k;
	char*nuovagriglia;
	struct snodo*x,*migliorfiglio;
	struct listaptr*ptr;
	if((A->valore=valuta(griglia))!=3){ // valuta la board corrente, se qualcuno ha vinto (foglia) esce dalla ricorsione
		corrente.scenaritotali=corrente.vittorie=1;
		if(A->valore!=2)corrente.vittorie=0; // se non ha vinto il computer restituisco {0;1}
		return corrente;} // se ha vinto il computer restituisco {1;1}
	errore.scenaritotali=errore.vittorie=-1;
	A->valore = giocatore?0:3; // assegna un valore iniziale all'albero, verrà aggiornato sin dal primo figlio
	migliorfiglio=NULL;
	migliore.vittorie=migliore.scenaritotali=0;
	k=rand()%M*N; // la casella da cui viene scelto il primo possibile figlio è randomica
	// in questo modo il computer non segue sempre lo stesso schema di gioco ma lo varia
	for(i=0;i<M*N;i++){ // per ogni casella della griglia allo stato corrente:
		if(griglia[(i+k)%(M*N)]==3){ // se è vuota (quindi il giocatore può mettere lì la sua pedina)
			nuovagriglia=(char*)malloc(sizeof(char)*M*N); // alloco spazio per il nuovo figlio
			if(!nuovagriglia)return errore;
			x=(struct snodo*)malloc(sizeof(struct snodo));
			if(!x){free(nuovagriglia);return errore;}
			for(j=0;j<M*N;j++)nuovagriglia[j]=griglia[j]; // copio la griglia precedente
			nuovagriglia[(i+k)%(M*N)]=!giocatore+1; // ma modifico la casella che ho scelto di occupare
			ptr=(struct listaptr*)malloc(sizeof(struct listaptr));
			if(!ptr){free(nuovagriglia);free(x);return errore;}
			ptr->nodo=x;
			if(A->figlio==NULL){
				A->figlio=ptr;
				A->figlio->next=NULL;}
			else{
				ptr->next=A->figlio;
				A->figlio=ptr;}
			x->casella=(i+k)%(M*N);
			x->figlio=NULL;
			corrente.vittorie=0;
			corrente.scenaritotali=1;
			if (migliorfiglio==NULL)migliorfiglio=x;
			if((x->valore=valuta(nuovagriglia))==3) // valuto il figlio, se la partita non è conclusa
				corrente=genera_albero(x,!giocatore,nuovagriglia); // chiamo ricorsivamente la funzione
			else {
				free(nuovagriglia);
				if(x->valore==2)corrente.vittorie=1;}
			if(corrente.vittorie==-1)return errore;
			if(giocatore?A->valore<x->valore:A->valore>x->valore){ // MINMAX: aggiorno il valore dell'albero (padre) a seconda del valore del figlio corrente
				A->valore=x->valore;
				if(giocatore!=0){
					migliore=corrente;
					migliorfiglio=x;}}
			if(giocatore!=0){
				if(x->valore==A->valore&&((float)corrente.vittorie/(float)corrente.scenaritotali)>(migliore.scenaritotali==0?0:(float)migliore.vittorie/(float)migliore.scenaritotali)){
					migliore=corrente; // confronto il rapporto vittorie/scenaritotali, il miglior rapporto indica la strada da seguire
					migliorfiglio=x;}
				if(x->valore==2){ // Se sto scegliendo la mossa del computer e ho trovato una mossa sicuramente vincente
					uccidi_fratelli(A,x); // elimino tutti i "fratelli" di quella mossa (se ce ne sono)
					break;}}// impedisce di creare altri "fratelli"
			else{
				migliore.scenaritotali+=corrente.scenaritotali;
				migliore.vittorie+=corrente.vittorie;}}}
	if(giocatore!=0&&A->figlio->next!=NULL) // Se sto scegliendo la mossa del computer MA non ho trovato una mossa sicuramente vincente
		uccidi_fratelli(A,migliorfiglio); // Elimino i fratelli della mossa migliore possibile
	if(giocatore!=2)
		free(griglia);
    return migliore;} // restituisco il numero di vittorie in quell'albero e il numero totale di scenari possibili

void free_player() {
	struct listaptr*ptr;
	M=N=K=0;
	if(BOARD!=NULL){free(BOARD);BOARD=NULL;}
	while(NODOBASE.figlio!=NULL){ // elimino il "primogenito" del nodo base fino ad aver eliminato l'intero albero (eccetto NODOBASE)
		uccidi_nodo(NODOBASE.figlio->nodo);
		ptr=NODOBASE.figlio->next;
		free(NODOBASE.figlio);
		NODOBASE.figlio=ptr;}}

int setup_player(unsigned int m, unsigned int n, unsigned int k){
	unsigned char i;
	struct doppioint risultato;
	if (n*m > 256) return 0;
	M=m;
	N=n;
	K=k;
	BOARD=(char*)malloc(m*n*sizeof(char));
	if (BOARD==NULL)return 0; // Ritorno 0 se non va a buon fine
	for(i=0;i<M*N;i++)BOARD[i]=3; // Setto la griglia come vuota
	srand(time(NULL));
	risultato=genera_albero(&NODOBASE,2,BOARD);
	if(risultato.scenaritotali==-1){free(BOARD);free_player();return 0;} // genero l'albero, in caso di errore restituisco 0
	return 1;}

int set_opponent_move(unsigned int i, unsigned int j){
	unsigned char posizione; //NODOBASE ha come figli le possibili mosse avversarie
	struct listaptr*ptr;
	if(i>=M||j>=N) return 0;
	BOARD[(posizione=getcoord(i,j))]=2; // posiziono nella casella indicata una pedina avversaria
	for(ptr=NODOBASE.figlio;ptr->nodo->casella!=posizione;ptr=ptr->next); // scorro i figli del nodo base fino a trovare il ramo che si è verificato
	uccidi_fratelli(&NODOBASE,ptr->nodo); // elimino tutti i rami che non si sono verificati
	NODOBASE.figlio=ptr->nodo->figlio;
	free(ptr->nodo);
	free(ptr); // elimino il nodo corrente, ora NODOBASE ha come unico figlio una mossa del computer
	return 1;}

int get_next_move(unsigned int *i, unsigned int *j){
	struct listaptr*ptr;
	if(NODOBASE.figlio==NULL)return 0;
	ptr=NODOBASE.figlio->nodo->figlio; //NODOBASE ha come unico figlio la mossa del computer
	getrowcol(NODOBASE.figlio->nodo->casella,i,j);
	BOARD[NODOBASE.figlio->nodo->casella]=1; // posiziono nella casella indicata nel nodo la pedina del computer
	free(NODOBASE.figlio->nodo);
	free(NODOBASE.figlio);
	NODOBASE.figlio=ptr; // elimino il nodo corrente, ora NODOBASE ha per figli le possibili mosse avversarie
	return 1;
}
