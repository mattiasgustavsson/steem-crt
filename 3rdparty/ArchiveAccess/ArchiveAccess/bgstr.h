///////////////////////////////////////////////////////////////////////////////
// bgstr.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// Template versions of strlen, strlcpy, strlcat functions allow for source, 
// target to be different from char*. All functions respect maximum string
// lengths
///////////////////////////////////////////////////////////////////////////////

#ifndef bgstr__h
#define bgstr__h

///////////////////////////////////////////////////////////////////////////////
// bgstrlen, version with MaxChars

template<typename S>
int bgstrlen (S* a, unsigned int MaxChars)
{
	unsigned int i = 0;

	// Find end of a 
	while (i < MaxChars-1 && a[i] != 0) {
		i++;
	}

   return i;
}

///////////////////////////////////////////////////////////////////////////////
// bgstrlen, version without MaxChars

template<typename S>
int bgstrlen (S* a)
{
	unsigned int i = 0;

	// Find end of a 
	while (a[i] != 0) {
		i++;
	}

   return i;
}


///////////////////////////////////////////////////////////////////////////////
// bgstrlcpy

template<typename S, typename T>
void bgstrlcpy (T* target, const S* source, unsigned int MaxChars)
{
	unsigned int i = 0;
	while (i < MaxChars-1 && source[i] != 0) {
		target[i] = (T) source[i];
		i++;
	}
	target[i] = 0;
}

///////////////////////////////////////////////////////////////////////////////
// bgstrlcat

template<typename S, typename T>
void bgstrlcat (T* a, const S* b, unsigned int MaxChars)
{
	unsigned int i = 0;
	unsigned j = 0;

	// Find end of a 
	while (i < MaxChars-1 && a[i] != 0) {
		i++;
	}

	// Append b to a
	while (i+j < MaxChars-1 && b[j] != 0) {
		a[i+j] = b[j];
		j++;
	}

	// Null-Terminate result
	a[i+j] = 0;
}

///////////////////////////////////////////////////////////////////////////////

template<typename S>
size_t bgReverseStrScan (const S* str, size_t start, S c)
{
	size_t i = start;
	while (i <= start)
		if (str[i] == c)
			return i; // Found c
		else
			--i;

	return start+1; // c not found
}

#endif
