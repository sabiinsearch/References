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
	enum TArduinoAnalogWindowType { wtHamming, wtHann, wtBartlett, wtBlackman, wtFlatTop, wtWelch, wtKaiser };
//---------------------------------------------------------------------------
	template<
		typename T_Alpha,
		typename T_Enabled,
		typename T_OutputPin,
		typename T_Type
	> class AnalogApplyWindow : 
		public T_Alpha,
		public T_Enabled,
		public T_OutputPin,
		public T_Type
	{
	public:
		_V_PIN_( OutputPin )

	public:
		_V_PROP_( Type )
		_V_PROP_( Enabled )
		_V_PROP_( Alpha )

	protected:
		float Bessel(double x)
		{
			float Sum = 0.0f, XtoIpower;
			int i, j, Factorial;
			for(i=1; i<10; i++)
			{
				XtoIpower = pow(x/2.0, (float)i);
				Factorial = 1;
				for(j=1; j<=i; j++)Factorial *= j;
					Sum += pow(XtoIpower / (float)Factorial, 2.0);
			}

			return(1.0 + Sum);
		}

	public:
		void InputPin_o_Receive( void *_Data )
		{
			if( ! Enabled() )
				T_OutputPin::PinNotify( _Data );

			else
			{
				TArray<float> *AInArray = (TArray<float>*)_Data;
				float *AOutData = new float[ AInArray->Size ];

				int ALengthMinusOne = AInArray->Size - 1;
				switch( Type() )
				{
					case wtHamming:
						for( int i = 0; i <= ALengthMinusOne; ++i )							
							AOutData[ i ] = AInArray->Data[ i ] * ( 0.54 - 0.46 * cos( ( 2 * PI * i ) / ALengthMinusOne));

						break;

					case wtHann:
						for( int i = 0; i <= ALengthMinusOne; ++i )							
							AOutData[ i ] = AInArray->Data[ i ] * ( 0.5 - 0.5 * cos( ( 2 * PI * i ) / ALengthMinusOne));

						break;

					case wtBartlett:
						for( int i = 0; i <= ALengthMinusOne; ++i )							
							AOutData[ i ] = AInArray->Data[ i ] * ( 1.0 - ((2.0 * abs(i - (float( ALengthMinusOne ) / 2.0))) / ALengthMinusOne));

						break;

					case wtBlackman:
						for( int i = 0; i <= ALengthMinusOne; ++i )							
							AOutData[ i ] = AInArray->Data[ i ] * ( (( Alpha() + 1 ) / 2 ) - 0.5 * cos( ( 2 * PI * i ) / ( ALengthMinusOne ) ) - ( Alpha() / 2 ) * cos( ( 4 * PI * i ) / ( ALengthMinusOne ) ));

						break;

					case wtWelch:
						for( int i = 0; i <= ALengthMinusOne; ++i )							
							AOutData[ i ] = AInArray->Data[ i ] * ( 1.0 - sq(( i - float( ALengthMinusOne ) / 2.0) / ( float( ALengthMinusOne ) / 2.0)) );

						break;

					case wtFlatTop:
						for( int i = 0; i <= ALengthMinusOne; ++i )							
							AOutData[ i ] = AInArray->Data[ i ] * ( 1 - 1.93 * cos( 2*PI*i / ALengthMinusOne )
																	 + 1.29 * cos( 4*PI*i / ALengthMinusOne )
																	 - 0.388 * cos( 6*PI*i / ALengthMinusOne )
																	 + 0.032 * cos( 8*PI*i / ALengthMinusOne ) );

						break;

					case wtKaiser:
					{
						float ALengthMinusOneHalf = float( ALengthMinusOne ) / 2.0;
						float ADivider = Bessel( Alpha() * ALengthMinusOneHalf );
						float ALengthMinusOneHalfSquare = ALengthMinusOneHalf * ALengthMinusOneHalf;
						for( int i = 0; i <= ALengthMinusOne; ++i )							
						{
							float Arg = Alpha() * sqrt( ALengthMinusOneHalfSquare - sq( i - ALengthMinusOneHalf ) );
							AOutData[ i ] = AInArray->Data[ i ] * ( Bessel(Arg) / ADivider );
						}

						break;
					}
				}

				TArray<float> ABuffer( AInArray->Size, AOutData );
				T_OutputPin::SetPinValue( ABuffer );

				delete [] AOutData;
			}
		}
	};
//---------------------------------------------------------------------------
}

#ifdef __TEST_FOR_DEBUG_PRINTS__
#undef Serial
#endif