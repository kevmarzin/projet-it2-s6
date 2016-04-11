/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
 *
 *   Copyright (C) 2014, 2015 Adrien Boussicault
 *
 *    This Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This Library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this Library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "automate.h"
#include "table.h"
#include "ensemble.h"
#include "outils.h"
#include "fifo.h"

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 

#include <assert.h>

#include <math.h>

void action_get_max_etat (const intptr_t element, void* data) {
	int* max = (int*) data;
	if (*max < element) *max = element;
}

int get_max_etat( const Automate* automate ){
	int max = INT_MIN;
	
	pour_tout_element (automate->etats, action_get_max_etat, &max);
	
	return max;
}

void action_get_min_etat( const intptr_t element, void* data ){
	int * min = (int*) data;
	if( *min > element ) *min = element;
}

int get_min_etat( const Automate* automate ){
	int min = INT_MAX;

	pour_tout_element( automate->etats, action_get_min_etat, &min );

	return min;
}

int comparer_cle(const Cle *a, const Cle *b) {
	if( a->origine < b->origine )
		return -1;
	if( a->origine > b->origine )
		return 1;
	if( a->lettre < b->lettre )
		return -1;
	if( a->lettre > b->lettre )
		return 1;
	return 0;
}

void print_cle( const Cle * a){
	printf( "(%d, %c)" , a->origine, (char) (a->lettre) );
}

void supprimer_cle( Cle* cle ){
	xfree( cle );
}

void initialiser_cle( Cle* cle, int origine, char lettre ){
	cle->origine = origine;
	cle->lettre = (int) lettre;
}

Cle * creer_cle( int origine, char lettre ){
	Cle * result = xmalloc( sizeof(Cle) );
	initialiser_cle( result, origine, lettre );
	return result;
}

Cle * copier_cle( const Cle* cle ){
	return creer_cle( cle->origine, cle->lettre );
}

Automate * creer_automate(){
	Automate * automate = xmalloc( sizeof(Automate) );
	automate->etats = creer_ensemble( NULL, NULL, NULL );
	automate->alphabet = creer_ensemble( NULL, NULL, NULL );
	automate->transitions = creer_table(
		( int(*)(const intptr_t, const intptr_t) ) comparer_cle , 
		( intptr_t (*)( const intptr_t ) ) copier_cle,
		( void(*)(intptr_t) ) supprimer_cle
	);
	automate->initiaux = creer_ensemble( NULL, NULL, NULL );
	automate->finaux = creer_ensemble( NULL, NULL, NULL );
	automate->vide = creer_ensemble( NULL, NULL, NULL ); 
	return automate;
}

Automate * translater_automate_entier( const Automate* automate, int translation ){
	Automate * res = creer_automate();

	Ensemble_iterateur it;
	for( 
		it = premier_iterateur_ensemble( get_etats( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat( res, get_element( it ) + translation );
	}

	for( 
		it = premier_iterateur_ensemble( get_initiaux( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat_initial( res, get_element( it ) + translation );
	}

	for( 
		it = premier_iterateur_ensemble( get_finaux( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat_final( res, get_element( it ) + translation );
	}

	// On ajoute les lettres
	for(
		it = premier_iterateur_ensemble( get_alphabet( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_lettre( res, (char) get_element( it ) );
	}

	Table_iterateur it1;
	Ensemble_iterateur it2;
	for(
		it1 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it1 );
		it1 = iterateur_suivant_table( it1 )
	){
		Cle * cle = (Cle*) get_cle( it1 );
		Ensemble * fins = (Ensemble*) get_valeur( it1 );
		for(
			it2 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it2 );
			it2 = iterateur_suivant_ensemble( it2 )
		){
			int fin = get_element( it2 );
			ajouter_transition(
				res, cle->origine + translation, cle->lettre, fin + translation
			);
		}
	};

	return res;
}


void liberer_automate( Automate * automate ){
	assert( automate );
	liberer_ensemble( automate->vide );
	liberer_ensemble( automate->finaux );
	liberer_ensemble( automate->initiaux );
	pour_toute_valeur_table(
		automate->transitions, ( void(*)(intptr_t) ) liberer_ensemble
	);
	liberer_table( automate->transitions );
	liberer_ensemble( automate->alphabet );
	liberer_ensemble( automate->etats );
	xfree(automate);
}

const Ensemble * get_etats( const Automate* automate ){
	return automate->etats;
}

const Ensemble * get_initiaux( const Automate* automate ){
	return automate->initiaux;
}

const Ensemble * get_finaux( const Automate* automate ){
	return automate->finaux;
}

const Ensemble * get_alphabet( const Automate* automate ){
	return automate->alphabet;
}

void ajouter_etat( Automate * automate, int etat ){
	ajouter_element( automate->etats, etat );
}

void ajouter_lettre( Automate * automate, char lettre ){
	ajouter_element( automate->alphabet, lettre );
}

void ajouter_transition(
	Automate * automate, int origine, char lettre, int fin
){
	ajouter_etat( automate, origine );
	ajouter_etat( automate, fin );
	ajouter_lettre( automate, lettre );

	Cle cle;
	initialiser_cle( &cle, origine, lettre );
	Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
	Ensemble * ens;
	if( iterateur_est_vide( it ) ){
		ens = creer_ensemble( NULL, NULL, NULL );
		add_table( automate->transitions, (intptr_t) &cle, (intptr_t) ens );
	}else{
		ens = (Ensemble*) get_valeur( it );
	}
	ajouter_element( ens, fin );
}

void ajouter_etat_final(
	Automate * automate, int etat_final
){
	ajouter_etat( automate, etat_final );
	ajouter_element( automate->finaux, etat_final );
}

void ajouter_etat_initial(
	Automate * automate, int etat_initial
){
	ajouter_etat( automate, etat_initial );
	ajouter_element( automate->initiaux, etat_initial );
}

const Ensemble * voisins( const Automate* automate, int origine, char lettre ){
	Cle cle;
	initialiser_cle( &cle, origine, lettre );
	Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
	if( ! iterateur_est_vide( it ) ){
		return (Ensemble*) get_valeur( it );
	}else{
		return automate->vide;
	}
}

Ensemble * delta1(
	const Automate* automate, int origine, char lettre
){
	Ensemble * res = creer_ensemble( NULL, NULL, NULL );
	ajouter_elements( res, voisins( automate, origine, lettre ) );
	return res; 
}

Ensemble * delta(
	const Automate* automate, const Ensemble * etats_courants, char lettre
){
	Ensemble * res = creer_ensemble( NULL, NULL, NULL );

	Ensemble_iterateur it;
	for( 
		it = premier_iterateur_ensemble( etats_courants );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		const Ensemble * fins = voisins(
			automate, get_element( it ), lettre
		);
		ajouter_elements( res, fins );
	}

	return res;
}

Ensemble * delta_star(
	const Automate* automate, const Ensemble * etats_courants, const char* mot
){
	int len = strlen( mot );
	int i;
	Ensemble * old = copier_ensemble( etats_courants );
	Ensemble * new = old;
	for( i=0; i<len; i++ ){
		new = delta( automate, old, *(mot+i) );
		liberer_ensemble( old );
		old = new;
	}
	return new;
}

void pour_toute_transition(
	const Automate* automate,
	void (* action )( int origine, char lettre, int fin, void* data ),
	void* data
){
	Table_iterateur it1;
	Ensemble_iterateur it2;
	for(
		it1 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it1 );
		it1 = iterateur_suivant_table( it1 )
	){
		Cle * cle = (Cle*) get_cle( it1 );
		Ensemble * fins = (Ensemble*) get_valeur( it1 );
		for(
			it2 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it2 );
			it2 = iterateur_suivant_ensemble( it2 )
		){
			int fin = get_element( it2 );
			action( cle->origine, cle->lettre, fin, data );
		}
	};
}

Automate* copier_automate( const Automate* automate ){
	Automate * res = creer_automate();
	Ensemble_iterateur it1;
	// On ajoute les états de l'automate
	for(
		it1 = premier_iterateur_ensemble( get_etats( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat( res, get_element( it1 ) );
	}
	// On ajoute les états initiaux
	for(
		it1 = premier_iterateur_ensemble( get_initiaux( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat_initial( res, get_element( it1 ) );
	}
	// On ajoute les états finaux
	for(
		it1 = premier_iterateur_ensemble( get_finaux( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat_final( res, get_element( it1 ) );
	}
	// On ajoute les lettres
	for(
		it1 = premier_iterateur_ensemble( get_alphabet( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_lettre( res, (char) get_element( it1 ) );
	}
	// On ajoute les transitions
	Table_iterateur it2;
	for(
		it2 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it2 );
		it2 = iterateur_suivant_table( it2 )
	){
		Cle * cle = (Cle*) get_cle( it2 );
		Ensemble * fins = (Ensemble*) get_valeur( it2 );
		for(
			it1 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it1 );
			it1 = iterateur_suivant_ensemble( it1 )
		){
			int fin = get_element( it1 );
			ajouter_transition( res, cle->origine, cle->lettre, fin );
		}
	}
	return res;
}

Automate * translater_automate(
	const Automate * automate, const Automate * automate_a_eviter
){
	if(
		taille_ensemble( get_etats(automate) ) == 0 ||
		taille_ensemble( get_etats(automate_a_eviter) ) == 0
	){
		return copier_automate( automate );
	}
	
	int translation = 
		get_max_etat( automate_a_eviter ) - get_min_etat( automate ) + 1; 

	return translater_automate_entier( automate, translation );
	
}

int est_une_transition_de_l_automate(
	const Automate* automate,
	int origine, char lettre, int fin
){
	return est_dans_l_ensemble( voisins( automate, origine, lettre ), fin );
}

int est_un_etat_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_etats( automate ), etat );
}

int est_un_etat_initial_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_initiaux( automate ), etat );
}

int est_un_etat_final_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_finaux( automate ), etat );
}

int est_une_lettre_de_l_automate( const Automate* automate, char lettre ){
	return est_dans_l_ensemble( get_alphabet( automate ), lettre );
}

void print_ensemble_2( const intptr_t ens ){
	print_ensemble( (Ensemble*) ens, NULL );
}

void print_lettre( intptr_t c ){
	printf("%c", (char) c );
}

void print_automate( const Automate * automate ){
	printf("- Etats : ");
	print_ensemble( get_etats( automate ), NULL );
	printf("\n- Initiaux : ");
	print_ensemble( get_initiaux( automate ), NULL );
	printf("\n- Finaux : ");
	print_ensemble( get_finaux( automate ), NULL );
	printf("\n- Alphabet : ");
	print_ensemble( get_alphabet( automate ), print_lettre );
	printf("\n- Transitions : ");
	print_table( 
		automate->transitions,
		( void (*)( const intptr_t ) ) print_cle, 
		( void (*)( const intptr_t ) ) print_ensemble_2,
		""
	);
	printf("\n");
}

int le_mot_est_reconnu( const Automate* automate, const char* mot ){
	Ensemble * arrivee = delta_star( automate, get_initiaux(automate) , mot ); 
	
	int result = 0;

	Ensemble_iterateur it;
	for(
		it = premier_iterateur_ensemble( arrivee );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		if( est_un_etat_final_de_l_automate( automate, get_element(it) ) ){
			result = 1;
			break;
		}
	}
	liberer_ensemble( arrivee );
	return result;
}

Automate * mot_to_automate( const char * mot ){
	Automate * automate = creer_automate();
	int i = 0;
	int size = strlen( mot );
	for( i=0; i < size; i++ ){
		ajouter_transition( automate, i, mot[i], i+1 );
	}
	ajouter_etat_initial( automate, 0 );
	ajouter_etat_final( automate, size );
	return automate;
}

void action_ajout_initiaux_union ( const intptr_t element, void* data ){
	ajouter_etat_initial (data, element);
}

void action_ajout_finaux_union ( const intptr_t element, void* data ){
	ajouter_etat_final (data, element);
}

void action_ajout_transitions_union ( int origine, char lettre, int fin, void* data ){
	ajouter_transition (data, origine, lettre, fin);
}

Automate * creer_union_des_automates(
	const Automate * automate_1, const Automate * automate_2
){
	// Renommage des sommet de l'automate_2 pour qu'il n'est pas les mêmes noms que ceux de l'automate_1
	Automate * automate_2_bis = translater_automate(automate_2, automate_1);
	
	// ajout des états et des transitions de l'automate_1 à celui de l'union
	Automate * automate_union = copier_automate (automate_1);
	
	// Ajout à l'automate de l'union des états finaux et initiaux de l'automate_2 
	pour_tout_element (get_initiaux (automate_2_bis), action_ajout_initiaux_union, automate_union);
	pour_tout_element (get_finaux (automate_2_bis), action_ajout_finaux_union, automate_union);
	
	// ajout des transitions de l'automate_2 à l'automate de l'union, ainsi que les états de 2 qui n'ont
	// pas encore été ajouté
	pour_toute_transition (automate_2_bis, action_ajout_transitions_union, automate_union);
	
	return automate_union;
}

Ensemble* etats_accessibles( const Automate * automate, int etat ){
	// Récupération de l'alphabet
	const Ensemble* alphabet = get_alphabet (automate);
	
	// Initialisation des ensembles
	Ensemble* accessibles_old = creer_ensemble (NULL, NULL, NULL);
	ajouter_element (accessibles_old, etat);
	Ensemble* accessibles_new = creer_ensemble (NULL, NULL, NULL);
	ajouter_elements (accessibles_new, accessibles_old);
	Ensemble* dest;
	
	// itérateur permettant de parcourrir l'ensemble de l'alphabet
	Ensemble_iterateur it_alphabet ;
	
	// On répète la procédure tant qu'on trouve de nouveaux états
	do {
		ajouter_elements (accessibles_old, accessibles_new);
		
		// on place l'it sur la première lettre
		it_alphabet = premier_iterateur_ensemble (alphabet);
		
		// Parcours des lettre de l'alphabet
		while (!iterateur_ensemble_est_vide (it_alphabet)) {
			// On récupère les états accessibles avec une lettre de l'alphabet depuis les états accessibles récupérés
			dest = delta (automate, accessibles_old, get_element (it_alphabet));
			
			// accessible_new = accessible_new UNION dest et on libère dest
			transferer_elements_et_libere (accessibles_new, dest);
			
			// On passe à la lettre suivante
			it_alphabet = iterateur_suivant_ensemble (it_alphabet);
		}
		
	} while (comparer_ensemble (accessibles_new, accessibles_old));
	
	liberer_ensemble (accessibles_old);
	
	return accessibles_new;
}

Ensemble* accessibles( const Automate * automate ){
	const Ensemble* initiaux = get_initiaux (automate);
	
	// Initialisation des ensembles
	Ensemble* etats_accessibles_depuis_initiaux = creer_ensemble (NULL, NULL, NULL);
	Ensemble* dest;
	
	// On se place sur le premier état initial
	Ensemble_iterateur it_initiaux = premier_iterateur_ensemble (initiaux);
	
	// On fait l'union des états accessibles depuis tous les états initiaux
	while (!iterateur_ensemble_est_vide (it_initiaux)) {
		// récupération des états accessibles depuis un état initial
		dest = etats_accessibles (automate, get_element (it_initiaux));
		
		// On effectue l'union avec les états récupéré jusque là et on libère de la mémoire
		transferer_elements_et_libere (etats_accessibles_depuis_initiaux, dest);
		
		// Etat initial suivant
		it_initiaux = iterateur_suivant_ensemble (it_initiaux);
	}
	
	return etats_accessibles_depuis_initiaux;
}

void action_ajout_transition_automate_accessible (int origine, char lettre, int fin, void* data){
	if(est_dans_l_ensemble(get_etats(data), origine) && est_dans_l_ensemble(get_etats(data), fin))
		ajouter_transition(data, origine, lettre, fin);
}

Automate *automate_accessible( const Automate * automate ){
	// Récupération des états accessibles dans un automate depuis des états initiaux
	Ensemble* ens_etats_accessible = accessibles (automate);
	
	// Les états finaux de l'automate accessible sont les états finaux accessible de automate
	Ensemble* nouv_etats_finaux = creer_intersection_ensemble (ens_etats_accessible, get_finaux (automate));
	
	// Création du nouvel automate
	Automate* nouv_automate = creer_automate ();
	
	// Les états accessibles de automate deviennent les états du nouvel automate
	nouv_automate->etats = ens_etats_accessible;
	
	// Même états initiaux
	nouv_automate->initiaux = copier_ensemble (get_initiaux (automate));
	
	// états finaux caculés précédemment
	nouv_automate->finaux = nouv_etats_finaux;
	
	// On parcours toute les transitions de l'automate source, si la transition lie deux états équivalent dans le nouvel automate on l'ajoute a celui ci
	pour_toute_transition (automate, action_ajout_transition_automate_accessible, nouv_automate);
	
	return nouv_automate;
}

Automate *miroir( const Automate * automate){
	Automate * res = creer_automate();
	
	// Le miroir a les mêmes états que l'automate source
	res->etats = copier_ensemble (get_etats (automate));
	
	// On transforme les états finaux en initiaux
	res->initiaux = copier_ensemble (get_finaux (automate));
	
	// On transforme les états initiaux en finaux
	res->finaux = copier_ensemble (get_initiaux (automate));
	
	// On parcours la toutes les transitions de l'automate
	Table_iterateur it;
	for(it = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it );
		it = iterateur_suivant_table( it )
	){
		Table_iterateur it2;
		
		// Récupération de l'état d'origine de la transition et de la lettre associée
		Cle * cle = (Cle*) get_cle( it );
		
		// récupération des états de destination
		Ensemble * dest = (Ensemble*) get_valeur( it );
		
		// On inverse les transitions récupérées
		for(it2 = premier_iterateur_ensemble( dest );
			! iterateur_ensemble_est_vide( it2 );
			it2 = iterateur_suivant_ensemble( it2 )
		){
			int dest = get_element( it2 );
			ajouter_transition( res, dest, cle->lettre, cle->origine );
		}
	}
	
	return res;
}

/*
 * On défini un couple de deux états,  
 *   - aut1 = état de l'automate 1
 *   - aut2 = état de l'automate 2
*/
struct My_couple_t { int aut1; int aut2; };   // On définit le couple
typedef struct My_couple_t* My_couple;

/*
 * Définition des fonctions permettants de gérer une table associative.
 */
int my_comparer_couple( const intptr_t c1, const intptr_t c2 ){ 
	if( ((My_couple) c1)->aut1 == ((My_couple) c2)->aut1 )
		return ((My_couple) c1)->aut2 - ((My_couple) c2)->aut2;
	return ((My_couple) c1)->aut1 - ((My_couple) c2)->aut1;
}
intptr_t my_copier_couple( const intptr_t c ){
	My_couple res = malloc( sizeof(My_couple) );
	res->aut1 = ((My_couple) c)->aut1;
	res->aut2 = ((My_couple) c)->aut2;
	return (intptr_t) res;
}
void my_supprimer_couple( intptr_t c ){
	free( ((My_couple)c) );
}

void ajouter_transitions_melange (
	int num_automate,
	Table* nouv_etats,
	const Automate* automate_1,
	const Automate* automate_2,
	Table_iterateur it_nouv_etats,
	char lettre, 
	int *id_etat,
	Automate* mela
){
	// Automate copie de l'automate choisie dans le bloc "if" suivant
	Automate* automate;
	
	int etat_src;
	// On défini l'état à partir duquel on va calculer les transitions
	// celui de l'automate_1 ou celui de l'automate_2 ?
	if (num_automate == 1){ // automate_1
		etat_src = ((My_couple) get_cle (it_nouv_etats))->aut1;
		automate = copier_automate (automate_1);
	}
	else{ // automate_2
		etat_src = ((My_couple) get_cle (it_nouv_etats))->aut2;
		automate = copier_automate (automate_2);
	}
	
	// On construit l'ensemble des états accessibles par l'état définie précédemment avec la lettre donnée
	Ensemble* dest = delta1 (automate, etat_src, lettre);
	liberer_automate (automate);
	
	// Parcours des états de destination
	Ensemble_iterateur it_dest = premier_iterateur_ensemble (dest);
	while(!iterateur_ensemble_est_vide (it_dest)){
		
		// Construction des nouveaux couples en fonction de l'état src choisi précédemment :
		// on remplace dans les nouveaux couple l'état source par les états de destination
		My_couple c = malloc (sizeof (*c));
		if (num_automate == 1){ // état de l'automate_1 du couple
			c->aut1 = get_element (it_dest);
			c->aut2 = ((My_couple) get_element (it_nouv_etats))->aut2;
		}
		else{ // état de l'automate_2 du couple
			c->aut1 = ((My_couple) get_element (it_nouv_etats))->aut1;
			c->aut2 = get_element (it_dest);
		}
		
		// Si le couple n'est pas déjà associé à un sommet du mélange, on l'ajoute à la table assiciative
		if (iterateur_est_vide (trouver_table (nouv_etats, (intptr_t) c))){
			// Si les deux états sont finaux l'état associé du mélange est final
			if (est_dans_l_ensemble (get_finaux (automate_1), c->aut1) && est_dans_l_ensemble (get_finaux (automate_2), c->aut2))
				ajouter_etat_final (mela, (*id_etat));
			add_table (nouv_etats, (intptr_t) c, (*id_etat));
			(*id_etat)++;
		}
		
		// Ajout de la transition à l'automate du mélanfe
		ajouter_transition (mela,
							get_valeur (it_nouv_etats),
							lettre,
							get_valeur (trouver_table (nouv_etats, (intptr_t) c)));
		
		// état suivant de destination
		it_dest = iterateur_suivant_ensemble (it_dest);
	}
	// On libère l'ensemble des états de destination
	liberer_ensemble (dest);
}
			
Automate * creer_automate_du_melange(
	const Automate* automate_1,  const Automate* automate_2
){
	// Automate du mélange
	Automate* mela = creer_automate();
	
	// Table associative qui associe un couple d'état de l'automate 1 et 2 à un nouvel état du mélange
	Table* nouv_etats = creer_table(
		my_comparer_couple, 
		my_copier_couple, 
		my_supprimer_couple
	);
	
	// Récupération des états initiaux des deux automates
	const Ensemble* initiaux_aut_1 = get_initiaux (automate_1);
	const Ensemble* initiaux_aut_2 = get_initiaux (automate_2);
	
	// Initialisation des itérateurs pour le parcours des initiaux
	Ensemble_iterateur it_initiaux_1 = premier_iterateur_ensemble (initiaux_aut_1);
	Ensemble_iterateur it_initiaux_2;
	
	// Numéro des nouveaux états du mélange
	int id_etat = 0;
	
	// On construit les couples d'états (aut1, aut2) tel que
	// aut1 (resp. aut2) appartient à l'automate_1 (resp. automate_2)
	// et on associe ces couples à un nouvel état du mélange
	while (!iterateur_ensemble_est_vide (it_initiaux_1)) {
		it_initiaux_2 = premier_iterateur_ensemble (initiaux_aut_2);
		
		while (!iterateur_ensemble_est_vide (it_initiaux_2)) {
			// Construction du couple d'états
			My_couple c = malloc (sizeof (*c));
			c->aut1 = get_element (it_initiaux_1);
			c->aut2 = get_element (it_initiaux_2);
			
			// Le nouvel état est un état initial du mélange
			ajouter_etat_initial (mela, id_etat);
			
			// On sauvegarde l'association du couple et du nouvel état
			add_table (nouv_etats, (intptr_t) c, id_etat );
			
			++id_etat;
			
			// état de l'automate_2 suivant
			it_initiaux_2 = iterateur_suivant_ensemble (it_initiaux_2);
		}
		
		// état de l'automate_1 suivant
		it_initiaux_1 = iterateur_suivant_ensemble (it_initiaux_1);
	}
	
	// construction de l'union de l'alphabet de l'automate_1 et de l'automate_2
	Ensemble* nouv_alphabet = copier_ensemble(get_alphabet (automate_1));
	ajouter_elements (nouv_alphabet, get_alphabet (automate_2));
	
	// On parcours la table qui associe les couples et leur numéro dans le mélange
	// En calculant les transitions qui sortent des états pour chaque lettre de l'alphabet construit
	Table_iterateur it_nouv_etats = premier_iterateur_table (nouv_etats);
	Ensemble_iterateur it_alphabet;
	
	int nb_nouv_etats_new = taille_table (nouv_etats);
	int nb_nouv_etats_old = nb_nouv_etats_new;
	
	while (!iterateur_est_vide (it_nouv_etats)){
		it_alphabet = premier_iterateur_ensemble (nouv_alphabet);
		while (!iterateur_ensemble_est_vide (it_alphabet)) {
			nb_nouv_etats_old = nb_nouv_etats_new;
			
			// Calcul des transitions à partir de l'état de l'automate_1 du couple
			ajouter_transitions_melange (1, nouv_etats, automate_1, automate_2, it_nouv_etats, 
											get_element (it_alphabet), &id_etat, mela);
			
			
			nb_nouv_etats_new = taille_table (nouv_etats);
			
			// Si l'opération n'a pas ajouter d'état on calcul les transitions à partir de l'état de l'automate_2
			if (nb_nouv_etats_new == nb_nouv_etats_old) {
				ajouter_transitions_melange (2, nouv_etats, automate_1, automate_2, it_nouv_etats, 
												get_element (it_alphabet), &id_etat, mela);
				nb_nouv_etats_new = taille_table (nouv_etats);
			}
			
			// Si des états ont été ajoutés durant une des deux opérations précédentes
			// Alors on recommence le parcours de la liste associative et de l'alphabet
			// (au cas ou des couples auraient été ajoutés avant l'itérateur de la liste associative courant)
			if (nb_nouv_etats_new > nb_nouv_etats_old) {
				it_alphabet = premier_iterateur_ensemble (nouv_alphabet);
				it_nouv_etats = premier_iterateur_table (nouv_etats);
			}
			else // Si aucun état n'a été ajouté on passe à la lettre suivante
				it_alphabet = iterateur_suivant_ensemble (it_alphabet);
		}
		
		// On passe à l'état suivant
		it_nouv_etats = iterateur_suivant_table (it_nouv_etats);
	}
	
	liberer_ensemble (nouv_alphabet);
	liberer_table (nouv_etats);
	
	return mela;
}

