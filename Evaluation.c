#include "Evaluation.h"

#include <wait.h>
#include <fcntl.h>
#include <unistd.h>

void verifier(int cond, char *s){
  if (!cond){
    perror(s);
    exit(EXIT_FAILURE);
  }
}


int evaluer_expr (Expression *e){
	
	int status = 0;
	int o;
	int i;
	int fd_test	;
	int r_value;
	int entree_backup, sortie_backup, erreur_backup;
	switch (e->type) {
		case VIDE:
			return 0;
		
 		case BG :
			if(fork() == 0) {
				r_value = evaluer_expr(e->gauche); 
			}
			return 0;

		case SIMPLE:
			if(fork()==0){ 			// on crée un fils qui executera le processus lié de la commande demandée
				execvp(e->arguments[0], &e->arguments[0]);
				perror(e->arguments[0]);
				exit(1);
			}

			wait(&status);

			if(WIFEXITED(status)){
				status = WEXITSTATUS(status);
			} else {
				status = WTERMSIG(status) + 128;
			}

			return status;
		
		case SEQUENCE:
			 evaluer_expr(e->gauche);
			 evaluer_expr(e->droite);
			 break;

		case SEQUENCE_ET:

			if(r_value = evaluer_expr(e->gauche) == 0){
				r_value = evaluer_expr(e->droite);
			}
			return r_value;
		
		case SEQUENCE_OU:
			if(r_value = evaluer_expr(e->gauche) != 0){
				r_value = evaluer_expr(e->droite);
			}
			return r_value;

	 	case PIPE:
		;
			entree_backup = dup(0);
			sortie_backup = dup(1);
			int tube[2];
			if (pipe(tube) == -1){
				perror("tube : ");
			}

			dup2(tube[1],1);			// redirection de la sortie standard
		    close(tube[1]);
			evaluer_expr(e->gauche);	// évaluation de l'expression de gauche 
			
			dup2(sortie_backup,1);		// rétabli la sortie standard

			dup2(tube[0], 0);			// redirection de l'entrée
			close(tube[0]);
			evaluer_expr(e->droite);

			dup2(entree_backup,0);		// rétabli l'entrée standard
			
			close(entree_backup);
			close(sortie_backup);
			break;

		case REDIRECTION_I:
		;
			entree_backup = dup(0);

			i = open(e->arguments[0],O_RDONLY);
  			verifier(i != -1, "file : ");
			
			fd_test = dup2(i,0);
  			verifier(fd_test != -1, "redirection failure : ");
			
			evaluer_expr(e->gauche);
			
			dup2(entree_backup,0);
			
			close(entree_backup);
			close(i);
			break;

		case REDIRECTION_O:
		;
			sortie_backup = dup(1);
			o = open(e->arguments[0], O_WRONLY | O_CREAT | O_TRUNC , 0666);
  			verifier(o != -1, "file : ");
			
			fd_test = dup2(o,1);
  			verifier(fd_test != -1, "redirection failure : ");
			
			evaluer_expr(e->gauche);
			
			dup2(sortie_backup,1);
			
			close(sortie_backup);
			close(o);
			break;

		case REDIRECTION_A:
		;
			sortie_backup = dup(1);
			o = open(e->arguments[0], O_WRONLY | O_CREAT | O_APPEND, 0666);
  			verifier(o != -1, "file : ");
			
			fd_test = dup2(o,1);
  			verifier(fd_test != -1, "redirection failure : ");
			
			evaluer_expr(e->gauche);
			
			dup2(sortie_backup,1);
			
			close(sortie_backup);
			close(o);
			break;

		case REDIRECTION_E:
		;
			erreur_backup = dup(2);

			o = open(e->arguments[0], O_WRONLY | O_CREAT , 0666);
  			verifier(o != -1, "file : ");
			
			fd_test = dup2(o,2);
  			verifier(fd_test != -1, "redirection failure : ");
			
			evaluer_expr(e->gauche);
			
			dup2(erreur_backup,2);

			close(erreur_backup);
			close(o);
			break;

		case REDIRECTION_EO:
		;
			sortie_backup = dup(1);
			erreur_backup = dup(2);

			o = open(e->arguments[0], O_WRONLY | O_CREAT , 0666);
  			verifier(o != -1, "file : ");
			
			fd_test = dup2(o,1);
  			verifier(fd_test != -1, "redirection failure : ");
			
			fd_test = dup2(o,2);
  			verifier(fd_test != -1, "redirection failure : ");
			
			evaluer_expr(e->gauche);
			
			dup2(sortie_backup,1);
			dup2(erreur_backup,2);
			
			close(sortie_backup);
			close(erreur_backup);
			close(o);
			break;

		default: 
			fprintf(stderr, "not yet implemented \n");
			return 1;
	}
	return 1;
}