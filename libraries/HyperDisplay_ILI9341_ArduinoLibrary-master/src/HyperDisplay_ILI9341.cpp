#include "HyperDisplay_ILI9341.h"

#define ARDUINO_STILL_BROKEN 1 // Referring to the epic fail that is SPI.transfer(buf, len)


ILI9341::ILI9341(uint8_t xSize, uint8_t ySize, ILI9341_INTFC_t intfc ) : hyperdisplay(xSize, ySize)
{
	_intfc = intfc;
}

ILI9341_color_18_t ILI9341::hsvTo18b( uint16_t h, uint8_t s, uint8_t v ){
	uint8_t r; 
	uint8_t g;
	uint8_t b;
	fast_hsv2rgb_8bit(h, s, v, &r, &g , &b);
	return rgbTo18b( r, g, b );
}
ILI9341_color_16_t ILI9341::hsvTo16b( uint16_t h, uint8_t s, uint8_t v ){
	uint8_t r; 
	uint8_t g;
	uint8_t b;
	fast_hsv2rgb_8bit(h, s, v, &r, &g , &b);
	return rgbTo16b( r, g, b );
}

ILI9341_color_18_t ILI9341::rgbTo18b( uint8_t r, uint8_t g, uint8_t b ){
	ILI9341_color_18_t retval;
	retval.r = r;
	retval.g = g;
	retval.b = b;
	return retval;
}

ILI9341_color_16_t ILI9341::rgbTo16b( uint8_t r, uint8_t g, uint8_t b ){
	ILI9341_color_16_t retval;
	retval.rgh = ((r & 0xF8) | ( g >> 5));
	retval.glb = (((g & 0x1C) << 3) | (b >> 3));
	return retval;
}




uint8_t ILI9341::getBytesPerPixel( void )
{
	uint8_t bpp = 0;
	switch(_pxlfmt)
	{
		case ILI9341_PXLFMT_18 :
			bpp = offsetof( ILI9341_color_18_t, b) + 1;
			break;

		case ILI9341_PXLFMT_16 :
			bpp = offsetof( ILI9341_color_16_t, glb) + 1;
			break;

		default :
			break;
	}
	return bpp;
}


// Pure virtual functions from HyperDisplay Implemented:
color_t ILI9341::getOffsetColor(color_t base, uint32_t numPixels)
{
	switch(_pxlfmt)
	{
		case ILI9341_PXLFMT_18 :
			return (color_t)((( ILI9341_color_18_t*)base) + numPixels );
			break;

		case ILI9341_PXLFMT_16 :
			return (color_t)((( ILI9341_color_16_t*)base) + numPixels );
			break;

		default :
			return base;
	}
}

void 	ILI9341::hwpixel(hd_hw_extent_t x0, hd_hw_extent_t y0, color_t data, hd_colors_t colorCycleLength, hd_colors_t startColorOffset)
{
	if(data == NULL){ return; }

	startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, 0);	// This line is needed to condition the user's input start color offset
	color_t value = getOffsetColor(data, startColorOffset);

	setColumnAddress( (uint16_t)x0, (uint16_t)x0);
	setRowAddress( (uint16_t)y0, (uint16_t)y0);
	uint8_t len = getBytesPerPixel( );

	writeToRAM( (uint8_t*)value, len );
}

void	ILI9341::swpixel( hd_extent_t x0, hd_extent_t y0, color_t data, hd_colors_t colorCycleLength, hd_colors_t startColorOffset)
{
	if(data == NULL){ return; }
	if(colorCycleLength == 0){ return; }

	startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, 0);	// This line is needed to condition the user's input start color offset because it could be greater than the cycle length
	color_t value = getOffsetColor(data, startColorOffset);

	hd_hw_extent_t x0w = (hd_hw_extent_t)x0;	// Cast to hw extent type to be sure of integer values
	hd_hw_extent_t y0w = (hd_hw_extent_t)y0;

	hd_pixels_t pixOffst = wToPix(pCurrentWindow, x0, y0);			// It was already ensured that this will be in range 
	color_t dest = getOffsetColor(pCurrentWindow->data, pixOffst);	// Rely on the user's definition of a pixel's width in memory
	uint32_t len = (uint32_t)getOffsetColor(0x00, 1);				// Getting the offset from zero for one pixel tells us how many bytes to copy

	memcpy((void*)dest, (void*)value, (size_t)len);		// Copy data into the window's buffer
}


// Basic Control Functions
ILI9341_STAT_t ILI9341::swReset( void )
{
 	ILI9341_STAT_t retval = 	ILI9341_STAT_Nominal;

 	ILI9341_CMD_t cmd = ILI9341_CMD_SWRST;
	retval = writePacket(&cmd);
	return retval;
}

ILI9341_STAT_t ILI9341::sleepIn( void )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;
	ILI9341_CMD_t cmd = ILI9341_CMD_SLPIN;
	retval = writePacket(&cmd);
	return retval;
}

ILI9341_STAT_t ILI9341::sleepOut( void )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_SLPOUT;
	retval = writePacket(&cmd );
	return retval;
}

ILI9341_STAT_t ILI9341::partialModeOn( void )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_PTLON;
	retval = writePacket(&cmd);
	return retval;
}

ILI9341_STAT_t ILI9341::normalDisplayModeOn( void )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_PTLON;
	retval = writePacket(&cmd);
	return retval;
}

ILI9341_STAT_t ILI9341::setInversion( bool on )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_INVOFF;
	if( on )
	{
		cmd = ILI9341_CMD_INVON;
	}
	retval = writePacket(&cmd);
	return retval;
}

ILI9341_STAT_t ILI9341::setPower( bool on )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_OFF;
	if( on )
	{
		cmd = ILI9341_CMD_ON;
	}
	retval = writePacket(&cmd);
	return retval;
}

ILI9341_STAT_t ILI9341::setColumnAddress( uint16_t start, uint16_t end )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_CASET;
	uint8_t buff[4] = {(start >> 8), (start & 0x00FF), (end >> 8), (end & 0x00FF)};
	retval = writePacket(&cmd, buff, 4);
	return retval;
}

ILI9341_STAT_t ILI9341::setRowAddress( uint16_t start, uint16_t end )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_RASET;
	uint8_t buff[4] = {(start >> 8), (start & 0x00FF), (end >> 8), (end & 0x00FF)};
	retval = writePacket(&cmd, buff, 4);
	return retval;
}

ILI9341_STAT_t ILI9341::writeToRAM( uint8_t* pdata, uint16_t numBytes )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRRAM;
	retval = writePacket(&cmd, pdata, numBytes);
	return retval;
}



// Functions to configure the display fully
ILI9341_STAT_t ILI9341::setMemoryAccessControl( bool mx, bool my, bool mv, bool ml, bool bgr, bool mh )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRMADCTL;
	uint8_t buff = 0x00;
	if( my ){ buff |= 0x80; }
	if( mx ){ buff |= 0x40; }
	if( mv ){ buff |= 0x20; }
	if( ml ){ buff |= 0x10; }
	if( bgr ){ buff |= 0x08; }
	if( mh ){ buff |= 0x04; }
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::selectGammaCurve( uint8_t bmNumber )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_GAMST;
	uint8_t buff = bmNumber;
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setPartialArea(uint16_t start, uint16_t end )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_PTLAREA;
	uint8_t buff[4] = {(start >> 8), (start & 0x00FF), (end >> 8), (end & 0x00FF)};
	retval = writePacket(&cmd, buff, 4);
	return retval;
}

ILI9341_STAT_t ILI9341::setVerticalScrolling( uint16_t tfa, uint16_t vsa, uint16_t bfa )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRVSCRL;
	uint8_t buff[6] = {(tfa >> 8), (tfa & 0x00FF), (vsa >> 8), (vsa & 0x00FF), (bfa >> 8), (bfa & 0x00FF)};
	retval = writePacket(&cmd, buff, 6);
	return retval;
}

ILI9341_STAT_t ILI9341::setVerticalScrollingStartAddress( uint16_t ssa )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRVSSA;
	uint8_t buff[2] = {(ssa >> 8), (ssa & 0x00FF)};
	retval = writePacket(&cmd, buff, 2);
	return retval;
}

ILI9341_STAT_t ILI9341::setIdleMode( bool on )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_IDLOFF;
	if( on )
	{
		cmd = ILI9341_CMD_IDLON;
	}
	retval = writePacket(&cmd);
	return retval;
}

ILI9341_STAT_t ILI9341::setInterfacePixelFormat( uint8_t CTRLintfc )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRPXFMT;
	uint8_t buff = (CTRLintfc & 0x07);

	if( buff == ILI9341_PXLFMT_16 ){ _pxlfmt = ILI9341_PXLFMT_16; }
	if( buff == ILI9341_PXLFMT_18 ){ _pxlfmt = ILI9341_PXLFMT_18; }

	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setTearingEffectLine( bool on )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_TELOFF;
	if( on )
	{
		cmd = ILI9341_CMD_TELON;
	}
	retval = writePacket(&cmd);
	return retval;
}

ILI9341_STAT_t ILI9341::setNormalFramerate( uint8_t diva, uint8_t vpa )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRNMLFRCTL;
	uint8_t buff[2] = {diva, vpa};
	retval = writePacket(&cmd, buff, 2);
	return retval;
}

ILI9341_STAT_t ILI9341::setIdleFramerate( uint8_t divb, uint8_t vpb )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRIDLFRCTL;
	uint8_t buff[2] = {divb, vpb};
	retval = writePacket(&cmd, buff, 2);
	return retval;
}

ILI9341_STAT_t ILI9341::setPartialFramerate( uint8_t divc, uint8_t vpc )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRPTLFRCTL;
	uint8_t buff[2] = {divc, vpc};
	retval = writePacket(&cmd, buff, 2);
	return retval;
}

ILI9341_STAT_t ILI9341::setPowerControl1( uint8_t vrh, uint8_t vc )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRPWCTL1;
	uint8_t buff[2] = {(vrh & 0x1F), (vc & 0x07)};
	retval = writePacket(&cmd, buff, 2);
	return retval;
}

ILI9341_STAT_t ILI9341::setPowerControl2( uint8_t bt )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRPWCTL2;
	uint8_t buff = (bt & 0x07);
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setPowerControl3( uint8_t apa )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRPWCTL3;
	uint8_t buff = (apa & 0x07);
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setPowerControl4( uint8_t apb )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRPWCTL4;
	uint8_t buff = (apb & 0x07);
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setPowerControl5( uint8_t apc )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRPWCTL5;
	uint8_t buff = (apc & 0x07);
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setVCOMControl1( uint8_t vmh, uint8_t vml )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRVCOMCTL1;
	uint8_t buff[2] = {(vmh & 0x7F), (vml & 0x7F)};
	retval = writePacket(&cmd, buff, 2);
	return retval;
}

// ILI9341_STAT_t ILI9341::setVCOMControl2( uint8_t vma )
// {
// 	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

// 	ILI9341_CMD_t cmd = ;
// 	uint8_t buff[] = {};
// 	retval = writePacket(&cmd, &buff, );
// 	return retval;
// }

ILI9341_STAT_t ILI9341::setVCOMOffsetControl( bool nVM, uint8_t vmf )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRVCMOFSTCTL;
	uint8_t buff = (vmf & 0x7F);
	if( nVM )
	{
		buff |= 0x80;
	}
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setSrcDriverDir( bool crl )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRSDRVDIR;
	uint8_t buff = 0x00;
	if( crl )
	{
		buff |= 0x01;
	}
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setGateDriverDir( bool ctb )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRGDRVDIR;
	uint8_t buff = 0x00;
	if( ctb )
	{
		buff |= 0x01;
	}
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setGamRSel( bool gamrsel )
{
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRGAMRS;
	uint8_t buff = 0x00;
	if( gamrsel )
	{
		buff |= 0x01;
	}
	retval = writePacket(&cmd, &buff, 1);
	return retval;
}

ILI9341_STAT_t ILI9341::setPositiveGamCorr( uint8_t* gam16byte )
{ 
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRPGCS;
	retval = writePacket(&cmd, gam16byte, 16);
	return retval;
}

ILI9341_STAT_t ILI9341::setNegativeGamCorr( uint8_t* gam16byte )
{ 
	ILI9341_STAT_t retval = ILI9341_STAT_Nominal;

	ILI9341_CMD_t cmd = ILI9341_CMD_WRNGCS;
	retval = writePacket(&cmd, gam16byte, 16);
	return retval;
}

















////////////////////////////////////////////////////////////
//		SSD1357_Arduino_SPI_OneWay Implementation		  //
////////////////////////////////////////////////////////////
ILI9341_4WSPI::ILI9341_4WSPI(uint16_t xSize, uint16_t ySize) : hyperdisplay( xSize, ySize ), ILI9341(xSize, ySize, ILI9341_INTFC_4WSPI)
{
	SPISettings tempSettings(ILI9341_SPI_MAX_FREQ, ILI9341_SPI_DATA_ORDER, ILI9341_SPI_MODE);
	_spisettings = tempSettings;
}

////////////////////////////////////////////////////////////
//				Display Interface Functions				  //
////////////////////////////////////////////////////////////
ILI9341_STAT_t ILI9341_4WSPI::writePacket(ILI9341_CMD_t* pcmd, uint8_t* pdata, uint16_t dlen)
{
	selectDriver();
	_spi->beginTransaction(_spisettings);

	if(pcmd != NULL)
	{
		digitalWrite(_dc, LOW);
		_spi->transfer(*(pcmd));
	}

	if( (pdata != NULL) && (dlen != 0) )
	{
		digitalWrite(_dc, HIGH);
		// _spi->transfer(pdata, dlen);
		transferSPIbuffer(pdata, dlen, ARDUINO_STILL_BROKEN );
	}		

	_spi->endTransaction();	
	deselectDriver();
	return ILI9341_STAT_Nominal;
}

ILI9341_STAT_t ILI9341_4WSPI::transferSPIbuffer(uint8_t* pdata, size_t count, bool arduinoStillBroken ){
	if(arduinoStillBroken){
		for(size_t indi = 0; indi < count; indi++){
			_spi->transfer((uint8_t)*(pdata + indi));
		}
		return ILI9341_STAT_Nominal;
	}
	else{
		_spi->transfer(pdata, count);
		return ILI9341_STAT_Nominal;
	}
}

ILI9341_STAT_t ILI9341_4WSPI::selectDriver( void )
{
	digitalWrite(_cs, LOW);
	return ILI9341_STAT_Nominal;
}

ILI9341_STAT_t ILI9341_4WSPI::deselectDriver( void )
{
	digitalWrite(_cs, HIGH);
	return ILI9341_STAT_Nominal;
}

ILI9341_STAT_t ILI9341_4WSPI::setSPIFreq( uint32_t freq )
{
	SPISettings tempSettings(freq, ILI9341_SPI_DATA_ORDER, ILI9341_SPI_MODE);
	_spisettings = tempSettings;
	return ILI9341_STAT_Nominal;
}

void    ILI9341_4WSPI::hwxline(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t len, color_t data, hd_colors_t colorCycleLength, hd_colors_t startColorOffset, bool goLeft)
{
	if(data == NULL){ return; }
	if( len < 1 ){ return; }

	startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, 0);	// This line is needed to condition the user's input start color offset

	color_t value = getOffsetColor(data, startColorOffset);
	uint8_t bpp = getBytesPerPixel( );

	if( goLeft )
	{ 
		setMemoryAccessControl( true, true, false, false, true, false ); 
		x0 = (xExt - 1) - x0;
	}
	hd_hw_extent_t x1 = x0 + (len - 1);

	// Setup the valid area to draw...
	setColumnAddress( x0, x1);
	setRowAddress(y0, y0);

	ILI9341_CMD_t cmd = ILI9341_CMD_WRRAM;
	writePacket(&cmd);					// Send the command to enable writing to RAM but don't send any data yet

	// Now, we need to send data with as little overhead as possible, while respecting the start offset and color cycle length and everything else...
	selectDriver();
	digitalWrite(_dc, HIGH);
	_spi->beginTransaction(_spisettings);

	if(colorCycleLength == 1)
	{
		// Special case that can be handled with a lot less thinking (so faster)
		uint8_t speedupArry[ILI9341_MAX_X*ILI9341_MAX_BPP];
		for(uint16_t indi = 0; indi < len; indi++)
		{
			for(uint8_t indj = 0; indj < bpp; indj++)
			{
				speedupArry[ indj + (indi*bpp) ] = *((uint8_t*)(data) + indj);
			}
		}
		// _spi->transfer(speedupArry, len*bpp);
		transferSPIbuffer(speedupArry, len*bpp, ARDUINO_STILL_BROKEN );
	}
	else
	{
		uint16_t pixelsToDraw = 0;
		uint16_t pixelsDrawn = 0;

		while(len != 0)
		{
			// Let's figure out how many pixels we can draw right now contiguously.. (thats from the start offset to the full legnth of the cycle)
			uint16_t pixelsAvailable = colorCycleLength - startColorOffset;
			value = getOffsetColor(data, startColorOffset);
			
			if( pixelsAvailable >= len ){ pixelsToDraw = len; }
			else if( pixelsAvailable < len ){ pixelsToDraw = pixelsAvailable; }

			// // Draw "pixelsToDraw" pixels using "bpp*pixelsToDraw" bytes
			// _spi->transfer(value, bpp*pixelsToDraw);
			transferSPIbuffer((uint8_t*)value, bpp*pixelsToDraw, ARDUINO_STILL_BROKEN );

			len -= pixelsToDraw;
			pixelsDrawn = pixelsToDraw;
			startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, pixelsDrawn);
		}
	}

	_spi->endTransaction();	
	deselectDriver();

	if( goLeft ){ setMemoryAccessControl( false, true, false, false, true, false ); } // Reset to defaults
}



void    ILI9341_4WSPI::hwyline(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t len, color_t data, hd_colors_t colorCycleLength, hd_colors_t startColorOffset, bool goUp)
{
	if(data == NULL){ return; } 
	if( len < 1 ){ return; }

	startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, 0);	// This line is needed to condition the user's input start color offset
	color_t value = getOffsetColor(data, startColorOffset);
	uint8_t bpp = getBytesPerPixel( );

	if( goUp )
	{ 
		//setMemoryAccessControl( false, true, false, false, true, false );
		setMemoryAccessControl( false, false, false, false, true, false ); 
		y0 = (yExt - 1) - y0; 
	}
	hd_hw_extent_t y1 = y0 + (len - 1);
	
	// Setup the valid area to draw...
	setColumnAddress( x0, x0);
	setRowAddress(y0, y1);

	ILI9341_CMD_t cmd = ILI9341_CMD_WRRAM;
	writePacket(&cmd);					// Send the command to enable writing to RAM but don't send any data yet

	// Now, we need to send data with as little overhead as possible, while respecting the start offset and color cycle length and everything else...
	selectDriver();
	digitalWrite(_dc, HIGH);
	_spi->beginTransaction(_spisettings);


	if(colorCycleLength == 1)
	{
		// Special case that can be handled with a lot less thinking (so faster)
		uint8_t speedupArry[ILI9341_MAX_Y*ILI9341_MAX_BPP];
		for(uint16_t indi = 0; indi < len; indi++)
		{
			for(uint8_t indj = 0; indj < bpp; indj++)
			{
				speedupArry[ indj + (indi*bpp) ] = *((uint8_t*)(data) + indj);
			}
		}
		// _spi->transfer(speedupArry, len*bpp);
		transferSPIbuffer(speedupArry, len*bpp, ARDUINO_STILL_BROKEN );
	}
	else
	{
		uint16_t pixelsToDraw = 0;
		uint16_t pixelsDrawn = 0;

		while(len != 0)
		{
			// Let's figure out how many pixels we can draw right now contiguously.. (thats from the start offset to the full legnth of the cycle)
			uint16_t pixelsAvailable = colorCycleLength - startColorOffset;
			value = getOffsetColor(data, startColorOffset);
			
			if( pixelsAvailable >= len ){ pixelsToDraw = len; }
			else if( pixelsAvailable < len ){ pixelsToDraw = pixelsAvailable; }

			// // Draw "pixelsToDraw" pixels using "bpp*pixelsToDraw" bytes
			// _spi->transfer(value, bpp*pixelsToDraw);
			transferSPIbuffer((uint8_t*)value, bpp*pixelsToDraw, ARDUINO_STILL_BROKEN );

			len -= pixelsToDraw;
			pixelsDrawn = pixelsToDraw;
			startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, pixelsDrawn);
		}
	}

	_spi->endTransaction();	
	deselectDriver();

	if( goUp )
	{ 
		setMemoryAccessControl( false, true, false, false, true, false );
	}
}

// void 	ILI9341_4WSPI::hwrectangle(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t x1, hd_hw_extent_t y1, bool filled, color_t data, hd_colors_t colorCycleLength, hd_colors_t startColorOffset, bool reverseGradient, bool gradientVertical)
// {
// // Hardware rectangle is left unimplemented because it's hard to squeeze out much more performance than the built-in function that uses our more efficient versions of hwxline and hwyline	
// }

void ILI9341_4WSPI::hwfillFromArray(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t x1, hd_hw_extent_t y1, color_t data, hd_pixels_t numPixels, bool Vh)
{
	if(numPixels == 0){ return; }
	if(data == NULL ){ return; }

	uint8_t bpp = getBytesPerPixel();

	if( Vh )
	{ 
		setMemoryAccessControl( true, true, true, false, true, false );

		setColumnAddress( y0, y1);
		setRowAddress(x0, x1);
	}
	else
	{
		setColumnAddress( x0, x1);
		setRowAddress(y0, y1);
	}
	

	ILI9341_CMD_t cmd = ILI9341_CMD_WRRAM;
	writePacket(&cmd);					// Send the command to enable writing to RAM but don't send any data yet

	selectDriver();
	digitalWrite(_dc, HIGH);
	_spi->beginTransaction(_spisettings);

	// _spi->transfer((uint8_t*)data, bpp*numPixels);
	transferSPIbuffer((uint8_t*)data, bpp*numPixels, ARDUINO_STILL_BROKEN );

	_spi->endTransaction();	
	deselectDriver();

	if( Vh ){ setMemoryAccessControl( true, true, false, false, true, false ); }
}





