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

#ifdef __TEST_FOR_DEBUG_PRINTS__
#define Serial UNGUARDED DEBUG PRINT!!!
#endif

namespace Mitov
{
//---------------------------------------------------------------------------
//	class ModbusDevice;
	class ModbusServerBasicProtocol;
//	class ModbusDeviceTableCoils;
//	class ModbusDeviceTableDigitalInputs;
//	class ModbusDeviceTableHoldingRegisters;
//	class ModbusDeviceTableInputRegisters;
//---------------------------------------------------------------------------
	namespace Modbus
	{
		uint16_t	Swap16( uint16_t AValue )
		{
			return ( AValue >> 8 ) | (( AValue & 0xFF) << 8 );
		}

		struct TModBusHeader
		{
			uint16_t	TransactionID;
			uint16_t	ProtocolID;
			uint16_t	RecLength;
		} __attribute__ ((packed));

		struct TModOperationHeader
		{
			uint8_t		DeviceID;
			uint8_t		FunctionCode;

		} __attribute__ ((packed));

		struct TModAddressHeader : public TModOperationHeader
		{
			uint16_t	RawAddress;

		public:
			inline uint16_t	GetAddress() { return Swap16( RawAddress ); }
			inline void SetAddress( uint16_t AValue ) { RawAddress = Swap16( AValue ); }

		};

		struct TModReadHeader : public TModAddressHeader
		{
			uint16_t	RawCount;

		public:
			inline uint16_t	GetCount() { return Swap16( RawCount ); }
			inline void SetCount( uint16_t AValue ) { RawCount = Swap16( AValue ); }

		} __attribute__ ((packed));

		struct TModWriteOneRegisterHeader : public TModAddressHeader
		{
			uint16_t	RawValue;

		public:
			inline uint16_t	GetValue() { return Swap16( RawValue ); }
			inline void SetValue( uint16_t AValue ) { RawValue = Swap16( AValue ); }

		} __attribute__ ((packed));

		struct TModWriteMultipleRegistersHeader : public TModAddressHeader
		{
			uint16_t	RawCountItems;
			uint8_t		CountBytes;

		public:
			inline uint16_t	GetCountItems() { return Swap16( RawCountItems ); }
			inline void SetCountItems( uint16_t AValue ) { RawCountItems = Swap16( AValue ); }

		} __attribute__ ((packed));

		struct TModBusWriteRegistersResponseHeader : public TModOperationHeader
		{
			uint16_t	RawAddress;
			uint16_t	RawNumberRegisters;

		public:
			inline uint16_t	GetAddress() { return Swap16( RawAddress ); }
			inline uint16_t	GetNumberRegisters() { return Swap16( RawNumberRegisters ); }

		} __attribute__ ((packed));

		struct TModBusExceptionHeader : public TModOperationHeader
		{
			uint8_t		ExceptionCode ;
		} __attribute__ ((packed));

		struct TModbusTransaction
		{
		public:
			uint8_t		FRefCount = 0;

		public:
			ModbusServerBasicProtocol	*SenderProtocol;

		public:
			Mitov::TDataBlock Buffer;

		public:
			void Acquire()
			{
				++FRefCount;
			}

			void Release()
			{
				--FRefCount;
				if( ! FRefCount )
					delete this;
			}

		public:
			~TModbusTransaction() 
			{
//				Serial.println( "~TModbusTransaction" );
				delete [] Buffer.Data;
			}
		};

		uint16_t CRC16_2( uint8_t *ABuffer, uint16_t ASize )
		{
			uint16_t Result = 0xFFFF;
			for( int i = 0; i < ASize; ++ i )
			{
				Result ^= uint16_t( ABuffer[ i ] );          // XOR byte into least sig. byte of crc
				for( int j = 0; j <= 7; ++j )
				{
					if( Result & 1 ) // If the LSB is set
						Result = ( Result >> 1 ) ^ 0xA001;                    // Shift right and XOR 0xA001

					else                            // Else LSB is not set
						Result >>= 1;               // Just shift right
				}
			}

			return Result;
		}

		uint8_t	CalculateLRC( uint8_t *ABuffer, uint16_t ASize )
		{
			int16_t AValue = 0;
			for( int i = 0; i < ASize; ++ i )
				AValue -= ABuffer[ i ];

			return AValue;
		}

		bool FromAscii( uint8_t AValue, uint8_t  &AResult )
		{
			if( AValue < '0' )
				return false;

			if( AValue <= '9' )
			{
				AResult = AValue - '0';
				return true;
			}

			if( AValue < 'A' )
				return false;

			if( AValue <= 'F' )
			{
				AResult = AValue - 'A' + 10;
				return true;
			}

			if( AValue < 'a' )
				return false;

			if( AValue <= 'f' )
			{
				AResult = AValue - 'a' + 10;
				return true;
			}

			return false;
		}

		bool ByteFromAscii( uint8_t *&AInBuffer, uint8_t &AOutValue )
		{
			uint8_t AHigh, ALow;
			if( ! FromAscii( *AInBuffer, AHigh ))
				return false;

			++AInBuffer;
			if( ! FromAscii( *AInBuffer, ALow ))
				return false;

			AOutValue = ( AHigh << 4 ) | ALow;

			++AInBuffer;
			return true;
		}

		bool BytesFromAscii( uint8_t *&AInBuffer, uint8_t *&AOutBuffer, uint16_t ASize )
		{
			for( int i = 0; i < ASize; ++i )
			{
				if( ! ByteFromAscii( AInBuffer, *AOutBuffer ))
					return false;

				++AOutBuffer;
			}

			return true;
		}

		uint8_t ToAscii( uint8_t AValue )
		{
			if( AValue < 10 )
				return( '0' + AValue );

			return( 'A' - 10 + AValue );
		}

		void ByteToAscii( uint8_t AInValue, uint8_t *&AOutBuffer )
		{
			*AOutBuffer ++ = ToAscii( AInValue >> 4 );
			*AOutBuffer ++ = ToAscii( AInValue & 0xF );
		}

		void BytesToAscii( uint8_t *&AInBuffer, uint8_t *&AOutBuffer, uint32_t ASize )
		{
			for( uint32_t i = 0; i < ASize; ++ i )
				ByteToAscii( *AInBuffer++, AOutBuffer );
		}

		uint8_t *MakeOutputBuffer( bool AAsciiFormat, Mitov::TDataBlock &ABuffer, uint32_t &AOutSize )
		{
			uint32_t ABufferSize = ABuffer.Size;
			uint8_t *AOutData;
			if( AAsciiFormat )
			{
				AOutSize = ABufferSize * 2 + 5;
				AOutData = new uint8_t[ AOutSize ];

				uint8_t *ADataOutPtr = AOutData;
				uint8_t *ADataInPtr = ABuffer.Data;

				*ADataOutPtr ++ = ':';
				BytesToAscii( ADataInPtr, ADataOutPtr, ABufferSize );
			    ByteToAscii( CalculateLRC( ABuffer.Data, ABufferSize ), ADataOutPtr );
				ADataOutPtr[ 0 ] = 0x0D; // CR
				ADataOutPtr[ 1 ] = 0x0A; // LF
			}

			else
			{
				AOutSize = ABufferSize + 2;
				AOutData = new uint8_t[ AOutSize ];
				memcpy( AOutData, ABuffer.Data, ABufferSize );
#ifdef VISUINO_MEMORY_ALLIGN
				uint16_t ASrc = Modbus::CRC16_2( AOutData, ABufferSize );
				memcpy( AOutData + ABufferSize, &ASrc, sizeof( uint16_t ));
#else
				*(uint16_t *)( AOutData + ABufferSize ) = Modbus::CRC16_2( AOutData, ABufferSize );
#endif
			}

			return AOutData;
		}

		bool TryExtractBuffer( SimpleList<uint8_t, uint32_t> &ABufferAccumulator, uint8_t *&ABuffer, uint16_t &AOutSize )
		{
/*
			Serial.println( "TryExtractBuffer" );

			Serial.print( "( " );
			for( int i = 0; i < ABufferAccumulator.size(); ++i )
			{
				Serial.print( ABufferAccumulator[ i ] );
				Serial.print( " " );
			}

			Serial.println( ")" );
*/
			uint8_t *APtr = ABufferAccumulator;
			int AStartIndex = -1;
			for( uint32_t i = 0; i < ABufferAccumulator.size(); )
			{
				if( *APtr == ':' )
				{
					AStartIndex = i;
//					Serial.println( ":" );
				}

				else if( AStartIndex >= 0 )
				{
//					Serial.print( "i = " );
//					Serial.println( i );
					if( i < ABufferAccumulator.size() - 1 )
						if(( APtr[ 0 ] == 0x0D ) && ( APtr[ 1 ] == 0x0A ))
						{
//							Serial.println( "CR" );
							if((( i - AStartIndex - 1 ) & 1 ) == 0 )
							{
//								Serial.println( "BUFFER" );
								AOutSize = ( i - AStartIndex - 1 ) / 2 - 1; // Exclude the LRC!
								ABuffer = new uint8_t[ AOutSize ];
								uint8_t *AOutPtr = ABuffer;
								APtr = ABufferAccumulator + AStartIndex + 1;
								if( BytesFromAscii( APtr, AOutPtr, AOutSize ))
								{
									uint8_t ALRC;
									if( ByteFromAscii( APtr, ALRC ))
										if( ALRC == CalculateLRC( ABuffer, AOutSize ))
										{
											ABufferAccumulator.Delete( 0, i + 2 );
//											Serial.println( "True" );
											return true;
										}

								}
							}

							ABufferAccumulator.Delete( 0, i + 2 ); // Invalid buffer size!
							i = 0;
							APtr = ABufferAccumulator;
							AStartIndex = -1;
							continue;
						}

				}

				++ APtr;
				++ i;
			}

//			Serial.println( "False" );
			return false;
		}

		const uint8_t MB_PROTOCOL = 0;

		const uint8_t mbfReadCoils = 0x01;
		const uint8_t mbfReadInputBits = 0x02;
		const uint8_t mbfReadHoldingRegs = 0x03;
		const uint8_t mbfReadInputRegs = 0x04;
		const uint8_t mbfWriteOneCoil = 0x05;
		const uint8_t mbfWriteOneReg = 0x06;
		const uint8_t mbfWriteCoils = 0x0F;
		const uint8_t mbfWriteRegs = 0x10;
		const uint8_t mbfReadFileRecord = 0x14;
		const uint8_t mbfWriteFileRecord = 0x15;
		const uint8_t mbfMaskWriteReg = 0x16;
		const uint8_t mbfReadWriteRegs = 0x17;
		const uint8_t mbfReadFiFoQueue = 0x18;

		// Responses
		const uint8_t mbeOk = 0x00;

		const uint8_t mbeIllegalFunction = 0x01;
		const uint8_t mbeIllegalRegister = 0x02;
		const uint8_t mbeIllegalDataValue = 0x03;
		const uint8_t mbeServerFailure = 0x04;
		const uint8_t mbeAcknowledge = 0x05;
		const uint8_t mbeServerBusy = 0x06;
		const uint8_t mbeGatewayPathNotAvailable = 0x0A;
		const uint8_t mbeGatewayNoResponseFromTarget = 0x0B;

		// Errors
		const uint8_t ErrorInvalidSize = 1;
		const uint8_t ErrorInvalidFunctionCode = 2;
		const uint8_t ErrorInvalidDeviceID = 3;
		const uint8_t ErrorInvalidNumberRegisters = 4;
		const uint8_t ErrorInvalidExceptionCode = 5;

	} // namespace Modbus
//---------------------------------------------------------------------------
	class ModbusServerBasicProtocol
	{		
	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) = 0;

	};
//---------------------------------------------------------------------------
/*
	class TArduinoModbusServerBasicTable
	{
//	protected:
//		ModbusDevice	&FOwner;

	public:
//		virtual void ProcessBitsCount( bool AIsInputBits, uint16_t &AAddress, uint16_t &ACount ) {}
//		virtual void ProcessBitReads( bool AIsInputBits, uint8_t *&AOutPtr, uint8_t &AOffset, uint16_t &AAddress, uint16_t &ACount ) {}
		virtual void ProcessRegistersCount( bool AIsInputRegisters, uint16_t &AAddress, uint16_t &ACount ) {}
		virtual void ProcessRegisterReads( bool AIsInputRegisters, uint8_t *&AOutPtr, uint16_t &AAddress, uint16_t &ACount ) {}
//		virtual void ProcessCoilWrites( uint16_t &AAddress, bool *&AValues, uint16_t &ACount ) {}
		virtual void ProcessRegisterWrites( uint16_t &AAddress, uint16_t *&AValues, uint16_t &ACount ) {}

//	public:
//		TArduinoModbusServerBasicTable( ModbusDevice &AOwner );

	};
*/
//---------------------------------------------------------------------------
	template <
		uint32_t C_INDEX,
		typename T_InitialValue,
		typename T_OutputPin
	> class ModbusDeviceCoil :
		public T_InitialValue,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( InitialValue )

	public:
		inline void SystemStart()
		{
			T_OutputPin::SetPinValue( InitialValue() );
		}

	public:
		void ProcessBitReads( uint32_t ATableAddress, uint8_t *&AOutPtr, uint8_t &AOffset, uint16_t &AAddress, uint16_t &ACount )
		{
/*
			Serial.print( "ProcessBitReads: " ); Serial.println( C_INDEX );
			Serial.println( ACount );
			Serial.println( ATableAddress );
			Serial.println( AOffset );
			Serial.println( "------" );
*/
			if( ! ACount )
				return;

			if( ( AAddress - ATableAddress ) <= C_INDEX )
			{
				if( InitialValue() )
					*AOutPtr |= ( 1 << AOffset );

				++AOffset;
				if( AOffset > 7 )
				{
					AOffset = 0;
					++AOutPtr;
				}

				++AAddress;
				--ACount;
			}
//			T_Coils_ProcessBitReads::Call( AOutPtr, AOffset, AAddress, ACount );
/*
			for( int i = 0; i < FItems.size(); ++ i )
			{
				if( ! ACount )
					break;

				if( ( AAddress - Address ) <= i )
				{
					if( FItems[ i ]->InitialValue )
						*AOutPtr |= ( 1 << AOffset );

					++AOffset;
					if( AOffset > 7 )
					{
						AOffset = 0;
						++AOutPtr;
					}

					++AAddress;
					--ACount;
				}
			}
*/
		}

		void ProcessCoilWrites( uint32_t ATableAddress, uint16_t &AAddress, bool *&AValues, uint16_t &ACount ) 
		{
			if( ! ACount )
				return;

//			Serial.println( "++++++" );
			if( ( AAddress - ATableAddress ) <= C_INDEX )
			{
//				Serial.println( "******" );
//				Serial.println( *AValues );
				InputPin_o_Receive( AValues );			
				++ AValues;
				++ AAddress;
				-- ACount;
			}
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
        {
			if( InitialValue() == *(bool *)_Data )
				return;

			InitialValue() = *(bool *)_Data;
			T_OutputPin::SetPinValue( InitialValue() );
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_Address,
		uint32_t C_COUNT_ITEMS,
		typename T_Coils_ProcessBitReads,
		typename T_Coils_ProcessCoilWrites
	> class ModbusDeviceTableCoils :
//		public TArduinoModbusServerBasicTable,
		public T_Address
	{
//		typedef	TArduinoModbusServerBasicTable	inherited;

	public:
		_V_PROP_( Address )

//	public:
//		SimpleList<ModbusDeviceCoil *, uint32_t>	FItems;

	public:
		void ProcessCoilBitsCount( uint16_t &AAddress, uint16_t &ACount )
		{
			if( AAddress < Address() )
				return;

			uint16_t ACountCoils = C_COUNT_ITEMS;
			if( AAddress >= Address() + ACountCoils )
				return;

			ACountCoils -= ( Address() - AAddress );
			if( ACount <= ACountCoils )
			{
				AAddress += ACount;
				ACount = 0;
				return;
			}

			AAddress += ACountCoils;
			ACount -= ACountCoils;
		}

		void ProcessCoilBitReads( uint8_t *&AOutPtr, uint8_t &AOffset, uint16_t &AAddress, uint16_t &ACount )
		{
			if( AAddress < Address().GetValue() )
				return;

//			uint32_t AIndex = 0;
			T_Coils_ProcessBitReads::Call( Address(), AOutPtr, AOffset, AAddress, ACount );
/*
			for( int i = 0; i < FItems.size(); ++ i )
			{
				if( ! ACount )
					break;

				if( ( AAddress - Address ) <= i )
				{
					if( FItems[ i ]->InitialValue )
						*AOutPtr |= ( 1 << AOffset );

					++AOffset;
					if( AOffset > 7 )
					{
						AOffset = 0;
						++AOutPtr;
					}

					++AAddress;
					--ACount;
				}
			}
*/
		}

		void ProcessCoilWrites( uint16_t &AAddress, bool *&AValues, uint16_t &ACount )
		{
//			Serial.println( "ProcessCoilWrites" );
			if( AAddress < Address().GetValue() )
				return;

			T_Coils_ProcessCoilWrites::Call( Address(), AAddress, AValues, ACount );
/*
			for( int i = 0; i < FItems.size(); ++ i )
			{
				if( ! ACount )
					break;

				if( ( AAddress - Address ) <= i )
					FItems[ i ]->ProcessCoilWrites( AAddress, AValues, ACount );
			}
*/
		}

/*
	public:
		ModbusDeviceTableCoils( T_OWNER &AOwner )
		{
			AOwner.FTables.push_back( this );
		}
*/
	};
//---------------------------------------------------------------------------
	template <
		uint32_t C_INDEX,
		typename T_InitialValue
	> class ModbusDeviceDigitalInput :
		public T_InitialValue
	{
	public:
		_V_PROP_( InitialValue )

	public:
		inline void InputPin_o_Receive( void *_Data )
        {
			InitialValue() = *(bool *)_Data;
		}

	public:
		void ProcessBitReads( uint32_t ATableAddress, uint8_t *&AOutPtr, uint8_t &AOffset, uint16_t &AAddress, uint16_t &ACount )
		{
			if( ! ACount )
				return;

			if( ( AAddress - ATableAddress ) <= C_INDEX )
			{
				if( InitialValue() )
					*AOutPtr |= ( 1 << AOffset );

				++AOffset;
				if( AOffset > 7 )
				{
					AOffset = 0;
					++AOutPtr;
				}

				++AAddress;
				--ACount;
			}
		}
	};
//---------------------------------------------------------------------------
	template <
		typename T_Address,
		uint32_t C_COUNT_ITEMS,
		typename T_DigitalInputs_ProcessBitReads
	> class ModbusDeviceTableDigitalInputs :
//		public TArduinoModbusServerBasicTable,
		public T_Address
	{
//		typedef	TArduinoModbusServerBasicTable	inherited;

	public:
		_V_PROP_( Address )

//	public:
//		SimpleList<ModbusDeviceDigitalInput *, uint32_t>	FItems;

	public:
		void ProcessDigitalInputBitsCount( uint16_t &AAddress, uint16_t &ACount )
		{
			if( AAddress < Address() )
				return;

			uint16_t ACountCoils = C_COUNT_ITEMS;
			if( AAddress >= Address() + ACountCoils )
				return;

			ACountCoils -= ( Address() - AAddress );
			if( ACount <= ACountCoils )
			{
				AAddress += ACount;
				ACount = 0;
				return;
			}

			AAddress += ACountCoils;
			ACount -= ACountCoils;
		}

		void ProcessDigitalInputBitReads( uint8_t *&AOutPtr, uint8_t &AOffset, uint16_t &AAddress, uint16_t &ACount )
		{
			if( AAddress < Address() )
				return;

			T_DigitalInputs_ProcessBitReads::Call( Address(), AOutPtr, AOffset, AAddress, ACount );
/*
			for( int i = 0; i < FItems.size(); ++ i )
			{
				if( ! ACount )
					break;

				if( ( AAddress - Address ) <= i )
				{
					if( FItems[ i ]->InitialValue )
						*AOutPtr |= ( 1 << AOffset );

					++AOffset;
					if( AOffset > 7 )
					{
						AOffset = 0;
						++AOutPtr;
					}

					++AAddress;
					--ACount;
				}
			}
*/
		}

	};
//---------------------------------------------------------------------------
/*
	class ModbusDeviceHoldingRegisterBasic
	{
	public:
		virtual uint16_t GetCountRegisters() { return 1; }
		virtual void GetCountRegisterValues( uint8_t *&AOutPtr, uint16_t &AAddress, int16_t &AAddressOffset, uint16_t &ACount ) {}
		virtual void ProcessRegisterWrites( uint16_t &AAddress, uint16_t *&AValues, int16_t &AAddressOffset, uint16_t &ACount ) {}

	};
*/
//---------------------------------------------------------------------------
	template <
		typename T_Address,
		typename T_Registers_GetCountHoldingRegisterValues,
		typename T_Registers_ProcessHoldingRegisterCount,
		typename T_Registers_ProcessHoldingRegisterWrites
	> class ModbusDeviceTableHoldingRegisters :
		public T_Address,
		public T_Registers_GetCountHoldingRegisterValues,
		public T_Registers_ProcessHoldingRegisterCount,
		public T_Registers_ProcessHoldingRegisterWrites
//		public TArduinoModbusServerBasicTable
	{
//		typedef	TArduinoModbusServerBasicTable	inherited;

	public:
		_V_PROP_( Address )

	public:
		inline void ProcessTableHoldingRegisterCount( uint16_t &AAddress, uint16_t &ACount )
		{
			if( ! ACount )
				return;

			if( AAddress < Address() )
				return;

			uint16_t ACountRegisters = 0;
			T_Registers_ProcessHoldingRegisterCount::Call( ACountRegisters );
//			for( int i = 0; i < FItems.size(); ++ i )
//				ACountRegisters += FItems[ i ]->GetCountRegisters();

			if( AAddress >= Address() + ACountRegisters )
				return;

			ACountRegisters -= ( Address() - AAddress );
			if( ACount <= ACountRegisters )
			{
				AAddress += ACount;
				ACount = 0;
				return;
			}

			AAddress += ACountRegisters;
			ACount -= ACountRegisters;
		}

		inline void ProcessTableHoldingRegisterReads( uint8_t *&AOutPtr, uint16_t &AAddress, uint16_t &ACount )
		{
			if( ! ACount )
				return;

//			Serial.println( "ProcessRegisterReads" );
//			Serial.println( "ProcessRegisterReads1" );
			if( AAddress < Address() )
				return;

//			Serial.println( "ProcessRegisterReads2" );
			int16_t AAddressOffset = AAddress - Address();
			T_Registers_GetCountHoldingRegisterValues::Call( AOutPtr, AAddress, AAddressOffset, ACount );
/*
			for( int i = 0; i < FItems.size(); ++ i )
			{
				if( ! ACount )
					break;

				FItems[ i ]->GetCountRegisterValues( AOutPtr, AAddress, AAddressOffset, ACount );
			}
*/
		}

		inline void ProcessTableHoldingRegisterWrites( uint16_t &AAddress, uint16_t *&AValues, uint16_t &ACount )
		{
			if( ! ACount )
				return;

			if( AAddress < Address() )
				return;

			int16_t AAddressOffset = AAddress - Address();
			T_Registers_ProcessHoldingRegisterWrites::Call( AAddress, AValues, AAddressOffset, ACount );
/*
			for( int i = 0; i < FItems.size(); ++ i )
			{
				if( ! ACount )
					break;

				FItems[ i ]->ProcessRegisterWrites( AAddress, AValues, AAddressOffset, ACount );
			}
*/
		}

	};
//---------------------------------------------------------------------------
	template<
		typename T_InitialValue,
		typename T_OutputPin,
		typename TYPE,
		typename TYPE_PIN
	> class ModbusDeviceHoldingRegister16Bit :
//		public ModbusDeviceHoldingRegisterBasic
		public T_InitialValue,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( InitialValue )

	public:
		inline void InputPin_o_Receive( void *_Data )
        {
			TYPE AValue = Mitov::Constrain16<TYPE>( *(TYPE_PIN *)_Data );
//			Serial.println( AValue );
//			uint32_t AValue = MitovMin( *(uint32_t *)_Data, 0xFFFF );
			if( InitialValue().GetValue() == AValue )
				return;

			InitialValue() = *((TYPE *) &AValue );
//			Serial.println( InitialValue().GetValue() );
			T_OutputPin::SetPinValue( InitialValue() );
		}

	public:
		inline void SystemStart()
		{
			T_OutputPin::SetPinValue( InitialValue() );
		}

	public:
		inline void ProcessHoldingRegisterCount( uint16_t &ACount )
		{
			++ ACount;
		}

		inline void GetCountHoldingRegisterValues( uint8_t *&AOutPtr, uint16_t &AAddress, int16_t &AAddressOffset, uint16_t &ACount )
		{
			if( ! ACount )
				return;

//			Serial.println( InitialValue );
			if( AAddressOffset <= 0 )
			{
				TYPE AInitialValue = InitialValue();
#ifdef VISUINO_MEMORY_ALLIGN
				uint16_t ASrc = Modbus::Swap16( *((uint16_t *)&AInitialValue ));
				memcpy( AOutPtr, &ASrc, sizeof( uint16_t ));
#else
				*((uint16_t *)AOutPtr ) = Modbus::Swap16( *((uint16_t *)&AInitialValue ));
#endif
				AOutPtr += sizeof( TYPE );
				--ACount;
				++AAddress;
			}

			-- AAddressOffset;
		}

		inline void ProcessHoldingRegisterWrites( uint16_t &AAddress, uint16_t *&AValues, int16_t &AAddressOffset, uint16_t &ACount )
		{
			if( AAddressOffset <= 0 )
			{
				uint16_t AValue16 = Modbus::Swap16( *AValues );
				TYPE_PIN AValue = *(TYPE *)&AValue16;
//				Serial.print( "ProcessRegisterWrites: " );
//				Serial.println( AValue );
				InputPin_o_Receive( &AValue );
				++ AValues;
				-- ACount;
			}

			-- AAddressOffset;
		}

	};
//---------------------------------------------------------------------------
/*
	template<> void ModbusDeviceHoldingRegister16Bit<int16_t, int32_t>::InputPin_o_Receive( void *_Data )
    {
		int32_t AValue = MitovConstrain<int32_t>( *(int32_t *)_Data, -0x8000l, 0x7FFFl );
		if( InitialValue == *((int16_t *) &AValue ))
			return;

		InitialValue = AValue;
		OutputPin.SendValue<int32_t>( InitialValue );
	}
*/
	//---------------------------------------------------------------------------
	template<
		typename T_InitialValue,
		typename T_MostSignificantWordFirst,
		typename T_OutputPin,
		typename TYPE
	> class ModbusDeviceHoldingRegister32Bit :
		public T_InitialValue,
		public T_MostSignificantWordFirst,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( InitialValue )
		_V_PROP_( MostSignificantWordFirst )

	public:
		inline void InputPin_o_Receive( void *_Data )
        {
			if( InitialValue() == *((TYPE *) _Data ))
				return;

//			Serial.print( "AValue: " );
//			Serial.println( AValue );
			InitialValue() = *((TYPE *) _Data );
//			Serial.print( "InputPin_o_Receive: " );
//			Serial.println( InitialValue );
			T_OutputPin::SetPinValue( InitialValue() );
		}

	public:
		inline void SystemStart()
		{
			T_OutputPin::SetPinValue( InitialValue() );
		}

	public:
		inline void ProcessHoldingRegisterCount( uint16_t &ACount )
		{
			ACount += 2;
		}

		inline void GetCountHoldingRegisterValues( uint8_t *&AOutPtr, uint16_t &AAddress, int16_t &AAddressOffset, uint16_t &ACount )
		{
			if( ! ACount )
				return;

			if( AAddressOffset <= 1 )
			{
//				Serial.print( "GetCountRegisterValues: " );
//				Serial.println( InitialValue() );
				TYPE AInitialValue = InitialValue();
				uint32_t AValue = *(uint32_t *)&AInitialValue;
				int AOffset = MitovMax<int>( 0, AAddressOffset );
				int AItemsCount = MitovMin<int>( 2 - AOffset, ACount );

//				Serial.println( AOffset );
//				Serial.println( AItemsCount );

				if( MostSignificantWordFirst() )
				{
					for( int i = 0; i < AItemsCount; ++ i )
					{
#ifdef VISUINO_MEMORY_ALLIGN
						uint16_t ASrc = Modbus::Swap16( AValue >> ( 16 * ( 1 - i )) );
						memcpy( AOutPtr + ( i ) * sizeof( uint16_t ), &ASrc, sizeof( uint16_t ));
#else
						((uint16_t *) AOutPtr )[ i ] = Modbus::Swap16( AValue >> ( 16 * ( 1 - i )) );
#endif
					}

				}

				else
				{
					for( int i = 0; i < AItemsCount; ++ i )
					{
#ifdef VISUINO_MEMORY_ALLIGN
						uint16_t ASrc = Modbus::Swap16( AValue >> ( 16 * i ) );
						memcpy( AOutPtr + ( i - AOffset ) * sizeof( uint16_t ), &ASrc, sizeof( uint16_t ));
#else
						((uint16_t *) AOutPtr )[ i - AOffset ] = Modbus::Swap16( AValue >> ( 16 * i ) );
#endif
					}

				}

				AOutPtr += AItemsCount * sizeof( uint16_t );
				ACount -= AItemsCount;
				AAddress += AItemsCount;

//				*((uint16_t *)AOutPtr ) = Modbus::Swap16( *((uint16_t *)&InitialValue ));
//				AOutPtr += sizeof( T );
//				--ACount;
//				++AAddress;
			}

			AAddressOffset -= 2;
		}

		inline void ProcessHoldingRegisterWrites( uint16_t &AAddress, uint16_t *&AValues, int16_t &AAddressOffset, uint16_t &ACount )
		{
			if( ! ACount )
				return;

			if( AAddressOffset <= 1 )
			{
				if( AAddressOffset <= 0 )
				{
					if( ACount > 1 )
					{
//						Serial.println( "WRITE" );
						TYPE AValue;
						uint32_t AUintValue;
						if( MostSignificantWordFirst() )
							AUintValue = (( uint32_t( Modbus::Swap16( AValues[ 0 ] ) ) << 16 ) | Modbus::Swap16( AValues[ 1 ] ));

						else
							AUintValue = (( uint32_t( Modbus::Swap16( AValues[ 1 ] ) ) << 16 ) | Modbus::Swap16( AValues[ 0 ] ));

						AValue = *(TYPE *)&AUintValue;
//						Serial.println( AValue );
						InputPin_o_Receive( &AValue );
//						SetValue( AValue );
						AValues += 2;
						ACount -= 2;
//						AValues.Delete( 0, 2 );
						AAddress += 2;
					}

					else
					{
						TYPE AValue = InitialValue();
						uint32_t AUintValue;
						if( MostSignificantWordFirst() )
							AUintValue = (( ( *(uint32_t *)&AValue ) & 0xFFFF ) | ( uint32_t( Modbus::Swap16( AValues[ 0 ] ) ) << 16 ));

						else
							AUintValue = (( ( *(uint32_t *)&AValue ) & 0xFFFF0000 ) | Modbus::Swap16( AValues[ 0 ] ));

						AValue = *(TYPE *)&AUintValue;
						InputPin_o_Receive( &AValue );
						++ AValues;
						-- ACount;
	//					AValues.Delete( 0, 1 );
						++ AAddress;
					}
				}

				else
				{
					TYPE AValue = InitialValue();
					uint32_t AUintValue;
					if( MostSignificantWordFirst() )
						AUintValue = (( ( *(uint32_t *)&AValue ) & 0xFFFF0000 ) | Modbus::Swap16( AValues[ 0 ] ));

					else
						AUintValue = (( ( *(uint32_t *)&AValue ) & 0xFFFF ) | ( uint32_t( Modbus::Swap16( AValues[ 0 ] ) ) << 16 ));

					AValue = *(TYPE *)&AUintValue;
					InputPin_o_Receive( &AValue );
					++ AValues;
					-- ACount;
	//				AValues.Delete( 0, 1 );
					++ AAddress;
				}
			}

			AAddressOffset -= 2;
/*
			if( AAddressOffset <= 0 )
			{
				uint32_t AValue = Modbus::Swap16( *AValues );
//				Serial.print( "ProcessRegisterWrites: " );
//				Serial.println( AValue );
				InputPin_o_Receive( &AValue );
				++ AValues;
				-- ACount;
			}

			-- AAddressOffset;
*/
		}

	};
//---------------------------------------------------------------------------
/*
	template<> void ModbusDeviceHoldingRegister16Bit<int16_t>::InputPin_o_Receive( void *_Data )
    {
		int32_t AValue =  MitovConstrain<int32_t>( *(int32_t *)_Data, -0x8000, 0x7FFF );
		if( InitialValue == *((int16_t *) &AValue ))
			return;

//			Serial.print( "InputPin_o_Receive: " );
//			Serial.println( AValue );
		InitialValue = AValue;
		OutputPin.Notify( &AValue );
	}
*/
//---------------------------------------------------------------------------
/*
	class ModbusDeviceInputRegisterBasic
	{
	public:
		virtual uint16_t GetCountRegisters() { return 1; }
		void ProcessInputRegisterCount( uint16_t &ACount )
		{
			++ ACount;
		}
		virtual void GetCountRegisterValues( uint8_t *&AOutPtr, uint16_t &AAddress, int16_t &AAddressOffset, uint16_t &ACount ) {}

//	public:
//		ModbusDeviceInputRegisterBasic( ModbusDeviceTableInputRegisters &AOwner );

	};
*/
//---------------------------------------------------------------------------
	template <
		typename T_Address,
		typename T_Registers_GetCountInputRegisterValues,
		typename T_Registers_ProcessInputRegisterCount
	> class ModbusDeviceTableInputRegisters :
//		public TArduinoModbusServerBasicTable,
		public T_Address
	{
//		typedef	TArduinoModbusServerBasicTable	inherited;

	public:
		_V_PROP_( Address )

	public:
		inline void ProcessTableInputRegisterCount( uint16_t &AAddress, uint16_t &ACount )
		{
			if( ! ACount )
				return;

//			if( ! AIsInputRegisters )
//				return;

			if( AAddress < Address() )
				return;

			uint16_t ACountRegisters = 0;
			T_Registers_ProcessInputRegisterCount::Call( ACountRegisters );
//			for( int i = 0; i < FItems.size(); ++ i )
//				ACountRegisters += FItems[ i ]->GetCountRegisters();

			if( AAddress >= Address() + ACountRegisters )
				return;

			ACountRegisters -= ( Address() - AAddress );
			if( ACount <= ACountRegisters )
			{
				AAddress += ACount;
				ACount = 0;
				return;
			}

			AAddress += ACountRegisters;
			ACount -= ACountRegisters;
		}

		inline void ProcessTableInputRegisterReads( uint8_t *&AOutPtr, uint16_t &AAddress, uint16_t &ACount )
		{
			if( ! ACount )
				return;

//			Serial.println( "ModbusDeviceTableInputRegisters::ProcessRegisterReads" );
//			if( ! AIsInputRegisters )
//				return;

//			Serial.println( "ProcessRegisterReads1" );
			if( AAddress < Address() )
				return;

//			Serial.println( "ProcessRegisterReads2" );
			int16_t AAddressOffset = AAddress - Address();
			T_Registers_GetCountInputRegisterValues::Call( AOutPtr, AAddress, AAddressOffset, ACount );
/*
			for( int i = 0; i < FItems.size(); ++ i )
			{
				if( ! ACount )
					break;

				FItems[ i ]->GetCountRegisterValues( AOutPtr, AAddress, AAddressOffset, ACount );
			}
*/
		}

	};
//---------------------------------------------------------------------------
	template<
		typename T_InitialValue,
		typename TYPE,
		typename TYPE_PIN
	> class ModbusDeviceInputRegister16Bit :
		public T_InitialValue
//		public ModbusDeviceInputRegisterBasic
	{
//		typedef	ModbusDeviceInputRegisterBasic	inherited;

	public:
		_V_PROP_( InitialValue )

	public:
		inline void InputPin_o_Receive( void *_Data )
        {
			uint32_t AValue = Mitov::Constrain16<TYPE>( *(TYPE_PIN *)_Data );
			InitialValue() = *((TYPE *) &AValue );
		}

	public:
		inline void ProcessInputRegisterCount( uint16_t &ACount )
		{
			++ ACount;
		}

		inline void GetCountInputRegisterValues( uint8_t *&AOutPtr, uint16_t &AAddress, int16_t &AAddressOffset, uint16_t &ACount )
		{
			if( ! ACount )
				return;

//			Serial.print( "AAddressOffset 16: " );
//			Serial.println( AAddressOffset );
			if( AAddressOffset <= 0 )
			{
				TYPE AInitialValue = InitialValue().GetValue();

//				Serial.println( AInitialValue );
#ifdef VISUINO_MEMORY_ALLIGN
				TYPE ASrc = Modbus::Swap16( *((TYPE *)&AInitialValue ));
				memcpy( AOutPtr, &ASrc, sizeof( TYPE ));
#else
				*((TYPE *)AOutPtr ) = Modbus::Swap16( *((TYPE *)&AInitialValue ));
#endif
				AOutPtr += sizeof( TYPE );
				--ACount;
				++AAddress;
			}

			-- AAddressOffset;
		}

	};
//---------------------------------------------------------------------------
	template<
		typename T_InitialValue,
		typename T_MostSignificantWordFirst,
		typename TYPE
	> class ModbusDeviceInputRegister32Bit : // : public ModbusDeviceInputRegisterBasic
		public T_InitialValue,
		public T_MostSignificantWordFirst
	{
//		typedef	ModbusDeviceInputRegisterBasic	inherited;

	public:
		_V_PROP_( InitialValue )
		_V_PROP_( MostSignificantWordFirst )

	public:
		inline void InputPin_o_Receive( void *_Data )
        {
			InitialValue() = *((TYPE *) _Data );
		}

	public:
		inline void ProcessInputRegisterCount( uint16_t &ACount )
		{
			ACount += 2;
		}

		inline void GetCountInputRegisterValues( uint8_t *&AOutPtr, uint16_t &AAddress, int16_t &AAddressOffset, uint16_t &ACount )
		{
			if( ! ACount )
				return;

//			Serial.print( "AAddressOffset 32: " );
//			Serial.println( AAddressOffset );

			if( AAddressOffset <= 1 )
			{
//				Serial.print( "GetCountRegisterValues: " );
//				Serial.println( InitialValue );

				TYPE AInitialValue = InitialValue();
				uint32_t AValue = *(uint32_t *)&AInitialValue;
				int AOffset = MitovMax<int>( 0, AAddressOffset );
				int AItemsCount = MitovMin<int>( 2 - AOffset, ACount );

//				Serial.println( AOffset );
//				Serial.println( AItemsCount );

				if( MostSignificantWordFirst() )
				{
					for( int i = 0; i < AItemsCount; ++ i )
#ifdef VISUINO_MEMORY_ALLIGN
					{
						uint16_t ASrc = Modbus::Swap16( AValue >> ( 16 * ( 3 - i )) );
						memcpy( AOutPtr + ( i ) * sizeof( uint16_t ), &ASrc, sizeof( uint16_t ));
					}
#else
						((uint16_t *) AOutPtr )[ i ] = Modbus::Swap16( AValue >> ( 16 * ( 1 - i )) );
#endif

				}

				else
				{
					for( int i = 0; i < AItemsCount; ++ i )
#ifdef VISUINO_MEMORY_ALLIGN
					{
						uint16_t ASrc = Modbus::Swap16( AValue >> ( 16 * i ) );
						memcpy( AOutPtr + ( i ) * sizeof( uint16_t ), &ASrc, sizeof( uint16_t ));
					}
#else
						((uint16_t *) AOutPtr )[ i ] = Modbus::Swap16( AValue >> ( 16 * i ) );
#endif

				}

				AOutPtr += AItemsCount * sizeof( uint16_t );
				ACount -= AItemsCount;
				AAddress += AItemsCount;

//				*((uint16_t *)AOutPtr ) = Modbus::Swap16( *((uint16_t *)&InitialValue ));
//				AOutPtr += sizeof( T );
//				--ACount;
//				++AAddress;
			}

			AAddressOffset -= 2;
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_ID,
		typename T_Tables_ProcessCoilBitReads,
		typename T_Tables_ProcessCoilBitsCount,
		typename T_Tables_ProcessCoilWrites,
		typename T_Tables_ProcessDigitalInputBitReads,
		typename T_Tables_ProcessDigitalInputBitsCount,
		typename T_Tables_ProcessTableHoldingRegisterCount,
		typename T_Tables_ProcessTableHoldingRegisterReads,
		typename T_Tables_ProcessTableHoldingRegisterWrites,
		typename T_Tables_ProcessTableInputRegisterCount,
		typename T_Tables_ProcessTableInputRegisterReads
	> class ModbusDevice :
		public T_ID
	{
	public:
		_V_PROP_( ID )

	public:
		void ModbusInputPin_o_Receive( void *_Data )
        {
			Modbus::TModbusTransaction *ATransaction = (Modbus::TModbusTransaction *)_Data;

			if( ATransaction->Buffer.Size < 2 )
				return;

			uint8_t	*AData = ATransaction->Buffer.Data;
			Modbus::TModOperationHeader *AHeader = (Modbus::TModOperationHeader *)AData;
			if( AHeader->DeviceID != ID() )
				return;

//			ATransaction->Acquire();
//			Serial.println( AHeader->FunctionCode );
			switch( AHeader->FunctionCode )
			{
				case Modbus::mbfReadCoils:
				case Modbus::mbfReadInputBits:
					HandleReadBoolean( ATransaction );
					break;

				case Modbus::mbfReadHoldingRegs:
				case Modbus::mbfReadInputRegs: 
					HandleReadRegisters( ATransaction );
					break;

				case Modbus::mbfWriteOneCoil:
					HandleWriteOneCoil( ATransaction );
					break;

				case Modbus::mbfWriteOneReg:
					HandleWriteOneRegister( ATransaction );
					break;

				case Modbus::mbfWriteCoils:
					HandleWriteCoils( ATransaction );
					break;

				case Modbus::mbfWriteRegs:
					HandleWriteRegisters( ATransaction );
					break;

			}
		}

	protected:
		void HandleReadBoolean( Modbus::TModbusTransaction *ATransaction )
		{
//			Serial.println( "HandleReadBoolean" );
			uint8_t	*AData = ATransaction->Buffer.Data;
			Modbus::TModReadHeader *AHeader = (Modbus::TModReadHeader *)AData;

			uint16_t ACount = AHeader->GetCount();
			if( ( ACount < 1 ) || ( ACount > 2000 ))
			{
				// Exception
				RespondException( ATransaction, Modbus::mbeIllegalRegister );
				return;
			}

			bool AIsInputBits = ( AHeader->FunctionCode == Modbus::mbfReadInputBits );
			uint16_t AOriginalAddress = AHeader->GetAddress();
			uint16_t AAddress = AOriginalAddress;

			uint16_t AProcessCount = ACount;

			if( AIsInputBits )
				T_Tables_ProcessDigitalInputBitsCount::Call( AAddress, AProcessCount );

			else
				T_Tables_ProcessCoilBitsCount::Call( AAddress, AProcessCount );
/*
			for( int i = 0; i < FTables.size(); ++i )
			{
				if( ! AProcessCount )
					break;

				FTables[ i ]->ProcessBitsCount( AIsInputBits, AAddress, AProcessCount );
			}
*/
			if( ACount == AProcessCount )
			{
//				RemoveTransaction( ATransaction );
				return;
			}

			AAddress = AOriginalAddress;
			uint8_t ACoutBytes = ( ACount + 7 ) / 8; // Size in Bytes!

//			AOutBuffer := TSLBlockBuffer.CreateSize( 3 + ACoutBytes );
//			AOutBuffer.Zero();
			uint8_t *AOutPtr = new uint8_t[ 3 + ACoutBytes ];
			AOutPtr[ 0 ] = AHeader->DeviceID;
			AOutPtr[ 1 ] = AHeader->FunctionCode;
			AOutPtr[ 2 ] = ACoutBytes; // Size in Bytes!

			uint8_t *AOutProcessPtr = AOutPtr + 3; // Skip the header
			memset( AOutProcessPtr, 0, ACoutBytes );

//			T_Tables_ProcessBitReads::Call( )

			uint8_t AOffset = 0;

			if( AIsInputBits )
				T_Tables_ProcessDigitalInputBitReads::Call( AOutProcessPtr, AOffset, AAddress, ACount );

			else
				T_Tables_ProcessCoilBitReads::Call( AOutProcessPtr, AOffset, AAddress, ACount );
/*
			for( int i = 0; i < FTables.size(); ++i )
			{
				if( ! ACount )
					break;

				FTables[ i ]->ProcessBitReads( AIsInputBits, AOutProcessPtr, AOffset, AAddress, ACount );
			}
*/
			SendResponse( ATransaction, AOutPtr, 3 + ACoutBytes );
		}

		void HandleReadRegisters( Modbus::TModbusTransaction *ATransaction )
		{
//			Serial.println( "HandleReadRegisters" );
			uint8_t	*AData = ATransaction->Buffer.Data;
			Modbus::TModReadHeader *AHeader = (Modbus::TModReadHeader *)AData;

			uint16_t ACount = AHeader->GetCount();
			if( ( ACount < 1 ) || ( ACount > 125 ))
			{
				// Exception
				RespondException( ATransaction, Modbus::mbeIllegalRegister );
				return;
			}

			bool AIsInputRegisters = ( AHeader->FunctionCode == Modbus::mbfReadInputRegs );
			uint16_t AOriginalAddress = AHeader->GetAddress();
			uint16_t AAddress = AOriginalAddress;

			uint16_t AProcessCount = ACount;

//			Serial.println( AIsInputRegisters );
			if( AIsInputRegisters )
				T_Tables_ProcessTableInputRegisterCount::Call( AAddress, AProcessCount );

			else
				T_Tables_ProcessTableHoldingRegisterCount::Call( AAddress, AProcessCount );

/*
			for( int i = 0; i < FTables.size(); ++i )
			{
				if( ! AProcessCount )
					break;

				FTables[ i ]->ProcessRegistersCount( AIsInputRegisters, AAddress, AProcessCount );
			}
*/
//			Serial.print( "AProcessCount: " );
//			Serial.println( AProcessCount );

			if( ACount == AProcessCount )
			{
//				RemoveTransaction( ATransaction );
				return;
			}

//			Serial.println( FTables.size() );
			uint8_t *AOutPtr = new uint8_t[ 3 + ACount * 2 ];
			AOutPtr[ 0 ] = AHeader->DeviceID;
			AOutPtr[ 1 ] = AHeader->FunctionCode;
			AOutPtr[ 2 ] = ACount * 2; // Size in Bytes!

			uint8_t *AOutProcessPtr = AOutPtr + 3; // Skip the header
			AAddress = AOriginalAddress;

			AProcessCount = ACount;
//			Serial.println( AProcessCount );
			if( AIsInputRegisters )
				T_Tables_ProcessTableInputRegisterReads::Call( AOutProcessPtr, AAddress, AProcessCount );

			else
				T_Tables_ProcessTableHoldingRegisterReads::Call( AOutProcessPtr, AAddress, AProcessCount );

/*
			for( int i = 0; i < FTables.size(); ++i )
			{
				if( ! AProcessCount )
					break;

//				Serial.println( AProcessCount );
				FTables[ i ]->ProcessRegisterReads( AIsInputRegisters, AOutProcessPtr, AAddress, AProcessCount );
//				Serial.println( AProcessCount );
			}
*/
			// Zero the invalid registers
			for( int i = 0; i < AProcessCount; ++ i )
			{
				AOutProcessPtr[ 0 ] = 0;
				AOutProcessPtr[ 1 ] = 0;
				AOutProcessPtr += 2;
			}

			SendResponse( ATransaction, AOutPtr, 3 + ACount * 2 );
		}

		void HandleWriteOneCoil( Modbus::TModbusTransaction *ATransaction )
		{
//			Serial.println( "HandleWriteOneCoil" );
//			Serial.println( ATransaction->Buffer.Size );

			uint8_t	*AData = ATransaction->Buffer.Data;
			Modbus::TModWriteOneRegisterHeader *AHeader = (Modbus::TModWriteOneRegisterHeader *)AData;
			uint16_t AAddress = AHeader->GetAddress();
			bool AValue = ( AHeader->RawValue != 0 );
			bool *AValues = &AValue;
			uint16_t AProcessCount = 1;

			T_Tables_ProcessCoilWrites::Call( AAddress, AValues, AProcessCount );

/*
			for( int i = 0; i < FTables.size(); ++i )
			{
				if( ! AProcessCount )
					break;

				FTables[ i ]->ProcessCoilWrites( AAddress, AValues, AProcessCount );
			}
*/
			if( AProcessCount )
			{
//				RemoveTransaction( ATransaction );
				return;
			}

			ATransaction->SenderProtocol->ProcessResponse( ATransaction );
//			ATransaction->Release(); // Release for self
		}

		void HandleWriteOneRegister( Modbus::TModbusTransaction *ATransaction )
		{
			uint8_t	*AData = ATransaction->Buffer.Data;
			Modbus::TModWriteOneRegisterHeader *AHeader = (Modbus::TModWriteOneRegisterHeader *)AData;
			uint16_t AAddress = AHeader->GetAddress();
			uint16_t AValue = AHeader->RawValue;

//			Serial.print( "HandleWriteOneRegister: " );
//			Serial.println( AValue );

			uint16_t *AValues = &AValue;

			uint16_t AProcessCount = 1;
			T_Tables_ProcessTableHoldingRegisterWrites::Call( AAddress, AValues, AProcessCount );
/*
			for( int i = 0; i < FTables.size(); ++i )
			{
				if( ! AProcessCount )
					break;

				FTables[ i ]->ProcessRegisterWrites( AAddress, AValues, AProcessCount );
			}
*/
			if( AProcessCount )
			{
//				RemoveTransaction( ATransaction );
				return;
			}

			ATransaction->SenderProtocol->ProcessResponse( ATransaction );
		}

		void HandleWriteCoils( Modbus::TModbusTransaction *ATransaction )
		{
			uint8_t	*AData = ATransaction->Buffer.Data;
			Modbus::TModWriteMultipleRegistersHeader *AHeader = (Modbus::TModWriteMultipleRegistersHeader *)AData;
			uint16_t AAddress = AHeader->GetAddress();

			uint16_t AProcessCount = AHeader->GetCountItems();
			bool *ABaseValues = new bool[ AProcessCount ];
			bool *AValues = ABaseValues;

			uint8_t *AValuesPtr = AData + sizeof( Modbus::TModWriteMultipleRegistersHeader );

			for( int i = 0; i < AProcessCount; ++ i )
			{
				uint8_t AOffset = ( i % 8 );
				*AValues ++ = ( ( *AValuesPtr & ( 1 << AOffset )) != 0 );

				if( AOffset == 7 )
					++AValuesPtr;
			}

			AValues = ABaseValues;

			T_Tables_ProcessCoilWrites::Call( AAddress, AValues, AProcessCount );
/*
			for( int i = 0; i < FTables.size(); ++i )
			{
				if( ! AProcessCount )
					break;

				FTables[ i ]->ProcessCoilWrites( AAddress, AValues, AProcessCount );
			}
*/
			delete [] ABaseValues;

			if( AProcessCount )
			{
//				RemoveTransaction( ATransaction );
				return;
			}

			Modbus::TModBusWriteRegistersResponseHeader *AResponseData = new Modbus::TModBusWriteRegistersResponseHeader;
			AResponseData->DeviceID = AHeader->DeviceID;
			AResponseData->FunctionCode = AHeader->FunctionCode;
			AResponseData->RawAddress = AHeader->RawAddress;
			AResponseData->RawNumberRegisters = AHeader->RawCountItems;
//			*AResponseData = *AHeader;
			SendResponse( ATransaction, (uint8_t *)AResponseData, sizeof( Modbus::TModBusWriteRegistersResponseHeader ) );
		}

		void HandleWriteRegisters( Modbus::TModbusTransaction *ATransaction )
		{
			uint8_t	*AData = ATransaction->Buffer.Data;
			Modbus::TModWriteMultipleRegistersHeader *AHeader = (Modbus::TModWriteMultipleRegistersHeader *)AData;
			uint16_t AAddress = AHeader->GetAddress();

			uint16_t *AValues = (uint16_t *)( AData + sizeof( Modbus::TModWriteMultipleRegistersHeader ));
			
			uint16_t AProcessCount = AHeader->GetCountItems();
			T_Tables_ProcessTableHoldingRegisterWrites::Call( AAddress, AValues, AProcessCount );
/*
			for( int i = 0; i < FTables.size(); ++i )
			{
				if( ! AProcessCount )
					break;

				FTables[ i ]->ProcessRegisterWrites( AAddress, AValues, AProcessCount );
			}
*/
			if( AProcessCount )
			{
//				RemoveTransaction( ATransaction );
				return;
			}

			Modbus::TModBusWriteRegistersResponseHeader *AResponseData = new Modbus::TModBusWriteRegistersResponseHeader;
			AResponseData->DeviceID = AHeader->DeviceID;
			AResponseData->FunctionCode = AHeader->FunctionCode;
			AResponseData->RawAddress = AHeader->RawAddress;
			AResponseData->RawNumberRegisters = AHeader->RawCountItems;
//			*AResponseData = *AHeader;
			SendResponse( ATransaction, (uint8_t *)AResponseData, sizeof( Modbus::TModBusWriteRegistersResponseHeader ) );
		}

/*
		void RemoveTransaction( Modbus::TModbusTransaction *ATransaction )
		{
//			ATransaction->Release(); // Release for self
		}
*/
		void RespondException( Modbus::TModbusTransaction *ATransaction, uint8_t AExceptionCode )
		{
			uint8_t	*AData = ATransaction->Buffer.Data;
			Modbus::TModOperationHeader *AInHeader = (Modbus::TModOperationHeader *)AData;

			Modbus::TModBusExceptionHeader *AResponseData = new Modbus::TModBusExceptionHeader;

			AResponseData->DeviceID = AInHeader->DeviceID;
			AResponseData->FunctionCode = AInHeader->FunctionCode | 0x80;
			AResponseData->ExceptionCode = AExceptionCode;

			SendResponse( ATransaction, (uint8_t *)AResponseData, sizeof( Modbus::TModBusExceptionHeader ) );
		}

		void SendResponse( Modbus::TModbusTransaction *ATransaction, uint8_t *AResponseData, uint16_t ASize )
		{
			delete [] ATransaction->Buffer.Data;
			ATransaction->Buffer.Data = nullptr;
			ATransaction->Buffer = TDataBlock( ASize, AResponseData );

			ATransaction->SenderProtocol->ProcessResponse( ATransaction );
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_ModbusOutputPin,
		typename T_OutputPin
	> class ModbusServerTCPProtocol :
		public ModbusServerBasicProtocol,
		public T_ModbusOutputPin,
		public T_OutputPin
	{
		typedef ModbusServerBasicProtocol inherited;

	public:
		_V_PIN_( ModbusOutputPin )
		_V_PIN_( OutputPin )

	protected:
		struct TTransaction : public Modbus::TModbusTransaction
		{
			uint16_t	ID;
		};

	protected:
		SimpleList<uint8_t, uint32_t>	FReceivedData;

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
//			Serial.println( "ProcessResponse" );
			uint8_t *AOutData = new uint8_t[ ATransaction->Buffer.Size + sizeof( Modbus::TModBusHeader ) ];
			memcpy( AOutData + sizeof( Modbus::TModBusHeader ), ATransaction->Buffer.Data, ATransaction->Buffer.Size );
			Modbus::TModBusHeader *AHeader = (Modbus::TModBusHeader *)AOutData;
			AHeader->TransactionID = (( TTransaction *)ATransaction )->ID;
			AHeader->ProtocolID = Modbus::MB_PROTOCOL;
			AHeader->RecLength = Modbus::Swap16( ATransaction->Buffer.Size );

//			ATransaction->Release();

			Mitov::TDataBlock ABlock( ATransaction->Buffer.Size + sizeof( Modbus::TModBusHeader ), AOutData );
			T_OutputPin::SetPinValue( ABlock );
			delete [] AOutData;
//			FTransactions.erase( (TTransaction *)ATransaction );
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
        {
			Mitov::TDataBlock ABlock = *(Mitov::TDataBlock *)_Data;
//			Serial.println( ABlock.ToString() );
			FReceivedData.append( ABlock.Data, ABlock.Size );
			if( FReceivedData.size() < sizeof( Modbus::TModBusHeader ))
				return;

			uint8_t *APtr = (uint8_t *)FReceivedData;
			Modbus::TModBusHeader *AHeader = (Modbus::TModBusHeader *)APtr;
			uint16_t	ASize = Modbus::Swap16( AHeader->RecLength );

			if( AHeader->ProtocolID != Modbus::MB_PROTOCOL )
				return;

			uint16_t	AExpectedSize = sizeof( Modbus::TModBusHeader ) + ASize;
			if( AExpectedSize > FReceivedData.size() )
				return;

			APtr += sizeof( Modbus::TModBusHeader );

			if( ! T_ModbusOutputPin::GetPinIsConnected() )
				FReceivedData.Delete( 0, AExpectedSize );

			else
			{
//				Serial.println( "BUFFER" );
				TTransaction *ATransaction = new TTransaction;
				ATransaction->SenderProtocol = this;
				ATransaction->ID = AHeader->TransactionID;
				uint8_t *ARequestData = new uint8_t[ ASize ];
				memcpy( ARequestData, APtr, ASize );
				ATransaction->Buffer = TDataBlock( ASize, ARequestData );

				FReceivedData.Delete( 0, AExpectedSize );

//				FTransactions.push_back( ATransaction );
				ATransaction->Acquire();
				T_ModbusOutputPin::PinNotify( ATransaction );
				ATransaction->Release();
			}
		}
	};
//---------------------------------------------------------------------------
	template <
		typename T_AsciiFormat,
		typename T_FTimerRunning,
		typename T_InputBufferSizeLimit,
		typename T_ModbusOutputPin,
		typename T_OutputPin,
		typename T_PacketDetectInterval
	> class ModbusServerSerialProtocol :
		public ModbusServerBasicProtocol,
		public T_AsciiFormat,
		public T_FTimerRunning,
		public T_InputBufferSizeLimit,
		public T_ModbusOutputPin,
		public T_OutputPin,
		public T_PacketDetectInterval
	{
		typedef ModbusServerBasicProtocol inherited;

	public:
		_V_PIN_( OutputPin )
		_V_PIN_( ModbusOutputPin )

	public:
		_V_PROP_( PacketDetectInterval )
		_V_PROP_( AsciiFormat )
		_V_PROP_( InputBufferSizeLimit )

	protected:
		_V_PROP_( FTimerRunning )
		uint32_t	FStartTime;

	protected:
		SimpleList<uint8_t, uint32_t>	FReceivedData;

	public:
		inline void InputPin_o_Receive( void *_Data )
        {
			Mitov::TDataBlock ABlock = *(Mitov::TDataBlock *)_Data;
//			Serial.println( ABlock.ToString() );
			FReceivedData.append( ABlock.Data, ABlock.Size );

			if( FReceivedData.size() > InputBufferSizeLimit().GetValue() )
				FReceivedData.Delete( 0, FReceivedData.size() - InputBufferSizeLimit().GetValue() );

			if( AsciiFormat() )
				TryProcessAsciiAcumulattor();

			else
			{
				// Reset the timer
				FTimerRunning() = true;
				FStartTime = millis();
			}
		}

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
//			Serial.println( "ProcessResponse" );
			uint32_t AOutSize;
			uint8_t *AOutData = Modbus::MakeOutputBuffer( AsciiFormat(), ATransaction->Buffer, AOutSize );
			Mitov::TDataBlock ABlock( AOutSize, AOutData );
//			Serial.println( ABlock.ToString() );
			T_OutputPin::SetPinValue( ABlock );
			delete [] AOutData;
		}

	protected:
		void TryProcessAsciiAcumulattor()
		{
			uint8_t *ABuffer;
			uint16_t AOutSize;
			while( Modbus::TryExtractBuffer( FReceivedData, ABuffer, AOutSize ))
			{
				Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
				ATransaction->SenderProtocol = this;

				ATransaction->Buffer = TDataBlock( AOutSize, ABuffer );

//				Serial.println( ATransaction->Buffer.ToString() );

				ATransaction->Acquire();
				T_ModbusOutputPin::Notify( ATransaction );
				ATransaction->Release();
			}
		}

		void TryProcessAcumulattor()
		{
			FTimerRunning() = false;

//			Serial.println( "TryProcessAcumulattor" );
			uint32_t ASize = FReceivedData.size();

/*
			Serial.print( "( " );
			for( int i = 0; i < ASize; ++i )
			{
				Serial.print( FReceivedData[ i ] );
				Serial.print( " " );
			}

			Serial.println( ")" );
*/
			if( ASize < 2 )
			{
				FReceivedData.clear();
				return;
			}

			uint8_t *APtr = FReceivedData;

			uint16_t ACRC = Modbus::CRC16_2( APtr, ASize - 2 );

#ifdef VISUINO_MEMORY_ALLIGN
			uint16_t ABufferCRC;
			memcpy( &ABufferCRC, APtr + ASize - 2, sizeof( uint16_t ) );
#else
			uint16_t ABufferCRC = *(uint16_t *)( APtr + ASize - 2 );
#endif
//			Serial.println( ACRC );
//			Serial.println( ABufferCRC );
			if( ACRC != ABufferCRC )
			{
				FReceivedData.clear();
				return;
			}

			Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
			ATransaction->SenderProtocol = this;

			uint8_t *ARequestData = new uint8_t[ ASize - 2 ];
			memcpy( ARequestData, APtr, ASize - 2 );
			FReceivedData.clear();

			ATransaction->Buffer = TDataBlock( ASize - 2, ARequestData );

			ATransaction->Acquire();
			T_ModbusOutputPin::Notify( ATransaction );
			ATransaction->Release();
		}

	public:
		void SystemLoopBegin()
		{
			if( FTimerRunning() )
				if( ( millis() - FStartTime ) >= PacketDetectInterval() )
					TryProcessAcumulattor();

//			inherited::SystemLoopBegin();
		}

	public:
		ModbusServerSerialProtocol()
		{
			FTimerRunning() = false;
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_ModbusOutputPin,
		typename T_OutputPin
	> class ModbusServerRawProtocol :
		public ModbusServerBasicProtocol,
		public T_ModbusOutputPin,
		public T_OutputPin
	{
		typedef ModbusServerBasicProtocol inherited;

	public:
		_V_PIN_( ModbusOutputPin )
		_V_PIN_( OutputPin )

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
			T_OutputPin::SetPinValue( ATransaction->Buffer );
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
        {
			Mitov::TDataBlock ABlock = *(Mitov::TDataBlock *)_Data;

//			Serial.println( ABlock.ToString() );

			Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
			ATransaction->SenderProtocol = this;

			uint8_t *ARequestData = new uint8_t[ ABlock.Size ];
			memcpy( ARequestData, ABlock.Data, ABlock.Size );
			ATransaction->Buffer = TDataBlock( ABlock.Size, ARequestData );

			ATransaction->Acquire();
			T_ModbusOutputPin::Notify( ATransaction );
			ATransaction->Release();

/*
//			Serial.println( ABlock.ToString() );
			FReceivedData.append( ABlock.Data, ABlock.Size );
			if( FReceivedData.size() < sizeof( Modbus::TModBusHeader ))
				return;

			uint8_t *APtr = (uint8_t *)FReceivedData;
			Modbus::TModBusHeader *AHeader = (Modbus::TModBusHeader *)APtr;
			uint16_t	ASize = Modbus::Swap16( AHeader->RecLength );

			if( AHeader->ProtocolID != Modbus::MB_PROTOCOL )
				return;

			uint16_t	AExpectedSize = sizeof( Modbus::TModBusHeader ) + ASize;
			if( AExpectedSize > FReceivedData.size() )
				return;

			APtr += sizeof( Modbus::TModBusHeader );

			if( ! ModbusOutputPin.IsConnected() )
				FReceivedData.Delete( 0, AExpectedSize );

			else
			{
				TTransaction *ATransaction = new TTransaction;
				ATransaction->SenderProtocol = this;
				ATransaction->ID = AHeader->TransactionID;
				uint8_t *ARequestData = new uint8_t[ ASize ];
				memcpy( ARequestData, APtr, ASize );
				ATransaction->Buffer = TDataBlock( ASize, ARequestData );

				FReceivedData.Delete( 0, AExpectedSize );

//				FTransactions.push_back( ATransaction );
				ATransaction->Acquire();
				ModbusOutputPin.Notify( ATransaction );
				ATransaction->Release();
			}
*/
		}

	};
//---------------------------------------------------------------------------
/*
	ModbusDeviceHoldingRegisterBasic::ModbusDeviceHoldingRegisterBasic( ModbusDeviceTableHoldingRegisters &AOwner )
	{
		AOwner.FItems.push_back( this );
	}
//---------------------------------------------------------------------------
	ModbusDeviceInputRegisterBasic::ModbusDeviceInputRegisterBasic( ModbusDeviceTableInputRegisters &AOwner )
	{
		AOwner.FItems.push_back( this );
	}
//---------------------------------------------------------------------------
	TArduinoModbusServerBasicTable::TArduinoModbusServerBasicTable( ModbusDevice &AOwner )
	{
		AOwner.FTables.push_back( this );
	}
*/
//---------------------------------------------------------------------------
}

#ifdef __TEST_FOR_DEBUG_PRINTS__
#undef Serial
#endif