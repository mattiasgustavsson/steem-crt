#pragma once
#ifndef DDERR_MEANING_H
#define DDERR_MEANING_H

unsigned short UNMAKE_DDHRESULT(long code);
int DDGetErrorDescription(HRESULT Error,char *buf,int size);

#endif//#ifndef DDERR_MEANING_H
