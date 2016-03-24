#include "automate.h"
#include "outils.h"

int test_creer_union_des_automates(){
	int result = 1;
	
	{
		Automate * automate_1 = creer_automate();
		Automate * automate_2 = creer_automate();

		ajouter_transition( automate_1, 1, 'a', 2 );
		ajouter_transition( automate_1, 2, 'a', 2 );
		ajouter_transition( automate_1, 2, 'b', 3 );
		
		ajouter_transition( automate_2, 1, 'b', 2 );
		ajouter_transition( automate_2, 2, 'b', 2 );
		ajouter_transition( automate_2, 2, 'a', 3 );

		ajouter_etat_final( automate_1, 3 );
		ajouter_etat_final( automate_2, 3 );
		ajouter_etat_initial( automate_1, 1 );
		ajouter_etat_initial( automate_2, 1 );
		
		Automate * aut_union = creer_union_des_automates( automate_1, automate_2 );
	
		TEST(
				1
				&& aut_union
				&& le_mot_est_reconnu( aut_union, "ab" )
				&& le_mot_est_reconnu( aut_union, "aab" )
				&& le_mot_est_reconnu( aut_union, "aaab" )
				&& le_mot_est_reconnu( aut_union, "aaaab" )
				&& le_mot_est_reconnu( aut_union, "ba" )
				&& le_mot_est_reconnu( aut_union, "bba" )
				&& le_mot_est_reconnu( aut_union, "bbba" )
				&& le_mot_est_reconnu( aut_union, "bbbba" )
				&& ! le_mot_est_reconnu( aut_union, "" )
				&& ! le_mot_est_reconnu( aut_union, "a" )
				&& ! le_mot_est_reconnu( aut_union, "aa" )
				&& ! le_mot_est_reconnu( aut_union, "aaa" )
				&& ! le_mot_est_reconnu( aut_union, "aba" )
				&& ! le_mot_est_reconnu( aut_union, "abb" )
				&& ! le_mot_est_reconnu( aut_union, "bb" )
				&& ! le_mot_est_reconnu( aut_union, "b" )
				&& ! le_mot_est_reconnu( aut_union, "bbb" )
				&& ! le_mot_est_reconnu( aut_union, "bab" )
				&& ! le_mot_est_reconnu( aut_union, "baa" )
				, result
			);
	
		liberer_automate( automate_1);
		liberer_automate( automate_2);
		liberer_automate( aut_union);
		
	}
	
	return result;
}

int main(){

	if( ! test_creer_union_des_automates() ){ return 1; };

	return 0;
	
}