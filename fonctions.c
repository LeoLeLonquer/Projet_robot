#include "fonctions.h"

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);

void envoyer(void * arg) {
  DMessage *msg;
  int err;

  while (1) {
    rt_printf("tenvoyer : Attente d'un message\n");
    if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
      rt_printf("tenvoyer : envoi d'un message au moniteur\n");
      serveur->send(serveur, msg);
      msg->free(msg);
    } else {
      rt_printf("Error msg queue write: %s\n", strerror(-err));

    }
  }
}

void connecter(void * arg) {
  int status;
  DMessage *message;

  rt_printf("tconnect : Debut de l'exécution de tconnect\n");

  while (1) {
    rt_printf("tconnect : Attente du sémarphore semConnecterRobot\n");
    rt_sem_p(&semConnecterRobot, TM_INFINITE);
    rt_printf("tconnect : Ouverture de la communication avec le robot\n");
    status = robot->open_device(robot);

    if (status == STATUS_OK) {
      status = robot->start(robot);
      if (status == STATUS_OK){
        rt_sem_v(&semCheckBattery);
        rt_sem_v(&semWatchRobot);
        rt_printf("tconnect : Robot démarrer\n");
      }
    }

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    etatCommRobot = status;
    rt_mutex_release(&mutexEtat);

    message = d_new_message();
    message->put_state(message, status);

    rt_printf("tconnect : Envoi message\n");
    message->print(message, 100);

    if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
      message->free(message);
    }
  }
}


void regarder(void * arg) {
	DImage *image ;
	DPosition * position ;
	DJpegimage * jpeg ;
	DMessage * message ;

    rt_printf("tregarder : Debut de l'exécution de tregardert\n");
    rt_printf("tregarder : Attente du sémarphore semConnecteMoniteur\n");
    rt_sem_p(&semConnecteMoniteur, TM_INFINITE);
    rt_printf("tregarder : connecte au moniteur, lancement du calcul périodique des images\n");

    rt_printf("tregarder : Debut de l'éxecution de periodique à 600ms\n");
    rt_task_set_periodic(NULL, TM_NOW, 600000000);
    while (1) {
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("tregarder : Activation périodique\n");
		rt_mutex_acquire(&mutexRegarderEtCalibrer, TM_INFINITE);

		if (ComputeContinuouslyPosition == 1) { // On a reçu le message ComputeContinuouslyPosition
			image = d_new_image() ;
			jpeg = d_new_jpegimage() ;
			message = d_new_message() ;
			camera->get_frame(camera, image) ;
			position = image->compute_robot_position(image, arene) ;
			d_imageshop_draw_position(image, arene) ;
			jpeg->compress(jpeg, image) ;

			rt_printf("tregarder : Envoi message 1 : image\n");
			message->put_jpeg_image(message, jpeg) ;
	        message->print(message, 100);
	        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
	            //message->free(message);
	        }
			rt_printf("tregarder : Envoi message 2 : position du robot\n");
			message->put_position(message, position) ;
	        message->print(message, 100);
	        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
	            message->free(message);
	        }
			image->free(image) ;
			jpeg->free(jpeg) ;
			position->free(position) ;
		} else if (ComputeContinuouslyPosition == 0) { // On n'a pas reçu le message ComputeContinuouslyPosition
			image = d_new_image() ;
			jpeg = d_new_jpegimage() ;
			message = d_new_message() ;
			camera->get_frame(camera, image) ;
			jpeg->compress(jpeg, image) ;

			rt_printf("tregarder : Envoi message\n");
			message->put_jpeg_image(message, jpeg) ;
	        message->print(message, 100);
	        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
	            message->free(message);
	        }
			image->free(image) ;
			jpeg->free(jpeg) ;
		} else {
			rt_printf("tregarder : Il y a une erreur dans la valeur de ComputeContinuouslyPosition\n");
		}

	    rt_mutex_release(&mutexRegarderEtCalibrer);
	}
}

void calibrer(void * arg) {
	DImage *image ;
	DMessage * message ;
	DJpegimage * jpeg ;
	int message_recu ;
	
	rt_printf("tcalibrer : Debut de l'exécution de tcalibrer\n");
	while(1) {
		rt_printf("tcalibrer : Attente de la réception d'un message de calibration\n");
		rt_sem_p(&semCalibrer, TM_INFINITE);
		message_recu = msgCalibrer ;

		if (message_recu = ACTION_FIND_ARENA) {
			rt_mutex_acquire(&mutexRegarderEtCalibrer, TM_INFINITE);
			
			//la première fois qu'on arrive ici, on entre forcément dans le boucle
			while (message_recu = ACTION_FIND_ARENA) {
				image = d_new_image() ;
				jpeg = d_new_jpegimage() ;
				message = d_new_message() ;

				camera->get_frame(camera, image) ;
				arene = image->compute_arena_position(image) ;
				d_imageshop_draw_arena(image, arene) ;
				jpeg->compress(jpeg, image) ;

				rt_printf("tcalibrer : Envoi message\n");
				message->put_jpeg_image(message, jpeg) ;
        		message->print(message, 100);
				if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
					message->free(message) ;
				}

				rt_printf("tcalibrer : Attente réponse de moniteur\n");
				rt_sem_p(&semCalibrer, TM_INFINITE);
				message_recu = msgCalibrer ; //si le message est de type FIND, on retourne dans la boucle ensuite

				image->free(image) ;
				jpeg->free(jpeg) ;
				message->free(message) ;
			}
			//message_recu contient ce qui a été reçu comme réponse à notre image de l'arène
			//if (message_recu = ACTION_ARENA_IS_FOUND) {} //?? dans le cahier des charges, il faut "sauvegarder" l'arène. Mais on l'a déjà fait en fait...
			// si on a ACTION_ARENA_FAILED, on ne fait rien et on reboucle tout de suite.

		    rt_mutex_release(&mutexRegarderEtCalibrer);
		} else {
			rt_printf("tcalibrer : Problème : reçu message incorrect, retour au début de la boucle\n");
		}
	}
}

void communiquer(void *arg) { 
    DMessage *msg = d_new_message();
    int var1 = 1;
    int num_msg = 0;
    int type_action;

    rt_printf("tserver : Début de l'exécution de serveur\n");
    serveur->open(serveur, "8000");
    rt_printf("tserver : Connexion\n");

	rt_printf("tserver : envoi du signal \"connecteMoniteur\"\n");
	rt_sem_v(&semConnecteMoniteur);

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    etatCommMoniteur = 0;
    rt_mutex_release(&mutexEtat);

    while (var1 > 0) {
        rt_printf("tserver : Attente d'un message\n");
        var1 = serveur->receive(serveur, msg);
        num_msg++;
        if (var1 > 0) { //Si var1 <= 0, sortir de la boucle
            switch (msg->get_type(msg)) {
                case MESSAGE_TYPE_ACTION:
                    rt_printf("tserver : Le message %d reçu est une action\n",
                            num_msg);
                    DAction *action = d_new_action();
                    action->from_message(action, msg);
                    type_action = action->get_order(action);
                    switch (type_action) {
                        case ACTION_CONNECT_ROBOT:
                            rt_printf("tserver : Action connecter robot\n");
                            rt_sem_v(&semConnecterRobot);
                            break;
                        case ACTION_COMPUTE_CONTINUOUSLY_POSITION:
                            rt_printf("tserver : Action calculer la position du robot en continu\n");
                            ComputeContinuouslyPosition = 1 ;
                            break;
                       // pout tout message lié à la calibration de l'arène, transmettre le message au thread calibrer
                        case : ACTION_FIND_ARENA:
                        case : ACTION_ARENA_FAILED:
                        case : ACTION_ARENA_IS_FOUND:
                            rt_printf("tserver : Action liée à la calibration de l'arène\n");
                            msgCalibrer = type_action; //mettre le mesage dans la variable,
                            rt_sem_v(&semCalibrer);//puis envoyer le signal //Est-ce que le TM_INFINITE pose problème ?
                            //Est-ce qu'il risque y avoir deux messages de calibration qui se suivent sans pouvoir les traiter ?
                            break;
                    }
                    break;
                case MESSAGE_TYPE_MOVEMENT:
                    rt_printf("tserver : Le message reçu %d est un mouvement\n",
                            num_msg);
                    rt_mutex_acquire(&mutexMove, TM_INFINITE);
                    move->from_message(move, msg);
                    move->print(move);
                    rt_mutex_release(&mutexMove);
                    break;
                case MESSAGE_TYPE_MISSION:
                    rt_printf("tserver : Le message reçu %d est une mission\n",
                            num_msg);
                    rt_printf("tserver : Pour l'instant, on ne traite pas ce genre de message\n");
                    break;
            }
        } else {
            rt_printf("tserver : problème à la réception d'un message, sortie de la boucle\n");
        }
    }
}

void deplacer(void *arg) {
  int status = 1;
  int gauche;
  int droite;

  rt_printf("tmove : Debut de l'éxecution de periodique à 200 ms\n");
  rt_task_set_periodic(NULL, TM_NOW, 200000000);

  while (1) {
    /* Attente de l'activation périodique */
    rt_task_wait_period(NULL);
    rt_printf("tmove : Activation périodique\n");

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    status = etatCommRobot;
    rt_mutex_release(&mutexEtat);

    if (status == STATUS_OK) {
      rt_mutex_acquire(&mutexMove, TM_INFINITE);
      switch (move->get_direction(move)) {
        case DIRECTION_FORWARD:
        gauche = MOTEUR_ARRIERE_LENT;
        droite = MOTEUR_ARRIERE_LENT;
        break;
        case DIRECTION_LEFT:
        gauche = MOTEUR_ARRIERE_LENT;
        droite = MOTEUR_AVANT_LENT;
        break;
        case DIRECTION_RIGHT:
        gauche = MOTEUR_AVANT_LENT;
        droite = MOTEUR_ARRIERE_LENT;
        break;
        case DIRECTION_STOP:
        gauche = MOTEUR_STOP;
        droite = MOTEUR_STOP;
        break;
        case DIRECTION_STRAIGHT:
        gauche = MOTEUR_AVANT_LENT;
        droite = MOTEUR_AVANT_LENT;
        break;
      }
      rt_mutex_release(&mutexMove);

      status = robot->set_motors(robot, gauche, droite);

      rt_queue_write(&queueErrMsg,&status,sizeof(int),Q_NORMAL);

    }
  }
}

void surveiller(void *arg) {
  int status;

  rt_printf("twatchrobot : Début de l'éxecution de periodique à 1s\n");
  rt_task_set_periodic(NULL, TM_NOW, 1000000000);

  while(1){

    rt_printf("twatchrobot : Attente du sémaphore semWatchRobot\n");
    rt_sem_p(&semWatchRobot, TM_INFINITE);
    rt_printf("twatchrobot : Début de l'envoi de reload_wdt\n");

    do{
      /* Attente de l'activation périodique */
      rt_task_wait_period(NULL);
      rt_printf("twatchrobot : Activation périodique\n");
      status = robot->reload_wdt(robot);

      rt_printf("twatchrobot : Envoi status à tcheckconnexion\n");
      rt_queue_write(&queueErrMsg,&status,sizeof(int),Q_NORMAL);

      rt_mutex_acquire(&mutexEtat, TM_INFINITE);
      status = etatCommRobot;
      rt_mutex_release(&mutexEtat);

    }while(status == STATUS_OK);

  }
}

void surveillerConnexion(void *arg) {
  int status;
  int loc_cpt;
  int err;
  DMessage *message;

  while (1) {
    rt_printf("tcheckconnexion : Attente d'un message\n");
    if ((err = rt_queue_read(&queueErrMsg, &status, sizeof (int), TM_INFINITE)) >= 0) { /* !!!!!!!!!!!!!!!!!!!! Erreur possible ICI ***************************/
      if(status == STATUS_OK){
        rt_mutex_acquire(&mutexCountErrors, TM_INFINITE);
        countErrors = 0;
        rt_mutex_release(&mutexCountErrors);
      }
      else{
        rt_mutex_acquire(&mutexCountErrors, TM_INFINITE);
        loc_cpt = countErrors;
        rt_mutex_release(&mutexCountErrors);
        if ( (loc_cpt+1) > 3 ){
          rt_printf("tcheckconnexion : Perte de connexion détectée\n");
          rt_mutex_acquire(&mutexCountErrors, TM_INFINITE);
          countErrors=0;
          rt_mutex_release(&mutexCountErrors);

          rt_mutex_acquire(&mutexEtat, TM_INFINITE);
          etatCommRobot = STATUS_ERR_UNKNOWN;
          rt_mutex_release(&mutexEtat);

          message = d_new_message();
          message->put_state(message, STATUS_ERR_UNKNOWN);

          rt_printf("tcheckconnexion : Envoi message\n");
          message->print(message, 100);

          if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
          }
        }
        else{
          rt_mutex_acquire(&mutexCountErrors, TM_INFINITE);
          countErrors++;
          rt_mutex_release(&mutexCountErrors);
        }
      }
    } else {
      rt_printf("tcheckconnexion - Error msg queue write: %s\n", strerror(-err));
    }
  }
}



int write_in_queue(RT_QUEUE *msgQueue, void * data, int size) {
  void *msg;
  int err;

  msg = rt_queue_alloc(msgQueue, size);
  memcpy(msg, &data, size);

  if ((err = rt_queue_send(msgQueue, msg, sizeof (DMessage), Q_NORMAL)) < 0) {
    rt_printf("Error msg queue send: %s\n", strerror(-err));
  }
  rt_queue_free(&queueMsgGUI, msg);

  return err;
}

void verifEtatBatterie(void *arg){
  DMessage *message;

  int status;
  int* level=(int*)malloc(sizeof(int));

  //rt_printf("tcheckbattery : attente du sémaphore semCheckBattery\n");
  rt_sem_p(&semCheckBattery,TM_INFINITE);
 // rt_printf("tcheckbattery : réception du sémaphore semCheckBattery\n");

  //rt_printf("tcheckbattery : Début de l'éxecution de periodique à 250ms\n");
  rt_task_set_periodic(NULL, TM_NOW, 250000000);

  while (1) {
	*level = -1;
    /* Attente de l'activation périodique */
    rt_task_wait_period(NULL);
    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    status = etatCommRobot;
    rt_mutex_release(&mutexEtat);

    if (status == STATUS_OK) {
      status=robot->get_vbat(robot, level);
	  if(*level >= 0 )
	  	battery->set_level(battery,*level);
        rt_printf("tcheckbattery : Envoi status %d\n",status);
      rt_queue_write(&queueErrMsg,&status,sizeof(int),Q_NORMAL);

      if (status ==STATUS_OK){
        message = d_new_message();
        message->put_battery_level(message, battery);

//        rt_printf("tcheckbattery : Envoi message\n");
        message->print(message, 100);

        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
          message->free(message);
        }
      }
    }
  }



}
