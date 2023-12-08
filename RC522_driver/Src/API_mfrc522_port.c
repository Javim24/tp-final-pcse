/**
 * @file API_mfrc522_port.c
 * @brief Implementa las funciones 
 *		  del módulo API_mfrc522_port.
 */

#include "API_mfrc522_port.h"

/**
 * @brief estructura global privada para manejar el periférico SPI.
 */
static SPI_HandleTypeDef SPI;

/**
 *	@brief Funciones privadas usadas durante la
 *		   inicialización del SPI.
 */
static bool_t SPI_Init();
static void GPIO_Init();

/**
 *   @brief Inicializa el periférico SPI y configura
 *		   el pin GPIO usado para el CS.
 *   @retval Verdadero si se inicia correctamente,
 *           o falso si no se puede inicializar.
 */
bool_t portInit() {
	bool_t estado = SPI_Init();
	GPIO_Init();
	return estado;
}

/**
 *   @brief Inicializa el periférico SPI.
 *   @retval Verdadero si se inicia correctamente,
 *           o falso si no se puede inicializar.
 */
static bool_t SPI_Init() {
	SPI.Instance = SPI_INSTANCE;
	SPI.Init.Mode = SPI_MODE_MASTER;
	SPI.Init.Direction = SPI_DIRECTION_2LINES;
	SPI.Init.DataSize = SPI_DATASIZE_8BIT;
	SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
	SPI.Init.CLKPhase = SPI_PHASE_1EDGE;
	SPI.Init.NSS = SPI_NSS_SOFT;
	SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
	SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
	SPI.Init.TIMode = SPI_TIMODE_DISABLE;
	SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	SPI.Init.CRCPolynomial = 10;

	bool_t estado = true;

	if (HAL_SPI_Init(&SPI) != HAL_OK) {
		estado = false;
	}

	return estado;
}

/**
 *   @brief Configura el pin de CS.
 */
static void GPIO_Init() {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	__HAL_RCC_GPIOB_CLK_ENABLE();
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CS_GPIO_Port, &GPIO_InitStruct);
}

/**
 *   @brief Escribe la cantidad size de bytes desde el buffer txData
 *          al registro red_addr del dispositivo que está conectado por SPI.
 *		   Primero transmite la dirección del registro, y luego los datos.
 */
void spiWrite(uint8_t reg_addr, uint8_t *txData, uint16_t size) {
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SPI, &reg_addr, 1, SPI_TIMEOUT);
	HAL_SPI_Transmit(&SPI, txData, size, SPI_TIMEOUT);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

/**
 *   @brief Lee la cantidad indicada por size de bytes
 *          desde el registro reg_addr y los guarda en
 *          el buffer rxData.
 *		   Primero transmite la dirección del registro,
 *		   y luego recibe los datos. Para leer un byte se envía un 0
 *		   y se el byte recibido por MISO.
 */
void spiRead(uint8_t reg_addr, uint8_t *rxData, uint16_t size) {
	uint8_t dummyTx[size];
	for (uint16_t i = 0; i < size; i++) {
		dummyTx[i] = 0;
	}
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SPI, &reg_addr, 1, SPI_TIMEOUT);
	HAL_SPI_TransmitReceive(&SPI, dummyTx, rxData, size, SPI_TIMEOUT);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}
