/*
 * File:   fonctions.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:19
 */

#ifndef FONCTIONS_H
#define	FONCTIONS_H

#include "global.h"
#include "includes.h"

#ifdef	__cplusplus
extern "C" {
#endif
        void connecter (void * arg);
        void communiquer(void *arg);
        void deplacer(void *arg);
        void envoyer(void *arg);
	void regarder(void *arg);
	void surveiller(void *arg);
	void surveillerConnexion(void *arg);
        void verifEtatBatterie(void *arg);
	void calibrer(void *arg);

#ifdef	__cplusplus
}
#endif

#endif	/* FONCTIONS_H */
