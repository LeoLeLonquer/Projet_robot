/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tServeur;
extern RT_TASK tconnect;
extern RT_TASK tmove;
extern RT_TASK tenvoyer;
extern RT_TASK twatchrobot;
extern RT_TASK tcheckconnexion;
extern RT_TASK tcheckbattery;
extern RT_TASK tregarder;
extern RT_TASK tcaliber;

/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;
extern RT_MUTEX mutexCountErrors;
extern RT_MUTEX mutexRegarderEtCalibrer;

/* @descripteurs des sempahore */
extern RT_SEM semConnecterRobot;
extern RT_SEM semCheckBattery;
extern RT_SEM semWatchRobot;
extern RT_SEM semCalibrer;
extern RT_SEM semConnecteMoniteur;

/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;
extern RT_QUEUE queueErrMsg;
extern RT_QUEUE queueCalibrer;

/* @variables partagées */
extern int etatCommMoniteur;
extern int etatCommRobot;
extern int countErrors;
extern DServer *serveur;
extern DRobot *robot;
extern DMovement *move;
extern DBattery *battery;
extern DCamera *camera ;
extern DArena * arene ;
extern int msgCalibrer; //Identification du type de message (is_found, failed, etc)
extern int ComputeContinuouslyPosition ;

/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TSERVEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TWATCHROBOT;
extern int PRIORITY_TCHECKCONNEXION;
extern int PRIORITY_TCHECKBATTERY;
extern int PRIORITY_TREGARDER;
extern int PRIORITY_TCALIBER;

#endif	/* GLOBAL_H */
