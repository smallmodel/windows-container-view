#pragma once

/*
extern OnBeginImageTransmission BeginImageTransmission;
extern OnReceivedImageData ReceivedImageData;
extern OnEndImageTransmission EndImageTransmission;
*/

void BeginImageTransmission(unsigned long Width, unsigned long Height, unsigned long Size);
void ReceivedImageData(unsigned long Length, const unsigned char* BitmapData);
void EndImageTransmission();
void InitVideo();
