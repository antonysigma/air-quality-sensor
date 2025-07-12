//----------------------------------------------------------------
// DSU (Digital Signal Co-processor) Register Definition
//----------------------------------------------------------------
.equ    DSCR,   0x00
.equ    DSUEN,  7   
.equ    MM,     6   
.equ    DSD1,   5   
.equ    DSD0,   4   
.equ    DSN,    2   
.equ    DSZ,    1   
.equ    DSC,    0   

.equ    DSIR,   0x01
.equ    DSSD,   0x02

.equ    DSDX,   0x10
.equ    DSDY,   0x11

.equ    DSAL,   0x38
.equ    DSAH,   0x39

//----------------------------------------------------------------
// DSU (Digital Signal Co-processor) opcodes
//----------------------------------------------------------------
// multiply
.equ    XMULUU, 0b01000100
.equ    XMULSS, 0b01110100
.equ    XMULSU, 0b01100100

// multiply add
.equ    XMADUU, 0b01000110
.equ    XMADSS, 0b01110110
.equ    XMADSU, 0b01100110

// clear accumulator
.equ	CLRA,	0b10000000

// shift accumulator (add/or with shift number)
.equ	SHRA,	0b11000000
.equ	SHLA,	0b11010000
.equ	SALA,	0b11110000
