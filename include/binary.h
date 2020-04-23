/*---------------------------------------------------------------------------
PROJECT: Steem SSE
Atari ST emulator
Copyright (C) 2020 by Anthony Hayward and Russel Hayward + SSE

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see https://www.gnu.org/licenses/.

DOMAIN: Emu
FILE: binary.h
DESCRIPTION: Macros for binary values.
---------------------------------------------------------------------------*/

#pragma once
#ifndef BINARY_H
#define BINARY_H

#define b00000000 (BYTE) 0x0
#define b00000001 (BYTE) 0x1
#define b00000010 (BYTE) 0x2
#define b00000011 (BYTE) 0x3
#define b00000100 (BYTE) 0x4
#define b00000101 (BYTE) 0x5
#define b00000110 (BYTE) 0x6
#define b00000111 (BYTE) 0x7
#define b00001000 (BYTE) 0x8
#define b00001001 (BYTE) 0x9
#define b00001010 (BYTE) 0xA
#define b00001011 (BYTE) 0xB
#define b00001100 (BYTE) 0xC
#define b00001101 (BYTE) 0xD
#define b00001110 (BYTE) 0xE
#define b00001111 (BYTE) 0xF
#define b00010000 (BYTE) 0x10
#define b00010001 (BYTE) 0x11
#define b00010010 (BYTE) 0x12
#define b00010011 (BYTE) 0x13
#define b00010100 (BYTE) 0x14
#define b00010101 (BYTE) 0x15
#define b00010110 (BYTE) 0x16
#define b00010111 (BYTE) 0x17
#define b00011000 (BYTE) 0x18
#define b00011001 (BYTE) 0x19
#define b00011010 (BYTE) 0x1A
#define b00011011 (BYTE) 0x1B
#define b00011100 (BYTE) 0x1C
#define b00011101 (BYTE) 0x1D
#define b00011110 (BYTE) 0x1E
#define b00011111 (BYTE) 0x1F
#define b00100000 (BYTE) 0x20
#define b00100001 (BYTE) 0x21
#define b00100010 (BYTE) 0x22
#define b00100011 (BYTE) 0x23
#define b00100100 (BYTE) 0x24
#define b00100101 (BYTE) 0x25
#define b00100110 (BYTE) 0x26
#define b00100111 (BYTE) 0x27
#define b00101000 (BYTE) 0x28
#define b00101001 (BYTE) 0x29
#define b00101010 (BYTE) 0x2A
#define b00101011 (BYTE) 0x2B
#define b00101100 (BYTE) 0x2C
#define b00101101 (BYTE) 0x2D
#define b00101110 (BYTE) 0x2E
#define b00101111 (BYTE) 0x2F
#define b00110000 (BYTE) 0x30
#define b00110001 (BYTE) 0x31
#define b00110010 (BYTE) 0x32
#define b00110011 (BYTE) 0x33
#define b00110100 (BYTE) 0x34
#define b00110101 (BYTE) 0x35
#define b00110110 (BYTE) 0x36
#define b00110111 (BYTE) 0x37
#define b00111000 (BYTE) 0x38
#define b00111001 (BYTE) 0x39
#define b00111010 (BYTE) 0x3A
#define b00111011 (BYTE) 0x3B
#define b00111100 (BYTE) 0x3C
#define b00111101 (BYTE) 0x3D
#define b00111110 (BYTE) 0x3E
#define b00111111 (BYTE) 0x3F
#define b01000000 (BYTE) 0x40
#define b01000001 (BYTE) 0x41
#define b01000010 (BYTE) 0x42
#define b01000011 (BYTE) 0x43
#define b01000100 (BYTE) 0x44
#define b01000101 (BYTE) 0x45
#define b01000110 (BYTE) 0x46
#define b01000111 (BYTE) 0x47
#define b01001000 (BYTE) 0x48
#define b01001001 (BYTE) 0x49
#define b01001010 (BYTE) 0x4A
#define b01001011 (BYTE) 0x4B
#define b01001100 (BYTE) 0x4C
#define b01001101 (BYTE) 0x4D
#define b01001110 (BYTE) 0x4E
#define b01001111 (BYTE) 0x4F
#define b01010000 (BYTE) 0x50
#define b01010001 (BYTE) 0x51
#define b01010010 (BYTE) 0x52
#define b01010011 (BYTE) 0x53
#define b01010100 (BYTE) 0x54
#define b01010101 (BYTE) 0x55
#define b01010110 (BYTE) 0x56
#define b01010111 (BYTE) 0x57
#define b01011000 (BYTE) 0x58
#define b01011001 (BYTE) 0x59
#define b01011010 (BYTE) 0x5A
#define b01011011 (BYTE) 0x5B
#define b01011100 (BYTE) 0x5C
#define b01011101 (BYTE) 0x5D
#define b01011110 (BYTE) 0x5E
#define b01011111 (BYTE) 0x5F
#define b01100000 (BYTE) 0x60
#define b01100001 (BYTE) 0x61
#define b01100010 (BYTE) 0x62
#define b01100011 (BYTE) 0x63
#define b01100100 (BYTE) 0x64
#define b01100101 (BYTE) 0x65
#define b01100110 (BYTE) 0x66
#define b01100111 (BYTE) 0x67
#define b01101000 (BYTE) 0x68
#define b01101001 (BYTE) 0x69
#define b01101010 (BYTE) 0x6A
#define b01101011 (BYTE) 0x6B
#define b01101100 (BYTE) 0x6C
#define b01101101 (BYTE) 0x6D
#define b01101110 (BYTE) 0x6E
#define b01101111 (BYTE) 0x6F
#define b01110000 (BYTE) 0x70
#define b01110001 (BYTE) 0x71
#define b01110010 (BYTE) 0x72
#define b01110011 (BYTE) 0x73
#define b01110100 (BYTE) 0x74
#define b01110101 (BYTE) 0x75
#define b01110110 (BYTE) 0x76
#define b01110111 (BYTE) 0x77
#define b01111000 (BYTE) 0x78
#define b01111001 (BYTE) 0x79
#define b01111010 (BYTE) 0x7A
#define b01111011 (BYTE) 0x7B
#define b01111100 (BYTE) 0x7C
#define b01111101 (BYTE) 0x7D
#define b01111110 (BYTE) 0x7E
#define b01111111 (BYTE) 0x7F
#define b10000000 (BYTE) 0x80
#define b10000001 (BYTE) 0x81
#define b10000010 (BYTE) 0x82
#define b10000011 (BYTE) 0x83
#define b10000100 (BYTE) 0x84
#define b10000101 (BYTE) 0x85
#define b10000110 (BYTE) 0x86
#define b10000111 (BYTE) 0x87
#define b10001000 (BYTE) 0x88
#define b10001001 (BYTE) 0x89
#define b10001010 (BYTE) 0x8A
#define b10001011 (BYTE) 0x8B
#define b10001100 (BYTE) 0x8C
#define b10001101 (BYTE) 0x8D
#define b10001110 (BYTE) 0x8E
#define b10001111 (BYTE) 0x8F
#define b10010000 (BYTE) 0x90
#define b10010001 (BYTE) 0x91
#define b10010010 (BYTE) 0x92
#define b10010011 (BYTE) 0x93
#define b10010100 (BYTE) 0x94
#define b10010101 (BYTE) 0x95
#define b10010110 (BYTE) 0x96
#define b10010111 (BYTE) 0x97
#define b10011000 (BYTE) 0x98
#define b10011001 (BYTE) 0x99
#define b10011010 (BYTE) 0x9A
#define b10011011 (BYTE) 0x9B
#define b10011100 (BYTE) 0x9C
#define b10011101 (BYTE) 0x9D
#define b10011110 (BYTE) 0x9E
#define b10011111 (BYTE) 0x9F
#define b10100000 (BYTE) 0xA0
#define b10100001 (BYTE) 0xA1
#define b10100010 (BYTE) 0xA2
#define b10100011 (BYTE) 0xA3
#define b10100100 (BYTE) 0xA4
#define b10100101 (BYTE) 0xA5
#define b10100110 (BYTE) 0xA6
#define b10100111 (BYTE) 0xA7
#define b10101000 (BYTE) 0xA8
#define b10101001 (BYTE) 0xA9
#define b10101010 (BYTE) 0xAA
#define b10101011 (BYTE) 0xAB
#define b10101100 (BYTE) 0xAC
#define b10101101 (BYTE) 0xAD
#define b10101110 (BYTE) 0xAE
#define b10101111 (BYTE) 0xAF
#define b10110000 (BYTE) 0xB0
#define b10110001 (BYTE) 0xB1
#define b10110010 (BYTE) 0xB2
#define b10110011 (BYTE) 0xB3
#define b10110100 (BYTE) 0xB4
#define b10110101 (BYTE) 0xB5
#define b10110110 (BYTE) 0xB6
#define b10110111 (BYTE) 0xB7
#define b10111000 (BYTE) 0xB8
#define b10111001 (BYTE) 0xB9
#define b10111010 (BYTE) 0xBA
#define b10111011 (BYTE) 0xBB
#define b10111100 (BYTE) 0xBC
#define b10111101 (BYTE) 0xBD
#define b10111110 (BYTE) 0xBE
#define b10111111 (BYTE) 0xBF
#define b11000000 (BYTE) 0xC0
#define b11000001 (BYTE) 0xC1
#define b11000010 (BYTE) 0xC2
#define b11000011 (BYTE) 0xC3
#define b11000100 (BYTE) 0xC4
#define b11000101 (BYTE) 0xC5
#define b11000110 (BYTE) 0xC6
#define b11000111 (BYTE) 0xC7
#define b11001000 (BYTE) 0xC8
#define b11001001 (BYTE) 0xC9
#define b11001010 (BYTE) 0xCA
#define b11001011 (BYTE) 0xCB
#define b11001100 (BYTE) 0xCC
#define b11001101 (BYTE) 0xCD
#define b11001110 (BYTE) 0xCE
#define b11001111 (BYTE) 0xCF
#define b11010000 (BYTE) 0xD0
#define b11010001 (BYTE) 0xD1
#define b11010010 (BYTE) 0xD2
#define b11010011 (BYTE) 0xD3
#define b11010100 (BYTE) 0xD4
#define b11010101 (BYTE) 0xD5
#define b11010110 (BYTE) 0xD6
#define b11010111 (BYTE) 0xD7
#define b11011000 (BYTE) 0xD8
#define b11011001 (BYTE) 0xD9
#define b11011010 (BYTE) 0xDA
#define b11011011 (BYTE) 0xDB
#define b11011100 (BYTE) 0xDC
#define b11011101 (BYTE) 0xDD
#define b11011110 (BYTE) 0xDE
#define b11011111 (BYTE) 0xDF
#define b11100000 (BYTE) 0xE0
#define b11100001 (BYTE) 0xE1
#define b11100010 (BYTE) 0xE2
#define b11100011 (BYTE) 0xE3
#define b11100100 (BYTE) 0xE4
#define b11100101 (BYTE) 0xE5
#define b11100110 (BYTE) 0xE6
#define b11100111 (BYTE) 0xE7
#define b11101000 (BYTE) 0xE8
#define b11101001 (BYTE) 0xE9
#define b11101010 (BYTE) 0xEA
#define b11101011 (BYTE) 0xEB
#define b11101100 (BYTE) 0xEC
#define b11101101 (BYTE) 0xED
#define b11101110 (BYTE) 0xEE
#define b11101111 (BYTE) 0xEF
#define b11110000 (BYTE) 0xF0
#define b11110001 (BYTE) 0xF1
#define b11110010 (BYTE) 0xF2
#define b11110011 (BYTE) 0xF3
#define b11110100 (BYTE) 0xF4
#define b11110101 (BYTE) 0xF5
#define b11110110 (BYTE) 0xF6
#define b11110111 (BYTE) 0xF7
#define b11111000 (BYTE) 0xF8
#define b11111001 (BYTE) 0xF9
#define b11111010 (BYTE) 0xFA
#define b11111011 (BYTE) 0xFB
#define b11111100 (BYTE) 0xFC
#define b11111101 (BYTE) 0xFD
#define b11111110 (BYTE) 0xFE
#define b11111111 (BYTE) 0xFF

#define b0000 0x0
#define b0001 0x1
#define b0010 0x2
#define b0011 0x3
#define b0100 0x4
#define b0101 0x5
#define b0110 0x6
#define b0111 0x7
#define b1000 0x8
#define b1001 0x9
#define b1010 0xA
#define b1011 0xB
#define b1100 0xC
#define b1101 0xD
#define b1110 0xE
#define b1111 0xF

#define hib00000000 0x000
#define hib00000001 0x100
#define hib00000010 0x200
#define hib00000011 0x300
#define hib00000100 0x400
#define hib00000101 0x500
#define hib00000110 0x600
#define hib00000111 0x700
#define hib00001000 0x800
#define hib00001001 0x900
#define hib00001010 0xA00
#define hib00001011 0xB00
#define hib00001100 0xC00
#define hib00001101 0xD00
#define hib00001110 0xE00
#define hib00001111 0xF00
#define hib00010000 0x1000
#define hib00010001 0x1100
#define hib00010010 0x1200
#define hib00010011 0x1300
#define hib00010100 0x1400
#define hib00010101 0x1500
#define hib00010110 0x1600
#define hib00010111 0x1700
#define hib00011000 0x1800
#define hib00011001 0x1900
#define hib00011010 0x1A00
#define hib00011011 0x1B00
#define hib00011100 0x1C00
#define hib00011101 0x1D00
#define hib00011110 0x1E00
#define hib00011111 0x1F00
#define hib00100000 0x2000
#define hib00100001 0x2100
#define hib00100010 0x2200
#define hib00100011 0x2300
#define hib00100100 0x2400
#define hib00100101 0x2500
#define hib00100110 0x2600
#define hib00100111 0x2700
#define hib00101000 0x2800
#define hib00101001 0x2900
#define hib00101010 0x2A00
#define hib00101011 0x2B00
#define hib00101100 0x2C00
#define hib00101101 0x2D00
#define hib00101110 0x2E00
#define hib00101111 0x2F00
#define hib00110000 0x3000
#define hib00110001 0x3100
#define hib00110010 0x3200
#define hib00110011 0x3300
#define hib00110100 0x3400
#define hib00110101 0x3500
#define hib00110110 0x3600
#define hib00110111 0x3700
#define hib00111000 0x3800
#define hib00111001 0x3900
#define hib00111010 0x3A00
#define hib00111011 0x3B00
#define hib00111100 0x3C00
#define hib00111101 0x3D00
#define hib00111110 0x3E00
#define hib00111111 0x3F00
#define hib01000000 0x4000
#define hib01000001 0x4100
#define hib01000010 0x4200
#define hib01000011 0x4300
#define hib01000100 0x4400
#define hib01000101 0x4500
#define hib01000110 0x4600
#define hib01000111 0x4700
#define hib01001000 0x4800
#define hib01001001 0x4900
#define hib01001010 0x4A00
#define hib01001011 0x4B00
#define hib01001100 0x4C00
#define hib01001101 0x4D00
#define hib01001110 0x4E00
#define hib01001111 0x4F00
#define hib01010000 0x5000
#define hib01010001 0x5100
#define hib01010010 0x5200
#define hib01010011 0x5300
#define hib01010100 0x5400
#define hib01010101 0x5500
#define hib01010110 0x5600
#define hib01010111 0x5700
#define hib01011000 0x5800
#define hib01011001 0x5900
#define hib01011010 0x5A00
#define hib01011011 0x5B00
#define hib01011100 0x5C00
#define hib01011101 0x5D00
#define hib01011110 0x5E00
#define hib01011111 0x5F00
#define hib01100000 0x6000
#define hib01100001 0x6100
#define hib01100010 0x6200
#define hib01100011 0x6300
#define hib01100100 0x6400
#define hib01100101 0x6500
#define hib01100110 0x6600
#define hib01100111 0x6700
#define hib01101000 0x6800
#define hib01101001 0x6900
#define hib01101010 0x6A00
#define hib01101011 0x6B00
#define hib01101100 0x6C00
#define hib01101101 0x6D00
#define hib01101110 0x6E00
#define hib01101111 0x6F00
#define hib01110000 0x7000
#define hib01110001 0x7100
#define hib01110010 0x7200
#define hib01110011 0x7300
#define hib01110100 0x7400
#define hib01110101 0x7500
#define hib01110110 0x7600
#define hib01110111 0x7700
#define hib01111000 0x7800
#define hib01111001 0x7900
#define hib01111010 0x7A00
#define hib01111011 0x7B00
#define hib01111100 0x7C00
#define hib01111101 0x7D00
#define hib01111110 0x7E00
#define hib01111111 0x7F00
#define hib10000000 0x8000
#define hib10000001 0x8100
#define hib10000010 0x8200
#define hib10000011 0x8300
#define hib10000100 0x8400
#define hib10000101 0x8500
#define hib10000110 0x8600
#define hib10000111 0x8700
#define hib10001000 0x8800
#define hib10001001 0x8900
#define hib10001010 0x8A00
#define hib10001011 0x8B00
#define hib10001100 0x8C00
#define hib10001101 0x8D00
#define hib10001110 0x8E00
#define hib10001111 0x8F00
#define hib10010000 0x9000
#define hib10010001 0x9100
#define hib10010010 0x9200
#define hib10010011 0x9300
#define hib10010100 0x9400
#define hib10010101 0x9500
#define hib10010110 0x9600
#define hib10010111 0x9700
#define hib10011000 0x9800
#define hib10011001 0x9900
#define hib10011010 0x9A00
#define hib10011011 0x9B00
#define hib10011100 0x9C00
#define hib10011101 0x9D00
#define hib10011110 0x9E00
#define hib10011111 0x9F00
#define hib10100000 0xA000
#define hib10100001 0xA100
#define hib10100010 0xA200
#define hib10100011 0xA300
#define hib10100100 0xA400
#define hib10100101 0xA500
#define hib10100110 0xA600
#define hib10100111 0xA700
#define hib10101000 0xA800
#define hib10101001 0xA900
#define hib10101010 0xAA00
#define hib10101011 0xAB00
#define hib10101100 0xAC00
#define hib10101101 0xAD00
#define hib10101110 0xAE00
#define hib10101111 0xAF00
#define hib10110000 0xB000
#define hib10110001 0xB100
#define hib10110010 0xB200
#define hib10110011 0xB300
#define hib10110100 0xB400
#define hib10110101 0xB500
#define hib10110110 0xB600
#define hib10110111 0xB700
#define hib10111000 0xB800
#define hib10111001 0xB900
#define hib10111010 0xBA00
#define hib10111011 0xBB00
#define hib10111100 0xBC00
#define hib10111101 0xBD00
#define hib10111110 0xBE00
#define hib10111111 0xBF00
#define hib11000000 0xC000
#define hib11000001 0xC100
#define hib11000010 0xC200
#define hib11000011 0xC300
#define hib11000100 0xC400
#define hib11000101 0xC500
#define hib11000110 0xC600
#define hib11000111 0xC700
#define hib11001000 0xC800
#define hib11001001 0xC900
#define hib11001010 0xCA00
#define hib11001011 0xCB00
#define hib11001100 0xCC00
#define hib11001101 0xCD00
#define hib11001110 0xCE00
#define hib11001111 0xCF00
#define hib11010000 0xD000
#define hib11010001 0xD100
#define hib11010010 0xD200
#define hib11010011 0xD300
#define hib11010100 0xD400
#define hib11010101 0xD500
#define hib11010110 0xD600
#define hib11010111 0xD700
#define hib11011000 0xD800
#define hib11011001 0xD900
#define hib11011010 0xDA00
#define hib11011011 0xDB00
#define hib11011100 0xDC00
#define hib11011101 0xDD00
#define hib11011110 0xDE00
#define hib11011111 0xDF00
#define hib11100000 0xE000
#define hib11100001 0xE100
#define hib11100010 0xE200
#define hib11100011 0xE300
#define hib11100100 0xE400
#define hib11100101 0xE500
#define hib11100110 0xE600
#define hib11100111 0xE700
#define hib11101000 0xE800
#define hib11101001 0xE900
#define hib11101010 0xEA00
#define hib11101011 0xEB00
#define hib11101100 0xEC00
#define hib11101101 0xED00
#define hib11101110 0xEE00
#define hib11101111 0xEF00
#define hib11110000 0xF000
#define hib11110001 0xF100
#define hib11110010 0xF200
#define hib11110011 0xF300
#define hib11110100 0xF400
#define hib11110101 0xF500
#define hib11110110 0xF600
#define hib11110111 0xF700
#define hib11111000 0xF800
#define hib11111001 0xF900
#define hib11111010 0xFA00
#define hib11111011 0xFB00
#define hib11111100 0xFC00
#define hib11111101 0xFD00
#define hib11111110 0xFE00
#define hib11111111 0xFF00
#define hib0000 0x000
#define hib0001 0x100
#define hib0010 0x200
#define hib0011 0x300
#define hib0100 0x400
#define hib0101 0x500
#define hib0110 0x600
#define hib0111 0x700
#define hib1000 0x800
#define hib1001 0x900
#define hib1010 0xA00
#define hib1011 0xB00
#define hib1100 0xC00
#define hib1101 0xD00
#define hib1110 0xE00
#define hib1111 0xF00

#define BIT_0 0x1
#define BIT_1 0x2
#define BIT_2 0x4
#define BIT_3 0x8
#define BIT_4 0x10
#define BIT_5 0x20
#define BIT_6 0x40
#define BIT_7 0x80
#define BIT_8 0x100
#define BIT_9 0x200
#define BIT_a 0x400
#define BIT_b 0x800
#define BIT_c 0x1000
#define BIT_d 0x2000
#define BIT_e 0x4000
#define BIT_f 0x8000
#define BIT_10 0x400
#define BIT_11 0x800
#define BIT_12 0x1000
#define BIT_13 0x2000
#define BIT_14 0x4000
#define BIT_15 0x8000

#define BIT_16 0x00010000
#define BIT_17 0x00020000
#define BIT_18 0x00040000
#define BIT_19 0x00080000
#define BIT_20 0x00100000
#define BIT_21 0x00200000
#define BIT_22 0x00400000
#define BIT_23 0x00800000
#define BIT_24 0x01000000
#define BIT_25 0x02000000
#define BIT_26 0x04000000
#define BIT_27 0x08000000
#define BIT_28 0x10000000
#define BIT_29 0x20000000
#define BIT_30 0x40000000
#define BIT_31 0x80000000

#endif//#ifndef BINARY_H
