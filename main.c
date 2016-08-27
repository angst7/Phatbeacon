/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup experimental_ble_sdk_app_eddystone_main main.c
 * @{
 * @ingroup experimental_ble_sdk_app_eddystone
 * @brief Eddystone Beacon UID Transmitter sample application main file.
 *
 * This file contains the source code for an Eddystone beacon transmitter sample application.
 */

#include <stdbool.h>
#include <stdint.h>
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#include "ble_fat.h"
#include "SEGGER_RTT.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                 /**< Include the service changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

#define CENTRAL_LINK_COUNT              0                                 /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                 /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                 /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables the time-out. */
#define APP_CFG_CONNECTABLE_ADV_TIMEOUT         60  
#define NON_CONNECTABLE_ADV_INTERVAL            MSEC_TO_UNITS(100, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100 ms and 10.24 s). */
#define APP_CFG_CONNECTABLE_ADV_INTERVAL_MS     100     

#define MIN_CONN_INTERVAL                       MSEC_TO_UNITS(50, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL                       MSEC_TO_UNITS(90, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                           0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                        MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY          APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY           APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT            3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEVICE_NAME                     "PhatBeacon"

// Eddystone common data
#define APP_EDDYSTONE_UUID              0xFEAA                            /**< UUID for Eddystone beacons according to specification. */
#define APP_EDDYSTONE_RSSI              0xEE                              /**< 0xEE = -18 dB is the approximate signal strength at 0 m. */
#define APP_FATBEACON_URI               0x0E                              /** 0x0E is the URL scheme for Fatbeacon */

// Eddystone URL data
#define APP_EDDYSTONE_URL_FRAME_TYPE    0x10                              /**< URL Frame type is fixed at 0x10. */

#define APP_EDDYSTONE_URL_URL           'E', 'd', 'd', 'y', \
                                        's', 't', 'o', 'n', \
                                        'e', ' ', 'L', 'i', \
                                        'g', 'h', 't'


#define STATIC_BORING_WEBPAGE '<', 'h', 't', 'm', 'l', '>', '<', 'h', 'e', 'a', 'd', '>', \
                              '<', 't', 'i', 't', 'l', 'e', '>', 'F', 'a', 't', 'b', 'e', \
                              'a', 'c', 'o', 'n', '<', '/', 't', 'i', 't', 'l', 'e', '>', \
                              '<', '/', 'h', 'e', 'a', 'd', '>', '<', 'b', 'o', 'd', 'y', \
                              '>', '<', 'h', '1', '>', 'I', 't', ' ', 'w', 'o', 'r', 'k', \
                              'e', 'd', '!', '<', '/', 'h', '1', '>', '<', '/', 'b', 'o', \
                              'd', 'y', '>', '<', '/', 'h', 't', 'm', 'l', '>'

#define STATIC_PAGE           "<html><head><title>The Eddystone Lighthouse</title></head><body>"\
                              "<h1>The Eddystone Lighthouse (from Wikipedia)</h1>"\
                              "<p>The Eddystone Lighthouse is on the dangerous Eddystone Rocks"\
                              ", 9 statute miles (14 km) south of Rame Head, England, United"\
                              " Kingdom. While Rame Head is in Cornwall, the rocks are in"\
                              " Devon and composed of Precambrian gneiss.</p>"\
                              "<img alt=\"Winstanley's lighthouse\" align=\"left\" src=\"data:image/jpeg;base64,"\
                              "/9j/4AAQSkZJRgABAQEASABIAAD/2wBDAFA3PEY8MlBGQUZaVVBfeMiCeG5uePWvuZHI"\
                              "////////////////////////////////////////////////////wgALCAGbASwBAREA"\
                              "/8QAFwABAQEBAAAAAAAAAAAAAAAAAAECA//aAAgBAQAAAAHoKAAAAAIUAAAAAIUABKAAA"\
                              "igAMtAAAIoABnQAACLFABjYAACKAAY1QAAIoACc9bAAASgAM897AAAigAOWXcAACKACOV"\
                              "augAAAAJnOdNbAABKABMc970AAAATOxjGmd7AAAAzl0HKU3oAAAJjNdRzzNN6AAADEzbu"\
                              "nKUvQAAJQwxXSscdk6aUAASjmm1rjlVXWwAADGqzbODpCdLQAACVM5TLcpdStAAAEHPm1"\
                              "OjOpd52AAABOEutsa6SygAlABnlNkb3nWdAAlABjkaF651nQAIoAY43VsnTWN50ACLKAY"\
                              "43W4jrnQACFAGOTpbMOwCBSAoi44t7ljoCAUgS0kODfSKqWyUCokgKOLrnUjSULaFjMFJ"\
                              "U5zouLnpYqGqAzEtErk3NZTpUQaUJZCWks4t24a2hm2qCAipNOU3bKrN0kzsogzWdpF5T"\
                              "ewrGei889iiEzdEkt5TXVirZNRk2oiMtUWOC62ubEtc5rdohMLpLTg1dXGbq0RdBCZznX"\
                              "RM7vPnbvFiddQFKRCJTG3FdWMxvdllKBkTUF4lqyE6WKW4u4SZDTkVdyXOWtCW3nOm4Ji"\
                              "Q052UN1mVdRNcmt7gmM2S5BdaNIyiNVbqBM5yJYqWqhK1dVUKSYZEs1KuWs2rLtRE0CZz"\
                              "NSqZilsmtUETQEqJnO6y1hupaCE0ABkqVJbQCE0AAAAAITQAAAAAiP/8QAIhAAAgEEAgM"\
                              "BAQEAAAAAAAAAAREAECAxQDBBAiFQYBIy/9oACAEBAAEFAvzr/ZHPxT8UzsfENB8Qz3H8"\
                              "I2gzvfMLgzOp38EZnQ3Tiwmwzxxsuht8oJ6nqgxsE8HlOoKeONc2G3yyDHHQY3yVd40es"\
                              "aKKHFexnsVwT7g2jihtUUUG2auxxxxwGOOOf1sqYoITRKAQ59WHaNTnEcEOU7DtHgFO4d"\
                              "rysUUVBmdw7R4BiOh2jigoqNTxNFuHFBXowf63TigszAN1w18bRV2PaEM6/qP4Qp0Phin"\
                              "Q+GNw8QqoOLvm7sENRwO0zvRUUVgj4FYZ1yd8XXjDaxRx1NO+R8XXjOxigiit9RaJmKk1"\
                              "ExcNZ2k2P1cDrOx8KsHCTBoqqsNRw5oNZ2iHhE9azvxHUUUUUWw471FFyqLkW6qKKKiis"\
                              "V62HYohUx7qiiuUX5z/xAAbEAACAQUAAAAAAAAAAAAAAAAxcIAAAUFgkP/aAAgBAQAGPw"\
                              "KRuF7dMFfnj+d/NGDH/8QAKhAAAgEDAwQBBQADAQAAAAAAAAERECExQVFhIEBxgTBQkaH"\
                              "B8GCx0eH/2gAIAQEAAT8h/wAddyX0XUWn0bDX0aX0cTlfRMDR9FvBDka/0R7wSE30M3CF"\
                              "d2/JL3F9jQHh9AlocFujqZTSku0t3jcdLwf6Kse8knd0xWFkv/If9Ya1hO6cDgR22FjoyEV2"\
                              "yOZDYkgx9w2kTWQhMvKncWOjQKlr9Uw9xcMnQRoF0MIJ/wCENvwQ/key1Nu4eaLU0NjWv"\
                              "vDzyfoVzNGtBMWfcqaZgQzGtXcYDcZMn8Ds7aUGkjPHbNCVcgxYMHRZRKGuSD1/B6DYxI"\
                              "jHbResoaVY9sT5Y901fhE04gk4sWbi5fcVLsRILwR2Icicqe1hbELZELZDR6GfA8UXmhK"\
                              "xiENXEgRyIngiHuQ+EIrCx3KSMWTOIOSXkwoKS+CIwa94aEZIR6JZL2H/AElmPeO+GtFN"\
                              "BS9aFmv4ExDaH75YmtFjBrhmuDTAompuosrus1UPbLtS8ZGwZMUj3tzT9yafoKYvoM7/A"\
                              "CRwO3BeNSORrdrJPDJ6JJowIq9jLx/6KTEklbkBOaSiBKJRPwySz3T2eyCKodHTK19KNk"\
                              "E9hPp9kfK63L9SGbmh+4/0afAksnPApYoVZv0r4n1eqwQPDpofuStyZGYssQqWq3FPZ7F"\
                              "8Uf0n2pZdFi1Hg1NB4IonBPH3Epc0tsMR9hrkSL/LYsS0EyaN3Fmjwbmh+48mlhLAiTsL"\
                              "WrFSCBu4nAn+j45E2MQjlkDJuWmFX8Erf8jdhMuLLq7hZFHBCpqFKFz7Erf4X/oTueheC"\
                              "5ccqFdI8ajNhociVjEX0E6tFhjQnzRoYmJul8boRPTA8DNTUIaBYFkWtEEogRpKHliYr+"\
                              "AviY3uKVcCzYv/ACo8DyaiyNQ3ZQnWYGxJ1IEd2JQM0LDTEBKKR13HI0R5EqWMBPUxU0"\
                              "qtceVrMTcl8XI3pgaVSaySeqx8TSMYom2aRwJRv9yBVRDQZUaojQc6jroMVydqeqey3Nf"\
                              "ZMddjB96cBMcsmA0SyNy6eBIhGYh3MjogsPNixDpqSujGSHA3U8E3pgzYTJ5Ysjd6RRHI"\
                              "k3qINxaEJbkxiBmQai0yOVgeBI1pMZZMiJdhF1SNognyTBYUPcmkdHokTYmjAnwOPdZbG"\
                              "SrrclCNBkjwyW6mR5PwWG1Rv4E41JPAyQQJRqiwidw7Mj2DgcGXBEkCIkIdccHgZC0PVP"\
                              "ZBHKqp6PwS0ckeBfc9l96wQ3oXAx8UED4HkyPNPRBBYkyQIjZkhkUsIidyPI7FxSKdfnY"\
                              "2X0JCVjyo8COD7iA/A76liBCjBCIo3eEJMQjtnHgaIbZffoH2pkLce3SCO5iQ6UCHKiP8"\
                              "b//aAAgBAQAAABBw/wDoAAOD/wDgEA4C86wYMG/feCDAP3/8Y2f+83uML/P/AP8AMB/a+"\
                              "TBAV+AAwwP/AIIBzC/cge2Afy4AAEDfYAAAAHu1DuAE8P8A/sDbtd/zAd9//wDuAP0H/w"\
                              "D0C5Rf/wDAiPg//wADPe8f/AHeVX/wAZiJ/wDgBXf3+oAAnM+LABkxcA0AagAAOAFYAPj"\
                              "8NGQP4FAiCyPBP6Crx47Zy+LPN/vGWL78XWZ0b00EGM2wFxJXlJz1PFpC4i7TC0kMtqJQ"\
                              "RaWkkjEYrlytPdir+N0mRhJK1I3aflgKCmYAtqssosMqYFldLKFAFaWc5QEuNrQ0AA+OO"\
                              "NAAfAACQAAYAAv/xAArEAEAAgEDAgQGAwEBAAAAAAABABEhMUFRYXGBkaHwECCxwdHhME"\
                              "DxUGD/2gAIAQEAAT8Q/wDOWSgmb/p1K/o8C9dpqsefj/DX9xB1CIUgL4P+NoR9dP47/nP"\
                              "41ouLdoxcqXz/AOIvVKtN6uu9wJYn/A3+ShdxVo09tIlLrN6e+fr/AMRmrLU6tQICLm9Y"\
                              "aZ/4Kwdz6y4eCXXC0ti4+kkrCrdq+00nrsRer/wAg1IjEt+GczWXhgGufZDCrWmZAYZYW"\
                              "SwB1/uALXB8upDj3T979Jz6P2gpHb6f3GuheczJtw6fK6wwkbdDTo/aZHXi35ljWq9uYl"\
                              "aVmMWXXb+y4iW9db7QaDdaxWW43isPJ8gxb2iy7hlt/Z+pWdYTat+0cjvH/YNUYnA3Y3S"\
                              "sRGwKzek3DZUpo2x8mvs/EMTA9tPEmihtFAlf2Rka6ZhwVvDOhiPjmaqczCw2f38g0yFG"\
                              "YRysXf8AaFfv+sxNAG9UllYDr9/ekajd6P8ARv50CHeKaeUGYuurVgNJ2dYavP41KNUs5"\
                              "Z1MDNCYVFZ2x9yatKHEpqtzS4DeQ6AaMRdBbx/WK6cxCx84gBUzJaUfX4JwPL4LHYvNzL"\
                              "uXFujdj8r7ktRWwfGVSmqDoaQgBfW9JW+V7t9v61y9oJqW/H00uFm0Vh5IqbpLRaNqicV"\
                              "VfE2PUVMUs77XNYBgbTaa0vWZFiaR2h1H36x1DVbG8ESzR/qtg7b/ABSXXHSWpbyitPc1j"\
                              "7UBvUvESu608JRVuYZTGL1iRdE0tz9ZQU28f7LA0LVi/mUFY0CpMlGTsKgAsZy4uIG8uq"\
                              "ZWpEmren5mSqgiGj/VUbQvUn+ZP8yBUjyiFKC5QRvfMRuZI1DTeW6LVqoZF6pfmS3DJW/"\
                              "QhhXP2iFqrN4hZY066sGKCOHHMs1OwXOcdfowAAwH9kTTDT76Slu0pqROi2sv095lgaCl"\
                              "t9YsqV007HhD6/tEazV0gBQA6RcffPw0Hf7MNP46/myVXKta56Q3MG8tMFjZRhwej+Y02"\
                              "LyfzEjV77JeStN7z9patlCX1+Dp8Pv8NJ3/ALWTuYIJe2sVofWcUd7gt/QxVKD4oC0S+t"\
                              "k0S0TXDKq5tw40+F2DoP1+Ggm3y38l/C/5qumOrFStUJnYTb157Rt0BwD+Jvb6aU/icfE"\
                              "pX2llMedk6W7Fo0mbgFGlwbB5gKfaDZf9ky6A3G84NazmYxHN1iuJSOfPPxG4VJteYLIr"\
                              "JsaRmrvuRReDMQZ364uBRWx/WPl1ImmzSOi9ppPbSJXCdbPfDLFK2YbM3+EuXFY/MHqf3"\
                              "slFvHWK3X2lSqa4VxMt/pAiy8ukwACKraO99EEbqu3y38l/yINZ0wDvjv8AIg6svvUAsR"\
                              "atZloPlEbgcMtr8v1HZhXs/qZBr1fiDqPH9RFduDpEGqE6TznX9IGh+CWrO55S7eu+IJo"\
                              "jKXVn8FxyxnxjsJXb9yl1ylU/eXxAPOW4HxlTTHSVVwBD1gz9pam/K4YdWuJkHad+79Iw"\
                              "VRhoxMXosteDGJVoC8tzTNtLICZsSdMeUpvXDPBvaN8k7wAYiXp6wHfFwPGA1vAefkr5Ar"\
                              "i4FdPGX7JktVW7DhL6MymWprjEzzHQcRdZStZRFYRuO7fpDaa4E9jtAtTmGRCst+sGhR0"\
                              "IQAtzK1smia3NxKe8G3IxLlcRebJqMeEDvCB38/mYcVQzQzbHjPSpnrAZ4X4yz8rl9Hzm"\
                              "rpF3hqHWwbzpM6WVD6Zp8f0iiNBMN/5KJWmg+E0e795hfo39Y7WhNvWCjJKOHE1aF9pjU"\
                              "omeSMDTPSV1bZVYYDu7xNBiZraHX52NtvVK6Qmc/eWlV53CuCY4JjgmrND3lIusd6gHth"\
                              "bgRW2vlC+WYwrNXjFxtmlorRlulIBxCo3pf3lBU1bxxINggBt6fqHAeUqjmkff1iKTWne"\
                              "Ne37luQHvvFChU99ZXt/sybX77y3ipnmeMPnolNcabwECXTANA4l2v1lNvrL93AMnTMZg"\
                              "PvEHXvXvMWcd0XCaYpa3fpCqYXdY9kSnO+8rFMH4gLwvPHV6QA29P1KHn1iB8cx2lON6l"\
                              "dWUOfOU1t84FC8S9KscWZ0jxvzQO/vxhnk+de/lLv8AyKKdkMTKsD1jUPXEagM9eJX9kq"\
                              "Zt82avKHiDEHO/1it1UWh69IukIu0aOWopmnhIUDPZXZmNLvwZqKuZBXnnmFhp78o43li"\
                              "z4hcTqagemXSoA5DxqeNSyrPWMC8yunz/AFBD9pRsg3kz81+MV4amg6PxDH3ffnLs157T"\
                              "DReVS+FRS7sdWtM03d+H6hFi44zHlnxH6nOLdJdK6H7MQA3j07wqLyfaar2X4wFdvbL6/"\
                              "SYdR9IKWX5QozXlANFI+w/UtrF+X6iMmFhDA0/EGL0qoCGuDpBXf0+d12i+/ZKpoXn6Sq"\
                              "pti42t8dILn0lvMWt30gXTHMtzUNPN4j1lYdoF90ma+p94qRy/aVvXH2nJBnu56stq6uK"\
                              "3rokRL/X4h23v+4hlF7f7LUstEv3mImRglQ1VQAy+eJUM2DFRNfuEb2fl3icHrKePWIVN"\
                              "psYNac/7CTZq3DXFe/CZ9v6iFBMXcRa1AGvjL5HmhfsmSzt8J0TS7tfWGvEQqOPxNXt9i"\
                              "Ktwu0V0WAdS7mPvB930mCjzCGWlfGaOlm/WAUu9Os78dYhqTpX77QsQC+WU1mUmD4Pw3j"\
                              "e0XH8fuatVfapY23e81UPk/cx4p66VBLyeF3950V6TeiooSohrB6THuo+nTSa6wckptrT"\
                              "b2jbBdyCsazQj1VOXkIpoGem0u2LEjgueYtm5NGWNVqwe/hKaWlbTxL3hn8v8gJsec13S"\
                              "AHV7yx0+VUlvhHJSX4RPCJM69yULCdWO8zpi84iNKt9mN3V4zpHaAUzdRmuk6vpMgKXuE"\
                              "wlbTXmC"\
                              "0DdbxC8LdolcuZppLxW0INDOsQLz9YghXklhu8pa6YOhEu8pg0qesPTvLQ48VhVbnpEYX"\
                              "njWDZ8mD/Jl0X0lcDxCKDLXYg27PvDBlY7LX4zBV6dZoLrszCCjuS0NuAjJeYJpMNZXL"\
                              "KVesabGntcq3IN1BKNVZpfhEGDK7vv39bJenVgcXrWkeqogWo2l7XD2iKtfWXeZWVXKn"\
                              "mWJYectqmb3jyYoN3FEDC19ZfWmfP3p8qY38IEduesaZYlbuTTMvRmGro40jY6ab1DinT"\
                              "/SJdT2ZmttMUmXEKA3r0nhVyQzQe/fpArW3ghKiPAQbqhxLpPd5xDnWCUtq4loE78wFty"\
                              "mYhvlYOnSoaAX4RNDGmYF1KGmqQc9en+xAwEUi3XBM0HYggx8K+RpHraj5zAuyne5lvAMg"\
                              "qWKztG2LPWVrt1mHN+kaaMEPzFKdV5mWintLvqnRfnAJ63MBu35gNATk0lDQG7OY6UZSNB"\
                              "R5e/fpALVU8RaA0Fy9G9fWGDU84qoRiqiA9NyWWpdNxpFbxFWBrAufNhdtB7/ADcmiO4"\
                              "V4lSqaq3KzB1H372iXSq391LNjxqYKPpUvGsF0Jqy+kIukxi0/EEYgwLfAz799Y6gsc6y"\
                              "15HhgK0hlj6wLXZ5xVi54x4y7QganipSBat8Rdd+Myb+EpVBBrcRqFNotqBzmFBrc4bAN"\
                              "S/gy/i9fzE8PCImgOSZPTrt5R3HnEDXLvBoya+BEFdD5yy1C9wp0ldoLUYYibHeoNTLTo"\
                              "bShMKupLVt+OkLZB8WK4eOZac0OkLHGEtYtEzoXKNME8jx+4a1K+kB1fCBTYDiY0J4wR1"\
                              "qH8DbeLHBjo1ORHeP4VXFji3WkiJVhfV/c11PhKcev6ntj9Shrd8w4B45l8KOkaOp31lC"\
                              "8j2H8TNWjwSCc/W4jovJMnGYGlWcrFs1dbZmTYedMwZp0r8St6I7Qhwl8Mz0KOhDrPU/E"\
                              "G4eBKNGHRuU/wAnYedToHm/iXub7ZiuWwgYN90mOQe/Gbbu+CIoaHvr+JQMeQ/UBd14j9"\
                              "TpZ6FR00C+buWpdumkDVv10hReOnWNQp3zM2AfGArQHnDAQ2Ob0j5NE8V10mbL9phoErH"\
                              "9Cjg+KKYae0BDKvaClPijGG+1/mfdkFe+PKF1mn1mTt5SzSvOFGA80pp199qjQF4DiXdR"\
                              "uNQb2PL8Qxy9Yhbldg8SV0l/wX/NUU0xAoU4TqTPBM8HnEbujPWeDygqymArNYeCoGB/I"\
                              "/1a/wCM/wA7H4//2Q==\"/>"\
                              "<h1>Winstanley's lighthouse</h1>"\
                              "<p>The first lighthouse on Eddystone Rocks was an octagonal wooden"\
                              " structure built by Henry Winstanley. The lighthouse was also the"\
                              " first recorded instance of an offshore lighthouse. Construction"\
                              " started in 1696 and the light was lit on 14 November 1698. During"\
                              " construction, a French privateer took Winstanley prisoner and destroyed"\
                              " the work done so far on the foundations, causing Louis XIV to order"\
                              " Winstanley's release with the words &quot;France is at war with England,"\
                              " not with humanity&quot;.</p>"\
                              "</body></html>"

#define DEAD_BEEF                       0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_TIMER_PRESCALER             0                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                 /**< Size of timer operation queues. */

static ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */
static ble_fat_t            m_ble_fat;
static uint8_t *            m_page_data = (uint8_t *) STATIC_PAGE; //{STATIC_BORING_WEBPAGE};
static uint16_t             m_conn_handle = BLE_CONN_HANDLE_INVALID;
static int16_t              m_last_data_pos = 0;

static uint8_t eddystone_url_data[] =   /**< Information advertised by the Eddystone URL frame type. */
{
    APP_EDDYSTONE_URL_FRAME_TYPE,   // Eddystone URL frame type.
    APP_EDDYSTONE_RSSI,             // RSSI value at 0 m.
    APP_FATBEACON_URI,              // Scheme or prefix for URL Fatbeacon
    APP_EDDYSTONE_URL_URL           // URL with a maximum length of 17 bytes. Last byte is suffix (".com", ".org", etc.)
};

static void advertising_start(void);


/**@brief handler for BLE fatbeacon read event */
static void fat_read_evt_handler(ble_fat_t* p_fat, uint16_t value_handle)
{   
    ret_code_t                            err_code;
    ble_gatts_rw_authorize_reply_params_t reply;

    uint8_t value_buffer[FAT_CHAR_MAX_LEN] = {0};
    uint16_t page_size = strlen((char *)m_page_data);
    
    if (page_size > 10000) {
        page_size = 10000;      // Imposing an arbitrary limit on page size.
    }

    ble_gatts_value_t value = {.len = sizeof(value_buffer), .offset = 0, .p_value = &(value_buffer[0])};

    memset(&reply, 0, sizeof(reply));
    reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;

    err_code = sd_ble_gatts_value_get(m_conn_handle, value_handle, &value);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "GATT Value Error %d\n", err_code);
    }

    if (m_last_data_pos >= 0) // Active request
    {
        if (m_last_data_pos + FAT_CHAR_MAX_LEN >= page_size) {
            reply.params.read.len = page_size - m_last_data_pos;
        } else {
            reply.params.read.len = FAT_CHAR_MAX_LEN;
        }

        reply.params.read.p_data      = m_page_data + m_last_data_pos;        
        reply.params.read.update      = 1;
        reply.params.read.offset      = 0;
        reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;

        if (m_last_data_pos + FAT_CHAR_MAX_LEN < page_size) {
            m_last_data_pos += reply.params.read.len;
        } else {
            m_last_data_pos = -1;
        }
    } else {    // Send empty response as last packet
        reply.params.read.p_data      = NULL;
        reply.params.read.len         = 0;
        reply.params.read.update      = 1;
        reply.params.read.offset      = 0;
        reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
        m_last_data_pos = 0; 
    }

    //SEGGER_RTT_printf(0, "Reply Char len: %d of %d at: 0x%x", reply.params.read.len, page_size, m_page_data);
    err_code = sd_ble_gatts_rw_authorize_reply(m_conn_handle, &reply);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "GATT Reply Error %d\n", err_code);
    }
    
}

static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    //uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
            {
        case BLE_GAP_EVT_CONNECTED:            
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            SEGGER_RTT_printf(0,"Got BLE Connection. Handle: %d\n", m_conn_handle);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            SEGGER_RTT_printf(0,"BLE Handle: %d Disconnected.\n", m_conn_handle);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            m_last_data_pos = 0;
            // Start the advertising back up
            advertising_start();
            break;

        case BLE_GAP_EVT_TIMEOUT:
            advertising_start();
            break;

        default:
            // No implementation needed.
            break;
    }
}

static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_fat_on_ble_evt(&m_ble_fat, p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    ble_advertising_on_sys_evt(sys_evt);
}

static void gap_params_init(void)
{
   uint32_t                err_code;
   ble_gap_conn_params_t   gap_conn_params;
   ble_gap_conn_sec_mode_t sec_mode;

   BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

   err_code = sd_ble_gap_device_name_set(&sec_mode,
                                         (const uint8_t *)DEVICE_NAME,
                                         strlen(DEVICE_NAME));
   APP_ERROR_CHECK(err_code);

   err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_TAG);
   APP_ERROR_CHECK(err_code);

   memset(&gap_conn_params, 0, sizeof(gap_conn_params));

   gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
   gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
   gap_conn_params.slave_latency     = SLAVE_LATENCY;
   gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

   err_code = sd_ble_gap_ppcp_set(&gap_conn_params);

   APP_ERROR_CHECK(err_code);
}

static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);

}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing the advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t adv_data;
    ble_advdata_t scrsp_data;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    ble_uuid_t    adv_uuids[] = {{APP_EDDYSTONE_UUID, BLE_UUID_TYPE_BLE}};
    ble_uuid_t    scrp_uuids[] = {{BLE_UUID_FAT_URL_SERVICE, m_ble_fat.uuid_type}};

    uint8_array_t eddystone_data_array;                             // Array for Service Data structure.
/** @snippet [Eddystone data array] */
    eddystone_data_array.p_data = (uint8_t *) eddystone_url_data;   // Pointer to the data to advertise.
    eddystone_data_array.size = sizeof(eddystone_url_data);         // Size of the data to advertise.
/** @snippet [Eddystone data array] */

    ble_advdata_service_data_t service_data;                        // Structure to hold Service Data.
    service_data.service_uuid = APP_EDDYSTONE_UUID;                 // Eddystone UUID to allow discoverability on iOS devices.
    service_data.data = eddystone_data_array;                       // Array for service advertisement data.

    // Build and set advertising data.
    memset(&adv_data, 0, sizeof(adv_data));

    adv_data.name_type               = BLE_ADVDATA_NO_NAME;
    adv_data.flags                   = flags;
    adv_data.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    adv_data.uuids_complete.p_uuids  = adv_uuids;
    adv_data.p_service_data_array    = &service_data;                // Pointer to Service Data structure.
    adv_data.service_data_count      = 1;

    memset(&scrsp_data, 0, sizeof(scrsp_data));
    scrsp_data.name_type            = BLE_ADVDATA_FULL_NAME;
    scrsp_data.include_appearance   = false;
    scrsp_data.uuids_complete.uuid_cnt = sizeof(scrp_uuids) / sizeof(scrp_uuids[0]);
    scrsp_data.uuids_complete.p_uuids = scrp_uuids;

    //err_code = ble_advdata_set(&adv_data, &scrsp_data);
    err_code = ble_advdata_set(&adv_data, NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));
    
    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.interval    = MSEC_TO_UNITS(APP_CFG_CONNECTABLE_ADV_INTERVAL_MS, UNIT_0_625_MS);
    m_adv_params.timeout     = APP_CFG_CONNECTABLE_ADV_TIMEOUT;
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_start(&m_adv_params);
    //err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Error %d\n", err_code);
    }
    //APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Softdevice getcfg Error %d\n", err_code);
    }
    //APP_ERROR_CHECK(err_code);

    ble_enable_params.common_enable_params.vs_uuid_count = 2;
    SEGGER_RTT_printf(0, "UUID Count %d\n", ble_enable_params.common_enable_params.vs_uuid_count);
    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Softlink Enable Error %d\n", err_code);
    }
    //APP_ERROR_CHECK(err_code);

    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Softdevice bleevt handler set Error %d\n", err_code);
    }
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Softdevice sysevt handler set Error %d\n", err_code);
    }
}


/**@brief Function for doing power management.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    ble_fat_init_t fat_init;

    // Initialize.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    err_code = bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);

    SEGGER_RTT_WriteString(0, "Starting up BeaconBuddy\n");

    ble_stack_init();
    gap_params_init();
    conn_params_init();

    memset(&fat_init, 0, sizeof(fat_init));
    fat_init.read_evt_handler = fat_read_evt_handler;
    fat_init.val_data = m_page_data;

    err_code = ble_fat_init(&m_ble_fat, &fat_init);
    APP_ERROR_CHECK(err_code);

    advertising_init();
    LEDS_ON(LEDS_MASK);
    // Start execution.
    advertising_start();

    // Enter main loop.
    for (;; )
    {
        power_manage();
    }
}


/**
 * @}
 */
