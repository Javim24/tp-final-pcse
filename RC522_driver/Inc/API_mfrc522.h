/**
 * @gile API_mfrc522.h
 * @brief Módulo que controla el lector RFID
 * 		  MFRC522. Permite  el  acceso a funciones
 * 		  para inicializar el módulo y detectar la
 * 		  presencia de una tarjeta en  el campo RF
 * 		  del lector.
 */

#ifndef API_INC_API_MFRC522_H_
#define API_INC_API_MFRC522_H_

#include "API_types.h"

/**
 *   @brief Inicializa el módulo MFRC522
 */
void mfrc522_init();

/**
 *   @brief Reinicializa los valores
 *          almacenados en los registros
 *          internos del MFRC522.
 */
void mfrc522_reset();

/**
 *   @brief Detecta si hay alguna tarjeta
 *          en proximidades del lector RFID.
 *          En caso afirmativo lee el UID de
 *          la tarjeta y lo almacena en la
 *          posición de memoria a la que apunta
 *          el puntero uid.
 *   @retval Devuelve verdadero si se logra detectar
 *           una tarjeta, en caso contrario devuelve falso.
 */
bool_t mfrc522_leerUIDTarjeta(uint8_t *uid);

#endif /* API_INC_API_MFRC522_H_ */
