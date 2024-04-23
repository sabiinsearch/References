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
#include <Mitov_Modbus.h>

#ifdef __TEST_FOR_DEBUG_PRINTS__
#define Serial UNGUARDED DEBUG PRINT!!!
#endif

//#define __MODBUS__CLIENT__DEBUG__

namespace Mitov
{
//---------------------------------------------------------------------------
	template <
		typename T_ErrorOutputPin,
		typename T_ExceptionOutputPin,
		typename T_ID,
		typename T_ModbusOutputPin
	> class ModbusRemoteDeviceAccess :
		public T_ErrorOutputPin,
		public T_ExceptionOutputPin,
		public T_ID,
		public T_ModbusOutputPin
	{
	public:
		_V_PIN_( ModbusOutputPin )
		_V_PIN_( ErrorOutputPin )
		_V_PIN_( ExceptionOutputPin )

	public:
		_V_PROP_( ID )

	public:
		inline void SendError( uint8_t AError )
		{
			T_ErrorOutputPin::SetPinValue( AError );
		}

		inline void SendException( uint8_t AError )
		{
			T_ExceptionOutputPin::SetPinValue( AError );
		}

	};
//---------------------------------------------------------------------------
namespace Modbus
{
	template<typename T_OWNER, typename T_REQUEST> void SendRequest( T_OWNER &AOwner, T_REQUEST *AOwnerRequest, uint16_t AAddress, uint16_t ACount, uint8_t AFunctionCode )
	{
#ifdef __MODBUS__CLIENT__DEBUG__
		Serial.println( "DoClockReceive" );
#endif
		Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
		ATransaction->SenderProtocol = AOwnerRequest;

		Modbus::TModReadHeader *ARequest = new Modbus::TModReadHeader;
		ARequest->DeviceID = AOwner.ID();
		ARequest->FunctionCode = AFunctionCode;
		ARequest->SetAddress( AAddress );
		ARequest->SetCount( ACount );

//		uint8_t *ABuffer = new uint8_t[ sizeof( Modbus::TModReadHeader ) ];
		ATransaction->Buffer = TDataBlock( sizeof( *ARequest ), ARequest );

		ATransaction->Acquire();
		AOwner.ModbusOutputPin().PinNotify( ATransaction );
		ATransaction->Release();
	}

/*
	template<typename T_INSTANCE> void ProcessResponse_ReadBoolean( T_INSTANCE *AInstance, Modbus::TModbusTransaction *ATransaction )
	{
//			Serial.println( "ProcessResponse" );

		if( ! ATransaction->Buffer.Data )
			return;

		uint8_t *AData = ATransaction->Buffer.Data;

		if( ATransaction->Buffer.Size == 3 )
		{
			if( AData[ 1 ] != ( 0x80 | GetFunctionCode() ))
			{
				FOwner.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			Exception( AData[ 2 ] );
			return;
		}

		uint16_t AByteSize = ( GetCount() + 7 ) / 8;
		if( ATransaction->Buffer.Size != ( 3 + AByteSize ))
		{
			FOwner.SendError( Modbus::ErrorInvalidSize );
			return;
		}

		uint8_t *APtr = ATransaction->Buffer.Data;
		if( APtr[ 1 ] != GetFunctionCode() )
		{
			FOwner.SendError( Modbus::ErrorInvalidFunctionCode );
			return;
		}

		if( APtr[ 0 ] != GetDeviceID())
		{
			FOwner.SendError( Modbus::ErrorInvalidDeviceID );
			return;
		}

		if( APtr[ 2 ] != AByteSize )
		{
			FOwner.SendError( Modbus::ErrorInvalidNumberRegisters );
			return;
		}

//  AOffset := 0;
		SendOutput( APtr + 3, ATransaction->Buffer.Size );
//  Exit( True );
	}
*/
}
//---------------------------------------------------------------------------
/*
	template <
		typename T_OWNER, T_OWNER &C_OWNER
	> class TCLModbusReadBasicRequest : public ModbusServerBasicProtocol
	{
	public:
		uint16_t	Address = 0;

	protected:
		virtual uint16_t GetCount()
		{
			return 1;
		}

		virtual uint8_t GetFunctionCode() = 0;

	public:
		inline void ClockInputPin_o_Receive( void *_Data )
		{
			Modbus::SendRequest<T_OWNER>( C_OWNER, Address, GetCount(), GetFunctionCode() );
/ *
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( "DoClockReceive" );
#endif
			Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
			ATransaction->SenderProtocol = this;

			Modbus::TModReadHeader *ARequest = new Modbus::TModReadHeader;
			ARequest->DeviceID = GetDeviceID();
			ARequest->FunctionCode = GetFunctionCode();
			ARequest->SetAddress( inherited::Address );
			ARequest->SetCount( GetCount() );

//			uint8_t *ABuffer = new uint8_t[ sizeof( Modbus::TModReadHeader ) ];
			ATransaction->Buffer = TDataBlock( sizeof( *ARequest ), ARequest );

			ATransaction->Acquire();
			C_OWNER.ModbusOutputPin().PinNotify( ATransaction );
			ATransaction->Release();
* /
		}

	};
*/
//---------------------------------------------------------------------------
	template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		uint32_t C_COUNT,
		uint8_t C_FUNCTION_CODE,
		typename T_OutputPin
	> class TCLModbusReadBooleanBasicRequest : public ModbusServerBasicProtocol, // TCLModbusReadBasicRequest
		public T_Address,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( Address )

	protected:
		inline void Exception( uint8_t ACode )
		{
			C_OWNER.SendException( ACode );
		}

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
//			Serial.println( "ProcessResponse" );

			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | C_FUNCTION_CODE ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

				Exception( AData[ 2 ] );
				return;
			}

			uint16_t AByteSize = ( C_COUNT + 7 ) / 8;
			if( ATransaction->Buffer.Size != ( 3 + AByteSize ))
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			uint8_t *APtr = ATransaction->Buffer.Data;
			if( APtr[ 1 ] != C_FUNCTION_CODE )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( APtr[ 0 ] != C_OWNER.ID() )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}

			if( APtr[ 2 ] != AByteSize )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidNumberRegisters );
				return;
			}

//  AOffset := 0;
			SendOutput( APtr + 3, ATransaction->Buffer.Size );
//  Exit( True );
		}

	protected:
		inline void SendOutput( uint8_t *ADataPtr, int ACount )
		{
			T_OutputPin::SetPinValue(( *ADataPtr & 1 ) != 0 );
		}

	public:
		inline void ClockInputPin_o_Receive( void *_Data )
		{
			Modbus::SendRequest( C_OWNER, this, Address(), C_COUNT, C_FUNCTION_CODE );
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		uint32_t C_COUNT,
		uint8_t C_FUNCTION_CODE,
		typename T_OutputPin,
		typename T_TYPE
	> class ModbusRead16BitRegisterRequest : public ModbusServerBasicProtocol,
		public T_Address,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( Address )

	protected:
		inline uint16_t GetByteSize()
		{
			return ( C_COUNT * 2 );
		}

		inline uint16_t ConvertToSize( uint16_t ASize )
		{
			return ASize / 2;
		}

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( ATransaction->Buffer.ToString() );
#endif

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | C_FUNCTION_CODE ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

//				Exception( AData[ 2 ] );
				C_OWNER.SendException( AData[ 2 ] );
				return;
			}

			uint16_t AByteSize = GetByteSize();
			if( ATransaction->Buffer.Size != ( 3 + AByteSize ))
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			if( AData[ 1 ] != C_FUNCTION_CODE )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( AData[ 0 ] != C_OWNER.ID() )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}

			if( AData[ 2 ] != AByteSize )
			{
//				Serial.println( "ErrorInvalidNumberRegisters" );
				C_OWNER.SendError( Modbus::ErrorInvalidNumberRegisters );
				return;
			}

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( "ProcessResponse" );
#endif

			uint8_t *ADataPtr = ( AData + 3 );
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( *ADataPtr, HEX );
#endif
			SendOutput( ADataPtr, ConvertToSize( ATransaction->Buffer.Size - 3 ));
		}

	protected:
		inline void SendOutput( uint8_t *ADataPtr, int ACount )
		{
#ifdef VISUINO_MEMORY_ALLIGN
			T_TYPE AValue;
			memcpy( &AValue, ADataPtr, sizeof( T_TYPE ));
			uint16_t AValue16 = Modbus::Swap16( AValue );
#else
			uint16_t AValue16 = Modbus::Swap16( *(T_TYPE *)ADataPtr );
#endif
			T_OutputPin::SetPinValue( *((T_TYPE *) &AValue16 ) );
//			Serial.println( "SendOutput" );
		}

	public:
		inline void ClockInputPin_o_Receive( void *_Data )
		{
			Modbus::SendRequest( C_OWNER, this, Address(), C_COUNT, C_FUNCTION_CODE );
		}
	};
//---------------------------------------------------------------------------
	template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		uint32_t C_COUNT,
		uint8_t C_FUNCTION_CODE,
		typename T_MostSignificantWordFirst,
		typename T_OutputPin,
		typename T_TYPE
	> class ModbusRead32BitRegisterRequest : public ModbusServerBasicProtocol,
		public T_Address,
		public T_MostSignificantWordFirst,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( Address )
		_V_PROP_( MostSignificantWordFirst )

	protected:
		inline uint16_t GetByteSize()
		{
			return ( C_COUNT * 2 );
		}

		inline uint16_t ConvertToSize( uint16_t ASize )
		{
			return ASize / 2;
		}

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( ATransaction->Buffer.ToString() );
#endif

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | C_FUNCTION_CODE ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

//				Exception( AData[ 2 ] );
				C_OWNER.SendException( AData[ 2 ] );
				return;
			}

			uint16_t AByteSize = GetByteSize();
			if( ATransaction->Buffer.Size != ( 3 + AByteSize ))
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			if( AData[ 1 ] != C_FUNCTION_CODE )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( AData[ 0 ] != C_OWNER.ID() )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}

			if( AData[ 2 ] != AByteSize )
			{
//				Serial.println( "ErrorInvalidNumberRegisters" );
				C_OWNER.SendError( Modbus::ErrorInvalidNumberRegisters );
				return;
			}

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( "ProcessResponse" );
#endif

			uint8_t *ADataPtr = ( AData + 3 );
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( *ADataPtr, HEX );
#endif
			SendOutput( ADataPtr, ConvertToSize( ATransaction->Buffer.Size - 3 ));
		}

	protected:
		inline void SendOutput( uint8_t *ADataPtr, int ACount )
		{
#ifdef VISUINO_MEMORY_ALLIGN
			uint16_t ADataPtr16[ 2 ];
			memcpy( &ADataPtr16, ADataPtr, sizeof( uint32_t ));
#else // VISUINO_MEMORY_ALLIGN
			uint16_t *ADataPtr16 = (uint16_t *)ADataPtr;
#endif // VISUINO_MEMORY_ALLIGN
			uint32_t AValue;
			if( MostSignificantWordFirst() )
				AValue = ( uint32_t( Modbus::Swap16( ADataPtr16[ 0 ] ) ) << 16 ) | Modbus::Swap16( ADataPtr16[ 1 ] );

			else
				AValue = ( uint32_t( Modbus::Swap16( ADataPtr16[ 1 ] ) ) << 16 ) | Modbus::Swap16( ADataPtr16[ 0 ] );

			T_OutputPin::SetPinValue( *(T_TYPE *)&AValue );

//			OutputPin.SendValue<T>( Modbus::Swap16( *(T1 *)ADataPtr ));
//			Serial.println( "SendOutput" );
		}

	public:
		inline void ClockInputPin_o_Receive( void *_Data )
		{
			Modbus::SendRequest( C_OWNER, this, Address(), C_COUNT, C_FUNCTION_CODE );
		}
	};
//---------------------------------------------------------------------------
	template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		typename T_ClockInputPin_o_IsConnected,
		typename T_InitialValue,
		typename T
	> class ModbusWriteHoldingRegister16BitRequest : public ModbusServerBasicProtocol,
		public T_Address,
		public T_ClockInputPin_o_IsConnected,
		public T_InitialValue
	{
	public:
		_V_PROP_( Address )
		_V_PROP_( InitialValue )

	protected:
		_V_PROP_( ClockInputPin_o_IsConnected )

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
//			Serial.println( "ProcessResponse" );

			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | Modbus::mbfWriteOneReg ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

				C_OWNER.SendException( AData[ 2 ] );
				return;
			}

			if( ATransaction->Buffer.Size != 6 )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			uint8_t *APtr = ATransaction->Buffer.Data;
			if( APtr[ 1 ] != Modbus::mbfWriteOneReg )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( APtr[ 0 ] != C_OWNER.ID())
			{
				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}

		}

	public:
		void ClockInputPin_o_Receive( void *_Data )
		{
//			Serial.println( "DoClockReceive" );
			Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
			ATransaction->SenderProtocol = this;

			Modbus::TModWriteOneRegisterHeader *ARequest = new Modbus::TModWriteOneRegisterHeader;
			ARequest->DeviceID = C_OWNER.ID();
			ARequest->FunctionCode = Modbus::mbfWriteOneReg;
			ARequest->SetAddress( Address() );

			T AValue = InitialValue();

			ARequest->SetValue( *(uint16_t *)&AValue );

			ATransaction->Buffer = TDataBlock( sizeof( *ARequest ), ARequest );

			ATransaction->Acquire();
			C_OWNER.ModbusOutputPin().Notify( ATransaction );
			ATransaction->Release();
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
		{
			T AValue = Mitov::Constrain16<T>( *(uint32_t *)_Data );
			if( InitialValue() == AValue )
				return;

			InitialValue() = AValue;
			if( ! ClockInputPin_o_IsConnected() )
				ClockInputPin_o_Receive( nullptr );
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		typename T_ClockInputPin_o_IsConnected,
		typename T_InitialValue,
		typename T_MostSignificantWordFirst,
		typename T
	> class ModbusWriteHoldingRegister32BitRequest : public ModbusServerBasicProtocol,
		public T_Address,
		public T_ClockInputPin_o_IsConnected,
		public T_InitialValue,
		public T_MostSignificantWordFirst
	{
	public:
		_V_PROP_( Address )
		_V_PROP_( InitialValue )
		_V_PROP_( MostSignificantWordFirst )

	protected:
		_V_PROP_( ClockInputPin_o_IsConnected )

	protected:
		inline constexpr uint16_t GetCount()
		{
			return 2;
		}

		inline uint8_t GetByteCount( uint8_t ACount )
		{
			return ACount * sizeof( uint16_t );
		}

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | Modbus::mbfWriteRegs ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

				C_OWNER.SendException( AData[ 2 ] );
				return;
			}

			if( ATransaction->Buffer.Size != 6 )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			uint8_t *APtr = ATransaction->Buffer.Data;
			if( APtr[ 1 ] != Modbus::mbfWriteRegs )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( APtr[ 0 ] != C_OWNER.ID())
			{
				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}
		}

		inline void PopulateValues( uint8_t *ADataPtr )
		{
			T AInitialValue = InitialValue();
#ifdef VISUINO_MEMORY_ALLIGN
			uint16_t AValues[ 2 ];
			memcpy( AValues, &AInitialValue, 2 * sizeof( uint16_t ));
			AValues[ 0 ] = Modbus::Swap16( AValues[ 0 ] );
			AValues[ 1 ] = Modbus::Swap16( AValues[ 1 ] );
#endif
			if( MostSignificantWordFirst() )
			{
#ifdef VISUINO_MEMORY_ALLIGN
				memcpy( ADataPtr + sizeof( uint16_t ), AValues, sizeof( uint16_t ));
				memcpy( ADataPtr, AValues + 1, sizeof( uint16_t ));
#else
				((uint16_t *)ADataPtr)[ 1 ] = Modbus::Swap16( ((uint16_t *)&AInitialValue )[ 0 ] );
				((uint16_t *)ADataPtr)[ 0 ] = Modbus::Swap16( ((uint16_t *)&AInitialValue )[ 1 ] );
#endif
			}

			else
			{
#ifdef VISUINO_MEMORY_ALLIGN
				memcpy( ADataPtr, AValues, sizeof( uint16_t ) * 2 );
#else
				((uint16_t *)ADataPtr)[ 0 ] = Modbus::Swap16( ((uint16_t *)&AInitialValue )[ 0 ] );
				((uint16_t *)ADataPtr)[ 1 ] = Modbus::Swap16( ((uint16_t *)&AInitialValue )[ 1 ] );
#endif
			}

			ADataPtr += 2 * sizeof( uint16_t );
		}

	public:
		void ClockInputPin_o_Receive( void *_Data )
		{
//			Serial.println( "DoClockReceive" );
			Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
			ATransaction->SenderProtocol = this;

			uint16_t ACount = GetCount();
			uint16_t ABufferSize = sizeof( Modbus::TModWriteMultipleRegistersHeader ) + ACount * sizeof( uint16_t );
			uint8_t *ABuffer = new uint8_t[ ABufferSize ];

			Modbus::TModWriteMultipleRegistersHeader *ARequest = (Modbus::TModWriteMultipleRegistersHeader *)ABuffer;
			ARequest->DeviceID = C_OWNER.ID();
			ARequest->FunctionCode = Modbus::mbfWriteRegs;
			ARequest->SetAddress( Address() );
			ARequest->SetCountItems( ACount );
			ARequest->CountBytes = GetByteCount( ACount );

			uint8_t *ADataPtr = ABuffer + sizeof( Modbus::TModWriteMultipleRegistersHeader );

			PopulateValues( ADataPtr );

//			ARequest->SetCount( GetCount() );

//			uint8_t *ABuffer = new uint8_t[ sizeof( Modbus::TModReadHeader ) ];
			ATransaction->Buffer = TDataBlock( ABufferSize, ARequest );

			ATransaction->Acquire();
			C_OWNER.ModbusOutputPin().PinNotify( ATransaction );
			ATransaction->Release();
		}

		inline void InputPin_o_Receive( void *_Data )
		{
//			uint32_t AValue = MitovMin( *(uint32_t *)_Data, 0xFFFF );
			T AValue = *(T *)_Data;
			if( InitialValue() == AValue )
				return;

			InitialValue() = AValue;
			if( ! ClockInputPin_o_IsConnected() )
				ClockInputPin_o_Receive( nullptr );
		}

	};
/*
	class ModbusReadCoilRequest : public TCLModbusReadBooleanBasicRequest
	{
		typedef TCLModbusReadBooleanBasicRequest inherited;

	protected:
		virtual uint8_t GetFunctionCode() override
		{
			return Modbus::mbfReadCoils;
		}

	public:
		using inherited::inherited;

	};
*/
//---------------------------------------------------------------------------
	template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		typename T_ClockInputPin_o_IsConnected,
		typename T_InitialValue
	> class ModbusWriteCoilRequest : public ModbusServerBasicProtocol,
		public T_Address,
		public T_ClockInputPin_o_IsConnected,
		public T_InitialValue
	{
	public:
		_V_PROP_( Address )
		_V_PROP_( InitialValue )

	protected:
		_V_PROP_( ClockInputPin_o_IsConnected )

	protected:
		inline void Exception( uint8_t ACode )
		{
			C_OWNER.SendException( ACode );
		}

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
//			Serial.println( "ProcessResponse" );

			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | Modbus::mbfWriteOneCoil ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

				Exception( AData[ 2 ] );
				return;
			}

			if( ATransaction->Buffer.Size != 6 )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			uint8_t *APtr = ATransaction->Buffer.Data;
			if( APtr[ 1 ] != Modbus::mbfWriteOneCoil )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( APtr[ 0 ] != C_OWNER.ID() )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}

		}

	public:
		void ClockInputPin_o_Receive( void *_Data )
		{
//			Serial.println( "DoClockReceive" );
			Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
			ATransaction->SenderProtocol = this;

			Modbus::TModWriteOneRegisterHeader *ARequest = new Modbus::TModWriteOneRegisterHeader;
			ARequest->DeviceID = C_OWNER.ID();
			ARequest->FunctionCode = Modbus::mbfWriteOneCoil;
			ARequest->SetAddress( Address() );

			if( InitialValue() )
				ARequest->SetValue( 0xFF00 );

			else
				ARequest->SetValue( 0x0000 );

//			ARequest->SetCount( GetCount() );

//			uint8_t *ABuffer = new uint8_t[ sizeof( Modbus::TModReadHeader ) ];
			ATransaction->Buffer = TDataBlock( sizeof( *ARequest ), ARequest );

//			Serial.println( ATransaction->Buffer.ToString( 100, 16 ) );

			ATransaction->Acquire();
			C_OWNER.ModbusOutputPin().PinNotify( ATransaction );
			ATransaction->Release();
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
		{
			bool AValue = *(bool *)_Data;
			if( InitialValue() == AValue )
				return;

			InitialValue() = AValue;
			if( ! ClockInputPin_o_IsConnected() )
				ClockInputPin_o_Receive( nullptr );
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		uint32_t C_COUNT,
		uint8_t  C_FUNCTION_CODE,
		typename T_Items_SendOutput
	> class ModbusReadDigitalRequest : public ModbusServerBasicProtocol,
		public T_Address
	{
	public:
		_V_PROP_( Address )

	protected:
		inline constexpr uint16_t GetCount()
		{
			return C_COUNT;
		}

		inline void SendOutput( uint8_t *ADataPtr, int ACount )
		{
			uint8_t AOffset = 0;
			T_Items_SendOutput::Call( ADataPtr, ACount, AOffset );
/*
			for( int i = 0; i < FItems.size(); ++i )
			{
				if( ACount <= 0 )
					break;

				FItems[ i ]->SendOutput( ADataPtr, ACount, AOffset );
			}
*/
//			OutputPin.SendValue<uint32_t>( Modbus::Swap16( *ADataPtr ));
//			Serial.println( "SendOutput" );
		}

	protected:
		inline uint16_t GetByteSize()
		{
			return ( GetCount() + 7 ) / 8;
		}

		inline uint16_t ConvertToSize( uint16_t ASize )
		{
//			Serial.print( "ConvertToSize: " ); Serial.println( ASize );
			return ASize;
		}

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( ATransaction->Buffer.ToString() );
#endif

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | C_FUNCTION_CODE ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

				C_OWNER.SendException( AData[ 2 ] );
				return;
			}

			uint16_t AByteSize = GetByteSize();
			if( ATransaction->Buffer.Size != ( 3 + AByteSize ))
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			if( AData[ 1 ] != C_FUNCTION_CODE )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( AData[ 0 ] != C_OWNER.ID() )
			{

				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}

			if( AData[ 2 ] != AByteSize )
			{
//				Serial.println( "ErrorInvalidNumberRegisters" );
				C_OWNER.SendError( Modbus::ErrorInvalidNumberRegisters );
				return;
			}

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( "ProcessResponse" );
#endif

			uint8_t *ADataPtr = ( AData + 3 );
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( *ADataPtr, HEX );
#endif
			SendOutput( ADataPtr, ConvertToSize( ATransaction->Buffer.Size - 3 ));
		}

	public:
		inline void ClockInputPin_o_Receive( void *_Data )
		{
//			Serial.println( "Test1" );
			Modbus::SendRequest( C_OWNER, this, Address(), C_COUNT, C_FUNCTION_CODE );
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_OutputPin
	> class ModbusReadDigital : // public ModbusReadRegisterBasic
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		inline void SendOutput( uint8_t *&ADataPtr, int &ACount, uint8_t &AOffset )
		{
			if( ACount <= 0 )
				return;

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( "ModbusReadDigital.SendOutput" );
			Serial.println( AOffset );
			Serial.println( ACount );
			Serial.println( *ADataPtr );
			Serial.println( ( *ADataPtr ) & ( 1 << AOffset ) );
#endif
			T_OutputPin::SetPinValue(( ( *ADataPtr ) & ( 1 << AOffset ) ) != 0 );
			++AOffset;

			if( AOffset == 8 )
			{
				AOffset = 0;
				--ACount;
				++ADataPtr;
			}

/*
			OutputPin.SendValue<uint32_t>( Modbus::Swap16( *ADataPtr ));
			--ACount;
			++ADataPtr;
*/
		}

	};
//---------------------------------------------------------------------------
	template<
		int C_NUM_PINS,
		typename T_OutputPins_SetPinValue
	> class ModbusReadDigitalArray
	{
	public:
		inline constexpr uint16_t GetCount()
		{
			return C_NUM_PINS;
		}

		void SendOutput( uint8_t *&ADataPtr, int &ACount, uint8_t &AOffset )
		{
			for( int i = 0; i < C_NUM_PINS; ++i )
			{
				if( ACount <= 0 )
					break;

				bool AValue = (( *ADataPtr & ( 1 << AOffset ) ) != 0 );
				T_OutputPins_SetPinValue::Call( i, AValue );
				++AOffset;

				if( AOffset == 8 )
				{
					AOffset = 0;
					--ACount;
					++ADataPtr;
				}
			}
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		uint32_t C_COUNT,
		typename T_Items_PopulateValues
	> class ModbusWriteCoilsRequest : public ModbusServerBasicProtocol,
		public T_Address
	{
	public:
		_V_PROP_( Address )

	protected:
		inline uint8_t GetByteCount( uint8_t ACount )
		{
			return ( ACount + 7 ) / 8;
		}

	protected:
		inline constexpr uint16_t GetCount()
		{
			return C_COUNT;
		}

		inline void PopulateValues( uint8_t *ADataPtr )
		{
			uint8_t AOffset = 0;
			T_Items_PopulateValues::Call( ADataPtr, AOffset );
		}

	public:
		void ClockInputPin_o_Receive( void *_Data )
		{
//			Serial.println( "DoClockReceive" );
			Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
			ATransaction->SenderProtocol = this;

			uint16_t ACount = GetCount();
			uint16_t ABufferSize = sizeof( Modbus::TModWriteMultipleRegistersHeader ) + ACount * sizeof( uint16_t );
			uint8_t *ABuffer = new uint8_t[ ABufferSize ];

			Modbus::TModWriteMultipleRegistersHeader *ARequest = (Modbus::TModWriteMultipleRegistersHeader *)ABuffer;
			ARequest->DeviceID = C_OWNER.ID();
			ARequest->FunctionCode = Modbus::mbfWriteCoils;
			ARequest->SetAddress( Address() );
			ARequest->SetCountItems( ACount );
			ARequest->CountBytes = GetByteCount( ACount );

			uint8_t *ADataPtr = ABuffer + sizeof( Modbus::TModWriteMultipleRegistersHeader );

			PopulateValues( ADataPtr );

			ATransaction->Buffer = TDataBlock( ABufferSize, ARequest );

			ATransaction->Acquire();
			C_OWNER.ModbusOutputPin().PinNotify( ATransaction );
			ATransaction->Release();
		}

		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | Modbus::mbfWriteRegs ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

				C_OWNER.SendException( AData[ 2 ] );
				return;
			}

			if( ATransaction->Buffer.Size != 6 )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			uint8_t *APtr = ATransaction->Buffer.Data;
			if( APtr[ 1 ] != Modbus::mbfWriteRegs )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( APtr[ 0 ] != C_OWNER.ID())
			{
				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_InitialValue
	> class ModbusWriteDigital :
		public T_InitialValue
	{
	public:
		_V_PROP_( InitialValue )

	public:
		inline void PopulateValues( uint8_t *&ADataPtr, uint8_t &AOffset )
		{
//			Serial.println( "PopulateValues" );
			if( InitialValue() )
				*ADataPtr |= ( 1 << AOffset );

			else
				*ADataPtr &= ~( 1 << AOffset );

			++ AOffset;
			if( AOffset == 8 )
			{
				AOffset = 0;
				++ ADataPtr;
			}
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
		{
			InitialValue() = *(bool *)_Data;
		}

	};
//---------------------------------------------------------------------------
	template<int C_NUM_PINS> class ModbusWriteDigitalArray
	{
	protected:
		uint8_t	FValues[ ( C_NUM_PINS + 7 ) / 8 ] = { 0 };

	public:
		inline constexpr uint16_t GetCount()
		{
			return C_NUM_PINS;
		}

	public:
		inline void PopulateValues( uint8_t *&ADataPtr, uint8_t &AOffset )
		{
			for( int i = 0; i < C_NUM_PINS; ++i )
			{
				uint8_t	AMask = 1 << ( i % 8 );
//				if( InputPins[ i ].Value )
				if( FValues[ i / 8 ] & AMask )
					*ADataPtr |= ( 1 << AOffset );

				else
					*ADataPtr &= ~( 1 << AOffset );

				++ AOffset;
				if( AOffset == 8 )
				{
					AOffset = 0;
					++ ADataPtr;
				}
			}
		}

	public:
		inline void InputPins_o_Receive( int AIndex, void *_Data )
		{
			bool AValue = *(bool *)_Data;
			uint8_t	AMask = 1 << ( AIndex % 8 );

			if( AValue )
				FValues[ AIndex / 8 ] |= AMask;

			else
				FValues[ AIndex / 8 ] &= ~AMask;
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		uint32_t C_COUNT,
		uint8_t C_FUNCTION_CODE,
		typename T_Items_SendOutput
	> class ModbusReadRegistersRequest : public ModbusServerBasicProtocol,
		public T_Address
	{
	public:
		_V_PROP_( Address )

	protected:
		inline constexpr uint16_t GetCount()
		{
			return C_COUNT;
/*
			uint16_t AResult = 0;
			for( int i = 0; i < FItems.size(); ++i )
				AResult += FItems[ i ]->GetCount();

			return AResult;
*/
		}

	protected:
		inline void SendOutput( uint8_t *ADataPtr, int ACount )
		{
			uint8_t AOffset = 0;
			T_Items_SendOutput::Call( ADataPtr, ACount, AOffset );
/*
			for( int i = 0; i < FItems.size(); ++i )
			{
				if( ACount <= 0 )
					break;

				FItems[ i ]->SendOutput( ADataPtr, ACount, AOffset );
			}
*/
//			OutputPin.SendValue<uint32_t>( Modbus::Swap16( *ADataPtr ));
//			Serial.println( "SendOutput" );
		}

	protected:
		inline uint16_t GetByteSize()
		{
			return ( GetCount() * 2 );
		}

		inline uint16_t ConvertToSize( uint16_t ASize )
		{
			return ASize / 2;
		}

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( ATransaction->Buffer.ToString() );
#endif

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | C_FUNCTION_CODE ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

				C_OWNER.SendException( AData[ 2 ] );
				return;
			}

			uint16_t AByteSize = GetByteSize();
			if( ATransaction->Buffer.Size != ( 3 + AByteSize ))
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			if( AData[ 1 ] != C_FUNCTION_CODE )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( AData[ 0 ] != C_OWNER.ID() )
			{

				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}

			if( AData[ 2 ] != AByteSize )
			{
//				Serial.println( "ErrorInvalidNumberRegisters" );
				C_OWNER.SendError( Modbus::ErrorInvalidNumberRegisters );
				return;
			}

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( "ProcessResponse" );
#endif

			uint8_t *ADataPtr = ( AData + 3 );
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( *ADataPtr, HEX );
#endif
			SendOutput( ADataPtr, ConvertToSize( ATransaction->Buffer.Size - 3 ));
		}

	public:
		inline void ClockInputPin_o_Receive( void *_Data )
		{
//			Serial.println( "Test1" );
			Modbus::SendRequest( C_OWNER, this, Address(), C_COUNT, C_FUNCTION_CODE );
		}

	};
//---------------------------------------------------------------------------
	template<
		typename T_OutputPin,
		typename T, typename T1
	> class ModbusReadRegister16Bit :
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		void SendOutput( uint8_t *&ADataPtr, int &ACount, uint8_t &AOffset )
		{
#ifdef VISUINO_MEMORY_ALLIGN
			T1 AValue;
			memcpy( &AValue, ADataPtr, sizeof( T1 ));
			uint16_t AConverted = Modbus::Swap16( AValue );
#else
			uint16_t AConverted = Modbus::Swap16( *(uint16_t *)ADataPtr );
#endif
			T_OutputPin::SetPinValue( *(T1 *)&AConverted );
			--ACount;
			ADataPtr += sizeof( uint16_t );
		}

	};
//---------------------------------------------------------------------------
	template<
		typename T_MostSignificantWordFirst,
		typename T_OutputPin,
		typename T
	> class ModbusReadRegister32Bit :
		public T_MostSignificantWordFirst,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( MostSignificantWordFirst )

	public:
		void SendOutput( uint8_t *&ADataPtr, int &ACount, uint8_t &AOffset )
		{
#ifdef VISUINO_MEMORY_ALLIGN
			uint16_t ADataPtr16[ 2 ];
			memcpy( &ADataPtr16, ADataPtr, sizeof( uint32_t ));
#else // VISUINO_MEMORY_ALLIGN
			uint16_t *ADataPtr16 = (uint16_t *)ADataPtr;
#endif // VISUINO_MEMORY_ALLIGN
			uint32_t AValue;
			if( MostSignificantWordFirst() )
				AValue = ( uint32_t( Modbus::Swap16( ADataPtr16[ 0 ] ) ) << 16 ) | Modbus::Swap16( ADataPtr16[ 1 ] );

			else
				AValue = ( uint32_t( Modbus::Swap16( ADataPtr16[ 1 ] ) ) << 16 ) | Modbus::Swap16( ADataPtr16[ 0 ] );

			T_OutputPin::SetPinValue( *(T *)&AValue );
			ACount -= 2;
			ADataPtr += sizeof( uint32_t );
		}

	};
//---------------------------------------------------------------------------
	template<
		int C_COUNT_OutputPins,
		typename T_OutputPins_SetPinValue,
		typename T,
		typename T1
	> class ModbusReadRegister16BitArray
	{
	public:
		inline void SendOutput( uint8_t *&ADataPtr, int &ACount, uint8_t &AOffset )
		{
			for( int i = 0; i < C_COUNT_OutputPins; ++i )
			{
				if( ACount <= 0 )
					break;

#ifdef VISUINO_MEMORY_ALLIGN
				T1 AValue1;
				memcpy( &AValue1, ADataPtr, sizeof( T1 ));
				uint16_t AValue = Modbus::Swap16( AValue1 );
#else
				uint16_t AValue = Modbus::Swap16( *(uint16_t *)ADataPtr );
#endif
				T AOutputValue = *(T1 *)&AValue;
				T_OutputPins_SetPinValue::Call( i, AOutputValue );
				-- ACount;
				ADataPtr += sizeof( uint16_t );
			}
		}

	};
//---------------------------------------------------------------------------
	template<
		int C_COUNT_OutputPins,
		typename T_MostSignificantWordFirst,
		typename T_OutputPins_SetPinValue,
		typename T
	> class ModbusReadRegister32BitArray :
		public T_MostSignificantWordFirst
	{
	public:
		_V_PROP_( MostSignificantWordFirst )

	public:
		inline void SendOutput( uint8_t *&ADataPtr, int &ACount, uint8_t &AOffset )
		{
			for( int i = 0; i < C_COUNT_OutputPins; ++i )
			{
				if( ACount <= 0 )
					break;

#ifdef VISUINO_MEMORY_ALLIGN
				uint16_t ADataPtr16[ 2 ];
				memcpy( &ADataPtr16, ADataPtr, sizeof( uint32_t ));
#else // VISUINO_MEMORY_ALLIGN
				uint16_t *ADataPtr16 = (uint16_t *)ADataPtr;
#endif // VISUINO_MEMORY_ALLIGN
				uint32_t AValue;
				if( MostSignificantWordFirst() )
					AValue = ( uint32_t( Modbus::Swap16( ADataPtr16[ 0 ] ) ) << 16 ) | Modbus::Swap16( ADataPtr16[ 1 ] );

				else
					AValue = ( uint32_t( Modbus::Swap16( ADataPtr16[ 1 ] ) ) << 16 ) | Modbus::Swap16( ADataPtr16[ 0 ] );

				T_OutputPins_SetPinValue::Call( i, AValue );
				ACount -= 2;
				ADataPtr += sizeof( uint32_t );
			}
		}

	};
//---------------------------------------------------------------------------
		template <
		typename T_OWNER, T_OWNER &C_OWNER,
		typename T_Address,
		uint32_t C_COUNT,
		typename T_Items_PopulateValues
	> class ModbusWriteHoldingRegistersRequest : public ModbusServerBasicProtocol,
		public T_Address
	{
	public:
		_V_PROP_( Address )

	protected:
		inline uint8_t GetByteCount( uint8_t ACount )
		{
			return ACount * sizeof( uint16_t );
		}

	protected:
		inline constexpr uint16_t GetCount()
		{
			return C_COUNT;
/*
			uint16_t AResult = 0;
			for( int i = 0; i < FItems.size(); ++i )
				AResult += FItems[ i ]->GetCount();

			return AResult;
*/
		}

		inline void PopulateValues( uint8_t *ADataPtr )
		{
			uint8_t AOffset = 0;
			T_Items_PopulateValues::Call( ADataPtr, AOffset );
/*
			uint8_t AOffset = 0;
			for( int i = 0; i < FItems.size(); ++i )
				FItems[ i ]->PopulateValues( ADataPtr, AOffset );
*/
		}

	public:
		void ClockInputPin_o_Receive( void *_Data )
		{
//			Serial.println( "DoClockReceive" );
			Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
			ATransaction->SenderProtocol = this;

			uint16_t ACount = GetCount();
			uint16_t ABufferSize = sizeof( Modbus::TModWriteMultipleRegistersHeader ) + ACount * sizeof( uint16_t );
			uint8_t *ABuffer = new uint8_t[ ABufferSize ];

			Modbus::TModWriteMultipleRegistersHeader *ARequest = (Modbus::TModWriteMultipleRegistersHeader *)ABuffer;
			ARequest->DeviceID = C_OWNER.ID();
			ARequest->FunctionCode = Modbus::mbfWriteRegs;
			ARequest->SetAddress( Address() );
			ARequest->SetCountItems( ACount );
			ARequest->CountBytes = GetByteCount( ACount );

			uint8_t *ADataPtr = ABuffer + sizeof( Modbus::TModWriteMultipleRegistersHeader );

			PopulateValues( ADataPtr );

//			ARequest->SetCount( GetCount() );

//			uint8_t *ABuffer = new uint8_t[ sizeof( Modbus::TModReadHeader ) ];
			ATransaction->Buffer = TDataBlock( ABufferSize, ARequest );

			ATransaction->Acquire();
			C_OWNER.ModbusOutputPin().PinNotify( ATransaction );
			ATransaction->Release();
		}

	public:
		virtual void ProcessResponse( Modbus::TModbusTransaction *ATransaction ) override
		{
			if( ! ATransaction->Buffer.Data )
				return;

			uint8_t *AData = ATransaction->Buffer.Data;

			if( ATransaction->Buffer.Size == 3 )
			{
				if( AData[ 1 ] != ( 0x80 | Modbus::mbfWriteRegs ))
				{
					C_OWNER.SendError( Modbus::ErrorInvalidSize );
					return;
				}

				C_OWNER.SendException( AData[ 2 ] );
				return;
			}

			if( ATransaction->Buffer.Size != 6 )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidSize );
				return;
			}

			uint8_t *APtr = ATransaction->Buffer.Data;
			if( APtr[ 1 ] != Modbus::mbfWriteRegs )
			{
				C_OWNER.SendError( Modbus::ErrorInvalidFunctionCode );
				return;
			}

			if( APtr[ 0 ] != C_OWNER.ID())
			{
				C_OWNER.SendError( Modbus::ErrorInvalidDeviceID );
				return;
			}
		}

	};
//---------------------------------------------------------------------------
	template<
		typename T_InitialValue,
		typename T
	> class ModbusWriteRegister16Bit :
		public T_InitialValue
	{
	public:
		_V_PROP_( InitialValue )

	public:
		inline void PopulateValues( uint8_t *&ADataPtr, uint8_t &AOffset )
		{
			T AInitialValue = InitialValue();
//			Serial.println( AInitialValue );
#ifdef VISUINO_MEMORY_ALLIGN
			uint16_t AValue = Modbus::Swap16( *(uint16_t *)&AInitialValue );
			memcpy( ADataPtr, &AValue, sizeof( uint16_t ));
#else
			*((uint16_t *)ADataPtr ) = Modbus::Swap16( *(uint16_t *)&AInitialValue );
//			Serial.println( *((uint16_t *)ADataPtr ), HEX );
#endif
			ADataPtr += sizeof( uint16_t );
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
		{
//			uint32_t AValue = MitovMin( *(uint32_t *)_Data, 0xFFFF );
			InitialValue() = Mitov::Constrain16<T>( *(uint32_t *)_Data );
//			Serial.println( InitialValue().GetValue() );
		}

	};
//---------------------------------------------------------------------------
	template<
		typename T_InitialValue,
		typename T_MostSignificantWordFirst,
		typename T
	> class ModbusWriteRegister32Bit :
		public T_InitialValue,
		public T_MostSignificantWordFirst
	{
	public:
		_V_PROP_( InitialValue )
		_V_PROP_( MostSignificantWordFirst )

	public:
		inline void PopulateValues( uint8_t *&ADataPtr, uint8_t &AOffset )
		{
			T AInitialValue = InitialValue();
#ifdef VISUINO_MEMORY_ALLIGN
			uint16_t AValues[ 2 ];
			memcpy( AValues, &AInitialValue, 2 * sizeof( uint16_t ));
			AValues[ 0 ] = Modbus::Swap16( AValues[ 0 ] );
			AValues[ 1 ] = Modbus::Swap16( AValues[ 1 ] );
#endif
			if( MostSignificantWordFirst() )
			{
#ifdef VISUINO_MEMORY_ALLIGN
				memcpy( ADataPtr + sizeof( uint16_t ), AValues, sizeof( uint16_t ));
				memcpy( ADataPtr, AValues + 1, sizeof( uint16_t ));
#else
				((uint16_t *)ADataPtr)[ 1 ] = Modbus::Swap16( ((uint16_t *)&AInitialValue )[ 0 ] );
				((uint16_t *)ADataPtr)[ 0 ] = Modbus::Swap16( ((uint16_t *)&AInitialValue )[ 1 ] );
#endif
			}

			else
			{
#ifdef VISUINO_MEMORY_ALLIGN
				memcpy( ADataPtr, AValues, sizeof( uint16_t ) * 2 );
#else
				((uint16_t *)ADataPtr)[ 0 ] = Modbus::Swap16( ((uint16_t *)&AInitialValue )[ 0 ] );
				((uint16_t *)ADataPtr)[ 1 ] = Modbus::Swap16( ((uint16_t *)&AInitialValue )[ 1 ] );
#endif
			}

			ADataPtr += sizeof( uint32_t );
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
		{
			InitialValue() = *(T *)_Data;
		}

	};
//---------------------------------------------------------------------------
	template<
		int C_NUM_PINS,
		typename T,
		typename TYPE_PIN
	> class ModbusWriteRegister16BitArray
	{
	protected:
		T	FValues[ C_NUM_PINS ] = { 0 };

	public:
		inline void InputPins_o_Receive( int AIndex, void *_Data )
		{
			FValues[ AIndex ] = Mitov::Constrain16<T>( *(TYPE_PIN *)_Data );
		}

	public:
		inline void PopulateValues( uint8_t *&ADataPtr, uint8_t &AOffset )
		{
			for( int i = 0; i < C_NUM_PINS; ++i )
			{
				T AValue = FValues[ i ];
#ifdef VISUINO_MEMORY_ALLIGN
				uint16_t AValue16 = Modbus::Swap16( *(uint16_t *)&AValue );
				memcpy( ADataPtr, &AValue16, sizeof( uint16_t ));
#else
				*((uint16_t *)ADataPtr ) = Modbus::Swap16( *(uint16_t *)&AValue );
#endif
				ADataPtr += sizeof( uint16_t );
			}
		}

	};
//---------------------------------------------------------------------------
	template<
		int C_NUM_PINS,
		typename T_MostSignificantWordFirst,
		typename T
	> class ModbusWriteRegister32BitArray :
		public T_MostSignificantWordFirst
	{
	protected:
		T	FValues[ C_NUM_PINS ] = { 0 };

	public:
		_V_PROP_( MostSignificantWordFirst )

	public:
		inline void InputPins_o_Receive( int AIndex, void *_Data )
		{
			FValues[ AIndex ] = *(T *)_Data;
		}

	public:
		inline void PopulateValues( uint8_t *&ADataPtr, uint8_t &AOffset )
		{
			for( int i = 0; i < C_NUM_PINS; ++i )
			{
				T AValue = FValues[ i ];

#ifdef VISUINO_MEMORY_ALLIGN
				uint16_t AValues[ 2 ];
				memcpy( AValues, &AValue, 2 * sizeof( uint16_t ));
				AValues[ 0 ] = Modbus::Swap16( AValues[ 0 ] );
				AValues[ 1 ] = Modbus::Swap16( AValues[ 1 ] );
#endif
				if( MostSignificantWordFirst() )
				{
#ifdef VISUINO_MEMORY_ALLIGN
					memcpy( ADataPtr + sizeof( uint16_t ), AValues, sizeof( uint16_t ));
					memcpy( ADataPtr, AValues + 1, sizeof( uint16_t ));
#else
					((uint16_t *)ADataPtr)[ 1 ] = Modbus::Swap16( ((uint16_t *)&AValue )[ 0 ] );
					((uint16_t *)ADataPtr)[ 0 ] = Modbus::Swap16( ((uint16_t *)&AValue )[ 1 ] );
#endif
				}

				else
				{
#ifdef VISUINO_MEMORY_ALLIGN
					memcpy( ADataPtr, AValues, sizeof( uint16_t ) * 2 );
#else
					((uint16_t *)ADataPtr)[ 0 ] = Modbus::Swap16( ((uint16_t *)&AValue )[ 0 ] );
					((uint16_t *)ADataPtr)[ 1 ] = Modbus::Swap16( ((uint16_t *)&AValue )[ 1 ] );
#endif
				}

				ADataPtr += sizeof( uint32_t );
			}
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_OutputPin
	> class ModbusClientRawProtocol :
		public T_OutputPin
	{		
	public:
		_V_PIN_( OutputPin )

	protected:
		Modbus::TModbusTransaction *FTransaction = nullptr;

	public:
		void ModbusInputPin_o_Receive( void *_Data )
		{		
//			Serial.println( "DoModbusReceive" );
			if( FTransaction )
				FTransaction->Release();

			FTransaction = (Modbus::TModbusTransaction *)_Data;
			FTransaction->Acquire();

//			FSenderProtocol = FTransaction->SenderProtocol;
			T_OutputPin::SetPinValue( FTransaction->Buffer );
		}

		inline void InputPin_o_Receive( void *_Data )
		{		
//			Serial.println( "DoReceive" );
			if( ! FTransaction )
				return;

			delete [] FTransaction->Buffer.Data;
			FTransaction->Buffer.Data = nullptr;

			Mitov::TDataBlock *ABlock = (Mitov::TDataBlock *)_Data;

			uint8_t *AResponseData = new uint8_t[ ABlock->Size ];
			memcpy( AResponseData, ABlock->Data, ABlock->Size );
			FTransaction->Buffer.Data = AResponseData; 
			FTransaction->Buffer.Size = ABlock->Size;
//			ATransaction->Buffer = TDataBlock( ABlock.Size, ARequestData );

//			FTransaction->Buffer = *(Mitov::TDataBlock *)_Data;
			FTransaction->SenderProtocol->ProcessResponse( FTransaction );
//			Serial.println( "FTransaction->Release" );
			FTransaction->Release();
//			Serial.println( "FTransaction = nullptr" );
			FTransaction = nullptr;
		}

	};
//---------------------------------------------------------------------------	
	template <
		typename T_MaxTransactions,
		typename T_OutputPin,
		typename T_ResponseTimeout
	> class ModbusClientTCPProtocol :
		public T_MaxTransactions,
		public T_OutputPin,
		public T_ResponseTimeout
	{		
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( ResponseTimeout )
		_V_PROP_( MaxTransactions )

	protected:
		uint16_t	FLastTransactionID = 0;

	protected:
		struct TTransactionInfo
		{
			uint16_t	ID;
			uint32_t	StartTime;
			Modbus::TModbusTransaction	*Transaction;
		};

	protected:
		SimpleList<uint8_t, uint32_t>	FReceivedData;
		SimpleList<TTransactionInfo, uint32_t>	FTransactions;

	public:
		void ModbusInputPin_o_Receive( void *_Data )
		{		
			Modbus::TModbusTransaction *ATransaction = (Modbus::TModbusTransaction *)_Data;
			ATransaction->Acquire();

			uint8_t *AOutData = new uint8_t[ ATransaction->Buffer.Size + sizeof( Modbus::TModBusHeader ) ];
			memcpy( AOutData + sizeof( Modbus::TModBusHeader ), ATransaction->Buffer.Data, ATransaction->Buffer.Size );
			Modbus::TModBusHeader *AHeader = (Modbus::TModBusHeader *)AOutData;
			AHeader->TransactionID = FLastTransactionID;
			AHeader->ProtocolID = Modbus::MB_PROTOCOL;
			AHeader->RecLength = Modbus::Swap16( ATransaction->Buffer.Size );

			TTransactionInfo AInfo;
			AInfo.ID = FLastTransactionID ++;
			AInfo.Transaction = ATransaction;
			AInfo.StartTime = millis();
			while( FTransactions.size() > MaxTransactions() )
			{
				FTransactions[ 0 ].Transaction->Release();
				FTransactions.Delete( FTransactions.size() );
			}

			FTransactions.push_back( AInfo );

//			Serial.print( "FTransactions: " );
//			Serial.println( FTransactions.size() );

			Mitov::TDataBlock ABlock( ATransaction->Buffer.Size + sizeof( Modbus::TModBusHeader ), AOutData );
			T_OutputPin::SetPinValue( ABlock );

			delete [] AOutData;
		}

		inline void InputPin_o_Receive( void *_Data )
		{		
			Mitov::TDataBlock ABlock = *(Mitov::TDataBlock *)_Data;
//			Serial.println( ABlock.ToString() );
			FReceivedData.append( ABlock.Data, ABlock.Size );
			if( FReceivedData.size() < sizeof( Modbus::TModBusHeader ))
				return;

			uint8_t *APtr = (uint8_t *)FReceivedData;
			Modbus::TModBusHeader *AHeader = (Modbus::TModBusHeader *)APtr;
			uint16_t ASize = Modbus::Swap16( AHeader->RecLength );

			if( AHeader->ProtocolID != Modbus::MB_PROTOCOL )
				return;

			uint16_t AExpectedSize = sizeof( Modbus::TModBusHeader ) + ASize;
			if( AExpectedSize > FReceivedData.size() )
				return;

			APtr += sizeof( Modbus::TModBusHeader );
			Modbus::TModbusTransaction *ATransaction = nullptr;
			for( int i = 0; i < FTransactions.size(); ++i )
				if( FTransactions[ i ].ID == AHeader->TransactionID )
				{
					ATransaction = FTransactions[ i ].Transaction;
					FTransactions.Delete( i );
					break;
				}


			if( ! ATransaction )
			{
				FReceivedData.Delete( 0, AExpectedSize );
				return;
			}

//				TTransaction *ATransaction = new TTransaction;
//				ATransaction->ID = AHeader->TransactionID;
			uint8_t *ARequestData = new uint8_t[ ASize ];
			memcpy( ARequestData, APtr, ASize );

			delete [] ATransaction->Buffer.Data;
			ATransaction->Buffer.Data = ARequestData;
			ATransaction->Buffer.Size = ASize;
//				ATransaction->Buffer = TDataBlock( ASize, ARequestData );

			FReceivedData.Delete( 0, AExpectedSize );

//				FTransactions.push_back( ATransaction );
//				ATransaction->Acquire();
//				ModbusInputPin.Notify( ATransaction );
			ATransaction->SenderProtocol->ProcessResponse( ATransaction );
			ATransaction->Release();
		}

	public:
		void SystemLoopBegin()
		{
			uint32_t ACurrentTime = millis();
			for( int i = FTransactions.size(); i--; )
				if( ( ACurrentTime - FTransactions[ i ].StartTime ) > ResponseTimeout() )
					FTransactions.Delete( i );

//			inherited::SystemLoopBegin();
		}
	};
//---------------------------------------------------------------------------
	template <
		typename T_AsciiFormat,
		typename T_FDetectTimerRunning,
		typename T_FTimeoutTimerRunning,
		typename T_InputBufferSizeLimit,
		typename T_MaxTransactions,
		typename T_OutputPin,
		typename T_PacketDetectInterval,
		typename T_ResponseTimeout
	> class ModbusClientSerialProtocol :
		public T_AsciiFormat,
		public T_FDetectTimerRunning,
		public T_FTimeoutTimerRunning,
		public T_InputBufferSizeLimit,
		public T_MaxTransactions,
		public T_OutputPin,
		public T_PacketDetectInterval,
		public T_ResponseTimeout
	{		
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( InputBufferSizeLimit )
		_V_PROP_( MaxTransactions )
		_V_PROP_( ResponseTimeout )
		_V_PROP_( PacketDetectInterval )
		_V_PROP_( AsciiFormat )

	protected:
		_V_PROP_( FDetectTimerRunning )
		uint32_t	FDetectStartTime;

		_V_PROP_( FTimeoutTimerRunning )
		uint32_t	FTimeoutStartTime;

	protected:
		Modbus::TModbusTransaction *FCurrentTransaction = nullptr;

	protected:
		SimpleList<uint8_t, uint32_t>	FReceivedData;
		SimpleList<Modbus::TModbusTransaction *, uint32_t>	FTransactions;

	protected:
		void TryProcessAsciiAcumulattor()
		{
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( "TryProcessAsciiAcumulattor" );
#endif
			uint8_t *ABuffer;
			uint16_t AOutSize;
			while( Modbus::TryExtractBuffer( FReceivedData, ABuffer, AOutSize ))
			{
				if( FCurrentTransaction )
				{
					delete [] FCurrentTransaction->Buffer.Data;
					FCurrentTransaction->Buffer.Data = ABuffer;
					FCurrentTransaction->Buffer.Size = AOutSize;
//					ATransaction->Buffer = TDataBlock( AOutSize, ABuffer );

//#ifdef __MODBUS__CLIENT__DEBUG__
//					Serial.println( FCurrentTransaction->Buffer.ToString() );
//#endif
					FCurrentTransaction->SenderProtocol->ProcessResponse( FCurrentTransaction );
					FCurrentTransaction->Release();
					FCurrentTransaction = nullptr;
					FTimeoutTimerRunning() = false;
					TrySend();
					if( ! FCurrentTransaction )
					{
						FReceivedData.clear();
						return;
					}
				}
			}
		}

		void TryProcessAcumulattor()
		{
			uint32_t ASize = FReceivedData.size();

			if( ASize < 2 )
			{
				FReceivedData.clear();
				return;
			}

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.print( "( " );
			for( int i = 0; i < ASize; ++i )
			{
				Serial.print( FReceivedData[ i ] );
				Serial.print( " " );
			}

			Serial.println( ")" );
#endif

#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( "Check CRC" );
#endif

			uint8_t *APtr = FReceivedData;

			uint16_t ACRC = Modbus::CRC16_2( APtr, ASize - 2 );

#ifdef VISUINO_MEMORY_ALLIGN
			uint16_t ABufferCRC;
			memcpy( &ABufferCRC, APtr + ASize - 2, sizeof( uint16_t ) );
#else
			uint16_t ABufferCRC = *(uint16_t *)( APtr + ASize - 2 );
#endif
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( ACRC );
			Serial.println( ABufferCRC );
#endif
			if( ACRC != ABufferCRC )
			{
				FReceivedData.clear();
				return;
			}

			FDetectTimerRunning() = false;
			if( ! FCurrentTransaction )
			{
				FReceivedData.clear();
				return;
			}

//			Serial.println( "Process1" );
			ASize -= 2;
			uint8_t *ARequestData = new uint8_t[ ASize ];
			memcpy( ARequestData, APtr, ASize );

//			Serial.println( "Process2" );
			delete [] FCurrentTransaction->Buffer.Data;
			FCurrentTransaction->Buffer.Data = ARequestData;
			FCurrentTransaction->Buffer.Size = ASize;

			FReceivedData.clear();

//			FReceivedData.Delete( 0, AExpectedSize );
//			Serial.println( "Process3" );
			FCurrentTransaction->SenderProtocol->ProcessResponse( FCurrentTransaction );
			FCurrentTransaction->Release();

			FCurrentTransaction = nullptr;

			TrySend();

/*
			Modbus::TModbusTransaction *ATransaction = new Modbus::TModbusTransaction;
			ATransaction->SenderProtocol = this;

			uint8_t *ARequestData = new uint8_t[ ASize - 2 ];
			memcpy( ARequestData, APtr, ASize - 2 );
			FReceivedData.clear();

			ATransaction->Buffer = TDataBlock( ASize, ARequestData );

			ATransaction->Acquire();
			ModbusOutputPin.Notify( ATransaction );
			ATransaction->Release();
*/
/*
			uint8_t *ARequestData = new uint8_t[ ASize ];
			memcpy( ARequestData, APtr, ASize );

			delete [] ATransaction->Buffer.Data;
			ATransaction->Buffer.Data = ARequestData;
			ATransaction->Buffer.Size = ASize;
//				ATransaction->Buffer = TDataBlock( ASize, ARequestData );

			FReceivedData.Delete( 0, AExpectedSize );

//				FTransactions.push_back( ATransaction );
//				ATransaction->Acquire();
//				ModbusInputPin.Notify( ATransaction );
			ATransaction->SenderProtocol->ProcessResponse( ATransaction );
			ATransaction->Release();
*/
		}

		inline bool CanSend()
		{
			return ( FCurrentTransaction == nullptr );
		}

		void TrySend()
		{
			if( ! FTransactions.size() )
				return;

			if( ! CanSend() )
				return;

			if( FCurrentTransaction )
				FCurrentTransaction->Release();

			FCurrentTransaction = FTransactions[ 0 ];
			FTransactions.Delete( 0 );

			uint32_t AOutSize;
			uint8_t *AOutData = Modbus::MakeOutputBuffer( AsciiFormat(), FCurrentTransaction->Buffer, AOutSize );
			Mitov::TDataBlock ABlock( AOutSize, AOutData );
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( ABlock.ToString() );
			Serial.println( "TrySend" );
#endif
			T_OutputPin::SetPinValue( ABlock );
			delete [] AOutData;

			FTimeoutTimerRunning() = true;
			FTimeoutStartTime = millis();
		}

	public:
		void ModbusInputPin_o_Receive( void *_Data )
		{		
//			Serial.println( "DoModbusReceive" );
			Modbus::TModbusTransaction *ATransaction = (Modbus::TModbusTransaction *)_Data;
			ATransaction->Acquire();

			while( FTransactions.size() > MaxTransactions() )
			{
				FTransactions[ 0 ]->Release();
				FTransactions.Delete( 0 );
			}

			FTransactions.push_back( ATransaction );

			TrySend();
		}

		inline void InputPin_o_Receive( void *_Data )
		{		
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( "DoReceive" );
#endif
			Mitov::TDataBlock ABlock = *(Mitov::TDataBlock *)_Data;
#ifdef __MODBUS__CLIENT__DEBUG__
			Serial.println( ABlock.ToString() );
#endif
			FReceivedData.append( ABlock.Data, ABlock.Size );

			if( FReceivedData.size() > InputBufferSizeLimit().GetValue() )
				FReceivedData.Delete( 0, FReceivedData.size() - InputBufferSizeLimit().GetValue() );

			if( AsciiFormat() )
				TryProcessAsciiAcumulattor();

			else
			{
				// Reset the timer
				FDetectTimerRunning() = true;
				FDetectStartTime = millis();
			}
		}

	public:
		void SystemLoopBegin()
		{
			if( FDetectTimerRunning() )
				if( ( millis() - FDetectStartTime ) >= PacketDetectInterval() )
					TryProcessAcumulattor();

			if( FTimeoutTimerRunning() )
				if( ( millis() - FTimeoutStartTime ) >= ResponseTimeout() )
				{
					// Timeout
					FDetectTimerRunning() = false;

					if( FCurrentTransaction )
						FCurrentTransaction->Release();

					FCurrentTransaction = nullptr;
					FReceivedData.clear();
					TrySend();
				}

//			inherited::SystemLoopBegin();
		}

	public:
		inline ModbusClientSerialProtocol()
		{
			FDetectTimerRunning() = false;
			FTimeoutTimerRunning() = false;
		}
	};
//---------------------------------------------------------------------------
/*
//---------------------------------------------------------------------------
	void TArduinoModbusBasicRequest::Exception( uint8_t ACode )
	{
		FOwner.SendException( ACode );
	}
//---------------------------------------------------------------------------
*/
//---------------------------------------------------------------------------
}

#ifdef __TEST_FOR_DEBUG_PRINTS__
#undef Serial
#endif