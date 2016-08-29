/* fatbeacon.h 

    This holds most of the definitions and static data for this fatbeacon implementation.  
*/

#ifndef FATBEACON_H__
#define FATBEACON_H__

#define DEVICE_NAME                     "FatBeacon"

// Eddystone URL Fatbeacon data
#define APP_EDDYSTONE_UUID              0xFEAA                            /**< UUID for Eddystone beacons according to specification. */
#define APP_EDDYSTONE_RSSI              0xEE                              /**< 0xEE = -18 dB is the approximate signal strength at 0 m. */
#define APP_FATBEACON_URI               0x0E                              /** 0x0E is the URL scheme for Fatbeacon */
#define APP_EDDYSTONE_URL_FRAME_TYPE    0x10                              /**< URL Frame type is fixed at 0x10. */

// Fatbeacon description
#define APP_FATBEACON_NAME              'H', 'e', 'l', 'l', \
                                        'o', ' ', 'F', 'a', \
                                        't', 'b', 'e', 'a', \
                                        'c', 'o', 'n' 

/*
#define APP_FATBEACON_NAME              'E', 'd', 'd', 'y', \
                                        's', 't', 'o', 'n', \
                                        'e', ' ', 'L', 'i', \
                                        'g', 'h', 't' 
*/

// A smaller static webpage (about 2kB, load time is 4-5 seconds)
/*
#define STATIC_PAGE         "<html><head><meta charset=\"utf-8\"><title>"\
                            "I'm a Fatbeacon</title><style>"\
                            "body{margin:0;padding:0;font-family:sans-serif"\
                            ";color:#444;background:#f5f5f5}header{display"\
                            ":flex;background:#EF6C00;height:100px}.card{ba"\
                            "ckground:#fff;padding:20px;margin:30px;"\
                            "border:1px solid #ccc;box-shadow:0px 0p"\
                            "x 5px #aaa}p{font-size:1em}a{color:#3F82C4}h1"\
                            "{margin:35px 10px;display:inline-block"\
                            ";font-size:1.2em;color:#fff}footer{text-align:"\
                            "center;font-size:.7em;color:#777}svg{margin-left"\
                            ":30px;display:inline-block;width:40px}"\
                            "</style></head><body><header>"\
                            "<svg viewBox=\"0 0 171 202\"><g fill=\"#fff\"><path "\
                            "d=\"M141.2 85.3c0-31-25-56-56-56-30.7 0-55.8 25-5"\
                            "5.8 56 0 17 7.8 32.5 20 42.8l10-10c-9.8-7.6-16-1"\
                            "9.4-16-32.7 0-23.2 18.8-42 42-42 23 0 41.8 18.8 "\
                            "41.8 42 0 13.3-6.2 25-16 32.8l10 10c12.2-10.2 20"\
                            "-25.6 20-42.7\"/><path d=\"M14 85.3C14 46 46 14 85"\
                            ".3 14s71.3 32 71.3 71.3c0 21.4-9.5 40.6-24.5 53."\
                            "7l10 10c17.6-15.7 28.6-38.4 28.6-63.7 0-47-38.2-"\
                            "85.3-85.3-85.3C38.3 0 0 38.2 0 85.3c0 25.3 11 48 "\
                            "28.5 63.6l10-10C23.5 126 14 106.7 14 85.3\"/>"\
                            "<path d=\"M89.2 200.3c-2 2-5.5 2-7.6 0l-35.8-35.8c"\
                            "-2-2-2-5.5 0-7.6l35.8-36c2-2 5.5-2 7.6 0l35.8 36c"\
                            "2 2 2 5.4 0 7.5l-35.8 35.8z\"/></g></svg>"\
                            "<h1>Welcome to Fatbeacon</h1>"\
                            "</header><section class=\"table\"><div class"\
                            "=\"card\"><b>What is Fatbeacon?</b><p>Fatbeacon"\
                            " is an experimental type of Physical Web beacon "\
                            " that can transmit its own data when limited or "\
                            " no internet connectivity is available.</p></div>"\
                            "</section><footer><hr />Fatbeacon powered!</footer>"\
                            "</body></html>"\
*/
/* Static web page.  You can put any single-page html/css/js you like here.  By default this
 * returns a short blurb about the eddystone lighthouse (this is from Wikipedia so if you use it
 * make sure to provide attribution).   
 *
 * Note: This is about 8kB, so it can take 30 seconds or so to load!
*/

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

#endif