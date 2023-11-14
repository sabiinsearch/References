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
	namespace Fourier
	{
		void PerformFourier( Mitov::TComplex *ABuffer, uint16_t samples, uint8_t order )
		{
			// Reverse bits
			uint16_t j = 0;
			for (uint16_t i = 0; i < (samples - 1); i++) 
			{
				if (i < j) 
				{
					swap( ABuffer[i].Real, ABuffer[j].Real);
					swap( ABuffer[i].Imaginary, ABuffer[j].Imaginary);
				}

				uint16_t k = (samples >> 1);
				while (k <= j) 
				{
					j -= k;
					k >>= 1;
				}

				j += k;
			}

			// Compute the FFT
			double c1 = -1.0;
			double c2 = 0.0;
			uint16_t l2 = 1;
			for (uint8_t l = 0; (l < order); l++) 
			{
//				Serial.println( l );

				uint16_t l1 = l2;
				l2 <<= 1;
				double u1 = 1.0;
				double u2 = 0.0;
				for (j = 0; j < l1; j++) 
				{
					for (uint16_t i = j; i < samples; i += l2) 
					{
//						Serial.println( i );
						uint16_t i1 = i + l1;
						double t1 = u1 * ABuffer[i1].Real - u2 * ABuffer[i1].Imaginary;
						double t2 = u1 * ABuffer[i1].Imaginary + u2 * ABuffer[i1].Real;
						ABuffer[i1].Real = ABuffer[i].Real - t1;
						ABuffer[i1].Imaginary = ABuffer[i].Imaginary - t2;
						ABuffer[i].Real += t1;
						ABuffer[i].Imaginary += t2;
					}

					double z = ((u1 * c1) - (u2 * c2));
					u2 = ((u1 * c2) + (u2 * c1));
					u1 = z;
				}

				c2 = -sqrt((1.0 - c1) / 2.0);
//				if (dir == FFT_FORWARD) 
//					c2 = -c2;

				c1 = sqrt((1.0 + c1) / 2.0);
			}

//			Serial.println( "DONE" );
			// Scaling for reverse transform
/*
			if (dir != FFT_FORWARD) 
				for (uint16_t i = 0; i < samples; i++) 
				{
					ABuffer[i].Real /= samples;
					ABuffer[i].Imaginary /= samples;
				}
*/
		}
//---------------------------------------------------------------------------
		void PerformInverseFourier( Mitov::TComplex *ABuffer, uint16_t samples, uint8_t order )
		{
			// Reverse bits
			uint16_t j = 0;
			for (uint16_t i = 0; i < (samples - 1); i++) 
			{
				if (i < j) 
				{
					swap( ABuffer[i].Real, ABuffer[j].Real);
					swap( ABuffer[i].Imaginary, ABuffer[j].Imaginary);
				}

				uint16_t k = (samples >> 1);
				while (k <= j) 
				{
					j -= k;
					k >>= 1;
				}

				j += k;
			}

			// Compute the FFT
			double c1 = -1.0;
			double c2 = 0.0;
			uint16_t l2 = 1;
			for (uint8_t l = 0; (l < order); l++) 
			{
//				Serial.println( l );

				uint16_t l1 = l2;
				l2 <<= 1;
				double u1 = 1.0;
				double u2 = 0.0;
				for (j = 0; j < l1; j++) 
				{
					for (uint16_t i = j; i < samples; i += l2) 
					{
//						Serial.println( i );
						uint16_t i1 = i + l1;
						double t1 = u1 * ABuffer[i1].Real - u2 * ABuffer[i1].Imaginary;
						double t2 = u1 * ABuffer[i1].Imaginary + u2 * ABuffer[i1].Real;
						ABuffer[i1].Real = ABuffer[i].Real - t1;
						ABuffer[i1].Imaginary = ABuffer[i].Imaginary - t2;
						ABuffer[i].Real += t1;
						ABuffer[i].Imaginary += t2;
					}

					double z = ((u1 * c1) - (u2 * c2));
					u2 = ((u1 * c2) + (u2 * c1));
					u1 = z;
				}

				c2 = sqrt((1.0 - c1) / 2.0);
//				if (dir == FFT_FORWARD) 
//					c2 = -c2;

				c1 = sqrt((1.0 + c1) / 2.0);
			}

//			Serial.println( "DONE" );
			// Scaling for reverse transform
//			if (dir != FFT_FORWARD) 
				for (uint16_t i = 0; i < samples; i++) 
				{
					ABuffer[i].Real /= samples;
					ABuffer[i].Imaginary /= samples;
				}

		}
	};
//---------------------------------------------------------------------------
	template <
		typename T_Enabled,
		typename T_FrequencyOutputPin,
		typename T_Order,
		typename T_OutputPin,
		typename T_SampleRate,
		typename T_SpectrumOutputPin
	> class ForwardFFT :
		public T_Enabled,
		public T_FrequencyOutputPin,
		public T_Order,
		public T_OutputPin,
		public T_SampleRate,
		public T_SpectrumOutputPin
	{
	public:
		_V_PIN_( OutputPin )
		_V_PIN_( SpectrumOutputPin )
		_V_PIN_( FrequencyOutputPin )

	public:
		_V_PROP_( Enabled )
		_V_PROP_( Order )
		_V_PROP_( SampleRate )

	protected:
		Mitov::TComplex	*FBuffer;
		uint16_t	FIndex = 0;

	public:
		void UpdateOrder()
		{
			delete [] FBuffer;

			FBuffer = new Mitov::TComplex[ 1 << Order ];
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
		{
			if( ! Enabled() )
				return;

			TArray<Mitov::TComplex> *AInArray = (TArray<Mitov::TComplex>*)_Data;

			uint16_t samples = 1 << Order();

//			Serial.println( "DoReceive" );

			Mitov::TComplex *AInDataPtr = AInArray->Data;
			uint16_t ARemainingSize = AInArray->Size;
//			Serial.print( "InSize = " ); Serial.println( ARemainingSize );
//			return;
			while( ARemainingSize )
			{
				uint16_t ARemainingOutSize = samples - FIndex;
				uint16_t ACopySize = MitovMin( ARemainingOutSize, ARemainingSize );

//				Serial.print( "FIndex = " ); Serial.println( FIndex );
//				Serial.print( "samples = " ); Serial.println( samples );
//				Serial.print( "ARemainingSize = " ); Serial.println( ARemainingSize );
//				Serial.print( "ARemainingOutSize = " ); Serial.println( ARemainingOutSize );
//				Serial.print( "ACopySize = " ); Serial.println( ACopySize );

				memcpy( FBuffer + FIndex, AInDataPtr, ACopySize * sizeof( Mitov::TComplex ) );
				FIndex += ACopySize;
				AInDataPtr += ACopySize;
				ARemainingSize -= ACopySize;

//				Serial.print( "FIndex = " ); Serial.println( FIndex );
//				delay( 1000 );
				if( FIndex >= samples )
				{
//					Serial.println( "FFT" );
//					delay( 1000 );
					FIndex = 0;
					Fourier::PerformFourier( FBuffer, samples, Order() );
					TArray<Mitov::TComplex> AComplexBuffer( samples, FBuffer );
//					Serial.println( "OUT" );
					T_OutputPin::SetPinValue( AComplexBuffer );
					if( T_SpectrumOutputPin::GetPinIsConnected() || T_FrequencyOutputPin::GetPinIsConnected() )
					{
						float *AData = new float[ samples ];

						for( uint16_t i = 0; i < samples; i++ ) 
							AData[i] = sqrt(sq(FBuffer[i].Real) + sq(FBuffer[i].Imaginary));

						if( T_SpectrumOutputPin::GetPinIsConnected() )
						{
							TArray<float> ARealBuffer( samples, AData );
							T_SpectrumOutputPin::SetPinValue( ARealBuffer );
						}

						if( T_FrequencyOutputPin::GetPinIsConnected() )
						{
							int AMaxIndex = 1;
							float AMaxValue = AData[ 1 ];
							for( uint16_t i = 1; i < samples; i++ ) 
								if( AMaxValue < AData[ i ] )
								{
									AMaxIndex = i;
									AMaxValue = AData[ i ];
								}

							float AMaxFrequency = ( SampleRate() / 2.0 );
							float AFrequency = AMaxFrequency * AMaxIndex / samples;

							T_FrequencyOutputPin::SetPinValue( AFrequency );
						}
	
						delete [] AData;
					}
				}
			}


//			Mitov::TComplex *AOutData = new Mitov::TComplex[ AInArray->Size ];


//			delete [] AOutData;
		}

	public:
		void SystemInit()
		{
//			inherited::SystemInit();
			FBuffer = new Mitov::TComplex[ 1 << Order() ];
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_Enabled,
		typename T_Order,
		typename T_OutputPin
	> class InverseComplexFFT :
		public T_Enabled,
		public T_Order,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( Enabled )
		_V_PROP_( Order )

	protected:
		Mitov::TComplex	*FBuffer;
		uint16_t	FIndex = 0;

	public:
		void UpdateOrder()
		{
			delete [] FBuffer;

			FBuffer = new Mitov::TComplex[ 1 << Order ];
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
		{
			if( ! Enabled() )
				return;

			TArray<Mitov::TComplex> *AInArray = (TArray<Mitov::TComplex>*)_Data;

			uint16_t samples = 1 << Order();

//			Serial.println( "DoReceive" );

			Mitov::TComplex *AInDataPtr = AInArray->Data;
			uint16_t ARemainingSize = AInArray->Size;

//			Serial.print( "InSize = " ); Serial.println( ARemainingSize );
//			return;
			while( ARemainingSize )
			{
				uint16_t ARemainingOutSize = samples - FIndex;
				uint16_t ACopySize = MitovMin( ARemainingOutSize, ARemainingSize );

//				Serial.print( "FIndex = " ); Serial.println( FIndex );
//				Serial.print( "samples = " ); Serial.println( samples );
//				Serial.print( "ARemainingSize = " ); Serial.println( ARemainingSize );
//				Serial.print( "ARemainingOutSize = " ); Serial.println( ARemainingOutSize );
//				Serial.print( "ACopySize = " ); Serial.println( ACopySize );

				memcpy( FBuffer + FIndex, AInDataPtr, ACopySize * sizeof( Mitov::TComplex ) );
				FIndex += ACopySize;
				AInDataPtr += ACopySize;
				ARemainingSize -= ACopySize;

//				Serial.print( "FIndex = " ); Serial.println( FIndex );
//				delay( 1000 );
				if( FIndex >= samples )
				{
//					Serial.println( "FFT" );
//					delay( 1000 );
					FIndex = 0;
					Fourier::PerformInverseFourier( FBuffer, samples, Order() );
					TArray<Mitov::TComplex> AComplexBuffer( samples, FBuffer );
//					Serial.println( "OUT" );
					T_OutputPin::SetPinValue( AComplexBuffer );
//					Serial.println( "OUT END" );
				}
			}
		}

	public:
		void SystemInit()
		{
//			inherited::SystemInit();
			FBuffer = new Mitov::TComplex[ 1 << Order() ];
		}

	};
//---------------------------------------------------------------------------
	template <
		typename T_Enabled,
		typename T_Order,
		typename T_OutputPin
	> class InverseAnalogFFT :
		public T_Enabled,
		public T_Order,
		public T_OutputPin
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( Enabled )
		_V_PROP_( Order )

	protected:
		Mitov::TComplex	*FBuffer;
		uint16_t	FIndex = 0;

	public:
		void UpdateOrder()
		{
			delete [] FBuffer;

			FBuffer = new Mitov::TComplex[ 1 << Order ];
		}

	public:
		inline void InputPin_o_Receive( void *_Data )
		{
			if( ! Enabled() )
				return;

			TArray<Mitov::TComplex> *AInArray = (TArray<Mitov::TComplex>*)_Data;

			uint16_t samples = 1 << Order();

//			Serial.println( "DoReceive" );

			Mitov::TComplex *AInDataPtr = AInArray->Data;
			uint16_t ARemainingSize = AInArray->Size;

//			Serial.print( "InSize = " ); Serial.println( ARemainingSize );
//			return;
			while( ARemainingSize )
			{
				uint16_t ARemainingOutSize = samples - FIndex;
				uint16_t ACopySize = MitovMin( ARemainingOutSize, ARemainingSize );

//				Serial.print( "FIndex = " ); Serial.println( FIndex );
//				Serial.print( "samples = " ); Serial.println( samples );
//				Serial.print( "ARemainingSize = " ); Serial.println( ARemainingSize );
//				Serial.print( "ARemainingOutSize = " ); Serial.println( ARemainingOutSize );
//				Serial.print( "ACopySize = " ); Serial.println( ACopySize );

				memcpy( FBuffer + FIndex, AInDataPtr, ACopySize * sizeof( Mitov::TComplex ) );
				FIndex += ACopySize;
				AInDataPtr += ACopySize;
				ARemainingSize -= ACopySize;

//				Serial.print( "FIndex = " ); Serial.println( FIndex );
//				delay( 1000 );
				if( FIndex >= samples )
				{
//					Serial.println( "FFT" );
//					delay( 1000 );
					FIndex = 0;
					Fourier::PerformInverseFourier( FBuffer, samples, Order() );
					TArray<Mitov::TComplex> AComplexBuffer( samples, FBuffer );
//					Serial.println( "OUT" );
					float *AData = new float[ samples ];

					for( uint16_t i = 0; i < samples; i++ ) 
						AData[i] = FBuffer[i].Real;

					TArray<float> ARealBuffer( samples, AData );

					T_OutputPin::SetPinValue( ARealBuffer );

					delete [] AData;
//					Serial.println( "OUT END" );
				}
			}
		}

	public:
		void SystemInit()
		{
//			inherited::SystemInit();
			FBuffer = new Mitov::TComplex[ 1 << Order() ];
		}

	};
//---------------------------------------------------------------------------
}

#ifdef __TEST_FOR_DEBUG_PRINTS__
#undef Serial
#endif