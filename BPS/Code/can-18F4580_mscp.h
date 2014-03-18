/*
 *                                 can-18F4580.c
 *
 * Prototypes, definitions, defines and macros used for and with the CCS CAN 
 * library for PIC18Fxx8 and PIC18Cxx8.
 *
 * (see can-18F4580.c)
 *
 * ===============================================================
 *
 *               (C) Copyright 1996,2011 Custom Computer Services
 * This source code may only be used by licensed users of the CCS C compiler.
 * This source code may only be distributed to other licensed users of the CCS C
 * compiler. No other use, reproduction or distribution is permitted without 
 * written permission. Derivative programs created using this software in
 * object code form are not restricted in any way.
 */

#ifndef __CCS_CAN18F4580_LIB_DEFINES__
#define __CCS_CAN18F4580_LIB_DEFINES__

#ifndef CAN_DO_DEBUG
#define CAN_DO_DEBUG FALSE
#endif

#ifndef CAN_USE_EXTENDED_ID
#define CAN_USE_EXTENDED_ID FALSE
#endif

//Synchronized jump width (def: 1 x Tq)
#ifndef CAN_BRG_SYNCH_JUMP_WIDTH
#define CAN_BRG_SYNCH_JUMP_WIDTH 0
#endif

// Baud rate generator prescalar (def: 4) ( Tq = (2 x (PRE + 1))/Fosc )
#ifndef CAN_BRG_PRESCALAR
#define CAN_BRG_PRESCALAR 4
#endif

//Phase segment 2 time select bit (def: freely programmable)
#ifndef CAN_BRG_SEG_2_PHASE_TS
#define CAN_BRG_SEG_2_PHASE_TS TRUE
#endif

//Sample of the CANbus line (def: bus is sampled once prior to sample point)
#ifndef CAN_BRG_SAM
#define CAN_BRG_SAM 1
#endif

//Phase segment 1 (def: 6 x Tq)
#ifndef CAN_BRG_PHASE_SEGMENT_1
#define CAN_BRG_PHASE_SEGMENT_1 5
#endif

//Propagation time select (def: 3 x Tq)
#ifndef CAN_BRG_PROPAGATION_TIME
#define CAN_BRG_PROPAGATION_TIME 6
#endif

//Selects CANbus line filter for wake up bit
#ifndef CAN_BRG_WAKE_FILTER
#define CAN_BRG_WAKE_FILTER FALSE
#endif

//Phase segment 2 time select (def: 6 x Tq)
#ifndef CAN_BRG_PHASE_SEGMENT_2
#define CAN_BRG_PHASE_SEGMENT_2 1
#endif

//If buffer 0 overflows, do NOT use buffer 1 to put buffer 0 data
#ifndef CAN_USE_RX_DOUBLE_BUFFER
#define CAN_USE_RX_DOUBLE_BUFFER TRUE
#endif

#ifndef CAN_ENABLE_DRIVE_HIGH
#define CAN_ENABLE_DRIVE_HIGH 0
#endif

#ifndef CAN_ENABLE_CAN_CAPTURE
#define CAN_ENABLE_CAN_CAPTURE 0
#endif

// Added 03/30/09 for PIC18F6585/8585/6680/8680
// 0 CANTX2 disabled, 1 CANTX2 enabled
#ifndef CAN_ENABLE_CANTX2
#define CAN_ENABLE_CANTX2 0
#endif

// Added 03/30/09 for PIC18F6585/8585/6680/8680
// 0 source is invert of CANTX1, 1 source is CAN Clock
#ifndef CAN_CANTX2_SOURCE
#define CAN_CANTX2_SOURCE 0
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////  CAN Control Register /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum CAN_OP_MODE {  CAN_OP_CONFIG=4,
                    CAN_OP_LISTEN=3,
                    CAN_OP_LOOPBACK=2,
                    CAN_OP_DISABLE=1,
                    CAN_OP_NORMAL=0 };

enum CAN_FUN_OP_MODE {  CAN_FUN_OP_LEGACY=0,
                        CAN_FUN_OP_ENHANCED=1,
                        CAN_FUN_OP_ENHANCED_FIFO=2 };

enum CAN_WIN_ADDRESS {  CAN_WIN_RX0=0,
                        CAN_WIN_RX1=5,
                        CAN_WIN_TX0=4,
                        CAN_WIN_TX1=3,
                        CAN_WIN_TX2=2 };

enum CAN_FIFO_READ {    CAN_FIFO_MB7=7,
                        CAN_FIFO_MB6=6,
                        CAN_FIFO_MB5=5,
                        CAN_FIFO_MB4=4,
                        CAN_FIFO_MB3=3,
                        CAN_FIFO_MB2=2,
                        CAN_FIFO_MB1=1,
                        CAN_FIFO_MB0=0 };

// Control register configurations for modes 0, 1, and 2
// CAN control
struct {
    int1 void0;             //0
    CAN_WIN_ADDRESS win:3;  //1:3   //Window address bits
    int1 abat;              //4     //Abort all pending transmissions
    CAN_OP_MODE reqop:3;    //5:7   //Request CAN operation mode bits
} CANCON;
#byte CANCON = getenv("SFR:CANCON") //0xF6F

// CAN control for mode 1
struct {
    int1 void3210:4;        //0:3
    int1 abat;              //4     //Abort all pending transmissions
    CAN_OP_MODE reqop:3;    //5:7   //Request CAN operation mode bits
} CANCON_MODE_1;
#byte CANCON_MODE_1 = getenv("SFR:CANCON")  //0xF6F

// CAN control for mode 2
struct {
    CAN_FIFO_READ fp:4;     //0:3   //Points to message buffer to read
    int1 abat;              //4     //Abort
    CAN_OP_MODE reqop:3;    //5:7   //Request CAN operation mode bits
} CANCON_MODE_2;
#byte CANCON_MODE_2 = getenv("SFR:CANCON")  //0xF6F

////////////////////////////////////////////////////////////////////////////////
/////////////////////////  ECAN Control Register ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum ECAN_MODE {    ECAN_LEGACY=0,
                    ECAN_ENHANCED_LEGACY=1,
                    ECAN_ENHANCED_FIFO=2 };

enum ECAN_WINDOW_ADDRESS {  AF012_BRGCON23=0,
                            AF345_BRGCON1_=1,
                            AFM_ERROR_ICON=2,
                            TX0=3,
                            TX1=4,
                            TX2=5,
                            AF678=6,
                            AccF91011=7,
                            AccF121314=8,
                            AccF15=9,
                            RXINT01=15,
                            RX0=16,
                            RX1=17,
                            TXRX0=18,
                            TXRX1=19,
                            TXRX2=20,
                            TXRX3=21,
                            TXRX4=22,
                            TXRX5=23 };

//ECAN control register mode 1, 2 & 3
struct {
    ECAN_WINDOW_ADDRESS ewin:5;  //0:4  //Access bank map
    int1 fifowm;                 //5    //FIFO high water mark
    ECAN_MODE mdsel:2;           //6:7  //Mode select bits
} ECANCON;
#byte ECANCON = getenv("SFR:ECANCON")   //0xF77

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////  CAN Status Register  ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum CAN_INT_CODE { CAN_INT_WAKEUP=7,
                    CAN_INT_RX0=6,
                    CAN_INT_RX1=5,
                    CAN_INT_TX0=4,
                    CAN_INT_TX1=3,
                    CAN_INT_TX2=2,
                    CAN_INT_ERROR=1,
                    CAN_INT_NO=0};

enum CAN_EINT_CODE {    CAN_EINT_NO=0,
                        CAN_EINT_ERROR=2,
                        CAN_EINT_TX2=4,
                        CAN_EINT_TX1=6,
                        CAN_EINT_TX0=8,
                        CAN_EINT_RX1=17,
                        CAN_EINT_RX0=16,
                        CAN_EINT_WAKEUP=14,
                        CAN_EINT_RXB0=16,
                        CAN_EINT_RXB1=17,
                        CAN_EINT_B0=18,
                        CAN_EINT_B1=19,
                        CAN_EINT_B2=20,
                        CAN_EINT_B3=21,
                        CAN_EINT_B4=22,
                        CAN_EINT_B5=23 };

//CAN status register READ-ONLY
struct {
    int1 void0;              //0
    CAN_INT_CODE icode:3;    //1:3   //Interrupt code
    int1 void4;              //4
    CAN_OP_MODE opmode:3;    //5:7   //Operation mode status
} CANSTAT;
#byte CANSTAT = getenv("SFR:CANSTAT")   //0xF6E

//CAN status register mode 1
struct {
    CAN_EINT_CODE eicode:5;  //0:4   //Interrupt code
    CAN_OP_MODE opmode:3;    //5:7   //Operation mode status
} CANSTAT_MODE_1;
#byte CANSTAT_MODE_1 = getenv("SFR:CANSTAT")    //0xF6E

//CAN status register mode 2
struct {
    CAN_EINT_CODE eicode:5;
    CAN_OP_MODE opmode:3;
} CANSTAT_MODE_2;
#byte CANSTAT_MODE_2 = getenv("SFR:CANSTAT")    //0xF6E

////////////////////////////////////////////////////////////////////////////////
////////////////////  Communication Status Register  ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

//Communication status register READ-ONLY
struct {
    int1 ewarn;     //0     //Error warning
    int1 rxwarn;    //1     //Receiver warning
    int1 txwarn;    //2     //Transmitter warning
    int1 rxbp;      //3     //Receiver bus passive
    int1 txbp;      //4     //Transmitter bus passive bit
    int1 txbo;      //5     //Transmitter bus off
    int1 rx1ovfl;   //6     //Receive buffer 1 overflow
    int1 rx0ovfl;   //7     //Receive buffer 0 overflow
} COMSTAT;
#byte COMSTAT = getenv("SFR:COMSTAT")   //0xF74

//Communication status register mode 1
struct {
    int1 ewarn;     //0     //Error warning
    int1 rxwarn;    //1     //Receiver warning
    int1 txwarn;    //2     //Transmitter warning
    int1 rxbp;      //3     //Receiver bus passive
    int1 txbp;      //4     //Transmitter bus passive bit
    int1 txbo;      //5     //Transmitter bus off
    int1 rxnovfl;   //6     //Recive buffer n overflow
    int1 void7;     //7
} COMSTAT_MODE_1;
#byte COMSTAT_MODE_1 = getenv("SFR:COMSTAT")    //0xF74

//Communication status register mode 2
struct {
    int1 ewarn;     //0     //Error warning
    int1 rxwarn;    //1     //Receiver warning
    int1 txwarn;    //2     //Transmitter warning
    int1 rxbp;      //3     //Receiver bus passive
    int1 txbp;      //4     //Transmitter bus passive bit
    int1 txbo;      //5     //Transmitter bus off
    int1 rxnovfl;   //6     //Recive buffer n overflow
    int1 fifoempty; //7     //FIFO not empty bit
} COMSTAT_MODE_2;
#byte COMSTAT_MODE_2 = getenv("SFR:COMSTAT")    //0xF74

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Baud Control Registers /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//Baud rate control register 1
struct {
    int brp:6;      //0:5   //Baud rate prescalar
    int sjw:2;      //6:7   //Synchronized jump width
} BRGCON1;
#byte BRGCON1 = getenv("SFR:BRGCON1")   //0xF70

//Baud rate control register 2
struct {
    int prseg:3;    //0:2   //Propagation time select
    int seg1ph:3;   //3:5   //Phase segment 1
    int1 sam;       //6     //Sample of the CANbus line
    int1 seg2phts;  //7     //Phase segment 2 time select
} BRGCON2;
#byte BRGCON2 = getenv("SFR:BRGCON2")   //0xF71

//Baud rate control register 3
struct {
    int seg2ph:3;   //0:2   //Phase segment 2 time select
    int void543:3;  //3:5
    int1 wakfil;    //6     //Selects CANbus line filter for wake-up
    int1 void7;     //7
} BRGCON3;
#byte BRGCON3 = getenv("SFR:BRGCON3")   //0xF72


//CAN i/o control register
struct {
    int void3210:4; //0:3
    int1 cancap;    //4   //CAN message receive caputre
    int1 endrhi;    //5   //Enable drive high
    int1 tx2en;     //6   //CANTX2 pin enable bit       //Added 3/24/09 for PIC18F6585/8585/6680/8680
    int1 tx2src;    //7   //CANTX2 pin data source bit  //Added 3/24/09 for PIC18F6585/8585/6680/8680
} CIOCON;
#byte CIOCON = getenv("SFR:CIOCON")     //0xF73

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Transmit Control Registers /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//Transmit buffer n control register for mode 0
struct txbNcon_struct {
    int  txpri:2;   //0:1   //Transmit priority bits
    int1 void2;     //2
    int1 txreq;     //3     //Transmit request status (clear to request message abort)
    int1 txerr;     //4     //Transmission error detected
    int1 txlarb;    //5     //Transmission lost arbitration status
    int1 txabt;     //6     //Transmission aborted status
    int1 void7;     //7
};
struct txbNcon_struct TXB0CON;
struct txbNcon_struct TXB1CON;
struct txbNcon_struct TXB2CON;
struct txbNcon_struct TXBaCON;
#byte TXB0CON = getenv("TXB0CON")      //0xF40
#byte TXB1CON = getenv("TXB1CON")      //0xF30
#byte TXB2CON = getenv("TXB2CON")      //0xF20
#byte TXBaCON = 0xF60   //TxbXcon when in the access bank


//Transmit buffer n control register for mode 1 & 2
struct txbNconm12_struct {
    int  txpri:2;   //0:1   //Transmit priority bits
    int1 void2;     //2
    int1 txreq;     //3     //Transmit request status (clear to request message abort)
    int1 txerr;     //4     //Transmission error detected
    int1 txlarb;    //5     //Transmission lost arbitration status
    int1 txabt;     //6     //Transmission aborted status
    int1 txbif;     //7     //Tramsmission complete, ready for new data
};

struct txbNconm12_struct TXB0CON_MODE_1;
struct txbNconm12_struct TXB0CON_MODE_2;
struct txbNconm12_struct TXB1CON_MODE_1;
struct txbNconm12_struct TXB1CON_MODE_2;
struct txbNconm12_struct TXB2CON_MODE_1;
struct txbNconm12_struct TXB2CON_MODE_2;
struct txbNconm12_struct TXBaCON_MODE_1;
struct txbNconm12_struct TXBaCON_MODE_2;
#byte TXB0CON_MODE_1 = getenv("TXB0CON")    //0xF40
#byte TXB0CON_MODE_2 = getenv("TXB0CON")    //0xF40
#byte TXB1CON_MODE_1 = getenv("TXB1CON")    //0xF30
#byte TXB1CON_MODE_2 = getenv("TXB1CON")    //0xF30
#byte TXB2CON_MODE_1 = getenv("TXB2CON")    //0xF20
#byte TXB2CON_MODE_2 = getenv("TXB2CON")    //0xF20
#byte TXBaCON_MODE_1=0xF60
#byte TXBaCON_MODE_2=0xF60


//Transmit buffer n standard identifier
#byte TXB0SIDH = getenv("SFR:TXB0SIDH")     //0xF41
#byte TXB0SIDL = getenv("SFR:TXB0SIDL")     //0xF42
#byte TXB1SIDH = getenv("SFR:TXB1SIDH")     //0xF31
#byte TXB1SIDL = getenv("SFR:TXB1SIDL")     //0xF32
#byte TXB2SIDH = getenv("SFR:TXB2SIDH")     //0xF21
#byte TXB2SIDL = getenv("SFR:TXB2SIDL")     //0xF22

//Transmit buffer n extended identifier
#byte TXB0EIDH = getenv("SFR:TXB0EIDH")     //0xF43
#byte TXB0EIDL = getenv("SFR:TXB0EIDL")     //0xF44
#byte TXB1EIDH = getenv("SFR:TXB1EIDH")     //0xF33
#byte TXB1EIDL = getenv("SFR:TXB1EIDL")     //0xF34
#byte TXB2EIDH = getenv("SFR:TXB2EIDH")     //0xF23
#byte TXB2EIDL = getenv("SFR:TXB2EIDL")     //0xF24

// ???: Why are these define macros and all others are byte macros?
#define RX0MASK     getenv("SFR:RXM0EIDL")  //0xF1B    //Rxm0eidl
#define RX1MASK     getenv("SFR:RXM1EIDL")  //0xF1F    //Rxm1eidl
#define RX0FILTER0  getenv("SFR:RXF0EIDL")  //0xF03    //Rxf0eidl
#define RX0FILTER1  getenv("SFR:RXF1EIDL")  //0xF07    //Rxf1eidl
#define RX1FILTER2  getenv("SFR:RXF2EIDL")  //0xF0B    //Rxf2eidl
#define RX1FILTER3  getenv("SFR:RXF3EIDL")  //0xF0F    //Rxf3eidl
#define RX1FILTER4  getenv("SFR:RXF4EIDL")  //0xF13    //Rxf4eidl
#define RX1FILTER5  getenv("SFR:RXF5EIDL")  //0xF17    //Rxf5eidl
#define RXB0ID      getenv("SFR:RXB0EIDL")  //0xF64    //Rxb0eidl
#define RXB1ID      getenv("SFR:RXB1EIDL")  //0xF54    //Rxb1eidl
#define TXB0ID      getenv("SFR:TXB0EIDL")  //0xF44    //Txb0eidl
#define TXB1ID      getenv("SFR:TXB1EIDL")  //0xF34    //Txb1eidl
#define TXB2ID      getenv("SFR:TXB2EIDL")  //0xF24    //Txb2eidl
#define B0ID        getenv("SFR:B0EIDL")    //0xE24    //B0eidl
#define B1ID        getenv("SFR:B1EIDL")    //0xE34    //B1eidl
#define B2ID        getenv("SFR:B2EIDL")    //0xE44    //B2eidl
#define B3ID        getenv("SFR:B3EIDL")    //0xE54    //B3eidl
#define B4ID        getenv("SFR:B4EIDL")    //0xE64    //B4eidl
#define B5ID        getenv("SFR:B5EIDL")    //0xE74    //B5eidl
#define TXRXBaID    0xF64

//Transmit buffer n data byte m
#byte TXB0D0 = getenv("SFR:TXB0D0")     //0xF46
#byte TXB0D7 = getenv("SFR:TXB0D7")     //0xF4D
#byte TXB1D0 = getenv("SFR:TXB1D0")     //0xF36
#byte TXB1D7 = getenv("SFR:TXB1D7")     //0xF3D
#byte TXB2D0 = getenv("SFR:TXB2D0")     //0xF26
#byte TXB2D7 = getenv("SFR:TXB2D7")     //0xF2D

//Transmit buffer n data length
struct txbNdlc_struct {
    int dlc:4;      //0:3
    int void54:2;   //4:5
    int1 rtr;       //6     //Transmission frame remote tranmission
    int1 void7;     //7
};
struct txbNdlc_struct TXB0DLC;
struct txbNdlc_struct TXB1DLC;
struct txbNdlc_struct TXB2DLC;
struct txbNdlc_struct TXBaDLC;
#byte TXB0DLC = getenv("SFR:TXB0DLC")   //0xF45
#byte TXB1DLC = getenv("SFR:TXB1DLC")   //0xF35
#byte TXB2DLC = getenv("SFR:TXB2DLC")   //0xF25
#byte TXBaDLC = 0xF65   //TxbXdlc when in the access bank

//Transmit error count register
#byte TXERRCNT = getenv("SFR:TXERRCNT") //0xF76


////////////////////////////////////////////////////////////////////////////////
//////////////////////// Recive Control Registers //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum CAN_RX_MODE {  CAN_RX_ALL=3,
                    CAN_RX_EXT=2,
                    CAN_RX_STD=1,
                    CAN_RX_VALID=0};
enum ECAN_FILTER_HIT {  RXF0=0,
                        RXF1=1,
                        RXF2=2,
                        RXF3=3,
                        RXF4=4,
                        RXF5=5,
                        RXF6=6,
                        RXF7=7,
                        RXF8=8,
                        RXF9=9,
                        RXF10=10,
                        RXF11=11,
                        RXF12=12,
                        RXF13=13,
                        RXF14=14,
                        RXF15=15 };


//Receive buffer 0 control register mode 0
struct {
    int1 filthit0;      //0     //Filter hit
    int1 jtoff;         //1     //Jump table offset
    int1 rxb0dben;      //2     //Receive buffer 0 double buffer enable
    int1 rxrtrro;       //3     //Receive remote transfer request
    int1 void4;         //4
    CAN_RX_MODE rxm:2;  //5:6   //Receiver buffer mode
    int1 rxful;         //7     //Receive full status
} RXB0CON;
#byte RXB0CON = getenv("SFR:RXB0CON")   //0xF60

//Receive buffer 0 control register mode 1 & 2
struct rxb01m12con {
    ECAN_FILTER_HIT filthit:5;  //1:4   //Acceptance filter bits
    int1 rtrro;                 //5     //Remote transmission request bit
    int1 rxm1;                  //6     //Recive buffer mode
    int1 rxful;                 //7     //Receive full status
};
struct rxb01m12con RXB0CON_MODE_1;
struct rxb01m12con RXB0CON_MODE_2;
#byte RXB0CON_MODE_1 = getenv("SFR:RXB0CON")    //0xF60
#byte RXB0CON_MODE_2 = getenv("SFR:RXB0CON")    //0xF60

//Receive buffer 1 control register mode 0
struct {
    int filthit:3;      //0:2
    int1 rxrtrro;       //3     //Receive remote transfer request
    int1 void4;         //4
    CAN_RX_MODE rxm:2;  //5:6   //Receive buffer mode
    int1 rxful;         //7     //Receive full
} RXB1CON;
#byte   RXB1CON = getenv("SFR:RXB1CON")         //0xF50

//Receive buffer 1 control register mode 1 & 2
struct rxb01m12con RXB1CON_MODE_1;
struct rxb01m12con RXB1CON_MODE_2;
#byte RXB1CON_MODE_1 = getenv("SFR:RXB1CON")    //0xF50
#byte RXB1CON_MODE_2 = getenv("SFR:RXB1CON")    //0xF50

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Buffer Select Register /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum PROG_BUFFER {  B0=0x04,
                    B1=0x08,
                    B2=0x10,
                    B3=0x20,
                    B4=0x40,
                    B5=0x80 };

// Bsel0
struct {
    int   void10:2; //0:1
    int1  b0txen;   //2     //Buffer 0 transmit enable bit
    int1  b1txen;   //3     //Buffer 1 transmit enable bit
    int1  b2txen;   //4     //Buffer 2 transmit enable bit
    int1  b3txen;   //5     //Buffer 3 transmit enable bit
    int1  b4txen;   //6     //Buffer 4 transmit enable bit
    int1  b5txen;   //7     //Buffer 5 transmit enable bit
} BSEL0;
#byte BSEL0 = getenv("SFR:BSEL0")   //0xDF8

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Bn Control Registers ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum ECAN_AF {  AF0=0,
                AF1=1,
                AF2=2,
                AF3=3,
                AF4=4,
                AF5=5,
                AF6=6,
                AF7=7,
                AF8=8,
                AF9=9,
                AF10=10,
                AF11=11,
                AF12=12,
                AF13=13,
                AF14=14,
                AF15=15 };

//Bn control register in recive mode
struct BaCON_recive {
    ECAN_AF filhit:5;   //0:4   //Acceptance filter bits
    int1 rxrtrro;       //5     //Read only remote transmission request
    int1 rxm1;          //6     //Recive buffer mode bit
    int1 rxful;         //7     //Receiver full status bit
};

//Bn control register in transmit mode
struct BaCON_transmit {
    int   txpri:2;      //0:1   //Priority Bits
    int1   rtren;       //2     //Automatic Remote Transmission request bit
    int1   txreq;       //3     //Transmit request status
    int1   txerr;       //4     //Transmission error detected
    int1   txlarb;      //5     //Transmission lost arbitration bit
    int1  txabt;        //6     //Transmission aborted status bit
    int1    txbif;      //7     //Transmit buffer interrupt flag bit
};

struct BaCON_recive B0CONR;
struct BaCON_recive B1CONR;
struct BaCON_recive B2CONR;
struct BaCON_recive B3CONR;
struct BaCON_recive B4CONR;
struct BaCON_recive B5CONR;
// Struct bytes, used for access to specific bits
#byte B0CONR = getenv("SFR:B0CON")      //0xE20
#byte B1CONR = getenv("SFR:B1CON")      //0xE30
#byte B2CONR = getenv("SFR:B2CON")      //0xE40
#byte B3CONR = getenv("SFR:B3CON")      //0xE50
#byte B4CONR = getenv("SFR:B4CON")      //0xE60
#byte B5CONR = getenv("SFR:B5CON")      //0xE70
// Access bytes, used for fast access to the entire byte
#byte B0CONRA = getenv("SFR:B0CON")     //0xE20
#byte B1CONRA = getenv("SFR:B1CON")     //0xE30
#byte B2CONRA = getenv("SFR:B2CON")     //0xE40
#byte B3CONRA = getenv("SFR:B3CON")     //0xE50
#byte B4CONRA = getenv("SFR:B4CON")     //0xE60
#byte B5CONRA = getenv("SFR:B5CON")     //0xE70

struct BaCON_transmit B0CONT;
struct BaCON_transmit B1CONT;
struct BaCON_transmit B2CONT;
struct BaCON_transmit B3CONT;
struct BaCON_transmit B4CONT;
struct BaCON_transmit B5CONT;
// Stuct bytes, used for access to specific bits
#byte B0CONT = getenv("SFR:B0CON")      //0xE20
#byte B1CONT = getenv("SFR:B1CON")      //0xE30
#byte B2CONT = getenv("SFR:B2CON")      //0xE40
#byte B3CONT = getenv("SFR:B3CON")      //0xE50
#byte B4CONT = getenv("SFR:B4CON")      //0xE60
#byte B5CONT = getenv("SFR:B5CON")      //0xE70
// Access bytes, used for fast access to the entire byte
#byte B0CONTA = getenv("SFR:B0CON")     //0xE20
#byte B1CONTA = getenv("SFR:B1CON")     //0xE30
#byte B2CONTA = getenv("SFR:B2CON")     //0xE40
#byte B3CONTA = getenv("SFR:B3CON")     //0xE50
#byte B4CONTA = getenv("SFR:B4CON")     //0xE60
#byte B5CONTA = getenv("SFR:B5CON")     //0xE70

////////////////////////////////////////////////////////////////////////////////
/////////////////////// B Data Length Code Registers ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

//DLC register for Bn in receive mode
struct BnDLC_receive {
    int    dlc:4;       //0:3   //Data length code bits
    int    void45:2;    //4:5
    int1    rxrtr;      //6     //Reciver remote transmission request
    int1    void7;      //7
};

//DLC regster for Bn in transmit mode
struct BnDLC_transmit {
    int   dlc:4;    //0:3   //Data length code bits
    int   void45:2; //4:5
    int1   txrtr;   //6     //Data length code bits
    int1   void7;   //7
};

struct BnDLC_receive B0DLCR;
struct BnDLC_receive B1DLCR;
struct BnDLC_receive B2DLCR;
struct BnDLC_receive B3DLCR;
struct BnDLC_receive B4DLCR;
struct BnDLC_receive B5DLCR;
#byte B0DLCR = getenv("SFR:B0DLC")  //0xE25
#byte B1DLCR = getenv("SFR:B1DLC")  //0xE35
#byte B2DLCR = getenv("SFR:B2DLC")  //0xE45
#byte B3DLCR = getenv("SFR:B3DLC")  //0xE55
#byte B4DLCR = getenv("SFR:B4DLC")  //0xE65
#byte B5DLCR = getenv("SFR:B5DLC")  //0xE75

struct BnDLC_transmit B0DLCT;
struct BnDLC_transmit B1DLCT;
struct BnDLC_transmit B2DLCT;
struct BnDLC_transmit B3DLCT;
struct BnDLC_transmit B4DLCT;
struct BnDLC_transmit B5DLCT;
#byte B0DLCT = getenv("SFR:B0DLC")  //0xE25
#byte B1DLCT = getenv("SFR:B1DLC")  //0xE35
#byte B2DLCT = getenv("SFR:B2DLC")  //0xE45
#byte B3DLCT = getenv("SFR:B3DLC")  //0xE55
#byte B4DLCT = getenv("SFR:B4DLC")  //0xE65
#byte B5DLCT = getenv("SFR:B5DLC")  //0xE75

////////////////////////////////////////////////////////////////////////////////
/////////////////////// General Purpose Buffers ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#byte B5D7 = getenv("SFR:B5D7")     //0xE7D
#byte B5D6 = getenv("SFR:B5D6")     //0xE7C
#byte B5D5 = getenv("SFR:B5D5")     //0xE7B
#byte B5D4 = getenv("SFR:B5D4")     //0xE7A
#byte B5D3 = getenv("SFR:B5D3")     //0xE79
#byte B5D2 = getenv("SFR:B5D2")     //0xE78
#byte B5D1 = getenv("SFR:B5D1")     //0xE77
#byte B5D0 = getenv("SFR:B5D0")     //0xE76

#byte B4D7 = getenv("SFR:B4D7")     //0xE6D
#byte B4D6 = getenv("SFR:B4D6")     //0xE6C
#byte B4D5 = getenv("SFR:B4D5")     //0xE6B
#byte B4D4 = getenv("SFR:B4D4")     //0xE6A
#byte B4D3 = getenv("SFR:B4D3")     //0xE69
#byte B4D2 = getenv("SFR:B4D2")     //0xE68
#byte B4D1 = getenv("SFR:B4D1")     //0xE67
#byte B4D0 = getenv("SFR:B4D0")     //0xE66

#byte B3D7 = getenv("SFR:B3D7")     //0xE5D
#byte B3D6 = getenv("SFR:B3D6")     //0xE5C
#byte B3D5 = getenv("SFR:B3D5")     //0xE5B
#byte B3D4 = getenv("SFR:B3D4")     //0xE5A
#byte B3D3 = getenv("SFR:B3D3")     //0xE59
#byte B3D2 = getenv("SFR:B3D2")     //0xE58
#byte B3D1 = getenv("SFR:B3D1")     //0xE57
#byte B3D0 = getenv("SFR:B3D0")     //0xE56

#byte B2D7 = getenv("SFR:B2D7")     //0xE4D
#byte B2D6 = getenv("SFR:B2D6")     //0xE4C
#byte B2D5 = getenv("SFR:B2D5")     //0xE4B
#byte B2D4 = getenv("SFR:B2D4")     //0xE4A
#byte B2D3 = getenv("SFR:B2D3")     //0xE49
#byte B2D2 = getenv("SFR:B2D2")     //0xE48
#byte B2D1 = getenv("SFR:B2D1")     //0xE47
#byte B2D0 = getenv("SFR:B2D0")     //0xE46

#byte B1D7 = getenv("SFR:B1D7")     //0xE3D
#byte B1D6 = getenv("SFR:B1D6")     //0xE3C
#byte B1D5 = getenv("SFR:B1D5")     //0xE3B
#byte B1D4 = getenv("SFR:B1D4")     //0xE3A
#byte B1D3 = getenv("SFR:B1D3")     //0xE39
#byte B1D2 = getenv("SFR:B1D2")     //0xE38
#byte B1D1 = getenv("SFR:B1D1")     //0xE37
#byte B1D0 = getenv("SFR:B1D0")     //0xE36

#byte B0D7 = getenv("SFR:B0D7")     //0xE2D
#byte B0D6 = getenv("SFR:B0D6")     //0xE2C
#byte B0D5 = getenv("SFR:B0D5")     //0xE2B
#byte B0D4 = getenv("SFR:B0D4")     //0xE2A
#byte B0D3 = getenv("SFR:B0D3")     //0xE29
#byte B0D2 = getenv("SFR:B0D2")     //0xE28
#byte B0D1 = getenv("SFR:B0D1")     //0xE27
#byte B0D0 = getenv("SFR:B0D0")     //0xE26

////////////////////////////////////////////////////////////////////////////////
//////////////////// General Purpose Buffer ID Registers ///////////////////////
////////////////////////////////////////////////////////////////////////////////

// Standard Identifier bytes
#byte B0SIDH = getenv("SFR:B0SIDH")    //0xE21
#byte B1SIDH = getenv("SFR:B1SIDH")    //0xE31
#byte B2SIDH = getenv("SFR:B2SIDH")    //0xE41
#byte B3SIDH = getenv("SFR:B3SIDH")    //0xE51
#byte B4SIDH = getenv("SFR:B4SIDH")    //0xE61
#byte B5SIDH = getenv("SFR:B5SIDH")    //0xE71

#byte B0SIDL = getenv("SFR:B0SIDL")    //0xE22
#byte B1SIDL = getenv("SFR:B1SIDL")    //0xE32
#byte B2SIDL = getenv("SFR:B2SIDL")    //0xE42
#byte B3SIDL = getenv("SFR:B3SIDL")    //0xE52
#byte B4SIDL = getenv("SFR:B4SIDL")    //0xE62
#byte B5SIDL = getenv("SFR:B5SIDL")    //0xE72

// Exteded identifier bytes
#byte B0EIDH = getenv("SFR:B0EIDH")    //0xE23
#byte B1EIDH = getenv("SFR:B1EIDH")    //0xE33
#byte B2EIDH = getenv("SFR:B2EIDH")    //0xE43
#byte B3EIDH = getenv("SFR:B3EIDH")    //0xE53
#byte B4EIDH = getenv("SFR:B4EIDH")    //0xE63
#byte B5EIDH = getenv("SFR:B5EIDH")    //0xE73

#byte B0EIDL = getenv("SFR:B0EIDL")    //0xE24
#byte B1EIDL = getenv("SFR:B1EIDL")    //0xE34
#byte B2EIDL = getenv("SFR:B2EIDL")    //0xE44
#byte B3EIDL = getenv("SFR:B3EIDL")    //0xE54
#byte B4EIDL = getenv("SFR:B4EIDL")    //0xE64
#byte B5EIDL = getenv("SFR:B5EIDL")    //0xE74

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Mask Select Registers ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum CAN_MASK_FILTER_ASSOCIATE{ ACCEPTANCE_MASK_0=0x00,
                                ACCEPTANCE_MASK_1=0x01,
                                FILTER_15=0x02,
                                NO_MASK=0x03};

//Msel0
struct {
    int fil0:2;     //0:1   //Filter zero select bits
    int fil1:2;     //2:3   //Filter one select bits
    int fil2:2;     //4:5   //Filter two select bits
    int fil3:2;     //6:7   //Filter three select bits
} msel0;

//Msel1
struct {
    int fil4:2;     //0:1   //Filter four select bits
    int fil5:2;     //2:3   //Filter five select bits
    int fil6:2;     //4:5   //Filter six select bits
    int fil7:2;     //6:7   //Filter seven select bits
} msel1;

//Msel2
struct {
    int fil8:2;     //0:1   //Filter eight select bits
    int fil9:2;     //2:3   //Filter nine select bits
    int fil10:2;    //4:5   //Filter ten select bits
    int fil11:2;    //6:7   //Filter eleven select bits
} msel2;

//Msel3
struct {
    int fil12:2;    //0:1   //Filter twelve select bits
    int fil13:2;    //2:3   //Filter thirteen select bits
    int fil14:2;    //4:5   //Filter fourteen select bits
    int fil15:2;    //6:7   //Filter fifteen select bits
} msel3;

#byte msel0 = getenv("SFR:MSEL0")   //0xDF0
#byte msel1 = getenv("SFR:MSEL1")   //0xDF1
#byte msel2 = getenv("SFR:MSEL2")   //0xDF2
#byte msel3 = getenv("SFR:MSEL3")   //0xDF3

////////////////////////////////////////////////////////////////////////////////
/////////////////// Transmit interrupts enable register ////////////////////////
////////////////////////////////////////////////////////////////////////////////

//Txbie
struct {
    int    void01:2;    //0:1
    int1   txb0ie;      //2     //Transmit buffer 0 interrupt enable bit
    int1   txb1ie;      //3     //Transmit buffer 1 interrupt enable bit
    int1    txb2ie;     //4     //Transmit buffer 2 interrupt enable bit
    int    void567;     //5:7
} txbie;

#byte txbie = getenv("SFR:TXBIE")   //0xDFC

////////////////////////////////////////////////////////////////////////////////
//////////////////// Buffer interrupt enable register //////////////////////////
////////////////////////////////////////////////////////////////////////////////

//Bie0
struct {
    int1 rxb0ie;    //0 //Dedicated recive buffer 0 interrupt enable bit
    int1 rxb1ie;    //1 //Dedicated recive buffer 1 interrupt enable bit
    int1 b0ie;      //2 //Programmable transmit/receive buffer 0 interrupt enable bit
    int1 b1ie;      //3 //Programmable transmit/receive buffer 1 interrupt enable bit
    int1 b2ie;      //4 //Programmable transmit/receive buffer 2 interrupt enable bit
    int1 b3ie;      //5 //Programmable transmit/receive buffer 3 interrupt enable bit
    int1 b4ie;      //6 //Programmable transmit/receive buffer 4 interrupt enable bit
    int1 b5ie;      //7 //Programmable transmit/receive buffer 5 interrupt enable bit
} bie0;

#byte bie0 = getenv("SFR:BIE0") //0xDFA

enum CAN_FILTER_CONTROL{RXF0EN=0x0001, RXF1EN=0x0002, RXF2EN=0x0004, RXF3EN=0x0008,
                        RXF4EN=0x0010, RXF5EN=0x0020, RXF6EN=0x0040, RXF7EN=0x0080,
                        RXF8EN=0x0100, RXF9EN=0x0200,RXF10EN=0x0400,RXF11EN=0x0800,
                        RXF12EN=0x1000,RXF13EN=0x2000,RXF14EN=0x4000,RXF15EN=0x8000};

//Recive filter control registers
#byte RXFCON0 = getenv("SFR:RXFCON0")   //0xDD4
#byte RXFCON1 = getenv("SFR:RXFCON1")   //0xDD5

//Standard data bytes filter length count register
#byte SDFLC = getenv("SFR:SDFLC")       //0xDD8

// Enumerated buffers and filters
enum CAN_FILTER_ASSOCIATION{F0BP=0x00,
                            F1BP=0x01,
                            F2BP=0x02,
                            F3BP=0x03,
                            F4BP=0x04,
                            F5BP=0x05,
                            F6BP=0x06,
                            F7BP=0x07,
                            F8BP=0x08,
                            F9BP=0x09,
                            F10BP=0x0A,
                            F11BP=0x0B,
                            F12BP=0x0C,
                            F13BP=0x0D,
                            F14BP=0x0E,
                            F15BP=0x0F};

enum CAN_FILTER_ASSOCIATION_BUFFERS {   ARXB0=0x00,
                                        ARXB1=0x01,
                                        AB0=0x02,
                                        AB1=0x03,
                                        AB2=0x04,
                                        AB3=0x05,
                                        AB4=0x06,
                                        AB5=0x07 };

//Recive filter biffer control registers
#byte RXFBCON0 = getenv("SFR:RXFBCON0")     //0xDE0
#byte RXFBCON1 = getenv("SFR:RXFBCON1")     //0xDE1
#byte RXFBCON2 = getenv("SFR:RXFBCON2")     //0xDE2
#byte RXFBCON3 = getenv("SFR:RXFBCON3")     //0xDE3
#byte RXFBCON4 = getenv("SFR:RXFBCON4")     //0xDE4
#byte RXFBCON5 = getenv("SFR:RXFBCON5")     //0xDE5
#byte RXFBCON6 = getenv("SFR:RXFBCON6")     //0xDE6
#byte RXFBCON7 = getenv("SFR:RXFBCON7")     //0xDE7

//Receive buffer n standard identifier
#byte   RXB0SIDH = getenv("SFR:RXB0SIDH")   //0xF61
#byte   RXB0SIDL = getenv("SFR:RXB0SIDH")   //0xF62
#byte   RXB1SIDH = getenv("SFR:RXB1SIDH")   //0xF51
#byte   RXB1SIDL = getenv("SFR:RXB0SIDL")   //0xF52

//Receive buffer n extended identifier
#byte   RXB0EIDH = getenv("SFR:RXB0EIDH")   //0xF63
#byte   RXB0EIDL = getenv("SFR:RXB0EIDL")   //0xF64
#byte   RXB1EIDH = getenv("SFR:RXB1EIDH")   //0xF53
#byte   RXB1EIDL = getenv("SFR:RXB1EIDL")   //0xF54

#byte TXRXBaEIDL = 0xF64

struct {
    int void012:3;  //0:2
    int1 ext;       //3     //Extendid id
    int1 srr;       //4     //Substitute remove request bit
    int void567:3;  //5:7
} TXRXBaSIDL;
#byte TXRXBaSIDL = 0xF62

//Receive buffer n data length code register
struct rxbNdlc_struct {
    int dlc:4;      //0:3   //Data length code
    int1 rb0;       //4     //Reserved
    int1 rb1;       //5     //Reserved
    int1 rtr;       //6     //Receiver remote transmission request bit
    int1 void7;     //7
};
struct rxbNdlc_struct RXB0DLC;
struct rxbNdlc_struct RXB1DLC;
struct rxbNdlc_struct RXBaDLC;
#byte   RXB0DLC = getenv("SFR:RXB0DLC")     //0xF65
#byte   RXB1DLC = getenv("SFR:RXB1DLC")     //0xF55
#byte   RXBaDLC = getenv("SFR:RXB0DLC")     //0xF65

//Receive buffer n data field byte m register
#byte RXB0D0 = getenv("SFR:RXB0D0")         //0xF66
#byte RXB0D7 = getenv("SFR:RXB0D7")         //0xF6D
#byte TXRXBaD0 = 0xF66
#byte TXRXBaD7 = 0xF6D

//Receive error count
#byte RXERRCNT = getenv("SFR:RXERRCNT")     //0xF75

//Receive acceptance filter n standard indifier
#byte RXF0SIDH = getenv("SFR:RXF0SIDH")     //0xF00
#byte RXF0SIDL = getenv("SFR:RXF0SIDL")     //0xF01
#byte RXF1SIDH = getenv("SFR:RXF1SIDH")     //0xF04
#byte RXF1SIDL = getenv("SFR:RXF1SIDL")     //0xF05
#byte RXF2SIDH = getenv("SFR:RXF2SIDH")     //0xF08
#byte RXF2SIDL = getenv("SFR:RXF2SIDL")     //0xF09
#byte RXF3SIDH = getenv("SFR:RXF3SIDH")     //0xF0C
#byte RXF3SIDL = getenv("SFR:RXF3SIDL")     //0xF0D
#byte RXF4SIDH = getenv("SFR:RXF4SIDH")     //0xF10
#byte RXF4SIDL = getenv("SFR:RXF4SIDL")     //0xF11
#byte RXF5SIDH = getenv("SFR:RXF5SIDH")     //0xF14
#byte RXF5SIDL = getenv("SFR:RXF5SIDL")     //0xF15

#byte RXF6SIDH = getenv("SFR:RXF6SIDH")     //0xD60
#byte RXF6SIDL = getenv("SFR:RXF6SIDL")     //0xD61
#byte RXF7SIDH = getenv("SFR:RXF7SIDH")     //0xD64
#byte RXF7SIDL = getenv("SFR:RXF7SIDL")     //0xD65
#byte RXF8SIDH = getenv("SFR:RXF8SIDH")     //0xD68
#byte RXF8SIDL = getenv("SFR:RXF8SIDL")     //0xD69
#byte RXF9SIDH = getenv("SFR:RXF9SIDH")     //0xD70
#byte RXF9SIDL = getenv("SFR:RXF9SIDL")     //0xD71
#byte RXF10SIDH = getenv("SFR:RXF10SIDH")   //0xD74
#byte RXF10SIDL = getenv("SFR:RXF10SIDL")   //0xD75
#byte RXF11SIDH = getenv("SFR:RXF11SIDH")   //0xD78
#byte RXF11SIDL = getenv("SFR:RXF11SIDL")   //0xD79
#byte RXF12SIDH = getenv("SFR:RXF12SIDH")   //0xD80
#byte RXF12SIDL = getenv("SFR:RXF12SIDL")   //0xD81
#byte RXF13SIDH = getenv("SFR:RXF13SIDH")   //0xD84
#byte RXF13SIDL = getenv("SFR:RXF13SIDL")   //0xD85
#byte RXF14SIDH = getenv("SFR:RXF14SIDH")   //0xD88
#byte RXF14SIDL = getenv("SFR:RXF14SIDL")   //0xD89
#byte RXF15SIDH = getenv("SFR:RXF15SIDH")   //0xD90
#byte RXF15SIDL = getenv("SFR:RXF15SIDL")   //0xD91

//Receive acceptance filter n extended indifier
#byte RXF0EIDH = getenv("SFR:RXF0EIDH")     //0xF02
#byte RXF0EIDL = getenv("SFR:RXF0EIDL")     //0xF03
#byte RXF1EIDH = getenv("SFR:RXF1EIDH")     //0xF06
#byte RXF1EIDL = getenv("SFR:RXF1EIDL")     //0xF07
#byte RXF2EIDH = getenv("SFR:RXF2EIDH")     //0xF0A
#byte RXF2EIDL = getenv("SFR:RXF2EIDL")     //0xF0B
#byte RXF3EIDH = getenv("SFR:RXF3EIDH")     //0xF0E
#byte RXF3EIDL = getenv("SFR:RXF3EIDL")     //0xF0F
#byte RXF4EIDH = getenv("SFR:RXF4EIDH")     //0xF12
#byte RXF4EIDL = getenv("SFR:RXF4EIDL")     //0xF13
#byte RXF5EIDH = getenv("SFR:RXF5EIDH")     //0xF16
#byte RXF5EIDL = getenv("SFR:RXF5EIDL")     //0xF17

#byte RXF6EIDH = getenv("SFR:RXF6EIDH")     //0xD62
#byte RXF6EIDL = getenv("SFR:RXF6EIDL")     //0xD63
#byte RXF7EIDH = getenv("SFR:RXF7EIDH")     //0xD66
#byte RXF7EIDL = getenv("SFR:RXF7EIDL")     //0xD67
#byte RXF8EIDH = getenv("SFR:RXF8EIDH")     //0xD6A
#byte RXF8EIDL = getenv("SFR:RXF8EIDL")     //0xD6B
#byte RXF9EIDH = getenv("SFR:RXF9EIDH")     //0xD72
#byte RXF9EIDL = getenv("SFR:RXF9EIDL")     //0xD73
#byte RXF10EIDH = getenv("SFR:RXF10EIDH")   //0xD76
#byte RXF10EIDL = getenv("SFR:RXF10EIDL")   //0xD77
#byte RXF11EIDH = getenv("SFR:RXF11EIDH")   //0xD7A
#byte RXF11EIDL = getenv("SFR:RXF11EIDL")   //0xD7B
#byte RXF12EIDH = getenv("SFR:RXF12EIDH")   //0xD82
#byte RXF12EIDL = getenv("SFR:RXF12EIDL")   //0xD83
#byte RXF13EIDH = getenv("SFR:RXF13EIDH")   //0xD86
#byte RXF13EIDL = getenv("SFR:RXF13EIDL")   //0xD87
#byte RXF14EIDH = getenv("SFR:RXF14EIDH")   //0xD8A
#byte RXF14EIDL = getenv("SFR:RXF14EIDL")   //0xD8B
#byte RXF15EIDH = getenv("SFR:RXF15EIDH")   //0xD92
#byte RXF15EIDL = getenv("SFR:RXF15EIDL")   //0xD93

// ???: Why are these define macros and all others are byte macros?

//Simple filter names
#define RXFILTER0 getenv("SFR:RXF0EIDL")    //0xF03
#define RXFILTER1 getenv("SFR:RXF1EIDL")    //0xF07
#define RXFILTER2 getenv("SFR:RXF2EIDL")    //0xF0B
#define RXFILTER3 getenv("SFR:RXF3EIDL")    //0xF0F
#define RXFILTER4 getenv("SFR:RXF4EIDL")    //0xF13
#define RXFILTER5 getenv("SFR:RXF5EIDL")    //0xF17
#define RXFILTER6 getenv("SFR:RXF6EIDL")    //0xD63
#define RXFILTER7 getenv("SFR:RXF7EIDL")    //0xD67
#define RXFILTER8 getenv("SFR:RXF8EIDL")    //0xD6B
#define RXFILTER9 getenv("SFR:RXF9EIDL")    //0xD73
#define RXFILTER10 getenv("SFR:RXF10EIDL")  //0xD77
#define RXFILTER11 getenv("SFR:RXF11EIDL")  //0xD7B
#define RXFILTER12 getenv("SFR:RXF12EIDL")  //0xD83
#define RXFILTER13 getenv("SFR:RXF13EIDL")  //0xD87
#define RXFILTER14 getenv("SFR:RXF14EIDL")  //0xD8B
#define RXFILTER15 getenv("SFR:RXF15EIDL")  //0xD93

//Receive acceptance mask n standard identifer mask
#byte RXM0SIDH = getenv("SFR:RXM0SIDH")     //0xF18
#byte RXM0SIDL = getenv("SFR:RXM0SIDL")     //0xF19
#byte RXM1SIDH = getenv("SFR:RXM1SIDH")     //0xF1C
#byte RXM1SIDL = getenv("SFR:RXM1SIDL")     //0xF1D

//Receive acceptance mask n extended identifer mask
#byte RXM0EIDH = getenv("SFR:RXM0EIDH")     //0xF1A
#byte RXM0EIDL = getenv("SFR:RXM0EIDL")     //0xF1B
#byte RXM1EIDH = getenv("SFR:RXM1EIDH")     //0xF1E
#byte RXM1EIDL = getenv("SFR:RXM1EIDL")     //0xF1F

//Value to put in mask field to accept all incoming id's
#define CAN_MASK_ACCEPT_ALL   0

//CAN interrupt flags
#bit CAN_INT_IRXIF = getenv("BIT:IRXIF")    //0xFA4.7
#bit CAN_INT_WAKIF = getenv("BIT:WAKIF")    //0xFA4.6
#bit CAN_INT_ERRIF = getenv("BIT:ERRIF")    //0xFA4.5
#bit CAN_INT_TXB2IF = getenv("BIT:TXB2IF")  //0xFA4.4
#bit CAN_INT_TXB1IF = getenv("BIT:TXB1IF")  //0xFA4.3
#bit CAN_INT_TXB0IF = getenv("BIT:TXB0IF")  //0xFA4.2
#bit CAN_INT_RXB1IF = getenv("BIT:RXB1IF")  //0xFA4.1
#bit CAN_INT_RXB0IF = getenv("BIT:RXB0IF")  //0xFA4.0

//PROTOTYPES

struct rx_stat {
    int1 err_ovfl;          // Buffer overflow
    unsigned int filthit;   // Fillter that allowed the frame into the buffer
    unsigned int buffer;    // Recieve buffer
    int1 rtr;               // Rtr requested
    int1 ext;               // Extended id
    int1 inv;               // Invalid id?
};

//MACROS
#define can_kbhit() (RXB0CON.rxful || RXB1CON.rxful || (B0CONR.rxful && !BSEL0.b0txen) || (B1CONR.rxful && !BSEL0.b1txen) || (B2CONR.rxful && !BSEL0.b2txen) || (B3CONR.rxful && !BSEL0.b3txen) || (B4CONR.rxful && !BSEL0.b4txen) || (B5CONR.rxful && !BSEL0.b5txen))
#define can_tbe()   (!TXB0CON.txreq || !TXB1CON.txreq || !TXB2CON.txreq || (!B0CONT.txreq && BSEL0.b0txen) || (!B1CONT.txreq && BSEL0.b1txen) || (!B2CONT.txreq && BSEL0.b2txen) || (!B3CONT.txreq && BSEL0.b3txen) || (!B4CONT.txreq && BSEL0.b4txen) || (!B5CONT.txreq && BSEL0.b5txen))
#define can_abort() (CANCON.abat=1)

void can_init(void);
void can_set_baud(void);
void can_set_mode(CAN_OP_MODE mode);
void can_set_functional_mode(CAN_FUN_OP_MODE mode);
void can_set_id(int8 *addr, int32 id, int1 ext);
int32 can_get_id(int8 *addr, int1 ext);
int8 can_putd(int32 id, int8 *data, int8 len, int8 priority, int1 ext, int1 rtr);
int1 can_getd(int32 &id, int8 *data, int8 &len, struct rx_stat &stat);
void can_enable_rtr(PROG_BUFFER b);
void can_disable_rtr(PROG_BUFFER b);
void can_load_rtr(PROG_BUFFER b, int8 *data, int8 len);
void can_enable_filter(int16 filter);
void can_disable_filter(int16 filter);
void can_associate_filter_to_buffer(CAN_FILTER_ASSOCIATION_BUFFERS buffer, CAN_FILTER_ASSOCIATION filter);
void can_associate_filter_to_mask(CAN_MASK_FILTER_ASSOCIATE mask, CAN_FILTER_ASSOCIATION filter);
int1 can_fifo_getd(int32 &id, int8 *data, int8 &len, struct rx_stat &stat);

#endif // __CCS_CAN18F4580_LIB_DEFINES__
