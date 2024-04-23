////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2022 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Mitov.h>
#include <SPI.h>

#ifdef __TEST_FOR_DEBUG_PRINTS__
#define Serial UNGUARDED DEBUG PRINT!!!
#endif

namespace Mitov
{
//---------------------------------------------------------------------------
	namespace Microchip_MCP23017_TRegisterIndex
	{
		enum TRegisterIndex { riDirection, riPullUp, riInvert, riValue };
	}
//---------------------------------------------------------------------------
	namespace Microchip_MCP23017_Const
	{
		const uint8_t	REG_IODIRA	= 0x00;
		const uint8_t	REG_IODIRB	= 0x01;
		const uint8_t	REG_IPOLA	= 0x02;
		const uint8_t	REG_IPOLB	= 0x03;
		const uint8_t	REG_GPPUA	= 0x0C;
		const uint8_t	REG_GPPUB	= 0x0D;
		const uint8_t	REG_GPIOA	= 0x12;
		const uint8_t	REG_GPIOB	= 0x13;
		const uint8_t	REG_OLATA	= 0x14;
	}
//---------------------------------------------------------------------------
	template<
		typename T_I2C, T_I2C &C_I2C,
		typename T_Address
	> class Microchip_MCP23017 :
		public T_Address
	{
	public:
		_V_PROP_( Address )

	protected:
		uint16_t	FRegValues[ 4 ][ 2 ];
		uint16_t	FReadRegisterMap = 0;
		uint16_t	FReadBits = 0;

//	public:
//		BasicMicrochip_MCP23017Channel *FChannels[ C_CHANNELS_COUNT ];

	protected:
		void BeginTransmissionAt( uint8_t ARegister )
		{
			C_I2C.beginTransmission( uint8_t( Address().GetValue() ));

			C_I2C.write( ARegister );
		}

		void WriteAt( uint8_t ARegister, uint16_t AData )
		{
//			Serial.print( "WriteAt: " ); Serial.print( ARegister ); Serial.print( " " ); Serial.println( AData ); 
			BeginTransmissionAt( ARegister );

			C_I2C.write( (uint8_t *)&AData, 2 );

			C_I2C.endTransmission();
		}

		void StartReadFrom( uint8_t ARegister, uint8_t ALength )
		{
			BeginTransmissionAt( ARegister );
			C_I2C.endTransmission();
			C_I2C.requestFrom( uint8_t( Address().GetValue()), ALength );
			while( C_I2C.available() < ALength );
		}

		void UpdateAll()
		{
			static const uint8_t CPorts[] = 
			{ 
				Microchip_MCP23017_Const::REG_IODIRA,
				Microchip_MCP23017_Const::REG_IPOLA,
				Microchip_MCP23017_Const::REG_GPPUA,
				Microchip_MCP23017_Const::REG_OLATA
			};

			for( int i = 0; i < 4; ++i )
				if( FRegValues[ i ][ 0 ] != FRegValues[ i ][ 1 ] )
				{
					WriteAt( CPorts[ i ], FRegValues[ i ][ 0 ] );
					FRegValues[ i ][ 1 ] = FRegValues[ i ][ 0 ];
				}

		}

	public:
		void PerformRead()
		{
			if( ! FReadRegisterMap )
				return;

//			Serial.println( FReadRegisterMap );

			if( FReadRegisterMap & 0x0F )
			{
				if( FReadRegisterMap & 0xF0 )
				{
					StartReadFrom( Microchip_MCP23017_Const::REG_GPIOA, 2 ); // GPIO register
					((uint8_t *)&FReadBits )[ 0 ] = C_I2C.read();
					((uint8_t *)&FReadBits )[ 1 ] = C_I2C.read();
				}

				else
				{
					StartReadFrom( Microchip_MCP23017_Const::REG_GPIOA, 1 ); // GPIO register
					((uint8_t *)&FReadBits )[ 0 ] = C_I2C.read();
					((uint8_t *)&FReadBits )[ 1 ] = 0;

//					Serial.print( "FReadBits: " ); Serial.println( FReadBits );
				}
			}
			else
			{
				// Read the port 
				StartReadFrom( Microchip_MCP23017_Const::REG_GPIOB, 1 ); // GPIO register
				((uint8_t *)&FReadBits )[ 0 ] = 0;
				((uint8_t *)&FReadBits )[ 1 ] = C_I2C.read();
			}

//			for( int i = 0; i < C_CHANNELS_COUNT; ++i )
//				FChannels[ i ]->UpdateOutput( GetBitValue( i ));

		}

	public:
		inline bool GetBitValue( uint32_t AIndex )
		{
			return( ( FReadBits & ( ((uint32_t)1 ) << AIndex )) != 0 );
		}

	public:
		void SetChannelValue( int AIndex, bool AValue )
		{
			SetChannelRegisterValue( AIndex, Microchip_MCP23017_TRegisterIndex::riValue, AValue );
		}

	protected:
//		void DoReadInputReceive( void * _Data )
//		{
//			PerformRead();
//		}

	public:
		inline void SystemLoopBegin()
		{
//			if( ! ReadInputPin.IsConnected() )
				PerformRead();
		}

		inline void SystemStart()
		{
			for( int i = 0; i < 4; ++i )
				FRegValues[ i ][ 1 ] = ~FRegValues[ i ][ 0 ];

			UpdateAll();
		}

		inline void SystemLoopUpdateHardware()
		{
			UpdateAll();
		}

	public:
		void SetChannelRegisterValue( int AIndex, int ARegisterIndex, bool AValue )
		{
			FRegValues[ ARegisterIndex ][ 0 ] &= ~( ((uint16_t) 0b1 ) << AIndex );
			FRegValues[ ARegisterIndex ][ 0 ] |= ( (uint16_t)( AValue & 1 ) ) << AIndex;

			if( ARegisterIndex == Microchip_MCP23017_TRegisterIndex::riDirection )
			{
				if( AValue )
					FReadRegisterMap |= ((uint16_t) 1) << AIndex;

				else
					FReadRegisterMap &= ~( ((uint16_t) 1) << AIndex );
			}
		}

	public:
		inline Microchip_MCP23017()
		{
			for( int i = 0; i < 4; ++i )
				FRegValues[ i ][ 0 ] = 0;
		}

	};
//---------------------------------------------------------------------------
	template<
		typename T_OWNER, T_OWNER &C_OWNER, 
		uint8_t C_INDEX,
		typename T_InitialValue,
		typename T_IsCombinedInOut,
		typename T_IsInverted,
		typename T_IsOutput,
		typename T_IsPullUp,
		typename T_OutputPin
	> class Microchip_MCP23017_Channel : 
		public T_InitialValue,
		public T_IsCombinedInOut,
		public T_IsInverted,
		public T_IsOutput,
		public T_IsPullUp,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )
		
	public:
		_V_PROP_( InitialValue )
        _V_PROP_( IsOutput )
        _V_PROP_( IsPullUp )
        _V_PROP_( IsInverted )

	public:
		inline void SetMode( int AMode ) // Placeholder for direct pin control!
		{
//			pinMode( FPinNumber, AMode );
		}

		inline void SetInitialMode( int AMode ) // Placeholder for direct pin control!
		{
		}

		inline bool DigitalRead()
		{
//			Serial.println( "TEST1" );
			C_OWNER.PerformRead();
//			Serial.println( FInValue );
			return C_OWNER.GetBitValue( C_INDEX );
		}

		inline void DigitalWrite( bool AValue )
		{
//			if( FValue == AValue )
//				return;

//			FValue = AValue;
			C_OWNER.SetChannelValue( C_INDEX, AValue );
		}

	public:
		inline void InputPin_o_Receive( void * _Data )
		{
			DigitalWrite( *(bool *)_Data );
		}

	public:
        void PinDirectionsInit()
        {
			C_OWNER.SetChannelRegisterValue( C_INDEX, Microchip_MCP23017_TRegisterIndex::riDirection, ! IsOutput() );
			C_OWNER.SetChannelRegisterValue( C_INDEX, Microchip_MCP23017_TRegisterIndex::riPullUp, IsPullUp() );
			C_OWNER.SetChannelRegisterValue( C_INDEX, Microchip_MCP23017_TRegisterIndex::riInvert, IsInverted() );
        }

	public:
		inline void SystemStart()
		{
			T_OutputPin::SetPinValue( InitialValue() );
		}

		inline void SystemLoopBegin()
		{
			bool AValue = C_OWNER.GetBitValue( C_INDEX );
			T_OutputPin::SetPinValue( AValue );
		}

	public:
//		Microchip_MCP23017_Channel( bool AInitialValue, bool AIsOutput, bool AIsCombinedInOut, bool AIsPullUp, bool AIsInverted ) :
//			IsOutput( AIsOutput ),
//			IsPullUp( AIsPullUp ),
//			IsInverted( AIsInverted ),
//			FValue( AInitialValue )
//			FCombinedInOut( AIsCombinedInOut )
		inline Microchip_MCP23017_Channel()
		{
//			C_OWNER.FChannels[ C_ARRAY_INDEX ] = this;
			PinDirectionsInit();
		}

	};
//---------------------------------------------------------------------------
}

#ifdef __TEST_FOR_DEBUG_PRINTS__
#undef Serial
#endif