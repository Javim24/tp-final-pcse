/**
 * @file API_mfrc522_port.h
 * @brief Módulo que implementa
 *        funciones de bajo nivel
 *        para utilizar el perifeco SPI.
 */

#ifndef API_INC_API_MFRC522_PORT_H_
#define API_INC_API_MFRC522_PORT_H_

#include "stm32f4xx.h"
#include "API_types.h"

//constantes para la comunicación SPI
#define SPI_INSTANCE        SPI1
#define CS_Pin              GPIO_PIN_6
#define CS_GPIO_Port        GPIOB
#define SPI_TIMEOUT         10

/**
 *   @brief Inicializa el periférico SPI.
 *   @retval Verdadero si se inicia correctamente,
 *           o falso si no se puede inicializar.
 */
bool_t portInit();

/**
 *   @brief Escribe la cantidad size de bytes desde el buffer txData
 *          al registro red_addr del dispositivo que está conectado por SPI.
 */
void spiWrite(uint8_t reg_addr, uint8_t *txData, uint16_t size);

/**
 *   @brief Lee la cantidad indicada por size de bytes
 *          desde el registro reg_addr y los guarda en
 *          el buffer rxData.
 */
void spiRead(uint8_t reg_addr, uint8_t *rxData, uint16_t size);

#endif /* API_INC_API_MFRC522_PORT_H_ */
