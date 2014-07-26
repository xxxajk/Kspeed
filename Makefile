# Select a board
#BOARD = teensy3
#BOARD = teensy31
BOARD = K22-64-0K-1M-128K-DSP-FPU-USB

# Select a speed
#F_CPU = 168000000
#F_CPU = 144000000
F_CPU = 120000000
#F_CPU = 96000000
#F_CPU = 72000000
#F_CPU = 48000000
#F_CPU = 24000000
#F_CPU = 16000000
#F_CPU = 8000000
#F_CPU = 4000000
#F_CPU = 2000000

# Console UART port
CPORT = /dev/ttyUSB0

# Upload UART port (if needed)
#PORT = /dev/ttyUSB1
PORT=localhost:4242

# And finally, the part that brings everything together for you.
include ../Arduino_Makefile_master/_Makefile.master
