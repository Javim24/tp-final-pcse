/**
* @mainpage Página principal del trabajo
* @section intro Introducción
*	Este trabajo final para la asignatura Protocolos de Comunicación de Sistemas embebidos consiste en la creación de un driver para el control de un módulo lector de RFID MFRC522 y un driver para la escritura de datos en una pantalla LCD.
* Estos drivers están desarrollados de manera que permiten abstraerse de el hardware en el que se utilizen. Para ello se divide cada driver en una implementación genérica de alto nivel, y una implementación de funciones de acceso al hardware. Esto con el objetivo de que se pueda trasladar fácilmente a otras plataformas.
*
* La utilización de estos drivers en un programa se encuentra en el repositorio del trabajo final de la materia Programación de Microcontroladores: https://github.com/Javim24/PdM_workspace/tree/main/CONTROL_ACCESO

Este trabajo consiste en el desarrollo de un sistema de control de acceso que utiliza tarjetas RFID para permitir o no el acceso de un usuario.
*
*
* @section info Información
* @subsection mfrc522 Módulo MFRC522
*	El MFRC522 es un módulo lector de tags RFID que funciona en 13.56 MHz. Soporta la comunicación con tarjetas que se basan en el protocolo ISO/IEC 14443.
*
*El driver desarrollado está conformado por los archivos API_mfrc522.h, API_mfrc522.c y sus correspondientes implementaciones para acceso al hardware API_mfrc522_port.h y API_mfrc522_port.c.
*
* El driver permite el acceso a una interfaz simple con funciones que permiten inicializar el módulo y leer el UID de una tarjeta. El resto de funciones necesarias para el correcto manejo del módulo están declaradas como static en el archivo API_mfrc522.c.
*
*
*
* @subsection display_lcd Display LCD
Se desarrolló un driver para un display LCD de 16x2 con adaptador para comunicación por I2C. 

El driver está compuesto por los archivos API_lcd.h, API_lcd.c y la implementación para acceder al hardware API_lcd_port.h y API_lcd_port.c. La implementación pública en API_lcd.h permite el acceso a funciones para la inicialización, borrado de pantalla, escritura de un caracter, escritura de un texto y configuración del cursor.
*
*
*
* @section autor Autor
*	Ing. Javier Mosconi

*	Contacto: jfmosconi@gmail.com

*	Carrera de Especialización en Sistemas Embebidos - FIUBA

*	Diciembre 2023
*

*/
