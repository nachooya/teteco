/*! \mainpage My Personal Index Page
 *
 * \section intro_sec Introduction
 *
 * libteteco library provides a communication stack for simplex VoIP communications. 
 * It's intended for very asimetric Internet links where a duplex
 * VoIP communication could cause problems flooding the upload link.
 * Feedback is provided by a bidirectional chat system.
 * Additionally libteteco provides file transfer functionality with bandwidth control
 * management.
 * libteteco also takes care of sound device management.
 *
 * \section install_sec Compilation
 *
 * \subsection step1 Step 1: Check dependencies
 *
 * * libevent 2.0.10-stable
 * ./configure --disable-openssl --disable-malloc-replacement
 * make
 */


/**
 * @file    teteco.h
 * @Author  Ignacio Martín Oya (nachooya@gmail.com)
 * @version 1.0
 * @date    October, 2010
 * @brief   Public libteteco API
 * @section LICENSE
 *
 *  Copyright (C) 2010  Ignacio Martin Oya
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * Public libteteco API
 *
 * This files exposes the API to use for software using it.
 */

/** @defgroup libteteco libteteco: Simplex VoIP, chat and bandwidth managed file transfer.
 * 
 * This library provides a communication stack for simplex VoIP communications. 
 * It's intended for very asimetric Internet links where a duplex
 * VoIP communication could cause problems flooding the upload link. 
 * Additionally libteteco provides file transfer functionality with bandwidth control
 * management.
 * libteteco also takes care of sound device management.
 *  @{
*/

#ifndef __TETECO_H__
#define __TETECO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \enum    teteco_speex_band_t
 * \brief   Represent the 3 different Speex Modes
**/
typedef enum {
    TETECO_SPEEX_WB = 0,///< Speex WideBand
    TETECO_SPEEX_NB,    ///< Speex NarrowBand
    TETECO_SPEEX_UWB    ///< Speex UltraWideBand
} teteco_speex_band_t;

/**
 * \enum    log_levet_t
 * \brief   Level of log messages
**/
typedef enum {
    LOG_DEBUG = 0 ///< Display all logmessages
} log_levet_t;

/**
 * \enum    teteco_status_t
 * \brief   Describes the status of the teteco_t struct
**/
typedef enum {
    TETECO_STATUS_DISCONNECTED = 0, ///< Disconnected
    TETECO_STATUS_CONNECTING,       ///< Trying to connect (only in client mode)
    TETECO_STATUS_WAITING,          ///< Waiting for a connection (only in server mode)
    TETECO_STATUS_CONNECTED,        ///< Connected
    TETECO_STATUS_TIMEDOUT          ///< When no communication from peer host has been received for a long time
} teteco_status_t;

/**
 * \enum    teteco_audio_mode_t
 * \brief   Describes if the client is going to receive or send audio/files
**/
typedef enum {
    TETECO_AUDIO_RECEIVER = 0, ///< Receiver mode
    TETECO_AUDIO_SENDER,       ///< Sender mode
    TETECO_AUDIO_UNDEFINED     ///< Undefined mode
} teteco_audio_mode_t;

/**
 * \enum    teteco_net_mode_t
 * \brief   Defines how the network mode
**/
typedef enum {
    TETECO_NET_SERVER = 0, /// Server mode: will wait for incoming connections
    TETECO_NET_CLIENT      /// Client mode: will try to connect to a server
} teteco_net_mode_t;

/**
 * \enum    teteco_file_transfer_status_t
 * \brief   Describes the file transfer status
**/
typedef enum {
    TETECO_FILE_TRANSFER_SENDING = 0, /// Is sending a file
    TETECO_FILE_TRANSFER_RECEIVING,   /// Is receiving a file
    TETECO_FILE_TRANSFER_END          /// File transfer has finished (because all data has been tranferred or an error occurred).
} teteco_file_transfer_status_t;

/**
 * \struct    teteco_t
 * \brief   Private variable which holds all stack status
**/
typedef struct{} teteco_t;


/**
 * \typedef log_callback_ft
 * \brief   Callback definition to print log messages.
 * The argument is the message to print.
**/
typedef void(*log_callback_ft)           (char*);
/**
 * \typedef chat_callback_ft
 * \brief   Callback definition for receiving chat messages. 
 * The argument is the received chat message.
**/
typedef void(*chat_callback_ft)          (char*);
/**
 * \typedef status_callback_ft
 * \brief   Callback definition to notifty status changes.
**/
typedef void(*status_callback_ft)        (teteco_status_t);
/**
 * \typedef file_transfer_callback_ft
 * \brief   Callback definition to notify file transfer events. 
 * First argument is filename, sencond file transfer status. Third total size of file in bytes. 
 * Fourth the number of bytes sent/received.
**/
typedef void(*file_transfer_callback_ft) (const char*, teteco_file_transfer_status_t, uint32_t, uint32_t);

/**
 * Initialize libteteco
 * 
 * Must be called before using any of the other libteteco functions
 * @pre none
 * @post  library is ready to use
 * @return 1 if initialization was ok. 0 in other case
 */
int         teteco_init        (void);
/**
 * Deinitialize libteteco
 * 
 * Should be called when finishing of using the library to free resources.
 * @pre library is initialized
 * @post  resources has been freeded
 * @return 1 if deinitialization was ok. 0 in other case
 */
int         teteco_end         (void);
/**
 * Sends a chat message
 * 
 * Used to send chat messages to the other peer
 * @pre teteco struct is initialized and status is connected
 * @post  the message is sent
 * @param teteco A initialized teteco_t struct
 * @param comment A \0 terminated string
 * @return 1 if message was sent. 0 in other case
 */
int         teteco_chat_send   (teteco_t* teteco, const char* comment);
/**
 * Starts sending a file
 * 
 * Used to send a file to the other peer
 * @pre teteco struct is initialized and status is connected
 * @post  file transfer starts
 * @param teteco A initialized teteco_t struct
 * @param file_path The complete file path of the file to sent
 * @return 1 if file transfer started. 0 in other case
 */
int         teteco_file_send   (teteco_t* teteco, const char* file_path);
/**
 * Sets the log callback
 * 
 * Used to set the function to call to print log messages
 * @pre none
 * @post  the log callback is set
 * @param log_callback_ref the callback function
 * @return 1 callback was set. 0 in other case
 */
int         teteco_set_log_callback           (log_callback_ft           log_callback_ref);
/**
 * Sets the chat callback
 * 
 * Used to set the function to call when a chat message is received
 * @pre none
 * @post  the chat callback is set
 * @param chat_callback_ref the callback function
 * @return 1 callback was set. 0 in other case
 */
int         teteco_set_chat_callback          (chat_callback_ft          chat_callback_ref);
/**
 * Sets the status callback
 * 
 * Used to set the function to call for notify status changes
 * @pre none
 * @post  the status callback is set
 * @param status_callback_ref the callback function
 * @return 1 callback was set. 0 in other case
 */
int         teteco_set_status_callback        (status_callback_ft        status_callback_ref);
/**
 * Sets the file_transfer callback
 * 
 * Used to set the function to call for notify about file transfer status
 * @pre none
 * @post  the status callback is set
 * @param file_transfer_callback_ref the callback function
 * @return 1 callback was set. 0 in other case
 */
int         teteco_set_file_transfer_callback (file_transfer_callback_ft file_transfer_callback_ref);

/**
 * Initilize a teteco_t strcut to be used for communication
 * 
 * This is the first function to be called when starting a communication.
 * Its parameters will define how the stack will behabe
 * @pre library is initialized
 * @post  Audio, codec, stack and net is ready to starts a session
 * @param teteco_net_mode_t Defines is the client will connect or wait for a connection
 * @param local_port Defines the local UDP port (1-65536) to be used for communication. Set 0 for any.
 * @param remote_port Defines the UDP port to connect. Only used in client mode. Set 0 in server mode.
 * @param remote_address Defines the full qualified domain name or IP to connect. Only in client mode. Set to NULL in server mode.
 * @param audio_device_in Defines the index of audio device to use for sound recording. Only used in SENDER mode.
 * @param audio_device_out Defines the index of audio device to use for sound playing. Only used in RECEIVER mode.
 * @param audio_mode Defines if it will act as audio SENDER o RECEIVER.
 * @param enc_speex_band Defines the SPEEX mode to use (NB, WB or UWB).
 * @param enc_quality Defines the SPEEX quality to use (from 0 to 10);
 * @param user_directory Defines where received files will be saved.
 * @return A initialized teteco_t struct is all was properly initialized. NULL in an error occurred.
 */
teteco_t* teteco_start        (teteco_net_mode_t    client_or_server,
                               uint16_t             local_port,
                               uint16_t             remote_port,
                               const char*          remote_address,
                               int                  audio_device_in,
                               int                  audio_device_out,
                               teteco_audio_mode_t  audio_mode,
                               teteco_speex_band_t  enc_speex_band,
                               int                  enc_quality,
                               const char*          user_directory);

/**
 * Stop a communication session
 * 
 * Notifies other peer to stop communication and free teteco_t struct resources
 * @pre teteco struct is initialized
 * @post  communication is finished and teteco_t struct is freeded
 * @param teteco A initialized teteco_t struct
 * @return NULL in any case.
 */
teteco_t* teteco_stop         (teteco_t* teteco);

/**
 * Return a list of the available audio recording devices
 * 
 * This function should be called to obtain a list of all available
 * recording devices and their index. Default device is always returned
 * in first position
 * @pre library is initialized
 * @post  index and devices params are populated. Those should be freeded by the caller.
 * @param index Where indexes are returned. Pass a &int* variable pointing to NULL;
 * @param device Where device names are returned. Pass a &char** pointing to NULL;
 * @return The number of devices (whichs maps with the arrays length returned in parameter)
 */
int teteco_get_in_devices      (int** index, char*** devices);
/**
 * Return a list of the available audio playing devices
 * 
 * This function should be called to obtain a list of all available
 * playing devices and their index. Default device is always returned
 * in first position
 * @pre library is initialized
 * @post  index and devices params are populated. Those should be freeded by the caller.
 * @param index Where indexes are returned. Pass a &int* variable pointing to NULL;
 * @param device Where device names are returned. Pass a &char** pointing to NULL;
 * @return The number of devices (whichs maps with the arrays length returned in parameter)
 */
int teteco_get_out_devices     (int** index, char*** devices);

/**
 * Sets the maximum tranferring rate for file transmission
 * 
 * @pre teteco struct is initialized
 * @post  maximum tranferring rate is set
 * @param teteco A initialized teteco_t struct
 * @param transfer_rate the maximum transfer rate in KB/s. 0 means no limit.
 */
void            teteco_set_max_transfer_rate (teteco_t* teteco, uint32_t transfer_rate);
/**
 * Gets the remote IP peer address as string "x.x.x.x:port"
 * 
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 * @return a null terminated string with the IP address and port ("x.x.x.x:port")
  */
char*           teteco_get_remote_address (teteco_t* teteco);
/**
 * Gets the pseudo dB of the sound emitted/received
 * 
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 * @return the pseudo dB of sound being emitted/received
 */
float           teteco_get_current_db (teteco_t* teteco);
/**
 * Gets the total bytes send in the audio channel
 * 
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 * @return the number of bytes send in the audio channel
 */
uint32_t        teteco_get_total_bytes_out (teteco_t* teteco);
/**
 * Gets the total bytes received in the audio channel
 * 
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 * @return the number of bytes received in the audio channel
 */
uint32_t        teteco_get_total_bytes_in (teteco_t* teteco);
/**
 * Gets the timestamp when connection was stablished
 * 
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 * @return the timestamp when connection was stablished
 */
uint32_t        teteco_get_time_start (teteco_t* teteco);
/**
 * Gets the status of the connection
 * 
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 * @return the status of the connection
 */
teteco_status_t teteco_get_status (teteco_t* teteco);
/**
 * Gets the number of received audio packet which were expected to receive 
 * 
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 * @return the number of received audio packet which were expected to receive 
 */
uint32_t        teteco_get_packets_expected (teteco_t* teteco);
/**
 * Gets the number of received audio packet actually received 
 * 
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 * @return the number of received audio packet actually received
 */
uint32_t        teteco_get_packets_received (teteco_t* teteco);
/**
 * Gets the real transfer rate of the file which is being sent/received
 * 
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 * @return the real transfer rate of the file which is being sent/received
 */
uint32_t        teteco_get_transfer_rate    (teteco_t* teteco);
/**
 * Prints the internal teteco_t struct status
 * 
 * For developing pupouses
 * @pre teteco struct is initialized
 * @post  none
 * @param teteco A initialized teteco_t struct
 */
void            teteco_print_conf (teteco_t* teteco);

#ifdef __cplusplus
}
#endif


#endif