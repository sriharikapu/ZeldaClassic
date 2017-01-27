
#include "Utility.h"

#include <time.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>



void SimpleTimer::Start()
{
	startTime = clock();
}


SMinuteSecondsInfo SimpleTimer::GetElapsedTime()
{
	long finishTime = clock();
	double elapsedSeconds = (finishTime - startTime) / (double)CLOCKS_PER_SEC;

	SMinuteSecondsInfo result = {
		int32(elapsedSeconds / 60.0),
		float(fmod(elapsedSeconds, 60.0))
	};

	return result;
}


u32 Strlen(const char* str)
{
	return (u32)(str ? strlen(str) : 0);
}


u32 FindFirstWhitespaceOrLE(const char* str, u32 stringLength, u32 position)
{
	for(u32 i(position); i < stringLength; ++i)
	{
		const char c = str[i];

		if(c == ' ' || c == '\t' || c == 10 || c == 13)
			return i;
	}

	return u32(-1);
}


u32 FindFirstNonWhitespaceOrLE(const char* str, u32 stringLength, u32 position)
{
	u32 i(position);
	for( ; i < stringLength; ++i)
	{
		const char c = str[i];

		if(c == ' ' || c == '\t' || c == 10 || c == 13)
			;
		else break;
	}

	return i;
}


bool SaveMemoryToFileWithMode(const char* filename, const void* data, u32 size, const char* mode)
{
	bool result = false;

	if(filename && data)
	{
		FILE* f = fopen(filename, mode);
		if(f)
		{
			fwrite(data, 1, size, f);
			fclose(f);

			result = true;
		}
		else
		{
			printf("Error, could not create or open file \"%s\".", filename);
		}
	}

	return result;
}


bool SaveMemoryToFile(const char* filename, const void* data, u32 size)
{
	return SaveMemoryToFileWithMode(filename, data, size, "wb");
}


bool SaveStringToFile(const char* filename, const char* str, u32 stringLength)
{
	return SaveMemoryToFileWithMode(filename, str, stringLength * sizeof(char), "w");
}


void* LoadFileIntoMemory(const char* filename, u32& filesize, bool addTerminatingNull /*= true*/)
{
	void* buffer = NULL;

	if(filename)
	{
		FILE* f = fopen(filename, "rb");
		if(f)
		{
			fseek(f, 0, SEEK_END);
			u32 actualFileSize = ftell(f);
			fseek(f, 0, SEEK_SET);

			u32 bufferSize = actualFileSize + (addTerminatingNull ? 1 : 0);
			buffer = malloc(bufferSize);

			if(buffer)
			{
				fread(buffer, 1, actualFileSize, f);
				fclose(f);

				if(addTerminatingNull)
					((u8*)buffer)[actualFileSize] = 0; //null terminate

				filesize = bufferSize;
			}
		}
	}

	return buffer;
}


u32 StringNormalizeLineEndingsCRLF(char* RESTRICT buffer, const char* RESTRICT str, u32 stringLength)
{
	char* RESTRICT p = buffer;
	for(u32 i(0); (*p = *str++) && (i < stringLength); ++p, ++i)
	{
		if(*p == 13)
		{
			*++p = 10;
			if(*str == 10)
			{
				++str;
				++i;
			}
		}
		else if(*p == 10)
		{
			*p = 13;
			*++p = 10;
		}
	}

	return u32(p - buffer);
}


u32 Itoa(u32 value, char* bufptr)
{
	char buffer[16];
	char* p = buffer;

	do
	{
		*p++ = (value % 10) + '0';
		value /= 10;
	}
	while(value != 0);

	char* begin = bufptr;
	while(p-- != buffer)
		*bufptr++ = *p;

	*bufptr = 0; //null terminate

	return u32(bufptr - begin);
}


u32 Itoa(int32 value, char* bufptr)
{
	char* p = bufptr;
	if(value < 0)
	{
		value = -value;
		*p++ = '-';
	}

	return Itoa((u32)value, p);
}


u32 Dtoa(double value, char* bufptr)
{
	char buffer[48];

	//nan
	if(value != value)
	{
		bufptr[0] = '0';
		bufptr[1] = 0;
		return (u32)2;
	}

	int32 neg = 0;
	if(value < 0.0)
		neg = 1, value = -value;

	int32 whole = (int32)value;
	int32 frac = (int32)((value - (double)whole) * 1000000.0);
	char* p = buffer;

	//exponent
	if(value > (float)(0x07FFFFFF))
	{
		*p++ = 'e';
		*p++ = neg ? '-' : '+';

		int32 m = (int32)log10f((float)value);
		while(m > 0)
		{
			*p++ = '0' + m % 10;
			m /= 10;
			m++;
		}
	}
	else
	{
		//decimal
		if(frac != 0)
		{
			while(frac && !(frac % 10))
				frac /= 10;

			do *p++ = (char)('0' + (frac % 10)); while(frac /= 10);
			*p++ = '.';
		}

		//whole
		do *p++ = (char)('0' + (whole % 10)); while(whole /= 10);
		if(neg) *p++ = '-';
	}

	char* begin = bufptr;
	while(p-- != buffer)
		*bufptr++ = *p;

	*bufptr = 0; //null terminate

	return (u32)(bufptr - begin);
}


