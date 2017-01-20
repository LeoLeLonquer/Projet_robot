/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tServeur;
RT_TASK tconnect;
RT_TASK tmove;
RT_TASK tenvoyer;
RT_TASK twatchrobot;
RT_TASK tcheckconnexion;
RT_TASK tcheckbattery;
RT_TASK tregarder;
RT_TASK tcaliber;

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;
RT_MUTEX mutexCountErrors;
RT_MUTEX mutexRegarderEtCalibrer;

RT_SEM semConnecterRobot;
RT_SEM semCheckBattery;
RT_SEM semWatchRobot;
RT_SEM semCalibrer;
RT_SEM semConnecteMoniteur;

RT_QUEUE queueMsgGUI;
RT_QUEUE queueErrMsg;


int etatCommMoniteur = 1;
int etatCommRobot = 1;
int countErrors = 0;
DRobot *robot;
DMovement *move;
DServer *serveur;
DBattery *battery;
DCamera *camera ;
DArena * arene ;
int msgCalibrer ; //Identification du type de message (is_found, failed, etc)
int ComputeContinuouslyPosition = 0 ;


int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 80;
int PRIORITY_TCONNECT = 70;
int PRIORITY_TMOVE = 50;
int PRIORITY_TENVOYER = 85;
int PRIORITY_TWATCHROBOT = 99;
int PRIORITY_TCHECKCONNEXION = 90;
int PRIORITY_TCHECKBATTERY= 5;
int PRIORITY_TREGARDER = 60;
int PRIORITY_TCALIBER= 65;
