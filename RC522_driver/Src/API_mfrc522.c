/**
 * @file API_mfrc522.c
 * @brief  Implementación de funciones
 *		   públicas y privadas del
 * 		   módulo MFRC522. 
 *	@note  Las definiciones de registros, bits y
 *		   funcionamiento del módulo se encuentran
 *		   en el manual del MFRC522.
 */

#include "API_mfrc522.h"
#include "API_mfrc522_port.h"

// Definición de máscaras para definir si se lee o escribe en un registro.
// Definidas en sección 8.1.2.3 del manual.
static const uint8_t READ_MASK = 0x80;
static const uint8_t WRITE_MASK = 0;

// Definiciones de algunos bits de registros utilizados
#define TxControlReg_Tx2RFEn				(1<<1)
#define TxControlReg_Tx1RFEn				1
#define ComIrqReg_RxIrq  					(1<<5)
#define ComIrqReg_TimerIrq					(1<<0)
#define FIFOLevelReg_FlushBuffer			0x80
#define BitFramingReg_StartSend				0x80
#define ComIrqReg_Set1						0x80

#define UID_SIZE							4
#define NVB_CL1								0x20

//registros definidos en sección 9.2 de la hoja de datos
typedef enum {
	//registros de comandos y estado
	CommandReg = 0x01,
	ComIEnReg = 0x02,
	DivIEnReg = 0x03,
	ComIrqReg = 0x04,
	DivIrqReg = 0x05,
	ErrorReg = 0x06,
	Status1Reg = 0x07,
	Status2Reg = 0x08,
	FIFODataReg = 0x09,
	FIFOLevelReg = 0x0A,
	WaterLevelReg = 0x0B,
	ControlReg = 0x0C,
	BitFramingReg = 0x0D,
	CollReg = 0x0E,

	//registros de comandos
	ModeReg = 0x11,
	TxModeReg = 0x12,
	RxModeReg = 0x13,
	TxControlReg = 0x14,
	TxASKReg = 0x15,
	TxSelReg = 0x16,
	RxSelReg = 0x17,
	RxThresholdReg = 0x18,
	DemodReg = 0x19,
	//
	//
	MfTxReg = 0x1C,
	MfRxReg = 0x1D,
	//
	SerialSpeedReg = 0x1F,

	//registros de configuracion
	CRCResultRegH = 0x21,
	CRCResultRegL = 0x22,
	//
	ModWidthReg = 0x24,
	//
	RFCfgReg = 0x26,
	GsNReg = 0x27,
	CWGsPReg = 0x28,
	ModGsPReg = 0x29,
	TModeReg = 0x2A,
	TPrescalerReg = 0x2B,
	TReloadRegH = 0x2C,
	TReloadRegL = 0x2D,
	TCounterValueRegH = 0x2E,
	TCounterValueRegL = 0x2F,

	//registros de prueba
	TestSel1Reg = 0x31,
	TestSel2Reg = 0x32,
	TestPinEnReg = 0x33,
	TestPinValueReg = 0x34,
	TestBusReg = 0x35,
	AutoTestReg = 0x36,
	VersionReg = 0x37,
	AnalogTestReg = 0x38,
	TestDAC1Reg = 0x39,
	TestDAC2Reg = 0x3A,
	TestADCReg = 0x3B
} registros_MFRC522_enum;

//comandos del MFRC522, sección 10.3 de la hoja de datos
typedef enum {
	Idle = 0x00,
	Mem = 0x01,
	GenerateRandomID = 0x02,
	CalcCRC = 0x03,
	Transmit = 0x04,
	NoCmdChange = 0x07,
	Receive = 0x08,
	Transceive = 0x0C,
	MFAuthent = 0x0E,
	SoftReset = 0x0F
} comandos_MFRC522_enum;

// Comandos a enviar a la tarjeta obtenidos de ISO/IEC 14443-3
typedef enum {
	CMD_REQA = 0x26, CMD_SEL_CL1 = 0x93
} comandos_tarjeta_enum;

/**
 *	@brief Declaración de funciones privadas
 *		   que se utilizan para manejar el
 *		   funcionamiento del MFRC522.
 */
static bool_t mfrc522_leerUID(uint8_t *uid);
static bool_t mfrc522_detectarTarjeta();
static bool_t mfrc522_leerBufferFIFO(uint8_t*, uint8_t);
static bool_t mfrc522_esperarRespuestaTarjeta();
static void mfrc522_enviarComandoTarjeta(uint8_t *comando, uint8_t largo,
		uint8_t txSize);
static void mfrc522_encenderAntena();

/**
 *	@brief Funciones para comunicarse con
 *		   el módulo por SPI.
 */
static void mfrc522_writeRegister(registros_MFRC522_enum, uint8_t);
static uint8_t mfrc522_readRegister(registros_MFRC522_enum);

/**
 *	@brief Inicializa el periférico SPI
 *		   y luego configura los parámetros
 *		   de funcionamiento del MFRC522.
 *
 */
void mfrc522_init() {
	bool_t spiActivo = portInit();

	if (!spiActivo)
		return;

	mfrc522_reset();

	// El módulo MFRC522 tiene un timer interno que puede ser utilizado para evitar
	// que una operación quede bloqueando el programa.
	// Se configura TModeReg = 0x80 para que el timer inicie automáticamente al finalizar una transmisión.
	mfrc522_writeRegister(TModeReg, 0x80);

	// Se configura el registro TPrescalerReg = 0xFF para una frecuencia de 26 kHz
	// y el valor de Reload en 0x0D = 13. Esto resulta en un timeout = 500 uS.
	mfrc522_writeRegister(TPrescalerReg, 0xFF);
	mfrc522_writeRegister(TReloadRegL, 0x0D);

	mfrc522_writeRegister(TxASKReg, 0x40);		// Configura modulación 100% ASK

	mfrc522_encenderAntena();			// Enciende la antena para transmitir
}

/**
 *	@brief Realiza un reseteo por software
 *		   del MFRC522. Esto se hace enviandole
 *		   el comando SoftReset (sección 10.3.1.10 del manual).s
 */
void mfrc522_reset() {
	mfrc522_writeRegister(CommandReg, SoftReset);
}

/**
 *	@brief Enciende la antena para transmitir
 *		   la portadora de RF.
 */
static void mfrc522_encenderAntena() {
	const uint8_t valor_deseado = TxControlReg_Tx1RFEn | TxControlReg_Tx2RFEn;

	uint8_t valor_registro = mfrc522_readRegister(TxControlReg);
	if (valor_deseado != (valor_registro & valor_deseado))
		mfrc522_writeRegister(TxControlReg, valor_registro | valor_deseado);
}

/**
 *	@brief Detecta si el usuario aproxima una tarjeta
 *		   al lector. En caso afirmativo, envía un comando
 *		   para que la tarjeta le devuelva su UID.
 *	@retval Verdadero si se lee correctamente el UID, o
 *			falso si no se detecta tarjeta o si no se puede
 *			leer el UID.
 */
bool_t mfrc522_leerUIDTarjeta(uint8_t *uid) {
	bool_t estado = false;
	bool_t tarjetaDetectada = mfrc522_detectarTarjeta();//enviar comando tarjeta
	if (tarjetaDetectada)
		return mfrc522_leerUID(uid);		// si tarjeta presente, leer uid

	return estado;
}

/**
 *	@brief Detecta la presencia de una tarjeta
 *		   cerca del lector. Para hacer esto,
 *		   el MFRC522 envía el comando REQA
 *		   (definido en ISO/IEC 14443-3).
 *		   Cuando una tarjeta recibe este comando,
 *		   envía una respuesta al MFRC522.
 *	@retval Verdadero si recibe respuesta de una tarjeta,
 *			o falso si no se recibe respuesta.
 */
static bool_t mfrc522_detectarTarjeta() {
	const uint8_t cmd = CMD_REQA;
	const uint8_t txSize = 7;		//el comando REQA tiene un largo de 7 bits.
	mfrc522_enviarComandoTarjeta(&cmd, 1, txSize);
	bool_t respuestaRecibida = mfrc522_esperarRespuestaTarjeta();
	return respuestaRecibida;

}

/**
 *	@brief Envía los comandos CMD_SEL_CL1 y
 *		   NVB_CL1 para que la tarjeta responda
 *		   con su UID de 4 bytes.
 *	@retval Verdadero si se lee correctamente el UID,
 *			falso si no se puede leer.
 */
static bool_t mfrc522_leerUID(uint8_t *uid) {
	uint8_t cmdBuffer[] = { CMD_SEL_CL1, NVB_CL1 };

	const uint8_t txSize = 0;		//txSize = 0 indica que se transmite 8 bits.
	mfrc522_enviarComandoTarjeta(cmdBuffer, sizeof(cmdBuffer), txSize);

	bool_t respuestaRecibida = mfrc522_esperarRespuestaTarjeta();
	if (!respuestaRecibida)
		return respuestaRecibida;

	uint8_t responseBuffer[UID_SIZE];
	bool_t lecturaUid = mfrc522_leerBufferFIFO(responseBuffer, UID_SIZE);

	if (lecturaUid) {
		for (uint8_t i = 0; i < UID_SIZE; i++) {
			uid[i] = responseBuffer[i];
		}
	}

	return lecturaUid;
}

/**
 *	@brief Envía uno o varios comandos a la tarjeta.
 *		   La cantidad depende del argumento largo.
 */
static void mfrc522_enviarComandoTarjeta(uint8_t *comando, uint8_t largo,
		uint8_t txSize) {
	mfrc522_writeRegister(CommandReg, Idle);

	// Se resetean los bits de interrupciones
	// en el registro ComIrqReg para luego
	// poder detectar si se activa alguno.
	uint8_t valor_ComIrqReg = mfrc522_readRegister(ComIrqReg);
	mfrc522_writeRegister(ComIrqReg, valor_ComIrqReg & (~ComIrqReg_Set1));

	mfrc522_writeRegister(FIFOLevelReg, FIFOLevelReg_FlushBuffer);
	for (uint8_t i = 0; i < largo; i++) {
		mfrc522_writeRegister(FIFODataReg, *(comando + i));
	}

	mfrc522_writeRegister(CommandReg, Transceive);

	// Inicia la transmisión e indica que se transmiten solo txSize bits.
	// txSize = 0 indica que se transmiten 8 bits.
	mfrc522_writeRegister(BitFramingReg, BitFramingReg_StartSend | txSize);
}

/**
 *	@brief Espera a que se reciba una
 *		   respuesta de la tarjeta, o
 *		   que se cumpla el timeout del
 *		   timer interno del MFRC522, lo
 *		   que ocurra primero. Para saber si
 *		   la tarjeta respondió, se consulta
 *		   el bit RxIrq del registro ComIrqReg,
 *		   que se pone en 1 cuando se finaliza de
 *		   recibir datos de la tarjeta.
 *	@retval Verdadero si se recibe una respuesta,
 *			falso si ocurre timeout.
 */
static bool_t mfrc522_esperarRespuestaTarjeta() {
	bool_t respuestaRecibida = false;
	uint16_t MAX_LOOPS = 10000;	//variable para evitar un  loop infinito en caso de que haya problemas con el módulo
	while (!respuestaRecibida) {
		uint8_t irqReg = mfrc522_readRegister(ComIrqReg);
		if (irqReg & ComIrqReg_TimerIrq)		//ver si ocurre un timeout
			break;
		if (irqReg & ComIrqReg_RxIrq)	//si se recibe una respuesta
			respuestaRecibida = true;
		if (--MAX_LOOPS == 0)//si se cumple la cantidad máxima de loops, terminar.
			break;
	}
	return respuestaRecibida;
}

/**
 *	@brief Lee el buffer FIFO interno del MFRC522.
 *		   Este se utiliza para intercambiar comandos
 *		   y datos desde el MFRC522 a la tarjeta, y viceversa.
 *		   Los datos leídos se guardan en la memoria indicada
 *		   por el argumento response.
 *	@retval Verdadero si se leen datos del buffer, falso si no
 *			hay datos para leer.
 */
static bool_t mfrc522_leerBufferFIFO(uint8_t *response, uint8_t max_bytes) {
	uint8_t n = mfrc522_readRegister(FIFOLevelReg);	//consulta cuantos bytes hay disponibles para leer
	if (n == 0)
		return false;
	if (n > max_bytes)
		n = max_bytes;					//limito el máximo de bytes que se  leen
	for (uint8_t i = 0; i < n; i++) {
		response[i] = mfrc522_readRegister(FIFODataReg);
	}
	return true;
}

/**
 *	@brief Escribe el valor data en el
 *		   registro reg. Agrega el
 *		   desplazamiento de un bit a la
 *		   izquierda y la mascara de escritura
 *		   según indica la sección 8.1.2.3 del manual.
 *		   Utiliza la función spiWrite del módulo API_mfrc522_port.
 */
static void mfrc522_writeRegister(registros_MFRC522_enum reg, uint8_t data) {
	uint8_t reg_addr = WRITE_MASK | reg << 1;
	spiWrite(reg_addr, &data, 1);
}

/**
 *	@brief Lee un byte del registro reg.
 *		   Agrega el desplazamiento de un bit a la
 *		   izquierda y la mascara de lectura
 *		   según indica la sección 8.1.2.3 del manual.
 *		   Utiliza la función spiRead del módulo API_mfrc522_port.
 *	@retval Devuelve el valor leido del registro.
 */
static uint8_t mfrc522_readRegister(registros_MFRC522_enum reg) {
	uint8_t rxBuffer = 0;
	uint8_t reg_addr = READ_MASK | reg << 1;
	spiRead(reg_addr, &rxBuffer, 1);
	return rxBuffer;
}
