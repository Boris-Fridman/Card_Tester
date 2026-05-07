/*
 * Network.h
 *
 *  Created on: 3 May 2026
 *      Author: boris
 */

#ifndef NETWORK_H_
#define NETWORK_H_


#include "CommonData.h"

#include <stdint.h>
#include <stdbool.h>

/*======================================================================================================================*/


/**
 * @brief Initializes the network UDP Server.
 *
 * @code
 * void InitNetwork(void);
 * @code
 *
 */
void InitNetwork(void);
void DeinitNetwork(void);


#endif /* NETWORK_H_ */
